# Node.js — Coding Standards

> Also read the top-level [`../STANDARDS.md`](../STANDARDS.md) first.

## Why Node.js?

Node.js runs JavaScript on V8 — the same JIT engine that powers Chrome.
V8 is a remarkably fast JIT: on tight numeric loops, warmed-up JS can
come within 2–5× of C. More interestingly, Node.js runs the same language
as every web browser, making it the natural bridge if you ever want to
port an algorithm to run in a web page.

For benchmarking: V8 has startup cost (~50ms) and a warm-up period.
After warm-up, numeric-heavy JS is faster than most people expect.
The gap vs C widens sharply with memory-intensive work (GC pauses).

---

## Toolchain

| Tool  | Version | Install |
|-------|---------|---------|
| node  | ≥ 22 LTS | `brew install node` |
| npm   | ≥ 10    | included with node |

We write **TypeScript** for all examples, compiled to JS before running.
TypeScript adds static types, which catch an entire class of bugs at
write-time rather than at runtime — especially important when doing
numeric work where silent type coercions cause subtle bugs.

Additional tools:
```bash
npm install -g typescript ts-node
```

Check: `node --version`, `tsc --version`

---

## Project Structure

```
<example>/
├── package.json     — dependencies and scripts
├── tsconfig.json    — TypeScript compiler config
├── src/
│   ├── main.ts      — entry point; no logic
│   └── <module>.ts  — pure logic; no I/O
└── test/
    └── <module>.test.ts  — tests using Vitest
```

---

## Style

### Formatting and Linting
- **Prettier** for formatting: `npm install --save-dev prettier`
  Run: `npx prettier --write .`
- **ESLint** for linting: `npm install --save-dev eslint @typescript-eslint/parser`
  Run: `npx eslint src/ test/`
- Line length: 100 characters.

### Naming
| Thing             | Convention       | Example              |
|-------------------|------------------|----------------------|
| Functions         | `camelCase`      | `updateBall`         |
| Variables         | `camelCase`      | `gridPsi2`           |
| Constants         | `UPPER_SNAKE`    | `RADIUS`             |
| Classes           | `PascalCase`     | `BallState`          |
| Types / Interfaces| `PascalCase`     | `BallState`          |
| Files             | `camelCase.ts`   | `physics.ts`         |

### TypeScript Rules
- `strict: true` in `tsconfig.json` — always.
- No `any` type. If you can't type it, use `unknown` and narrow it.
- Prefer `interface` for object shapes, `type` for unions and aliases.
- All function parameters and return types are explicitly annotated.

```typescript
function updateBall(
    x: number, y: number,
    vx: number, vy: number,
    radius: number, width: number, height: number,
): [number, number, number, number] { ... }
```

### Numbers
- JavaScript has only `number` (64-bit float). There is no `float32`.
  This means JS is always using double precision — note this in benchmarks.
- Use `Float32Array` / `Float64Array` for large numeric arrays (avoids
  GC pressure and enables better JIT optimisation).
- Integer arithmetic: use `Math.trunc()` or bitwise ops (`| 0`) when
  you genuinely need integer truncation. Document why.

### Modules
- Use ES modules (`import`/`export`), not CommonJS (`require`).
- One concept per file.

---

## Documentation

Use JSDoc `/** ... */` comments. TypeScript-aware IDEs render these inline:

```typescript
/**
 * Advances the ball position by one frame and reflects off walls.
 *
 * Modifies no shared state — returns new [x, y, vx, vy] values.
 * Reflection is elastic (no energy loss). Units are pixels and px/frame.
 *
 * @param x      Ball centre x in pixels
 * @param y      Ball centre y in pixels
 * @param vx     Horizontal velocity in pixels/frame
 * @param vy     Vertical velocity in pixels/frame
 * @param radius Ball radius in pixels
 * @param width  Window width in pixels
 * @param height Window height in pixels
 * @returns      Updated [x, y, vx, vy]
 */
```

File-level comment at the top:

```typescript
/**
 * @file   physics.ts
 * @brief  Ball physics for bounce_demo — no rendering dependency.
 */
```

---

## Testing with Vitest

Vitest is a fast Jest-compatible test runner built for TypeScript:

```bash
npm install --save-dev vitest
```

```typescript
// test/physics.test.ts
import { describe, it, expect } from 'vitest';
import { updateBall } from '../src/physics';

describe('updateBall', () => {
    it('reflects off the left wall', () => {
        const [, , vx] = updateBall(5, 100, -4, 3, 10, 800, 600);
        expect(vx).toBeCloseTo(4.0, 4);
    });

    it('intensity is non-negative everywhere', () => {
        const { interference } = await import('../src/physics');
        for (let x = 0; x < 800; x += 10) {
            expect(interference(x)).toBeGreaterThanOrEqual(0);
        }
    });
});
```

Run: `npx vitest run`
Watch mode: `npx vitest`

---

## Benchmarking

Use `performance.now()` (nanosecond resolution, available in Node ≥ 16):

```typescript
function bench(fn: () => void, n = 10_000): { meanMs: number; stdMs: number } {
    const times: number[] = [];
    for (let i = 0; i < n; i++) {
        const t0 = performance.now();
        fn();
        times.push(performance.now() - t0);
    }
    const mean = times.reduce((a, b) => a + b) / n;
    const std  = Math.sqrt(times.reduce((a, t) => a + (t - mean) ** 2, 0) / n);
    return { meanMs: mean, stdMs: std };
}
```

- Warm up V8 with 1000 un-timed iterations before measuring.
- Run at least 5000 timed iterations for stable stats.

---

## Common Pitfalls

| Pitfall | Rule |
|---------|------|
| `==` instead of `===` | always use strict equality `===` |
| Mutating function arguments | prefer returning new values |
| `var` | never use `var`; always `const` or `let` |
| Implicit `any` via untyped params | set `strict: true` and annotate everything |
| GC pauses during benchmark | pre-allocate `Float32Array` buffers; avoid object creation in hot loops |
| `0.1 + 0.2 !== 0.3` | floating point is IEEE 754; use toleranced comparison |
