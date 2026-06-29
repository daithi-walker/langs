//! Benchmark one nbody step — N=1000 bodies, O(n²) pairwise gravity.
//!
//! Outputs JSON: {"language":"rust","example":"nbody","ns_per_op":...,"iterations":...}

mod physics;
use physics::{nbody_init, nbody_step};
use std::hint::black_box;
use std::time::Instant;

const WARMUP: usize = 20;
const ITERS:  usize = 200;

fn main() {
    let mut bodies = nbody_init();

    for _ in 0..WARMUP {
        nbody_step(&mut bodies, 1e-3);
    }
    bodies = nbody_init();

    let start = Instant::now();
    for _ in 0..ITERS {
        nbody_step(black_box(&mut bodies), 1e-3);
    }
    let elapsed = start.elapsed();

    let ns = elapsed.as_nanos() as f64 / ITERS as f64;
    println!(
        r#"{{"language":"rust","example":"nbody","ns_per_op":{:.2},"iterations":{}}}"#,
        ns, ITERS
    );
}
