# Cross-Language Coding Standards

## Purpose

This document defines the rules that apply to **every** language in this
repo. Each language subdirectory has its own `STANDARDS.md` covering
language-specific idioms, but all code must also satisfy the rules here.

The goal is to make examples easy to compare across languages — same
problem, same structure, consistent quality everywhere.

---

## 1. Every Example Must Have the Same Shape

Regardless of language, each example is a directory containing:

```
<example-name>/
├── README.md       — what the example does, how to run it, expected output
├── main.<ext>      — entry point (or equivalent for that language)
├── <logic>.<ext>   — pure logic separated from I/O and rendering
└── test/           — unit tests for the logic (not the I/O)
```

The separation of logic from I/O is non-negotiable. It is what makes
the code testable and what isolates the part worth benchmarking.

---

## 2. Documentation Rules

- Every file has a header comment stating: what it does, what concept
  it demonstrates, how to compile/run it.
- Every non-trivial function has a doc comment with: what it does,
  its parameters and their units/ranges, and its return value.
- Comments explain **why**, not **what**. Never paraphrase the code.
- No commented-out code left in finished examples.

Format per language:
| Language   | Doc comment style     |
|------------|-----------------------|
| C          | Doxygen `/** ... */`  |
| Assembly   | `; ---` block comment |
| Rust       | `///` rustdoc         |
| Go         | `//` godoc            |
| Java       | Javadoc `/** ... */`  |
| Python     | Google-style docstring|
| Node.js    | JSDoc `/** ... */`    |

---

## 3. Testing Rules

- Tests live in a `test/` subdirectory.
- Tests cover pure logic functions — never rendering, I/O, or main().
- Each test has a name that describes what it verifies, not how.
  Good: `test_ball_reflects_off_left_wall`
  Bad:  `test_update_ball_1`
- A test must be runnable with a single command (see per-language
  standards for the exact command).
- All tests must pass before any example is considered complete.
- Tests are regression guards: if you change logic, existing tests
  must still pass or be explicitly updated with a reason.

---

## 4. Benchmarking Rules

- The benchmarked function must be **identical in purpose** across
  all languages: same algorithm, same input size, same output.
- Warm up the runtime before measuring (JVM, V8, and Python JIT
  all have startup costs that distort the first run).
- Report: mean, min, max, and standard deviation across N runs
  (default N=10).
- No I/O inside the timed region. Print results after timing ends.
- The benchmark runner (`bench/run_all.sh`) collects results and
  feeds them to the HTML report generator.
- Results are reproducible: seed any RNG with a fixed value,
  document the machine spec in the report header.

---

## 5. Code Style (Universal)

- Prefer clarity over cleverness. This is a learning repo.
- Names are descriptive. Single-letter variables only in short math
  loops where the letter matches standard notation (e.g. `i`, `x`,
  `r`, `k`).
- No magic numbers — give constants a name and explain their units.
- Error handling at boundaries (bad input, failed allocation, file
  not found). Inside validated logic, trust the invariants.
- Keep functions small. If a function needs more than a few lines of
  explanation in its doc comment, it is probably doing too much.

---

## 6. File Naming

| Artefact          | Convention                        |
|-------------------|-----------------------------------|
| Source files      | `snake_case.<ext>`                |
| Test files        | `test_<module>.<ext>`             |
| Build output      | `<example-name>` (no extension)   |
| Benchmark results | `bench/results/<lang>_<example>.json` |

---

## 7. CLAUDE.md Presence

Every language directory has a `CLAUDE.md` with:
- Toolchain setup (compiler/interpreter version, install command).
- How to run an example.
- How to run its tests.
- How to run its benchmark.
- Any gotchas specific to this machine (Apple Silicon paths, etc.).
