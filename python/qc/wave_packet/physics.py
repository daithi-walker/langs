"""
Split-operator TDSE physics using NumPy.

Identical parameters to c/qc/wave_packet/bench.c so results are comparable.
The FFT is backed by NumPy's pocketfft (C), and all array ops are vectorized —
no Python loops in the hot path.
"""

import numpy as np

N             = 1024
DX            = 0.1
DT            = 0.004
MASS          = 1.0
X0            = N * DX * 0.25
SIGMA         = N * DX * 0.06
K0            = 4.0
BARRIER_X0    = N * DX * 0.55
BARRIER_WIDTH = N * DX * 0.04
BARRIER_H_DEF = 8.0


class Sim:
    def __init__(self, barrier_height: float = BARRIER_H_DEF):
        x = np.arange(N, dtype=np.float32) * DX

        # Potential
        V = np.where(np.abs(x - BARRIER_X0) < BARRIER_WIDTH * 0.5,
                     barrier_height, 0.0).astype(np.float32)

        # Position-space half-step phase: exp(-i V dt/2)
        self.phase_v = np.exp(-1j * V * DT * 0.5).astype(np.complex64)

        # Momentum-space full-step phase: exp(-i k² dt / 2m)
        j = np.arange(N, dtype=np.float32)
        kj = np.where(j <= N // 2, j, j - N) * (2.0 * np.pi / (N * DX))
        self.phase_t = np.exp(-1j * kj**2 * DT / (2.0 * MASS)).astype(np.complex64)

        self.psi = np.zeros(N, dtype=np.complex64)
        self.init_psi()

    def init_psi(self):
        x = np.arange(N, dtype=np.float32) * DX
        env = np.exp(-(x - X0)**2 / (4.0 * SIGMA**2))
        self.psi = (env * np.exp(1j * K0 * x)).astype(np.complex64)
        norm = np.sqrt(np.sum(np.abs(self.psi)**2) * DX)
        self.psi /= norm

    def step(self):
        """One split-operator TDSE step — entirely vectorized, no Python loops."""
        self.psi *= self.phase_v
        self.psi = np.fft.fft(self.psi) / N
        self.psi *= self.phase_t
        self.psi = np.fft.ifft(self.psi)
        self.psi *= self.phase_v
