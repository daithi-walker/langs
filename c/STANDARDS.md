# C — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why C?

C is the lingua franca of systems programming. It compiles to native
machine code with no runtime overhead. Understanding C gives you a
mental model of what every other language is doing underneath — memory
layout, the stack, pointers, calling conventions. It is the baseline
against which all benchmarks in this repo are measured.

---

## Toolchain

| Tool    | Version | Install |
|---------|---------|---------|
| clang   | ≥ 15    | ships with Xcode (`xcode-select --install`) |
| make    | any     | ships with Xcode |
| Unity   | 2.6.x   | embedded in `../../c-examples/lib/unity/` |

Standard: **C11** (`-std=c11`). Use C99 complex numbers (`<complex.h>`)
where needed for QC examples.

---

## Project Structure

```
<example>/
├── main.c          — SDL2 / entry point only; no logic
├── <module>.c      — pure logic (no SDL2, no main)
├── <module>.h      — declarations for the module
├── Makefile
└── test/
    └── test_<module>.c
```

Logic lives in `<module>.c` so tests can link against it without
pulling in SDL2 or `main()`. This is the single most important
structural rule in C examples.

---

## Style

### Formatting
- 4-space indent. No tabs.
- Opening brace on the same line as the statement (`K&R` style).
- One blank line between functions.
- Lines ≤ 100 characters.

### Naming
| Thing           | Convention      | Example              |
|-----------------|-----------------|----------------------|
| Functions       | `snake_case`    | `update_ball`        |
| Variables       | `snake_case`    | `grid_psi2`          |
| Constants/macros| `UPPER_SNAKE`   | `RADIUS`, `WIDTH`    |
| Types (typedef) | `snake_case_t`  | `vec2_t`             |
| Files           | `snake_case.c`  | `physics.c`          |

### Types
- Use `int` for general integers. Use `float` for physics/graphics
  (matches GPU types; `double` is rarely needed at this scale).
- Use `size_t` for array indices and sizes.
- Use `float complex` (C99) for wavefunctions.
- Avoid `void *` casts unless interfacing with a C API that requires it.

### Memory
- Prefer stack allocation. Only heap-allocate when the size is
  runtime-determined or the lifetime crosses function boundaries.
- Every `malloc` is checked for NULL before use.
- Every `malloc` has a matching `free` before the program exits.
- No memory leaks. Run with `leaks ./binary` on macOS to verify.

### Error Handling
- Functions that can fail return an `int` (0 = success, non-zero = error)
  or a pointer (NULL = error).
- SDL2 errors: check return values and call `SDL_GetError()` on failure.
- Do not silently swallow errors.

---

## Documentation

Every `.c` file starts with a Doxygen file header:

```c
/**
 * @file   physics.c
 * @brief  One-line description.
 *
 * Longer explanation — what concept this demonstrates, what is
 * stubbed, what is complete.
 *
 * @section deps Dependencies
 * @section compile Compile
 * @section nextsteps Next Steps
 */
```

Every non-trivial function:

```c
/**
 * @brief  What it does.
 * @param  x    Description and units (e.g. "ball centre x, pixels").
 * @return What is returned; what values mean success/failure.
 */
```

---

## Makefile Template

```makefile
CC     = clang
CFLAGS = -Wall -Wextra -std=c11 -g
UNITY  = ../../lib/unity          # adjust depth as needed

SDL_INC = -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include
SDL_LIB = -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 \
          -Wl,-framework,Cocoa

TARGET = <name>
TEST   = test/test_<name>

.PHONY: all test clean

all: $(TARGET)

$(TARGET): main.c <module>.c
	$(CC) $(CFLAGS) $(SDL_INC) main.c <module>.c -o $(TARGET) $(SDL_LIB) -lm

test: $(TEST)
	./$(TEST)

$(TEST): test/test_<module>.c <module>.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) -I$(UNITY) test/test_<module>.c <module>.c \
	      $(UNITY)/unity.c -o $(TEST) -lm

clean:
	rm -f $(TARGET) $(TEST)
```

---

## Testing with Unity

Unity is a lightweight C test framework: one `.c` and one `.h` file.

```c
#include "unity.h"

void setUp(void)    {}   /* runs before each test */
void tearDown(void) {}   /* runs after each test  */

void test_something_meaningful(void) {
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected, actual);
    TEST_ASSERT_EQUAL_INT(expected, actual);
    TEST_ASSERT_GREATER_THAN_FLOAT(threshold, value);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_something_meaningful);
    return UNITY_END();   /* returns 0 if all pass, 1 if any fail */
}
```

Run: `make test`

---

## Benchmarking

Timing in C uses `clock_gettime(CLOCK_MONOTONIC)`:

```c
#include <time.h>

struct timespec t0, t1;
clock_gettime(CLOCK_MONOTONIC, &t0);
// ... work ...
clock_gettime(CLOCK_MONOTONIC, &t1);

double elapsed_ms = (t1.tv_sec - t0.tv_sec) * 1e3
                  + (t1.tv_nsec - t0.tv_nsec) / 1e6;
```

- Compile with `-O2` for benchmarks (not `-g`).
- Run 10 times, discard the first (cache warm-up), report mean ± stddev.
- The bench Makefile target compiles with `-O2` and runs the bench binary.

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| Integer overflow | use `int64_t` for accumulators over large arrays |
| Float comparison | never `==`; always `fabsf(a-b) < epsilon` |
| Uninitialized memory | always initialise; use `-Wall` which catches most cases |
| Buffer overrun | double-check array bounds; never `strcpy` without length |
| Forgetting `-lm` | math functions (`sqrt`, `exp`, etc.) need `-lm` at link time |
