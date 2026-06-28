import { describe, it, expect } from "vitest";
import { Ball, update, speed } from "../src/physics";

describe("update", () => {
    it("moves ball by velocity each frame", () => {
        const b: Ball = { x: 100, y: 100, vx: 4, vy: 3 };
        update(b, 10, 800, 600);
        expect(b.x).toBeCloseTo(104, 2);
        expect(b.y).toBeCloseTo(103, 2);
    });

    it("reflects off left wall", () => {
        const b: Ball = { x: 10, y: 100, vx: -5, vy: 3 };
        update(b, 10, 800, 600);
        expect(b.vx).toBeCloseTo(5, 2);
    });

    it("reflects off right wall", () => {
        const b: Ball = { x: 794, y: 100, vx: 5, vy: 3 };
        update(b, 10, 800, 600);
        expect(b.vx).toBeCloseTo(-5, 2);
    });

    it("reflects off top wall", () => {
        const b: Ball = { x: 100, y: 8, vx: 4, vy: -5 };
        update(b, 10, 800, 600);
        expect(b.vy).toBeCloseTo(5, 2);
    });

    it("reflects off bottom wall", () => {
        const b: Ball = { x: 100, y: 594, vx: 4, vy: 5 };
        update(b, 10, 800, 600);
        expect(b.vy).toBeCloseTo(-5, 2);
    });
});

describe("speed", () => {
    it("returns pythagorean magnitude", () => {
        const b: Ball = { x: 0, y: 0, vx: 3, vy: 4 };
        expect(speed(b)).toBeCloseTo(5, 3);
    });

    it("is preserved on elastic bounce", () => {
        const b: Ball = { x: 10, y: 100, vx: -3, vy: 4 };
        const before = speed(b);
        update(b, 10, 800, 600);
        expect(speed(b)).toBeCloseTo(before, 3);
    });
});
