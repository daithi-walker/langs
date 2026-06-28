// Wave packet tunneling — ASCII demo and optional benchmark.
//
// Usage:
//
//	go run . --bench   print JSON benchmark result
//	go run .           run ASCII visualisation for 500 steps
package main

import (
	"fmt"
	"os"
	"strings"
	"time"
)

const (
	warmup = 200
	iters  = 2_000
	cols   = 80
	rows   = 12
)

func main() {
	if len(os.Args) > 1 && os.Args[1] == "--bench" {
		runBench()
		return
	}
	runDemo()
}

func runBench() {
	sim := NewSim(BarrierHDef)

	for i := 0; i < warmup; i++ {
		sim.Step()
	}
	sim.InitPsi()

	start := time.Now()
	for i := 0; i < iters; i++ {
		sim.Step()
	}
	elapsed := time.Since(start)

	ns := float64(elapsed.Nanoseconds()) / float64(iters)
	fmt.Printf(`{"language":"go","example":"wave_packet","ns_per_op":%.2f,"iterations":%d}`+"\n", ns, iters)
}

func runDemo() {
	sim := NewSim(BarrierHDef)

	for step := 0; step < 500; step++ {
		sim.Step()
		if (step+1)%50 == 0 {
			peak := sim.ComputeProb()
			fmt.Printf("--- step %d ---\n", step+1)

			// Downsample prob to cols
			heights := make([]int, cols)
			for i, p := range sim.Prob {
				col := i * cols / N
				h := int((p / peak) * float64(rows))
				if h > heights[col] {
					heights[col] = h
				}
			}
			for r := rows - 1; r >= 0; r-- {
				var sb strings.Builder
				for _, h := range heights {
					if h > r {
						sb.WriteByte('#')
					} else {
						sb.WriteByte(' ')
					}
				}
				fmt.Printf("|%s|\n", sb.String())
			}
			fmt.Println()
		}
	}
}
