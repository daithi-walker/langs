/**
 * @file   bench.ts
 * @brief  Benchmark for ball update — outputs JSON for the report generator.
 *
 * Run: npx ts-node src/bench.ts
 */

import { Ball, update } from "./physics";

const WARMUP = 100_000;
const RUNS   = 1_000_000;

const ball: Ball = { x: 400, y: 300, vx: 4, vy: 3 };

// Warm up V8's JIT
for (let i = 0; i < WARMUP; i++) update(ball, 30, 800, 600);

const t0 = performance.now();
for (let i = 0; i < RUNS; i++) update(ball, 30, 800, 600);
const t1 = performance.now();

const meanNs = ((t1 - t0) * 1e6) / RUNS;  // performance.now() is in ms

console.log(JSON.stringify({
    lang:    "nodejs",
    example: "bounce",
    mean_ns: parseFloat(meanNs.toFixed(4)),
    n:       RUNS,
}));
