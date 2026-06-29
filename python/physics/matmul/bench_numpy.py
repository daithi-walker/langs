"""
Matrix multiply benchmark — numpy (BLAS/Accelerate backend).

numpy.dot dispatches into Apple's Accelerate BLAS on M4 — expect this to
beat naive C by 5-20x via hand-tuned NEON SIMD and tiling.
Outputs JSON.
"""

import time
import json
import numpy as np

N      = 256
WARMUP = 5
ITERS  = 200


def main():
    a = np.arange(N * N, dtype=np.float64).reshape(N, N) / (N * N)
    b = a.copy()

    for _ in range(WARMUP):
        np.dot(a, b)

    t0 = time.perf_counter_ns()
    for _ in range(ITERS):
        c = np.dot(a, b)
    t1 = time.perf_counter_ns()

    _ = c[0, 0]

    ns = (t1 - t0) / ITERS
    print(json.dumps({
        "language":   "python_numpy",
        "example":    "matmul",
        "ns_per_op":  round(ns, 2),
        "iterations": ITERS,
    }))


if __name__ == "__main__":
    main()
