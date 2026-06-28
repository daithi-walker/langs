/**
 * Wave packet TDSE benchmark — Node.js / TypeScript.
 *
 * Pure-JS Cooley-Tukey FFT, matching Go and Java implementations.
 * Outputs JSON: {"language":"nodejs","example":"wave_packet","ns_per_op":...,"iterations":...}
 */

const N             = 1024;
const DX            = 0.1;
const DT            = 0.004;
const MASS          = 1.0;
const X0            = N * DX * 0.25;
const SIGMA         = N * DX * 0.06;
const K0            = 4.0;
const BARRIER_X0    = N * DX * 0.55;
const BARRIER_WIDTH = N * DX * 0.04;
const BARRIER_H_DEF = 8.0;
const WARMUP        = 200;
const ITERS         = 2_000;

// Parallel float arrays for complex numbers (avoids object overhead)
let psiRe    = new Float64Array(N);
let psiIm    = new Float64Array(N);
const phaseVRe = new Float64Array(N);
const phaseVIm = new Float64Array(N);
const phaseTRe = new Float64Array(N);
const phaseTIm = new Float64Array(N);

function buildPhases(barrierHeight: number): void {
    for (let i = 0; i < N; i++) {
        const x = i * DX;
        const d = Math.abs(x - BARRIER_X0);
        const v = d < BARRIER_WIDTH * 0.5 ? barrierHeight : 0.0;
        const angle = -v * DT * 0.5;
        phaseVRe[i] = Math.cos(angle);
        phaseVIm[i] = Math.sin(angle);
    }
    for (let j = 0; j < N; j++) {
        let kj = j <= N / 2 ? j : j - N;
        kj *= 2.0 * Math.PI / (N * DX);
        const angle = -(kj * kj) * DT / (2.0 * MASS);
        phaseTRe[j] = Math.cos(angle);
        phaseTIm[j] = Math.sin(angle);
    }
}

function initPsi(): void {
    let norm = 0.0;
    for (let i = 0; i < N; i++) {
        const x   = i * DX;
        const env = Math.exp(-(x - X0) * (x - X0) / (4.0 * SIGMA * SIGMA));
        psiRe[i] = env * Math.cos(K0 * x);
        psiIm[i] = env * Math.sin(K0 * x);
        norm += psiRe[i] * psiRe[i] + psiIm[i] * psiIm[i];
    }
    norm = Math.sqrt(norm * DX);
    for (let i = 0; i < N; i++) { psiRe[i] /= norm; psiIm[i] /= norm; }
}

/** In-place Cooley-Tukey FFT. N must be power of 2. */
function fft(re: Float64Array, im: Float64Array, inverse: boolean): void {
    const n = re.length;
    // Bit-reversal
    for (let i = 1, j = 0; i < n; i++) {
        let bit = n >> 1;
        for (; (j & bit) !== 0; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            [re[i], re[j]] = [re[j], re[i]];
            [im[i], im[j]] = [im[j], im[i]];
        }
    }
    // Butterfly
    for (let len = 2; len <= n; len <<= 1) {
        const ang = 2.0 * Math.PI / len * (inverse ? 1 : -1);
        const wRe = Math.cos(ang), wIm = Math.sin(ang);
        for (let i = 0; i < n; i += len) {
            let curRe = 1.0, curIm = 0.0;
            for (let k = 0; k < len / 2; k++) {
                const uRe = re[i+k], uIm = im[i+k];
                const vRe = re[i+k+len/2]*curRe - im[i+k+len/2]*curIm;
                const vIm = re[i+k+len/2]*curIm + im[i+k+len/2]*curRe;
                re[i+k] = uRe+vRe; im[i+k] = uIm+vIm;
                re[i+k+len/2] = uRe-vRe; im[i+k+len/2] = uIm-vIm;
                const nextRe = curRe*wRe - curIm*wIm;
                curIm = curRe*wIm + curIm*wRe;
                curRe = nextRe;
            }
        }
    }
}

function step(): void {
    // Half V-step
    for (let i = 0; i < N; i++) {
        const re = psiRe[i]*phaseVRe[i] - psiIm[i]*phaseVIm[i];
        const im = psiRe[i]*phaseVIm[i] + psiIm[i]*phaseVRe[i];
        psiRe[i] = re; psiIm[i] = im;
    }
    // FFT forward, apply phase_t, inverse FFT
    fft(psiRe, psiIm, false);
    const invN = 1.0 / N;
    for (let j = 0; j < N; j++) {
        const re = psiRe[j]*invN*phaseTRe[j] - psiIm[j]*invN*phaseTIm[j];
        const im = psiRe[j]*invN*phaseTIm[j] + psiIm[j]*invN*phaseTRe[j];
        psiRe[j] = re; psiIm[j] = im;
    }
    fft(psiRe, psiIm, true);
    // Half V-step
    for (let i = 0; i < N; i++) {
        const re = psiRe[i]*phaseVRe[i] - psiIm[i]*phaseVIm[i];
        const im = psiRe[i]*phaseVIm[i] + psiIm[i]*phaseVRe[i];
        psiRe[i] = re; psiIm[i] = im;
    }
}

buildPhases(BARRIER_H_DEF);
initPsi();

for (let i = 0; i < WARMUP; i++) step();
initPsi();

const t0 = process.hrtime.bigint();
for (let i = 0; i < ITERS; i++) step();
const t1 = process.hrtime.bigint();

const ns = Number(t1 - t0) / ITERS;
console.log(JSON.stringify({
    language: "nodejs",
    example: "wave_packet",
    ns_per_op: Math.round(ns * 100) / 100,
    iterations: ITERS,
}));
