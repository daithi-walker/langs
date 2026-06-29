/**
 * @file  bench.c
 * @brief Benchmark one nbody step — N=1000 bodies, O(n²) pairwise gravity.
 *
 * Outputs JSON: {"language":"c","example":"nbody","ns_per_op":...,"iterations":...}
 */

#include <stdio.h>
#include <time.h>
#include "nbody.h"

#define WARMUP  20
#define ITERS  200

static double now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

int main(void) {
    static Body bodies[N_BODIES];

    nbody_init(bodies, N_BODIES);

    for (int i = 0; i < WARMUP; i++)
        nbody_step(bodies, N_BODIES, 1e-3);

    nbody_init(bodies, N_BODIES);

    double t0 = now_ns();
    for (int i = 0; i < ITERS; i++)
        nbody_step(bodies, N_BODIES, 1e-3);
    double t1 = now_ns();

    volatile double sink = bodies[0].x;
    (void)sink;

    double ns = (t1 - t0) / ITERS;
    printf("{\"language\":\"c\",\"example\":\"nbody\",\"ns_per_op\":%.2f,\"iterations\":%d}\n",
           ns, ITERS);
    return 0;
}
