const N      = 256;
const WARMUP = 5;
const ITERS  = 50;

const a = new Float64Array(N * N);
const b = new Float64Array(N * N);
const c = new Float64Array(N * N);

function fill(): void {
    const n2 = N * N;
    for (let i = 0; i < n2; i++) { a[i] = i / n2; b[i] = i / n2; }
}

function matmul(): void {
    for (let i = 0; i < N; i++) {
        for (let k = 0; k < N; k++) {
            const aik = a[i * N + k];
            for (let j = 0; j < N; j++)
                c[i * N + j] += aik * b[k * N + j];
        }
    }
}

fill();
for (let i = 0; i < WARMUP; i++) { c.fill(0); matmul(); }
fill();

const t0 = process.hrtime.bigint();
for (let i = 0; i < ITERS; i++) { c.fill(0); matmul(); }
const t1 = process.hrtime.bigint();

const ns = Number(t1 - t0) / ITERS;
console.log(JSON.stringify({
    language:   "nodejs",
    example:    "matmul",
    ns_per_op:  Math.round(ns * 100) / 100,
    iterations: ITERS,
}));
