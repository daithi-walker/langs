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
| `nbody` | physics | O(n²) pairwise gravity, cache misses, irregular memory access |

### Key findings so far

- **Simple loops** (bounce): all JIT-compiled languages within 4× of C. Language choice barely matters.
- **SIMD/FFT-intensive work** (wave_packet): gap blows out to 7×. Library choice matters more than language.
- **Python numpy** beats Go and Java on FFT: not a Python win — numpy dispatches into C (pocketfft), Go runs pure Cooley-Tukey with no SIMD.
- **O(n²) cache-miss work** (nbody): C wins via auto-vectorization of the inner loop; Java JIT is surprisingly close (1.3×); Go/Rust/Node cluster at 2.3–2.4×; Python numpy 12.6× despite vectorized broadcasting (allocates a 1000×1000×3 matrix per step).

---

## Visualizers

Interactive SDL2 visualizers live in C — the right language for real-time
graphics. Not planned for other languages.

### N-body gravity (`c/physics/nbody/`)

```bash
make -C c/physics/nbody && ./c/physics/nbody/nbody
```

N=1000 point masses under pairwise Newtonian gravity. Full on-screen UI:

**Init modes**
- **Galaxy** — rotating disk seeded with circular velocities; watch spiral structure form
- **Static** — zero velocity; pure gravitational collapse
- **Solar Sys** — one dominant central mass + light bodies in circular orbits

**Mass modes**
- **Equal** — all mass = 1
- **Random** — log-uniform 0.1–10; dot size scales with mass
- **Massive** — one body at 100× in the center

**Controls**
- **N buttons** — 100 / 500 / 1000 / 2000 / 5000 bodies; changes take effect on reset
- **G slider** — log-scale 0.05 → 50; dt auto-scales with G to prevent numerical blow-up
- **Orbit fraction slider** — 0.0 = pure freefall collapse, 1.0 = stable circular orbits; resets sim on drag
- **Dark Halo toggle** — adds a Hernquist dark matter halo potential; stabilises rotation curve, suppresses disk fragmentation, enables spiral-like structure
- **Steps/frame** — 1 / 5 / 10 / 20; speeds up time without changing physics timestep
- **Zoom** — ± buttons
- **Pause / Resume** — or Space key
- **Reset** — or R key

**Black hole**
- **Drop BH** — injects a 5000× mass body at the current centre of mass mid-simulation
- **Pin BH** — freezes BH position (zeroes velocity each step); compare pinned vs free to see Newton's 3rd law recoil
- **Accrete** — bodies crossing the accretion radius (0.04 world units) are swallowed; BH disk grows visually as mass accumulates; body count ticks down
- Body colour = speed: dark blue (slow) → cyan → orange → white (fast)
- BH rendered as a black disk (occludes stars) with an orange accretion ring that grows with accreted mass

### Wave packet (`c/qc/wave_packet/`)

```bash
make -C c/qc/wave_packet && ./c/qc/wave_packet/wave_packet
```

Split-operator TDSE simulation of a Gaussian wave packet tunnelling through a
potential barrier. Shows |ψ|² (probability density) as a cyan curve, the
barrier in orange, and a legend explaining the physics.

### Hydrogen orbitals (`c/qc/hydrogen_orbitals/`)

```bash
make -C c/qc/hydrogen_orbitals && ./c/qc/hydrogen_orbitals/orbitals
```

Renders |ψ_{nlm}(x,y,0)|² — the probability density of finding the electron
in the z=0 cross-section of a hydrogen atom. On-screen panel with +/- buttons
for quantum numbers n (1–7), l, m; preset orbital buttons (1s, 2p, 3d, 4f…);
colour gradient from black (zero probability) to white (peak).

---

## Planned Examples

| Example | What it tests | Why it matters |
|---------|---------------|----------------|
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
│   ├── physics/
│   │   ├── bounce/        ← bounce benchmark
│   │   └── nbody/         ← N-body visualizer + benchmark
│   └── qc/
│       ├── hydrogen_orbitals/   ← SDL2 orbital visualizer
│       ├── wave_packet/         ← SDL2 TDSE tunneling simulation + benchmark
│       └── double_slit/         ← quantum interference
│
├── rust/
│   ├── physics/bounce/
│   ├── physics/nbody/     ← nbody benchmark
│   └── qc/wave_packet/    ← rustfft split-operator TDSE
│
├── go/
│   ├── physics/bounce/
│   ├── physics/nbody/     ← nbody benchmark
│   └── qc/wave_packet/    ← pure-Go FFT TDSE
│
├── java/
│   ├── physics/bounce/
│   ├── physics/nbody/     ← nbody benchmark
│   └── qc/wave_packet/    ← Cooley-Tukey FFT TDSE
│
├── python/
│   ├── physics/bounce/    ← pure Python + numpy variants
│   ├── physics/nbody/     ← pure Python + numpy variants
│   └── qc/wave_packet/    ← numpy FFT TDSE
│
├── nodejs/
│   ├── physics/bounce/    ← TypeScript
│   ├── physics/nbody/     ← TypeScript
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
bash bench/run_all.sh nbody

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
