#pragma once

#define N 256

void matmul(const double * restrict A,
            const double * restrict B,
            double       * restrict C,
            int n);
