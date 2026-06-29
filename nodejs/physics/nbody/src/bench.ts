/**
 * N-body gravitational simulation benchmark — Node.js / TypeScript.
 *
 * N=1000 bodies, O(n²) direct pairwise forces.
 * Outputs JSON: {"language":"nodejs","example":"nbody","ns_per_op":...,"iterations":...}
 */

const N_BODIES = 1000;
const G        = 6.674e-11;
const SOFT     = 1e-4;
const WARMUP   = 20;
const ITERS    = 200;

const x    = new Float64Array(N_BODIES);
const y    = new Float64Array(N_BODIES);
const z    = new Float64Array(N_BODIES);
const vx   = new Float64Array(N_BODIES);
const vy   = new Float64Array(N_BODIES);
const vz   = new Float64Array(N_BODIES);
const mass = new Float64Array(N_BODIES);
const ax   = new Float64Array(N_BODIES);
const ay   = new Float64Array(N_BODIES);
const az   = new Float64Array(N_BODIES);

function init(): void {
    let s = BigInt(12345);
    const MUL = BigInt("6364136223846793005");
    const ADD = BigInt("1442695040888963407");
    const MASK = (BigInt(1) << BigInt(64)) - BigInt(1);
    const lcg = (): number => {
        s = (s * MUL + ADD) & MASK;
        return Number(s >> BigInt(33)) / Math.pow(2, 31) - 1.0;
    };
    for (let i = 0; i < N_BODIES; i++) {
        x[i] = lcg(); y[i] = lcg(); z[i] = lcg();
        vx[i] = 0; vy[i] = 0; vz[i] = 0;
        mass[i] = 1.0;
    }
}

function step(dt: number): void {
    ax.fill(0); ay.fill(0); az.fill(0);
    for (let i = 0; i < N_BODIES; i++) {
        for (let j = 0; j < N_BODIES; j++) {
            if (i === j) continue;
            const dx = x[j] - x[i];
            const dy = y[j] - y[i];
            const dz = z[j] - z[i];
            const r2 = dx*dx + dy*dy + dz*dz + SOFT*SOFT;
            const invR3 = 1.0 / (r2 * Math.sqrt(r2));
            const gm = G * mass[j];
            ax[i] += gm * dx * invR3;
            ay[i] += gm * dy * invR3;
            az[i] += gm * dz * invR3;
        }
    }
    for (let i = 0; i < N_BODIES; i++) {
        vx[i] += ax[i] * dt; vy[i] += ay[i] * dt; vz[i] += az[i] * dt;
        x[i]  += vx[i] * dt; y[i]  += vy[i] * dt; z[i]  += vz[i] * dt;
    }
}

init();
for (let i = 0; i < WARMUP; i++) step(1e-3);
init();

const t0 = process.hrtime.bigint();
for (let i = 0; i < ITERS; i++) step(1e-3);
const t1 = process.hrtime.bigint();

const ns = Number(t1 - t0) / ITERS;
console.log(JSON.stringify({
    language:   "nodejs",
    example:    "nbody",
    ns_per_op:  Math.round(ns * 100) / 100,
    iterations: ITERS,
}));
