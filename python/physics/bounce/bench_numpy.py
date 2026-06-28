"""NumPy-accelerated bounce benchmark.

Simulates N_BALLS balls simultaneously using vectorised array operations.
Reports nanoseconds *per ball per call* so the result is directly comparable
to the pure-Python single-ball benchmark.

The key idea: instead of a Python loop over balls, we store all ball state
in four numpy arrays (x, y, vx, vy) and let numpy's compiled C internals
update all balls in a single operation.

Run: python3 bench_numpy.py
"""

import json
import timeit
import numpy as np

N_BALLS = 10_000
WARMUP  = 1_000
RUNS    = 10_000   # each run updates all N_BALLS at once


def make_balls(n: int) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    """Initialise n balls with random positions and fixed velocity."""
    rng = np.random.default_rng(42)
    x  = rng.uniform(30, 770, n).astype(np.float32)
    y  = rng.uniform(30, 570, n).astype(np.float32)
    vx = np.full(n, 4.0, dtype=np.float32)
    vy = np.full(n, 3.0, dtype=np.float32)
    return x, y, vx, vy


def update_numpy(
    x: np.ndarray, y: np.ndarray,
    vx: np.ndarray, vy: np.ndarray,
    radius: int, width: int, height: int,
) -> None:
    """Update all balls in one vectorised pass — no Python loop.

    Each line operates on the entire array simultaneously using numpy's
    compiled C backend. This is what makes numpy fast.

    Args:
        x, y:          Ball centre positions (pixels), modified in place.
        vx, vy:        Velocities (pixels/frame), modified in place.
        radius, width, height: Window geometry.
    """
    x += vx
    y += vy
    # np.where returns a new array — assign back to flip velocities
    vx[:] = np.where((x - radius < 0) | (x + radius > width),  -vx, vx)
    vy[:] = np.where((y - radius < 0) | (y + radius > height), -vy, vy)


def main() -> None:
    x, y, vx, vy = make_balls(N_BALLS)

    # Warm up
    for _ in range(WARMUP):
        update_numpy(x, y, vx, vy, 30, 800, 600)

    # Time RUNS batches of N_BALLS balls
    elapsed = timeit.timeit(
        lambda: update_numpy(x, y, vx, vy, 30, 800, 600),
        number=RUNS,
    )

    total_updates = RUNS * N_BALLS
    mean_ns_per_ball = elapsed / total_updates * 1e9

    print(json.dumps({
        "lang":      "python-numpy",
        "example":   "bounce",
        "mean_ns":   round(mean_ns_per_ball, 4),
        "n":         total_updates,
        "note":      f"{N_BALLS} balls vectorised",
    }))


if __name__ == "__main__":
    main()
