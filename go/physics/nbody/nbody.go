package main

import "math"

const (
	NBodies   = 1000
	g         = 6.674e-11
	softening = 1e-4
)

type Body struct {
	x, y, z     float64
	vx, vy, vz  float64
	mass        float64
}

func nbodyInit() []Body {
	bodies := make([]Body, NBodies)
	s := uint64(12345)
	lcg := func() float64 {
		s = s*6364136223846793005 + 1442695040888963407
		return float64(s>>33)/float64(uint64(1)<<31) - 1.0
	}
	for i := range bodies {
		bodies[i] = Body{x: lcg(), y: lcg(), z: lcg(), mass: 1.0}
	}
	return bodies
}

func nbodyStep(bodies []Body, dt float64) {
	n := len(bodies)
	ax := make([]float64, n)
	ay := make([]float64, n)
	az := make([]float64, n)
	for i := 0; i < n; i++ {
		for j := 0; j < n; j++ {
			if i == j {
				continue
			}
			dx := bodies[j].x - bodies[i].x
			dy := bodies[j].y - bodies[i].y
			dz := bodies[j].z - bodies[i].z
			r2 := dx*dx + dy*dy + dz*dz + softening*softening
			invR3 := 1.0 / (r2 * math.Sqrt(r2))
			gm := g * bodies[j].mass
			ax[i] += gm * dx * invR3
			ay[i] += gm * dy * invR3
			az[i] += gm * dz * invR3
		}
	}
	for i := range bodies {
		bodies[i].vx += ax[i] * dt
		bodies[i].vy += ay[i] * dt
		bodies[i].vz += az[i] * dt
		bodies[i].x += bodies[i].vx * dt
		bodies[i].y += bodies[i].vy * dt
		bodies[i].z += bodies[i].vz * dt
	}
}
