/**
 * @file   main.c
 * @brief  SDL2 bouncing ball — entry point and rendering only.
 *
 * All physics logic lives in physics.c. This file is responsible solely
 * for the window, renderer, event loop, and drawing. Keeping it separate
 * means the physics can be unit-tested and benchmarked without SDL2.
 *
 * @section deps Dependencies
 *   SDL2: brew install sdl2
 *
 * @section compile Compile
 *   make
 *
 * @section run Run
 *   ./bounce
 */

#include <SDL2/SDL.h>
#include <math.h>
#include "physics.h"

#define WIDTH  800
#define HEIGHT 600
#define RADIUS 30
#define FPS_MS 16   /* ~60 fps */

/**
 * @brief  Draw a filled circle via horizontal scanlines.
 *
 * SDL2 has no built-in filled-circle primitive. For each row dy we derive
 * the half-chord width from the Pythagorean theorem and draw a line.
 *
 * @param r   Renderer to draw into.
 * @param cx  Centre x (pixels).
 * @param cy  Centre y (pixels).
 * @param rad Radius (pixels).
 */
static void draw_circle(SDL_Renderer *r, int cx, int cy, int rad) {
    for (int dy = -rad; dy <= rad; dy++) {
        int dx = (int)sqrt((double)(rad * rad - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

/**
 * @brief  Program entry point.
 * @return 0 on clean exit, 1 on SDL2 init failure.
 */
int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window   *win = SDL_CreateWindow("Bounce — C",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    Ball ball = { .x = WIDTH / 2.0f, .y = HEIGHT / 2.0f, .vx = 4.0f, .vy = 3.0f };

    SDL_Event ev;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&ev))
            if (ev.type == SDL_QUIT) running = 0;

        ball_update(&ball, RADIUS, WIDTH, HEIGHT);

        SDL_SetRenderDrawColor(ren, 20, 20, 20, 255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 255, 80, 80, 255);
        draw_circle(ren, (int)ball.x, (int)ball.y, RADIUS);
        SDL_RenderPresent(ren);
        SDL_Delay(FPS_MS);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
