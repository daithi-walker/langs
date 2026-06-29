#include "matmul.h"

/* Row-major, cache-friendly i-k-j order so the inner loop streams C and B. */
void matmul(const double * restrict A,
            const double * restrict B,
            double       * restrict C,
            int n) {
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double a = A[i*n + k];
            for (int j = 0; j < n; j++)
                C[i*n + j] += a * B[k*n + j];
        }
    }
}
