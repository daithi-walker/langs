#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "matmul.h"

#define WARMUP  5
#define ITERS  50

static double now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

static void fill(double *m, int n) {
    for (int i = 0; i < n*n; i++)
        m[i] = (double)i / (n*n);
}

int main(void) {
    static double A[N*N], B[N*N], C[N*N];
    fill(A, N); fill(B, N);

    for (int i = 0; i < WARMUP; i++) {
        memset(C, 0, sizeof(C));
        matmul(A, B, C, N);
    }

    double t0 = now_ns();
    for (int i = 0; i < ITERS; i++) {
        memset(C, 0, sizeof(C));
        matmul(A, B, C, N);
    }
    double t1 = now_ns();

    volatile double sink = C[0];
    (void)sink;

    double ns = (t1 - t0) / ITERS;
    printf("{\"language\":\"c\",\"example\":\"matmul\",\"ns_per_op\":%.2f,\"iterations\":%d}\n",
           ns, ITERS);
    return 0;
}
