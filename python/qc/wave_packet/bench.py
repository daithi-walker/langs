"""
Benchmark the NumPy wave packet step.

Outputs JSON: {"language":"python_numpy","example":"wave_packet","ns_per_op":...,"iterations":...}
"""

import time
import json
from physics import Sim, BARRIER_H_DEF

WARMUP = 200
ITERS  = 2_000

def main():
    sim = Sim(BARRIER_H_DEF)

    for _ in range(WARMUP):
        sim.step()
    sim.init_psi()

    t0 = time.perf_counter_ns()
    for _ in range(ITERS):
        sim.step()
    t1 = time.perf_counter_ns()

    ns = (t1 - t0) / ITERS
    print(json.dumps({
        "language":   "python_numpy",
        "example":    "wave_packet",
        "ns_per_op":  round(ns, 2),
        "iterations": ITERS,
    }))

if __name__ == "__main__":
    main()
