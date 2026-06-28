/**
 * @file   physics.ts
 * @brief  Ball physics — no rendering dependency.
 *
 * Contains the two functions worth benchmarking: `update` (position +
 * wall reflection) and `speed` (scalar magnitude).
 */

/** Mutable state for a single bouncing ball. */
export interface Ball {
    x:  number;   // Centre x position (pixels)
    y:  number;   // Centre y position (pixels)
    vx: number;   // Horizontal velocity (pixels/frame)
    vy: number;   // Vertical velocity (pixels/frame)
}

/**
 * Advances the ball by one frame and reflects elastically off walls.
 *
 * Mutates the ball object in place. Position is updated first, then each
 * velocity component is negated when the ball edge crosses a boundary.
 *
 * @param ball   Ball state, mutated in place.
 * @param radius Ball radius in pixels (must be > 0).
 * @param width  Window width in pixels.
 * @param height Window height in pixels.
 */
export function update(ball: Ball, radius: number, width: number, height: number): void {
    ball.x += ball.vx;
    ball.y += ball.vy;
    if (ball.x - radius < 0 || ball.x + radius > width)  ball.vx = -ball.vx;
    if (ball.y - radius < 0 || ball.y + radius > height) ball.vy = -ball.vy;
}

/**
 * Returns the scalar speed of the ball in pixels/frame.
 *
 * @param ball Ball state (read-only).
 * @returns sqrt(vx² + vy²).
 */
export function speed(ball: Ball): number {
    return Math.sqrt(ball.vx ** 2 + ball.vy ** 2);
}
