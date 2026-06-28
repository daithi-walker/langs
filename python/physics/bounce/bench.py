"""Benchmark for ball update — outputs JSON for the report generator.

Run: python bench.py
"""

import json
import timeit
from physics import Ball, update

WARMUP = 10_000
RUNS   = 1_000_000


def main() -> None:
    ball = Ball(x=400.0, y=300.0, vx=4.0, vy=3.0)

    # Warm up the interpreter
    for _ in range(WARMUP):
        update(ball, 30, 800, 600)

    # Time RUNS iterations; timeit returns total seconds for number=RUNS
    elapsed = timeit.timeit(
        lambda: update(ball, 30, 800, 600),
        number=RUNS,
    )

    mean_ns = elapsed / RUNS * 1e9
    print(json.dumps({
        "lang":    "python",
        "example": "bounce",
        "mean_ns": round(mean_ns, 4),
        "n":       RUNS,
    }))


if __name__ == "__main__":
    main()
