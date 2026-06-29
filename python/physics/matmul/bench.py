"""
Matrix multiply benchmark — pure Python.

N=256, naive triple loop. Outputs JSON.
"""

import time
import json

N      = 256
WARMUP = 3
ITERS  = 10


def make_matrix():
    n2 = N * N
    flat = [i / n2 for i in range(n2)]
    return [flat[i*N:(i+1)*N] for i in range(N)]


def matmul(a, b):
    c = [[0.0] * N for _ in range(N)]
    for i in range(N):
        for k in range(N):
            aik = a[i][k]
            bk  = b[k]
            ci  = c[i]
            for j in range(N):
                ci[j] += aik * bk[j]
    return c


def main():
    a, b = make_matrix(), make_matrix()
    for _ in range(WARMUP):
        matmul(a, b)

    a, b = make_matrix(), make_matrix()
    t0 = time.perf_counter_ns()
    for _ in range(ITERS):
        matmul(a, b)
    t1 = time.perf_counter_ns()

    ns = (t1 - t0) / ITERS
    print(json.dumps({
        "language":   "python",
        "example":    "matmul",
        "ns_per_op":  round(ns, 2),
        "iterations": ITERS,
    }))


if __name__ == "__main__":
    main()
