# Rust — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why Rust?

Rust achieves C-level performance with memory safety guaranteed at
compile time. There is no garbage collector — the compiler enforces
ownership rules that prevent the entire class of bugs (use-after-free,
data races, null pointer dereferences) that plague C programs. It is
the language most likely to replace C in new systems code.

For benchmarking: Rust typically matches C within a few percent. The
interesting comparison is not raw speed but expressiveness — the same
algorithm is often clearer in Rust than C while being equally fast.

---

## Toolchain

| Tool    | Version | Install |
|---------|---------|---------|
| rustc   | stable  | `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs \| sh` |
| cargo   | stable  | included with rustup |
| clippy  | stable  | `rustup component add clippy` |

Check: `rustc --version`, `cargo --version`

---

## Project Structure

Each example is a **Cargo binary crate**:

```
<example>/
├── Cargo.toml      — package manifest and dependencies
├── src/
│   ├── main.rs     — entry point only; no logic
│   └── <module>.rs — pure logic (no I/O)
└── tests/
    └── test_<module>.rs   — integration tests
```

Unit tests for small functions go directly in `<module>.rs` in a
`#[cfg(test)]` block. Integration tests that test the public API
go in `tests/`.

---

## Style

### Formatting
- Run `cargo fmt` before committing. It is non-negotiable.
- Line length: 100 characters (configured in `rustfmt.toml`).

### Naming
| Thing           | Convention       | Example              |
|-----------------|------------------|----------------------|
| Functions       | `snake_case`     | `update_ball`        |
| Variables       | `snake_case`     | `grid_psi2`          |
| Constants       | `UPPER_SNAKE`    | `RADIUS`             |
| Types / Structs | `PascalCase`     | `BallState`          |
| Modules         | `snake_case`     | `physics`            |
| Files           | `snake_case.rs`  | `physics.rs`         |

### Types
- Prefer `f32` for graphics and physics (matches GPU, consistent with C).
- Use `f64` only where precision is demonstrably needed.
- Use `usize` for indices and counts.
- Use `num::complex::Complex<f32>` for wavefunctions (add `num` crate).

### Ownership and Borrowing
- Prefer borrowing (`&T`, `&mut T`) over cloning.
- Only clone when you genuinely need an independent copy.
- Use `Vec<T>` for heap-allocated arrays; use slices `&[T]` in
  function signatures so callers can pass both `Vec` and arrays.

### Error Handling
- Use `Result<T, E>` for functions that can fail.
- Use `?` to propagate errors up to `main`.
- Never `.unwrap()` in library code. `.unwrap()` is acceptable only
  in `main()` or tests where a panic is the right failure mode.
- Use `anyhow` crate for application-level error handling.

---

## Documentation

Rust uses `///` for doc comments, rendered by `cargo doc`:

```rust
/// Computes the interference intensity at a point on the detector screen.
///
/// Both slits are treated as coherent point sources. The returned value
/// is |ψ|² — proportional to the probability of detecting a particle at `x`.
///
/// # Arguments
/// * `screen_x` - x position on the detector screen (pixels)
///
/// # Returns
/// Intensity value ≥ 0.0. Not normalised; caller should normalise to [0, 1].
fn interference(screen_x: f32) -> f32 { ... }
```

File-level doc comments use `//!`:

```rust
//! Double-slit quantum interference simulation.
//!
//! Computes |ψ(x)|² for two coherent point sources and renders the
//! resulting interference pattern using SDL2.
```

---

## Testing

Rust has built-in test support. No external framework needed.

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ball_reflects_off_left_wall() {
        let mut state = BallState { x: 5.0, y: 100.0, vx: -4.0, vy: 3.0 };
        state.update(10, 800, 600);
        assert!((state.vx - 4.0).abs() < 1e-5, "vx should flip sign");
    }
}
```

Run: `cargo test`

---

## Benchmarking

Use the `criterion` crate for statistically rigorous benchmarks:

```toml
# Cargo.toml
[dev-dependencies]
criterion = "0.5"

[[bench]]
name = "bench_physics"
harness = false
```

```rust
// benches/bench_physics.rs
use criterion::{criterion_group, criterion_main, Criterion};

fn bench_update_ball(c: &mut Criterion) {
    c.bench_function("update_ball", |b| {
        b.iter(|| {
            // ... timed work ...
        })
    });
}

criterion_group!(benches, bench_update_ball);
criterion_main!(benches);
```

Run: `cargo bench`

---

## Linting

Run `cargo clippy -- -D warnings` before any example is considered done.
Clippy catches common anti-patterns and suggests idiomatic Rust.

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| Fighting the borrow checker | it is almost always right; restructure data instead of reaching for `Rc<RefCell<T>>` |
| Premature `clone()` | ask whether a borrow would work first |
| `unwrap()` in library code | use `?` and `Result` instead |
| Integer casting | always use `as` explicitly; Rust does not coerce numeric types |
| Panicking on index | use `.get(i)` which returns `Option<&T>` if the index might be out of bounds |
