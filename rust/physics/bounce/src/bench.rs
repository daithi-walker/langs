//! Standalone benchmark binary — outputs JSON for the report generator.
//!
//! Run: cargo run --release --bin bench_bounce

mod physics;
use physics::Ball;
use std::time::Instant;

const WARMUP: usize = 100_000;
const RUNS:   usize = 1_000_000;

fn main() {
    let mut ball = Ball::new(400.0, 300.0, 4.0, 3.0);

    for _ in 0..WARMUP {
        ball.update(30, 800, 600);
    }

    let start = Instant::now();
    for _ in 0..RUNS {
        std::hint::black_box(ball.update(
            std::hint::black_box(30),
            std::hint::black_box(800),
            std::hint::black_box(600),
        ));
    }
    let elapsed = start.elapsed();

    let mean_ns = elapsed.as_nanos() as f64 / RUNS as f64;
    println!(
        "{{\"lang\":\"rust\",\"example\":\"bounce\",\"mean_ns\":{:.4},\"n\":{}}}",
        mean_ns, RUNS
    );
}
