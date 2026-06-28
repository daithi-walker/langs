# Python — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why Python?

Python is the slowest language in this repo for raw computation — often
50–200× slower than C. That is the honest benchmark result and it is
informative: it explains why numpy, scipy, and pytorch are written in C
and Fortran under the hood, and why Python scientists reach for those
libraries instead of pure loops.

Python's value here is as the **scripting and orchestration layer**: the
benchmark report generator, data visualisation, and configuration scripts
are all Python. It is also the language you already know well, so Python
examples serve as a reference point when reading unfamiliar C or Rust.

---

## Toolchain

| Tool   | Version | Install |
|--------|---------|---------|
| python | ≥ 3.12  | `brew install python@3.12` |
| uv     | any     | `curl -LsSf https://astral.sh/uv/install.sh \| sh` |

We use **uv** for dependency and virtualenv management (faster than pip,
replaces venv + pip + pyproject.toml boilerplate).

Check: `python3 --version`, `uv --version`

---

## Project Structure

```
<example>/
├── pyproject.toml  — package metadata and dependencies
├── main.py         — entry point; minimal logic
├── <module>.py     — pure logic; no I/O
└── tests/
    └── test_<module>.py
```

---

## Style

### Formatting and Linting
- **ruff** handles both formatting and linting: `uv add --dev ruff`
- Run `ruff format .` and `ruff check .` before committing.
- Line length: 100 characters.
- We follow **PEP 8** (ruff enforces it).

### Naming
| Thing           | Convention      | Example              |
|-----------------|-----------------|----------------------|
| Functions       | `snake_case`    | `update_ball`        |
| Variables       | `snake_case`    | `grid_psi2`          |
| Constants       | `UPPER_SNAKE`   | `RADIUS`             |
| Classes         | `PascalCase`    | `BallState`          |
| Modules / files | `snake_case.py` | `physics.py`         |

### Type Hints
Type hints are **required** on all function signatures. They serve as
documentation and enable static analysis.

```python
def update_ball(
    x: float,
    y: float,
    vx: float,
    vy: float,
    radius: int,
    width: int,
    height: int,
) -> tuple[float, float, float, float]:
    ...
```

Run `mypy .` or `pyright` to check types statically.

### Immutability
- Prefer returning new values over mutating arguments (unlike C).
  Python's function call overhead makes mutation less critical.
- Use `dataclasses.dataclass(frozen=True)` for immutable state objects.

### NumPy for Performance
- Pure Python loops over large arrays are unacceptably slow for benchmarks.
- Use numpy vectorised operations for any computation over arrays.
- The benchmark should have **two implementations**:
  - `_pure`: plain Python loops (shows the worst case).
  - `_numpy`: vectorised (shows what Python can do with the right tools).
- Both are timed and reported separately.

---

## Documentation

Python uses **Google-style docstrings** (compatible with Sphinx and most
IDEs):

```python
def interference(screen_x: float) -> float:
    """Compute the interference intensity at a detector screen position.

    Both slits are treated as coherent point sources. The result is |ψ|²,
    proportional to the probability of detecting a particle at screen_x.

    Args:
        screen_x: x position on the detector screen in pixels.

    Returns:
        Intensity value >= 0.0, not normalised.
    """
```

Module-level docstring at the top of every file:

```python
"""Double-slit quantum interference simulation.

Computes |ψ(x)|² for two coherent point sources and plots the
resulting interference pattern.
"""
```

---

## Testing with pytest

```bash
uv add --dev pytest pytest-cov
```

```python
# tests/test_physics.py
import pytest
from physics import update_ball

def test_ball_reflects_off_left_wall():
    x, y, vx, vy = update_ball(x=5.0, y=100.0, vx=-4.0, vy=3.0,
                                radius=10, width=800, height=600)
    assert abs(vx - 4.0) < 1e-5, f"expected vx=4.0, got {vx}"

def test_intensity_is_nonnegative():
    from physics import interference
    for x in range(0, 800, 10):
        assert interference(float(x)) >= 0.0
```

Run: `pytest -v`
Coverage: `pytest --cov=. --cov-report=term-missing`

---

## Benchmarking

Use `timeit` for microbenchmarks or `time.perf_counter` for larger blocks:

```python
import timeit

def bench(fn, n=10_000):
    times = timeit.repeat(fn, number=1, repeat=n)
    mean  = sum(times) / n * 1e6   # microseconds
    std   = (sum((t * 1e6 - mean) ** 2 for t in times) / n) ** 0.5
    return mean, std
```

- Report in microseconds (μs) for small operations.
- Run at least 1000 repetitions to get stable statistics.
- Time both `_pure` and `_numpy` variants and report the ratio.

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| Pure loops over large arrays | use numpy; document when you intentionally don't |
| Mutable default arguments | never `def f(x, data=[])` — use `None` and assign inside |
| Missing type hints | required on all function signatures |
| Bare `except:` | always catch a specific exception type |
| Global state | avoid module-level mutable state; pass data explicitly |
| Comparing floats with `==` | use `math.isclose(a, b, rel_tol=1e-5)` |
