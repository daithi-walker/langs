pub const N_BODIES: usize = 1000;
const G: f64 = 6.674e-11;
const SOFTENING: f64 = 1e-4;

#[derive(Clone, Copy)]
pub struct Body {
    pub x: f64, pub y: f64, pub z: f64,
    pub vx: f64, pub vy: f64, pub vz: f64,
    pub mass: f64,
}

pub fn nbody_init() -> Vec<Body> {
    let mut bodies = Vec::with_capacity(N_BODIES);
    let mut s: u64 = 12345;
    let lcg = |s: &mut u64| -> f64 {
        *s = s.wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
        (*s >> 33) as f64 / (1u64 << 31) as f64 - 1.0
    };
    for _ in 0..N_BODIES {
        bodies.push(Body {
            x: lcg(&mut s), y: lcg(&mut s), z: lcg(&mut s),
            vx: 0.0, vy: 0.0, vz: 0.0,
            mass: 1.0,
        });
    }
    bodies
}

pub fn nbody_step(bodies: &mut [Body], dt: f64) {
    let n = bodies.len();
    let mut acc = vec![(0.0f64, 0.0f64, 0.0f64); n];
    for i in 0..n {
        for j in 0..n {
            if i == j { continue; }
            let dx = bodies[j].x - bodies[i].x;
            let dy = bodies[j].y - bodies[i].y;
            let dz = bodies[j].z - bodies[i].z;
            let r2 = dx*dx + dy*dy + dz*dz + SOFTENING*SOFTENING;
            let inv_r3 = 1.0 / (r2 * r2.sqrt());
            let gm = G * bodies[j].mass;
            acc[i].0 += gm * dx * inv_r3;
            acc[i].1 += gm * dy * inv_r3;
            acc[i].2 += gm * dz * inv_r3;
        }
    }
    for i in 0..n {
        bodies[i].vx += acc[i].0 * dt;
        bodies[i].vy += acc[i].1 * dt;
        bodies[i].vz += acc[i].2 * dt;
        bodies[i].x  += bodies[i].vx * dt;
        bodies[i].y  += bodies[i].vy * dt;
        bodies[i].z  += bodies[i].vz * dt;
    }
}
