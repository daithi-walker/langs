// Bounce — Go entry point.
//
// Pass --bench to run the benchmark and output JSON.
// Without flags, prints a note about the SDL2 rendering stub.
//
// Run tests:     go test ./...
// Run benchmark: go run . --bench
package main

import (
	"fmt"
	"os"
	"time"
)

const (
	benchWarmup = 100_000
	benchRuns   = 1_000_000
)

func runBench() {
	ball := Ball{X: 400, Y: 300, VX: 4, VY: 3}

	for range benchWarmup {
		ball.Update(30, 800, 600)
	}

	start := time.Now()
	for range benchRuns {
		ball.Update(30, 800, 600)
	}
	elapsed := time.Since(start)

	meanNs := float64(elapsed.Nanoseconds()) / float64(benchRuns)
	fmt.Printf("{\"lang\":\"go\",\"example\":\"bounce\",\"mean_ns\":%.4f,\"n\":%d}\n",
		meanNs, benchRuns)
}

func main() {
	if len(os.Args) > 1 && os.Args[1] == "--bench" {
		runBench()
		return
	}
	fmt.Fprintln(os.Stderr, "Go SDL2 rendering stub. Run `go run . --bench` for benchmark.")
}
