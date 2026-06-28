/**
 * @file   physics.c
 * @brief  Ball physics — isolated from SDL2 and main() for testability.
 *
 * Contains the only two functions worth benchmarking: ball_update (position
 * + wall reflection) and ball_speed (magnitude of velocity vector).
 * Everything else in this example is rendering boilerplate.
 *
 * @section compile Compile (via Makefile)
 *   make
 */

#include "physics.h"
#include <math.h>

/**
 * @brief  Advance ball by one frame; reflect elastically off window walls.
 *
 * Position is updated first, then each velocity component is negated if
 * the ball edge has crossed a boundary. Reflection is instantaneous and
 * lossless — no penetration correction, no energy damping.
 *
 * @param b       Ball state, modified in place.
 * @param radius  Ball radius in pixels (must be > 0).
 * @param width   Window width in pixels.
 * @param height  Window height in pixels.
 */
void ball_update(Ball *b, int radius, int width, int height) {
    b->x += b->vx;
    b->y += b->vy;
    if (b->x - radius < 0 || b->x + radius > width)  b->vx = -b->vx;
    if (b->y - radius < 0 || b->y + radius > height) b->vy = -b->vy;
}

/**
 * @brief  Return the scalar speed of the ball (pixels/frame).
 * @param  b  Ball state (read-only).
 * @return    sqrt(vx² + vy²).
 */
float ball_speed(const Ball *b) {
    return sqrtf(b->vx * b->vx + b->vy * b->vy);
}
