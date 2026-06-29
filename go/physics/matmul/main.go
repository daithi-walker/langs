package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"time"
)

const N = 256

func fill(m []float64) {
	n2 := float64(len(m))
	for i := range m {
		m[i] = float64(i) / n2
	}
}

func matmul(a, b, c []float64, n int) {
	for i := 0; i < n; i++ {
		for k := 0; k < n; k++ {
			aik := a[i*n+k]
			for j := 0; j < n; j++ {
				c[i*n+j] += aik * b[k*n+j]
			}
		}
	}
}

func main() {
	bench := flag.Bool("bench", false, "run benchmark")
	flag.Parse()

	const warmup = 5
	const iters = 50

	a := make([]float64, N*N)
	b := make([]float64, N*N)
	c := make([]float64, N*N)
	fill(a)
	fill(b)

	if *bench {
		for i := 0; i < warmup; i++ {
			for j := range c { c[j] = 0 }
			matmul(a, b, c, N)
		}

		t0 := time.Now()
		for i := 0; i < iters; i++ {
			for j := range c { c[j] = 0 }
			matmul(a, b, c, N)
		}
		ns := float64(time.Since(t0).Nanoseconds()) / float64(iters)

		_ = c[0]
		out, _ := json.Marshal(map[string]any{
			"language":   "go",
			"example":    "matmul",
			"ns_per_op":  ns,
			"iterations": iters,
		})
		fmt.Println(string(out))
	} else {
		fill(a); fill(b)
		for j := range c { c[j] = 0 }
		matmul(a, b, c, N)
		fmt.Printf("C[0]=%f C[%d]=%f\n", c[0], N*N-1, c[N*N-1])
	}
}
