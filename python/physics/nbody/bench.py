"""
N-body gravitational simulation benchmark — pure Python.

N=1000 bodies, O(n²) direct pairwise forces.
Outputs JSON: {"language":"python","example":"nbody","ns_per_op":...,"iterations":...}
"""

import time
import math
import json

N      = 1000
G      = 6.674e-11
SOFT   = 1e-4
WARMUP = 20
ITERS  = 200


def _lcg_seq(n):
    s = 12345
    M = 2**64
    vals = []
    for _ in range(n * 3):
        s = (s * 6364136223846793005 + 1442695040888963407) % M
        vals.append((s >> 33) / (1 << 31) - 1.0)
    return vals


def make_bodies():
    vals = _lcg_seq(N)
    x  = [vals[i*3]   for i in range(N)]
    y  = [vals[i*3+1] for i in range(N)]
    z  = [vals[i*3+2] for i in range(N)]
    vx = [0.0]*N; vy = [0.0]*N; vz = [0.0]*N
    m  = [1.0]*N
    return x, y, z, vx, vy, vz, m


def step(x, y, z, vx, vy, vz, mass, dt):
    ax = [0.0]*N; ay = [0.0]*N; az = [0.0]*N
    for i in range(N):
        for j in range(N):
            if i == j:
                continue
            dx = x[j] - x[i]; dy = y[j] - y[i]; dz = z[j] - z[i]
            r2 = dx*dx + dy*dy + dz*dz + SOFT*SOFT
            inv_r3 = 1.0 / (r2 * math.sqrt(r2))
            gm = G * mass[j]
            ax[i] += gm * dx * inv_r3
            ay[i] += gm * dy * inv_r3
            az[i] += gm * dz * inv_r3
    for i in range(N):
        vx[i] += ax[i]*dt; vy[i] += ay[i]*dt; vz[i] += az[i]*dt
        x[i]  += vx[i]*dt; y[i]  += vy[i]*dt; z[i]  += vz[i]*dt


def main():
    x, y, z, vx, vy, vz, m = make_bodies()
    for _ in range(WARMUP):
        step(x, y, z, vx, vy, vz, m, 1e-3)

    x, y, z, vx, vy, vz, m = make_bodies()
    t0 = time.perf_counter_ns()
    for _ in range(ITERS):
        step(x, y, z, vx, vy, vz, m, 1e-3)
    t1 = time.perf_counter_ns()

    ns = (t1 - t0) / ITERS
    print(json.dumps({
        "language":   "python",
        "example":    "nbody",
        "ns_per_op":  round(ns, 2),
        "iterations": ITERS,
    }))


if __name__ == "__main__":
    main()
