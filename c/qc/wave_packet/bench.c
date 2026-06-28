/**
 * @file   bench.c
 * @brief  Benchmark the split-operator TDSE step in isolation (no SDL2).
 *
 * Outputs JSON: {"language":"c","example":"wave_packet","ns_per_op":...,"iterations":...}
 */

#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N            1024
#define DX           0.1f
#define DT           0.004f
#define MASS         1.0f
#define X0           (N * DX * 0.25f)
#define SIGMA        (N * DX * 0.06f)
#define K0           4.0f
#define BARRIER_X0   (N * DX * 0.55f)
#define BARRIER_W    (N * DX * 0.04f)
#define BARRIER_H    8.0f
#define WARMUP       200
#define ITERS        2000

static fftwf_complex *psi, *psi_k, *phase_v, *phase_t;
static fftwf_plan plan_fwd, plan_inv;

static inline void cmul(fftwf_complex a, const fftwf_complex b, fftwf_complex out) {
    float re = a[0]*b[0] - a[1]*b[1];
    float im = a[0]*b[1] + a[1]*b[0];
    out[0] = re; out[1] = im;
}

static void build_phases(void) {
    for (int i = 0; i < N; i++) {
        float x = i * DX;
        float d = fabsf(x - BARRIER_X0);
        float v = (d < BARRIER_W * 0.5f) ? BARRIER_H : 0.0f;
        float angle = -v * DT * 0.5f;
        phase_v[i][0] = cosf(angle); phase_v[i][1] = sinf(angle);
    }
    for (int j = 0; j < N; j++) {
        float kj = (j <= N/2) ? (float)j : (float)(j - N);
        kj *= 2.0f * (float)M_PI / (N * DX);
        float angle = -(kj*kj) * DT / (2.0f * MASS);
        phase_t[j][0] = cosf(angle); phase_t[j][1] = sinf(angle);
    }
}

static void init_psi(void) {
    float norm = 0.0f;
    for (int i = 0; i < N; i++) {
        float x = i * DX;
        float env = expf(-(x-X0)*(x-X0) / (4.0f*SIGMA*SIGMA));
        psi[i][0] = env * cosf(K0*x);
        psi[i][1] = env * sinf(K0*x);
        norm += psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1];
    }
    norm = sqrtf(norm * DX);
    for (int i = 0; i < N; i++) { psi[i][0] /= norm; psi[i][1] /= norm; }
}

static void step(void) {
    for (int i = 0; i < N; i++) {
        fftwf_complex tmp = {psi[i][0], psi[i][1]};
        cmul(tmp, phase_v[i], psi[i]);
    }
    fftwf_execute(plan_fwd);
    float inv_n = 1.0f / N;
    for (int j = 0; j < N; j++) {
        fftwf_complex tmp = {psi_k[j][0]*inv_n, psi_k[j][1]*inv_n};
        cmul(tmp, phase_t[j], psi_k[j]);
    }
    fftwf_execute(plan_inv);
    for (int i = 0; i < N; i++) {
        fftwf_complex tmp = {psi[i][0], psi[i][1]};
        cmul(tmp, phase_v[i], psi[i]);
    }
}

int main(void) {
    psi     = fftwf_alloc_complex(N);
    psi_k   = fftwf_alloc_complex(N);
    phase_v = fftwf_alloc_complex(N);
    phase_t = fftwf_alloc_complex(N);

    plan_fwd = fftwf_plan_dft_1d(N, psi, psi_k, FFTW_FORWARD,  FFTW_MEASURE);
    plan_inv = fftwf_plan_dft_1d(N, psi_k, psi, FFTW_BACKWARD, FFTW_MEASURE);

    build_phases();
    init_psi();

    for (int i = 0; i < WARMUP; i++) step();
    init_psi();

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < ITERS; i++) step();
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double ns = ((t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec)) / ITERS;
    printf("{\"language\":\"c\",\"example\":\"wave_packet\",\"ns_per_op\":%.2f,\"iterations\":%d}\n",
           ns, ITERS);

    fftwf_destroy_plan(plan_fwd);
    fftwf_destroy_plan(plan_inv);
    fftwf_free(psi); fftwf_free(psi_k);
    fftwf_free(phase_v); fftwf_free(phase_t);
    return 0;
}
