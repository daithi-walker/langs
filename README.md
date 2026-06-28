# langs

A multi-language benchmarking repository. The same algorithms are implemented
across C, Rust, Go, Java, Python (numpy), and Node.js so you can directly
compare performance, idioms, and where each language breaks down.

**Goal:** understand when a service written in Go or Python needs to be
rewritten in C or Rust — backed by data, not intuition.

→ **[See benchmark results](BENCHMARKS.md)**

---

## Current Examples

| Example | Category | What it tests |
|---------|----------|---------------|
| `bounce` | physics | Simple loop, float arithmetic — the baseline |
| `wave_packet` | qc | FFT-heavy, SIMD-critical, memory-intensive TDSE step |

### Key findings so far

- **Simple loops** (bounce): all JIT-compiled languages within 4× of C. Language choice barely matters.
- **SIMD/FFT-intensive work** (wave_packet): gap blows out to 7×. Library choice matters more than language.
- **Python numpy** beats Go and Java on FFT: not a Python win — numpy dispatches into C (pocketfft), Go runs pure Cooley-Tukey with no SIMD.

---

## Planned Examples

| Example | What it tests | Why it matters |
|---------|---------------|----------------|
| `nbody` | O(n²) pairwise gravity, cache misses, pointer chasing | Reveals how JVM/V8 handle irregular memory access at scale |
| `matmul` | Memory bandwidth, cache utilization, auto-vectorization | Shows how much `-O2` vs JIT can exploit hardware SIMD |

---

## Structure

```
langs/
├── BENCHMARKS.md          ← benchmark results table (auto-generated)
├── README.md
├── STANDARDS.md           ← cross-language rules
│
├── c/
│   ├── STANDARDS.md
│   ├── lib/unity/         ← embedded C test framework
│   ├── physics/bounce/    ← bounce example
│   └── qc/
│       ├── hydrogen_orbitals/   ← SDL2 orbital visualizer
│       ├── wave_packet/         ← SDL2 TDSE tunneling simulation
│       └── double_slit/         ← quantum interference
│
├── rust/
│   ├── physics/bounce/
│   └── qc/wave_packet/    ← rustfft split-operator TDSE
│
├── go/
│   ├── physics/bounce/
│   └── qc/wave_packet/    ← pure-Go FFT TDSE
│
├── java/
│   ├── physics/bounce/
│   └── qc/wave_packet/    ← Cooley-Tukey FFT TDSE
│
├── python/
│   ├── physics/bounce/    ← pure Python + numpy variants
│   └── qc/wave_packet/    ← numpy FFT TDSE
│
├── nodejs/
│   ├── physics/bounce/    ← TypeScript
│   └── qc/wave_packet/    ← TypeScript Cooley-Tukey
│
└── bench/
    ├── run_all.sh         ← run all benchmarks, write JSON results
    ├── report.py          ← generate BENCHMARKS.md + HTML report
    └── results/           ← committed JSON + HTML results
```

---

## Running Benchmarks

```bash
# Run all examples, all languages
bash bench/run_all.sh

# Run one example
bash bench/run_all.sh wave_packet

# Regenerate BENCHMARKS.md and HTML report
python3 bench/report.py
```

Each benchmark binary prints one JSON line:
```json
{"language":"rust","example":"wave_packet","ns_per_op":3143.50,"iterations":2000}
```

Results are committed to `bench/results/` — re-run to update them on your machine.

---

## Standards

- Cross-language: [`STANDARDS.md`](STANDARDS.md)
- Per-language: `<lang>/STANDARDS.md`
