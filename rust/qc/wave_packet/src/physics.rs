//! Split-operator TDSE physics for a 1D wave packet.
//!
//! Implements the same algorithm as the C version in c/qc/wave_packet/main.c
//! using rustfft for the FFT step. All parameters match exactly so benchmarks
//! are comparing identical work.

use rustfft::{FftPlanner, num_complex::Complex};
use std::f32::consts::PI;

pub const N: usize = 1024;
pub const DX: f32  = 0.1;
pub const DT: f32  = 0.004;
pub const MASS: f32 = 1.0;

const X0: f32    = N as f32 * DX * 0.25;
const SIGMA: f32 = N as f32 * DX * 0.06;
const K0: f32    = 4.0;

const BARRIER_X0: f32    = N as f32 * DX * 0.55;
const BARRIER_WIDTH: f32 = N as f32 * DX * 0.04;
pub const BARRIER_H_DEF: f32 = 8.0;

/// All simulation state in one struct so bench.rs can own it cleanly.
pub struct Sim {
    pub psi:     Vec<Complex<f32>>,
    pub prob:    Vec<f32>,
    phase_v:     Vec<Complex<f32>>,
    phase_t:     Vec<Complex<f32>>,
    fft_fwd:     std::sync::Arc<dyn rustfft::Fft<f32>>,
    fft_inv:     std::sync::Arc<dyn rustfft::Fft<f32>>,
    scratch:     Vec<Complex<f32>>,
}

impl Sim {
    /// Allocate and initialise a new simulation with the given barrier height.
    pub fn new(barrier_height: f32) -> Self {
        let mut planner = FftPlanner::new();
        let fft_fwd = planner.plan_fft_forward(N);
        let fft_inv = planner.plan_fft_inverse(N);
        let scratch_len = fft_fwd.get_inplace_scratch_len().max(fft_inv.get_inplace_scratch_len());

        let mut sim = Sim {
            psi:     vec![Complex::new(0.0, 0.0); N],
            prob:    vec![0.0f32; N],
            phase_v: vec![Complex::new(0.0, 0.0); N],
            phase_t: vec![Complex::new(0.0, 0.0); N],
            fft_fwd,
            fft_inv,
            scratch: vec![Complex::new(0.0, 0.0); scratch_len],
        };
        sim.build_phases(barrier_height);
        sim.init_psi();
        sim
    }

    /// Build phase factors from the potential. Called once at init.
    fn build_phases(&mut self, barrier_height: f32) {
        for i in 0..N {
            let x = i as f32 * DX;
            let d = (x - BARRIER_X0).abs();
            let v = if d < BARRIER_WIDTH * 0.5 { barrier_height } else { 0.0 };
            let angle = -v * DT * 0.5;
            self.phase_v[i] = Complex::new(angle.cos(), angle.sin());
        }
        for j in 0..N {
            let kj = if j <= N / 2 { j as f32 } else { j as f32 - N as f32 };
            let kj = kj * 2.0 * PI / (N as f32 * DX);
            let angle = -(kj * kj) * DT / (2.0 * MASS);
            self.phase_t[j] = Complex::new(angle.cos(), angle.sin());
        }
    }

    /// Reset ψ to the initial normalised Gaussian wave packet.
    pub fn init_psi(&mut self) {
        let mut norm = 0.0f32;
        for i in 0..N {
            let x = i as f32 * DX;
            let env = (-(x - X0) * (x - X0) / (4.0 * SIGMA * SIGMA)).exp();
            self.psi[i] = Complex::new(env * (K0 * x).cos(), env * (K0 * x).sin());
            norm += self.psi[i].norm_sqr();
        }
        norm = (norm * DX).sqrt();
        for i in 0..N {
            self.psi[i] /= norm;
        }
    }

    /// Advance ψ one split-operator time step.
    ///
    /// Operator splitting: e^{-iH dt} ≈ e^{-iV dt/2} · e^{-iT dt} · e^{-iV dt/2}
    /// Norm-preserving (symplectic) to machine precision.
    pub fn step(&mut self) {
        let inv_n = 1.0 / N as f32;

        // Half V-step
        for i in 0..N {
            self.psi[i] *= self.phase_v[i];
        }

        // Full T-step: FFT → apply phase_t → IFFT
        self.fft_fwd.process_with_scratch(&mut self.psi, &mut self.scratch);
        for j in 0..N {
            self.psi[j] = self.psi[j] * inv_n * self.phase_t[j];
        }
        self.fft_inv.process_with_scratch(&mut self.psi, &mut self.scratch);

        // Half V-step
        for i in 0..N {
            self.psi[i] *= self.phase_v[i];
        }
    }

    /// Compute |ψ|² into self.prob. Returns the peak value.
    pub fn compute_prob(&mut self) -> f32 {
        let mut peak = 1e-12f32;
        for i in 0..N {
            self.prob[i] = self.psi[i].norm_sqr();
            if self.prob[i] > peak { peak = self.prob[i]; }
        }
        peak
    }
}
