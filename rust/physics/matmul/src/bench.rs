const N: usize = 256;
const WARMUP: usize = 5;
const ITERS: usize = 50;

fn fill(m: &mut [f64]) {
    let n2 = m.len() as f64;
    for (i, v) in m.iter_mut().enumerate() {
        *v = i as f64 / n2;
    }
}

fn matmul(a: &[f64], b: &[f64], c: &mut [f64], n: usize) {
    for i in 0..n {
        for k in 0..n {
            let a_ik = a[i * n + k];
            for j in 0..n {
                c[i * n + j] += a_ik * b[k * n + j];
            }
        }
    }
}

fn main() {
    let mut a = vec![0.0f64; N * N];
    let mut b = vec![0.0f64; N * N];
    let mut c = vec![0.0f64; N * N];
    fill(&mut a);
    fill(&mut b);

    for _ in 0..WARMUP {
        c.fill(0.0);
        matmul(&a, &b, &mut c, N);
    }

    let t0 = std::time::Instant::now();
    for _ in 0..ITERS {
        c.fill(0.0);
        matmul(&a, &b, &mut c, N);
    }
    let elapsed = t0.elapsed();

    let _ = std::hint::black_box(c[0]);

    let ns = elapsed.as_nanos() as f64 / ITERS as f64;
    println!(
        "{{\"language\":\"rust\",\"example\":\"matmul\",\"ns_per_op\":{:.2},\"iterations\":{}}}",
        ns, ITERS
    );
}
