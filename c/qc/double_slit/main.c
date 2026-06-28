/*
 * DOUBLE-SLIT EXPERIMENT — quantum interference simulation
 *
 * What this models:
 *   In classical physics, particles fired at two slits make two bands.
 *   In quantum mechanics, each particle interferes with itself — even
 *   when sent one at a time — producing an interference pattern.
 *
 *   We model the particle as a wave described by:
 *     ψ(x) = exp(ikr₁)/r₁ + exp(ikr₂)/r₂
 *   where r₁, r₂ are distances from each slit, k = 2π/λ.
 *   The detection probability at a screen point is |ψ|².
 *
 *   A full simulation fires particles one at a time, placing each
 *   probabilistically (Monte Carlo sampling of |ψ|²), and you watch
 *   the interference pattern emerge from apparent randomness.
 *
 * What a full implementation needs:
 *   1. Complex number arithmetic  (C99 has <complex.h> built in)
 *   2. Compute |ψ(x)|² across the screen
 *   3. Normalize to a probability distribution
 *   4. Monte Carlo: for each "particle", pick a screen point via
 *      inverse CDF sampling or rejection sampling
 *   5. Accumulate hits into a histogram, render as brightness
 *   6. Parameters to play with: slit separation, slit width, wavelength
 *
 * Complexity: low-moderate. <complex.h> removes the hard part.
 * No extra libraries needed beyond SDL2.
 *
 * What this stub does:
 *   Computes and renders |ψ|² analytically across a screen strip,
 *   showing what the final pattern looks like without Monte Carlo yet.
 */

#include <SDL2/SDL.h>
#include <complex.h>
#include <math.h>

#define WIDTH       800
#define HEIGHT      600
#define SCREEN_Y    500.0f    /* detector screen distance from slits (px) */
#define SLIT_SEP    40.0f     /* slit separation (px) */
#define WAVELENGTH  20.0f     /* de Broglie wavelength (px) */

static float interference(float screen_x) {
    float cx = WIDTH / 2.0f;
    float s1x = cx - SLIT_SEP / 2.0f;
    float s2x = cx + SLIT_SEP / 2.0f;

    float r1 = sqrtf((screen_x - s1x) * (screen_x - s1x) + SCREEN_Y * SCREEN_Y);
    float r2 = sqrtf((screen_x - s2x) * (screen_x - s2x) + SCREEN_Y * SCREEN_Y);

    float k = 2.0f * M_PI / WAVELENGTH;
    float complex psi = cexpf(I * k * r1) / r1 + cexpf(I * k * r2) / r2;
    return cabsf(psi) * cabsf(psi);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window   *window   = SDL_CreateWindow("Double Slit (stub)",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
    SDL_RenderClear(renderer);

    /* find max for normalisation */
    float max_val = 0.0f;
    for (int x = 0; x < WIDTH; x++) {
        float v = interference((float)x);
        if (v > max_val) max_val = v;
    }

    /* draw interference pattern as vertical bars at the screen line */
    for (int x = 0; x < WIDTH; x++) {
        float v = interference((float)x) / max_val;
        int brightness = (int)(v * 255.0f);
        SDL_SetRenderDrawColor(renderer, brightness, brightness, (int)(brightness * 0.6f), 255);
        /* draw a column of height proportional to intensity */
        int bar_h = (int)(v * 150.0f);
        SDL_RenderDrawLine(renderer, x, HEIGHT - 20, x, HEIGHT - 20 - bar_h);
    }

    /* draw the slits */
    SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);
    int cx = WIDTH / 2;
    SDL_RenderDrawLine(renderer, cx - (int)(SLIT_SEP/2), 10, cx - (int)(SLIT_SEP/2), 30);
    SDL_RenderDrawLine(renderer, cx + (int)(SLIT_SEP/2), 10, cx + (int)(SLIT_SEP/2), 30);

    SDL_RenderPresent(renderer);

    SDL_Event event;
    int running = 1;
    while (running)
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT) running = 0;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*
 * Compile:
 *   clang main.c -o double_slit -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include \
 *         -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 -lm -Wl,-framework,Cocoa
 *
 * Next steps to make this real:
 *   1. Add Monte Carlo particle-by-particle emission
 *   2. Add slit width (sinc envelope on each slit's contribution)
 *   3. Make wavelength / slit params keyboard-adjustable at runtime
 *   4. Animate: show particle hits accumulating over time
 */
