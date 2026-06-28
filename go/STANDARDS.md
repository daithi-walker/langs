# Go — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why Go?

Go was designed at Google for large-scale systems work. It compiles to
native code (fast), has a built-in concurrency model (goroutines), and
enforces a single opinionated style (gofmt). It is slower than C/Rust
for pure computation but competitive for concurrent and I/O-bound work.
Its simplicity makes it a good language to read even as a newcomer.

For benchmarking: expect Go to be 1.5–3× slower than C on CPU-bound
single-threaded tasks. The interesting comparisons are concurrent tasks
where goroutines shine, and startup/toolchain ergonomics.

---

## Toolchain

| Tool  | Version | Install |
|-------|---------|---------|
| go    | ≥ 1.22  | `brew install go` |

Check: `go version`

---

## Project Structure

Each example is a Go module:

```
<example>/
├── go.mod          — module declaration
├── main.go         — entry point only; no logic
├── <module>.go     — pure logic
└── <module>_test.go  — tests (Go convention: same package, _test.go suffix)
```

Go co-locates tests with source (`physics_test.go` next to `physics.go`)
rather than in a separate directory. This is the language convention and
we follow it.

---

## Style

### Formatting
- Run `gofmt -w .` before committing. It is non-negotiable.
- `goimports` (superset of gofmt) is preferred: `go install golang.org/x/tools/cmd/goimports@latest`

### Naming
| Thing             | Convention      | Example             |
|-------------------|-----------------|---------------------|
| Functions (public)| `PascalCase`    | `UpdateBall`        |
| Functions (private)| `camelCase`    | `drawCircle`        |
| Variables         | `camelCase`     | `gridPsi2`          |
| Constants         | `PascalCase`    | `Radius`            |
| Types / Structs   | `PascalCase`    | `BallState`         |
| Files             | `snake_case.go` | `physics.go`        |

Public (exported) means visible outside the package. Use public names
only for things that tests or main.go need to call directly.

### Types
- Use `float32` for physics and graphics (consistent with C baseline).
- Use `float64` only where the extra precision is required.
- Use `complex64` for wavefunctions (built-in Go type).
- Use `int` for general counters; `int64` for large accumulators.

### Error Handling
- Functions that can fail return `(value, error)`.
- Always check errors. Never assign to `_` unless you have a written
  reason why the error is safe to ignore.
- Wrap errors with context: `fmt.Errorf("update_ball: %w", err)`.
- Panic only for programmer errors (nil pointer, bad index) — not for
  runtime conditions that can happen in normal use.

---

## Documentation

Go uses `//` doc comments directly above the declaration:

```go
// UpdateBall advances the ball position by one frame and reflects off walls.
//
// x, y are modified in place. vx, vy are negated when the ball edge
// reaches a window boundary. Units are pixels and pixels-per-frame.
func UpdateBall(x, y, vx, vy *float32, radius, width, height int) { ... }
```

Package-level doc comment goes above `package <name>`:

```go
// Package physics implements the ball simulation logic for bounce_demo.
// It has no dependency on SDL2 and can be tested independently.
package physics
```

Run `go doc ./...` to view rendered docs.

---

## Testing

Go's testing package is built in. No external framework needed.

```go
// physics_test.go
package physics

import (
    "math"
    "testing"
)

func TestBallReflectsOffLeftWall(t *testing.T) {
    x, y, vx, vy := float32(5), float32(100), float32(-4), float32(3)
    UpdateBall(&x, &y, &vx, &vy, 10, 800, 600)
    if math.Abs(float64(vx)-4.0) > 1e-5 {
        t.Errorf("expected vx=4.0 after left wall bounce, got %f", vx)
    }
}
```

Run: `go test ./...`
Verbose: `go test -v ./...`

---

## Benchmarking

Go has built-in benchmark support in the test package:

```go
func BenchmarkUpdateBall(b *testing.B) {
    x, y, vx, vy := float32(400), float32(300), float32(4), float32(3)
    for range b.N {
        UpdateBall(&x, &y, &vx, &vy, 30, 800, 600)
    }
}
```

Run: `go test -bench=. -benchtime=5s -count=5`

The `-count=5` flag runs 5 independent benchmark runs, which is enough
to compute mean and variance for the HTML report.

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| Goroutine leaks | every goroutine must have a clear exit path; use `context.Context` for cancellation |
| Ignoring errors | never `_` an error without a comment explaining why |
| Shadowing with `:=` | easy to shadow outer variables; use `=` when assigning to an existing variable |
| Slice vs array | `[]float32` is a slice (reference); `[4]float32` is a value. Know which you have |
| `float32` vs `float64` | Go defaults to `float64` in literals; suffix with `f` via explicit cast |
