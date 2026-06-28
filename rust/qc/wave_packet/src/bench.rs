//! Benchmark the split-operator TDSE step.
//!
//! Outputs JSON: {"language":"rust","example":"wave_packet","ns_per_op":...,"iterations":...}

mod physics;
use physics::Sim;
use std::hint::black_box;
use std::time::Instant;

const WARMUP: usize  = 200;
const ITERS:  usize  = 2_000;

fn main() {
    let mut sim = Sim::new(physics::BARRIER_H_DEF);

    // Warmup — let CPU reach steady-state clock and rustfft cache plans
    for _ in 0..WARMUP {
        black_box(&mut sim).step();
    }
    sim.init_psi();

    // Timed loop
    let start = Instant::now();
    for _ in 0..ITERS {
        black_box(&mut sim).step();
    }
    let elapsed = start.elapsed();

    let ns = elapsed.as_nanos() as f64 / ITERS as f64;
    println!(
        r#"{{"language":"rust","example":"wave_packet","ns_per_op":{:.2},"iterations":{}}}"#,
        ns, ITERS
    );
}
