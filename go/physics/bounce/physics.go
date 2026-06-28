// Package main implements ball physics for the bounce example.
//
// This file contains all logic that is independent of rendering —
// the only part worth unit-testing and benchmarking.
package main

import "math"

// Ball holds the mutable state of a single bouncing ball.
type Ball struct {
	X, Y   float32 // centre position (pixels)
	VX, VY float32 // velocity (pixels/frame)
}

// Update advances the ball by one frame and reflects elastically off walls.
//
// Position is updated first, then each velocity component is negated when
// the ball edge crosses a boundary. No penetration correction is applied.
//
// Parameters:
//   - radius: ball radius in pixels (must be > 0)
//   - width:  window width in pixels
//   - height: window height in pixels
func (b *Ball) Update(radius, width, height int) {
	b.X += b.VX
	b.Y += b.VY
	if b.X-float32(radius) < 0 || b.X+float32(radius) > float32(width) {
		b.VX = -b.VX
	}
	if b.Y-float32(radius) < 0 || b.Y+float32(radius) > float32(height) {
		b.VY = -b.VY
	}
}

// Speed returns the scalar speed of the ball in pixels/frame.
func (b *Ball) Speed() float32 {
	return float32(math.Sqrt(float64(b.VX*b.VX + b.VY*b.VY)))
}
