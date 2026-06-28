//! Bounce — Rust entry point.
//!
//! SDL2 window rendering is omitted until `sdl2` crate is added to
//! Cargo.toml (`cargo add sdl2`). The physics and tests work standalone.
//! Run `cargo test` to verify and `cargo run --bin bench_bounce` to benchmark.

mod physics;

fn main() {
    eprintln!("Rust SDL2 rendering stub.");
    eprintln!("Run `cargo test` for unit tests.");
    eprintln!("Run `cargo run --release --bin bench_bounce` for the benchmark.");
}
