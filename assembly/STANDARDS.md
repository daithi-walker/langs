# Assembly (ARM64) — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why Assembly?

Assembly is the layer between source code and machine instructions. You
are writing exactly what the CPU executes — no compiler decisions, no
abstraction. Understanding it explains:
- Why certain C patterns are faster than others.
- How SIMD (single instruction, multiple data) achieves 10–60× speedups
  over scalar C (this is what FFmpeg uses for codec hot paths).
- What "calling conventions" and "the stack" actually mean in hardware.

We write **ARM64 assembly** (also called AArch64) because this machine
is Apple Silicon. The syntax used is **GAS** (GNU Assembler), compatible
with the `as` assembler that ships with Xcode.

---

## Toolchain

| Tool  | Version | Install |
|-------|---------|---------|
| as    | any     | ships with Xcode |
| clang | ≥ 15    | ships with Xcode (used to link) |
| lldb  | any     | ships with Xcode (debugger) |

---

## Project Structure

```
<example>/
├── main.s          — entry point and top-level logic in assembly
├── <module>.s      — isolated routines (the part worth benchmarking)
├── Makefile
└── test/
    └── test_<module>.c   — C test harness calling into assembly routines
```

Tests are written in C (using Unity) and call into assembly functions.
This is the standard approach: assembly provides the fast inner loops,
C provides the test harness.

---

## ARM64 Architecture Primer

### Registers
| Register    | Role                              |
|-------------|-----------------------------------|
| x0–x7       | Function arguments and return values |
| x8–x18      | Caller-saved temporaries          |
| x19–x28     | Callee-saved (must preserve)      |
| x29 (fp)    | Frame pointer                     |
| x30 (lr)    | Link register (return address)    |
| sp          | Stack pointer (must stay 16-byte aligned) |
| xzr         | Zero register (reads as 0)        |

`w0`–`w28` are the 32-bit views of the same registers.
`v0`–`v31` are the 128-bit SIMD/FP registers.

### Calling Convention (Apple ARM64 ABI)
- Arguments 1–8 go in `x0`–`x7` (or `w0`–`w7` for 32-bit).
- Return value goes in `x0` (or `w0`).
- Callee must save and restore `x19`–`x28`, `x29`, `x30`.
- Stack pointer must be 16-byte aligned at every `bl` instruction.

---

## Style

### File Header
Every `.s` file starts with a structured comment block:

```asm
; =============================================================================
; file:    module.s
; brief:   One-line description.
;
; What this does, what concept it demonstrates, what is stubbed.
;
; Calling convention: ARM64 Apple ABI
; Inputs:  x0 = ..., x1 = ...
; Output:  x0 = ...
; Clobbers: x8, x9 (caller-saved, no need to restore)
; Preserved: x19-x28 (restored before ret)
; =============================================================================
```

### Function Documentation
Every exported function (`.global`) has:

```asm
; -----------------------------------------------------------------------------
; function: dot_product
; brief:    Compute the dot product of two float arrays.
; params:
;   x0 = pointer to array A (float32)
;   x1 = pointer to array B (float32)
;   x2 = number of elements (must be multiple of 4 for SIMD path)
; returns:
;   s0 = dot product result (float32)
; -----------------------------------------------------------------------------
```

### Naming
| Thing              | Convention         | Example           |
|--------------------|--------------------|-------------------|
| Exported functions | `snake_case`       | `dot_product`     |
| Local labels       | `L_<name>`         | `L_loop`, `L_end` |
| Constants          | `.equ NAME, value` | `.equ N_LANES, 4` |
| Data labels        | `snake_case`       | `coeff_table`     |

### Layout
- 4-space indent for instructions (not tabs — some assemblers care).
- Align opcodes in a column so arguments are scannable.
- One blank line between logical blocks.
- Keep routines short. If a routine needs more than ~30 instructions,
  consider whether it should be split.

---

## SIMD Guidelines

ARM64 SIMD uses NEON instructions operating on `v` registers.

```asm
; Load 4 floats from memory into a SIMD register
ld1  {v0.4s}, [x0], #16    ; load and post-increment pointer

; Fused multiply-accumulate: v2 += v0 * v1
fmla v2.4s, v0.4s, v1.4s

; Horizontal add across lanes
faddp v0.4s, v2.4s, v2.4s
faddp v0.4s, v0.4s, v0.4s  ; result in s0
```

Rules:
- Always handle the scalar tail (when N is not divisible by lane count).
- Document which registers are clobbered vs preserved in the header.
- Benchmark SIMD vs scalar and report the ratio — that's the point.

---

## Testing

Tests are C files (Unity) that `extern` the assembly functions:

```c
extern float dot_product(const float *a, const float *b, int n);

void test_dot_product_known_values(void) {
    float a[] = {1, 2, 3, 4};
    float b[] = {4, 3, 2, 1};
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, 20.0f, dot_product(a, b, 4));
}
```

The Makefile assembles the `.s` file and links it with the C test:

```makefile
test/test_module: test/test_module.c module.s
    clang test/test_module.c module.s $(UNITY)/unity.c -o $@ -lm
```

---

## Makefile Template

```makefile
AS     = as
CC     = clang
UNITY  = ../../lib/unity

TARGET = <name>
TEST   = test/test_<name>

all: $(TARGET)

$(TARGET): main.s <module>.s
    $(CC) main.s <module>.s -o $(TARGET)

test: $(TEST)
    ./$(TEST)

$(TEST): test/test_<module>.c <module>.s $(UNITY)/unity.c
    $(CC) test/test_<module>.c <module>.s $(UNITY)/unity.c \
          -I$(UNITY) -o $(TEST) -lm

clean:
    rm -f $(TARGET) $(TEST)
```

---

## Benchmarking

Assembly benchmarks are timed from a C harness (same `clock_gettime`
pattern as C examples). The assembly routine is the inner loop; the
C harness handles timing, iteration, and output.

Compile with no debug flags — assembly is already "optimised" by
definition (you wrote every instruction).

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| Stack misalignment | `sp` must be 16-byte aligned before any `bl`; use `sub sp, sp, #16` in pairs with `add sp, sp, #16` |
| Forgetting to restore callee-saved regs | save `x19`+ at function entry with `stp`, restore at exit with `ldp` |
| Off-by-one in loop counters | draw the iteration table on paper first |
| Scalar tail missing | SIMD loops that process 4-at-a-time must handle `N % 4` leftover elements |
| Wrong lane size | `.4s` = 4×float32, `.2d` = 2×float64 — mixing them silently corrupts data |
