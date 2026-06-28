//! Wave packet tunneling — text-mode demo (no SDL2 dependency in Rust).
//!
//! Prints a simple ASCII visualisation of |ψ|² and runs for 500 steps.
//! The physics is identical to the C SDL2 version.

mod physics;
use physics::Sim;

const COLS: usize = 80;
const ROWS: usize = 12;

fn main() {
    let mut sim = Sim::new(physics::BARRIER_H_DEF);

    for step in 0..500 {
        sim.step();
        if step % 50 == 49 {
            let peak = sim.compute_prob();
            println!("--- step {} ---", step + 1);
            // Downsample prob to COLS
            let mut row = vec![0usize; COLS];
            for (i, &p) in sim.prob.iter().enumerate() {
                let col = i * COLS / physics::N;
                let h = ((p / peak) * ROWS as f32) as usize;
                if h > row[col] { row[col] = h; }
            }
            for r in (0..ROWS).rev() {
                let line: String = row.iter().map(|&h| if h > r { '#' } else { ' ' }).collect();
                println!("|{}|", line);
            }
            println!();
        }
    }
}
