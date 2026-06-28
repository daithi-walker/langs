# langs

A multi-language learning and benchmarking repository. The same algorithms
are implemented across languages so you can directly compare syntax, idioms,
performance, and toolchain ergonomics.

---

## Directory Structure

```
langs/
├── STANDARDS.md        ← cross-language rules that apply everywhere
├── README.md           ← this file
│
├── c/
│   ├── STANDARDS.md    ← C-specific standards
│   └── physics/        ← example category
│       └── bounce/     ← specific example
│           ├── main.c
│           ├── physics.c
│           ├── physics.h
│           ├── Makefile
│           └── test/
│               └── test_physics.c
│
├── assembly/
│   ├── STANDARDS.md
│   └── physics/
│       └── bounce/
│           ├── main.s
│           ├── physics.s
│           ├── Makefile
│           └── test/
│               └── test_physics.c
│
├── rust/
│   ├── STANDARDS.md
│   └── physics/
│       └── bounce/
│           ├── src/main.rs
│           ├── src/physics.rs
│           └── Cargo.toml
│
├── go/
│   ├── STANDARDS.md
│   └── physics/
│       └── bounce/
│           ├── main.go
│           ├── physics.go
│           ├── physics_test.go
│           └── go.mod
│
├── java/
│   ├── STANDARDS.md
│   └── physics/
│       └── bounce/
│           ├── src/Main.java
│           ├── src/BallPhysics.java
│           ├── test/TestBallPhysics.java
│           └── Makefile
│
├── python/
│   ├── STANDARDS.md
│   └── physics/
│       └── bounce/
│           ├── main.py
│           ├── physics.py
│           ├── tests/test_physics.py
│           └── pyproject.toml
│
└── nodejs/
    ├── STANDARDS.md
    └── physics/
        └── bounce/
            ├── src/main.ts
            ├── src/physics.ts
            ├── test/physics.test.ts
            └── package.json
```

---

## Example Categories (planned)

| Category  | Description                                        |
|-----------|----------------------------------------------------|
| `physics/`| Simulation and numeric computation (bounce, orbital, wave packet) |
| `data/`   | Sorting, searching, data structure benchmarks      |
| `concurrency/` | Parallel computation patterns                 |

Not every example will exist in every language — where a language is
not a natural fit (e.g., assembly for high-level data structures), that
is noted rather than forced.

---

## Benchmarking

A shared benchmark runner collects timing results from each language and
generates an HTML report.

```bash
# From the repo root (once wired up):
make benchmark          # run all benchmarks
make report             # generate HTML report in bench/report.html
```

Each language's benchmark runs the **same algorithm** with the **same
input size**, isolated to pure computation with no I/O in the timed region.

Results report: mean, min, max, stddev across N=10 runs.

---

## Standards

- Cross-language rules: [`STANDARDS.md`](STANDARDS.md)
- Per-language rules: `<lang>/STANDARDS.md`
