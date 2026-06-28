// Package wave_packet implements split-operator TDSE physics.
//
// Matches the C version in c/qc/wave_packet/main.c exactly so benchmarks
// compare identical work. Uses Go's math/cmplx and a pure-Go FFT (gonum).
package main

import (
	"math"
	"math/cmplx"
)

const (
	N            = 1024
	dx           = 0.1
	dt           = 0.004
	mass         = 1.0
	x0           = N * dx * 0.25
	sigma        = N * dx * 0.06
	k0           = 4.0
	barrierX0    = N * dx * 0.55
	barrierWidth = N * dx * 0.04
	BarrierHDef  = 8.0
)

// Sim holds all simulation state.
type Sim struct {
	Psi    []complex128
	Prob   []float64
	phaseV []complex128
	phaseT []complex128
}

// NewSim allocates and initialises a simulation with the given barrier height.
func NewSim(barrierHeight float64) *Sim {
	s := &Sim{
		Psi:    make([]complex128, N),
		Prob:   make([]float64, N),
		phaseV: make([]complex128, N),
		phaseT: make([]complex128, N),
	}
	s.buildPhases(barrierHeight)
	s.InitPsi()
	return s
}

func (s *Sim) buildPhases(barrierHeight float64) {
	for i := 0; i < N; i++ {
		x := float64(i) * dx
		d := math.Abs(x - barrierX0)
		v := 0.0
		if d < barrierWidth*0.5 {
			v = barrierHeight
		}
		angle := -v * dt * 0.5
		s.phaseV[i] = complex(math.Cos(angle), math.Sin(angle))
	}
	for j := 0; j < N; j++ {
		kj := float64(j)
		if j > N/2 {
			kj = float64(j - N)
		}
		kj *= 2.0 * math.Pi / (N * dx)
		angle := -(kj * kj) * dt / (2.0 * mass)
		s.phaseT[j] = complex(math.Cos(angle), math.Sin(angle))
	}
}

// InitPsi resets ψ to the normalised Gaussian wave packet.
func (s *Sim) InitPsi() {
	norm := 0.0
	for i := 0; i < N; i++ {
		x := float64(i) * dx
		env := math.Exp(-(x-x0)*(x-x0) / (4.0 * sigma * sigma))
		s.Psi[i] = complex(env*math.Cos(k0*x), env*math.Sin(k0*x))
		norm += real(s.Psi[i])*real(s.Psi[i]) + imag(s.Psi[i])*imag(s.Psi[i])
	}
	norm = math.Sqrt(norm * dx)
	for i := 0; i < N; i++ {
		s.Psi[i] /= complex(norm, 0)
	}
}

// Step advances ψ one split-operator time step (symplectic, norm-preserving).
func (s *Sim) Step() {
	// Half V-step
	for i := 0; i < N; i++ {
		s.Psi[i] *= s.phaseV[i]
	}

	// Full T-step via FFT
	fft(s.Psi, false)
	invN := complex(1.0/N, 0)
	for j := 0; j < N; j++ {
		s.Psi[j] = s.Psi[j] * invN * s.phaseT[j]
	}
	fft(s.Psi, true)

	// Half V-step
	for i := 0; i < N; i++ {
		s.Psi[i] *= s.phaseV[i]
	}
}

// ComputeProb computes |ψ|² and returns the peak value.
func (s *Sim) ComputeProb() float64 {
	peak := 1e-12
	for i := 0; i < N; i++ {
		s.Prob[i] = real(s.Psi[i])*real(s.Psi[i]) + imag(s.Psi[i])*imag(s.Psi[i])
		if s.Prob[i] > peak {
			peak = s.Prob[i]
		}
	}
	return peak
}

// fft performs an in-place Cooley-Tukey FFT (inverse=false: forward, inverse=true: backward).
// Pure Go, no dependencies — N must be a power of 2.
func fft(a []complex128, inverse bool) {
	n := len(a)
	// Bit-reversal permutation
	j := 0
	for i := 1; i < n; i++ {
		bit := n >> 1
		for ; j&bit != 0; bit >>= 1 {
			j ^= bit
		}
		j ^= bit
		if i < j {
			a[i], a[j] = a[j], a[i]
		}
	}
	// Butterfly stages
	for length := 2; length <= n; length <<= 1 {
		angle := -2.0 * math.Pi / float64(length)
		if inverse {
			angle = -angle
		}
		w := cmplx.Exp(complex(0, angle))
		for i := 0; i < n; i += length {
			wn := complex(1, 0)
			for k := 0; k < length/2; k++ {
				u := a[i+k]
				v := a[i+k+length/2] * wn
				a[i+k] = u + v
				a[i+k+length/2] = u - v
				wn *= w
			}
		}
	}
}
