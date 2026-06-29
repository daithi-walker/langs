"""
N-body gravitational simulation benchmark — numpy vectorized.

Uses broadcasting for O(n²) pairwise distances — no Python loops in hot path.
Outputs JSON: {"language":"python_numpy","example":"nbody","ns_per_op":...,"iterations":...}
"""

import time
import json
import numpy as np

N      = 1000
G      = 6.674e-11
SOFT   = 1e-4
WARMUP = 20
ITERS  = 200


def make_bodies():
    rng = np.random.default_rng(12345)
    pos  = rng.uniform(-1.0, 1.0, (N, 3))
    vel  = np.zeros((N, 3))
    mass = np.ones(N)
    return pos.copy(), vel.copy(), mass


def step(pos, vel, mass, dt):
    # diff[i,j] = pos[j] - pos[i]  shape (N,N,3)
    diff   = pos[np.newaxis, :, :] - pos[:, np.newaxis, :]  # (N,N,3)
    r2     = np.sum(diff**2, axis=2) + SOFT**2               # (N,N)
    inv_r3 = 1.0 / (r2 * np.sqrt(r2))                       # (N,N)
    np.fill_diagonal(inv_r3, 0.0)
    # acc[i] = G * sum_j  m_j * diff[i,j] / r_ij^3
    acc = G * np.einsum('ij,j,ijk->ik', inv_r3, mass, diff)  # (N,3)
    vel += acc * dt
    pos += vel * dt


def main():
    pos, vel, mass = make_bodies()
    for _ in range(WARMUP):
        step(pos, vel, mass, 1e-3)

    pos, vel, mass = make_bodies()
    t0 = time.perf_counter_ns()
    for _ in range(ITERS):
        step(pos, vel, mass, 1e-3)
    t1 = time.perf_counter_ns()

    ns = (t1 - t0) / ITERS
    print(json.dumps({
        "language":   "python_numpy",
        "example":    "nbody",
        "ns_per_op":  round(ns, 2),
        "iterations": ITERS,
    }))


if __name__ == "__main__":
    main()
