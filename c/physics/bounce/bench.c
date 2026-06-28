/**
 * @file   bench.c
 * @brief  Benchmark for ball_update — outputs JSON consumed by the report generator.
 *
 * Runs ball_update N times in a tight loop with no I/O inside the timed
 * region. The volatile sink prevents the compiler from optimising the
 * loop away entirely (dead-code elimination).
 *
 * Output format (one JSON object to stdout):
 *   {"lang":"c","example":"bounce","mean_ns":42.3,"std_ns":1.2,"n":1000000}
 */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "physics.h"

#define WARMUP  100000
#define RUNS    1000000

static double now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

int main(void) {
    Ball ball = { .x = 400.0f, .y = 300.0f, .vx = 4.0f, .vy = 3.0f };

    /* Warm up — not timed */
    for (int i = 0; i < WARMUP; i++)
        ball_update(&ball, 30, 800, 600);

    /* Timed runs */
    double t0 = now_ns();
    for (int i = 0; i < RUNS; i++)
        ball_update(&ball, 30, 800, 600);
    double t1 = now_ns();

    /* Prevent dead-code elimination */
    volatile float sink = ball.x;
    (void)sink;

    double mean_ns = (t1 - t0) / RUNS;
    printf("{\"lang\":\"c\",\"example\":\"bounce\",\"mean_ns\":%.4f,\"n\":%d}\n",
           mean_ns, RUNS);
    return 0;
}
