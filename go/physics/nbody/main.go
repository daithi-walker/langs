// N-body gravitational simulation benchmark.
//
// Usage:
//
//	go run . --bench   print JSON benchmark result
package main

import (
	"fmt"
	"os"
	"time"
)

const (
	warmup = 20
	iters  = 200
)

func main() {
	if len(os.Args) > 1 && os.Args[1] == "--bench" {
		runBench()
		return
	}
	fmt.Println("Run with --bench to execute the benchmark.")
}

func runBench() {
	bodies := nbodyInit()

	for i := 0; i < warmup; i++ {
		nbodyStep(bodies, 1e-3)
	}
	bodies = nbodyInit()

	start := time.Now()
	for i := 0; i < iters; i++ {
		nbodyStep(bodies, 1e-3)
	}
	elapsed := time.Since(start)

	ns := float64(elapsed.Nanoseconds()) / float64(iters)
	fmt.Printf(`{"language":"go","example":"nbody","ns_per_op":%.2f,"iterations":%d}`+"\n", ns, iters)
}
