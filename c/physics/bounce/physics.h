/**
 * @file   physics.h
 * @brief  Ball physics declarations — no SDL2 dependency.
 */

#ifndef BOUNCE_PHYSICS_H
#define BOUNCE_PHYSICS_H

/** Mutable state for a single bouncing ball. */
typedef struct {
    float x;   /**< Centre x position (pixels). */
    float y;   /**< Centre y position (pixels). */
    float vx;  /**< Horizontal velocity (pixels/frame). */
    float vy;  /**< Vertical velocity (pixels/frame). */
} Ball;

void  ball_update(Ball *b, int radius, int width, int height);
float ball_speed(const Ball *b);

#endif
