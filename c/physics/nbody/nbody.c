#include "nbody.h"
#include <math.h>

#define G    6.674e-11
#define SOFTENING 1e-4

void nbody_init(Body *bodies, int n) {
    /* Deterministic pseudo-random layout — same seed every run */
    unsigned long s = 12345;
    for (int i = 0; i < n; i++) {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        bodies[i].x    = (double)(s >> 33) / (double)(1u << 31) - 1.0;
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        bodies[i].y    = (double)(s >> 33) / (double)(1u << 31) - 1.0;
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        bodies[i].z    = (double)(s >> 33) / (double)(1u << 31) - 1.0;
        bodies[i].vx   = 0.0;
        bodies[i].vy   = 0.0;
        bodies[i].vz   = 0.0;
        bodies[i].mass = 1.0;
    }
}

void nbody_step(Body *bodies, int n, double dt) {
    for (int i = 0; i < n; i++) {
        double ax = 0.0, ay = 0.0, az = 0.0;
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;
            double dz = bodies[j].z - bodies[i].z;
            double r2 = dx*dx + dy*dy + dz*dz + SOFTENING*SOFTENING;
            double inv_r3 = 1.0 / (r2 * sqrt(r2));
            double gm = G * bodies[j].mass;
            ax += gm * dx * inv_r3;
            ay += gm * dy * inv_r3;
            az += gm * dz * inv_r3;
        }
        bodies[i].vx += ax * dt;
        bodies[i].vy += ay * dt;
        bodies[i].vz += az * dt;
    }
    for (int i = 0; i < n; i++) {
        bodies[i].x += bodies[i].vx * dt;
        bodies[i].y += bodies[i].vy * dt;
        bodies[i].z += bodies[i].vz * dt;
    }
}
