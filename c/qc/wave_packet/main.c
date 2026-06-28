/**
 * @file   main.c
 * @brief  Time-dependent Schrödinger equation — animated wave packet tunneling.
 *
 * WHAT YOU ARE LOOKING AT
 * -----------------------
 * A quantum particle (like an electron) moves through space as a wave.
 * Unlike a classical ball, it doesn't have a definite position — it has
 * a *probability* of being found at each location.
 *
 * The CYAN CURVE is the probability density |ψ(x)|².
 *   - Height at each x = how likely you are to find the particle there.
 *   - The total area under the curve always equals 1 (particle must be
 *     somewhere), so as it spreads out, the peak gets shorter.
 *   - The curve's peak moves like a classical particle until it hits
 *     the barrier.
 *
 * The ORANGE RECTANGLE is an energy barrier — a region where it costs
 *   energy to be (like a wall, but quantum).
 *
 * QUANTUM TUNNELING
 * -----------------
 * Classically, if a particle has less energy than the wall, it bounces
 * back. Quantum mechanically, there is a probability of passing through
 * anyway — called tunneling. You'll see the wave split at the barrier:
 *   - Part reflects leftward (the particle bounced back).
 *   - Part transmits rightward (the particle tunneled through the wall).
 *   - How much tunnels depends on barrier height. Lower barrier → more
 *     tunneling. Try pressing '-' to lower it and watch more get through.
 *
 * This is not a simulation trick — quantum tunneling is real and powers
 * tunnel diodes, flash memory, and nuclear fusion in the sun.
 *
 * Method: Split-Operator (split-step Fourier) on the TDSE.
 *   iħ ∂ψ/∂t = [ -ħ²/2m ∂²/∂x² + V(x) ] ψ
 *
 * Controls:
 *   R        — reset wave packet to initial state
 *   +/-      — raise/lower barrier height
 *   Space    — pause/unpause
 *   Escape   — quit
 *
 * @section deps Dependencies
 *   SDL2:     brew install sdl2
 *   SDL2_ttf: brew install sdl2_ttf
 *   FFTW3:    brew install fftw
 *
 * @section units Natural units
 *   ħ = 1, m = 1. Length in arbitrary units mapped to pixels.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Layout
 * ---------------------------------------------------------------------- */
#define WIDTH        1024     /* window width (pixels) */
#define PLOT_HEIGHT   480     /* height of the physics plot area */
#define LEGEND_H      160     /* height of the legend panel below */
#define HEIGHT       (PLOT_HEIGHT + LEGEND_H)

/* -------------------------------------------------------------------------
 * Simulation parameters
 * ---------------------------------------------------------------------- */
#define N       1024      /* grid points (power of 2 for FFT) */
#define DX      0.1f
#define DT      0.004f
#define MASS    1.0f

#define X0      (N * DX * 0.25f)
#define SIGMA   (N * DX * 0.06f)
#define K0      4.0f

#define BARRIER_X0    (N * DX * 0.55f)
#define BARRIER_WIDTH (N * DX * 0.04f)
#define BARRIER_H_DEF  8.0f

/* -------------------------------------------------------------------------
 * Simulation globals
 * ---------------------------------------------------------------------- */
static fftwf_complex *psi;
static fftwf_complex *psi_k;
static float         *V;
static float         *prob;
static fftwf_complex *phase_V;
static fftwf_complex *phase_T;
static fftwf_plan plan_fwd, plan_inv;
static float barrier_height = BARRIER_H_DEF;
static int   sim_step       = 0;

/* -------------------------------------------------------------------------
 * Text rendering
 * ---------------------------------------------------------------------- */
static TTF_Font *font_md = NULL;
static TTF_Font *font_sm = NULL;

#define FONT_PATH  "/System/Library/Fonts/SFNSMono.ttf"
#define FONT_BOLD  "/System/Library/Fonts/SFNSText.ttf"

/**
 * @brief  Render a UTF-8 string at (x,y).
 */
static void draw_text(SDL_Renderer *ren, TTF_Font *font,
                      const char *text, int x, int y,
                      Uint8 r, Uint8 g, Uint8 b) {
    SDL_Color col = {r, g, b, 255};
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

/* -------------------------------------------------------------------------
 * Physics
 * ---------------------------------------------------------------------- */

/** @brief  Build the rectangular barrier potential V(x). */
static void build_potential(void) {
    for (int i = 0; i < N; i++) {
        float x = i * DX;
        float d = fabsf(x - BARRIER_X0);
        V[i] = (d < BARRIER_WIDTH * 0.5f) ? barrier_height : 0.0f;
    }
}

/** @brief  Precompute exp(-iV dt/2) and exp(-ik² dt/2m) phase factors. */
static void build_phases(void) {
    for (int i = 0; i < N; i++) {
        float angle = -V[i] * DT * 0.5f;
        phase_V[i][0] = cosf(angle);
        phase_V[i][1] = sinf(angle);
    }
    for (int j = 0; j < N; j++) {
        float kj = (j <= N/2) ? (float)j : (float)(j - N);
        kj *= (2.0f * (float)M_PI) / (N * DX);
        float angle = -(kj * kj) * DT / (2.0f * MASS);
        phase_T[j][0] = cosf(angle);
        phase_T[j][1] = sinf(angle);
    }
}

/** @brief  Initialise ψ to a normalised Gaussian wave packet. */
static void init_psi(void) {
    float norm = 0.0f;
    for (int i = 0; i < N; i++) {
        float x   = i * DX;
        float env = expf(-(x - X0)*(x - X0) / (4.0f*SIGMA*SIGMA));
        psi[i][0] = env * cosf(K0 * x);
        psi[i][1] = env * sinf(K0 * x);
        norm += psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1];
    }
    norm = sqrtf(norm * DX);
    for (int i = 0; i < N; i++) { psi[i][0] /= norm; psi[i][1] /= norm; }
    sim_step = 0;
}

/** @brief  Complex multiply: out = a * b. */
static inline void cmul(fftwf_complex a, const fftwf_complex b, fftwf_complex out) {
    float re = a[0]*b[0] - a[1]*b[1];
    float im = a[0]*b[1] + a[1]*b[0];
    out[0] = re; out[1] = im;
}

/**
 * @brief  Advance ψ one split-operator time step.
 *
 * Operator splitting: e^{-iH dt} ≈ e^{-iV dt/2} e^{-iT dt} e^{-iV dt/2}
 * This is symplectic (norm-preserving) to machine precision.
 */
static void step(void) {
    for (int i = 0; i < N; i++) {
        fftwf_complex tmp = {psi[i][0], psi[i][1]};
        cmul(tmp, phase_V[i], psi[i]);
    }
    fftwf_execute(plan_fwd);
    float inv_n = 1.0f / N;
    for (int j = 0; j < N; j++) {
        fftwf_complex tmp = {psi_k[j][0]*inv_n, psi_k[j][1]*inv_n};
        cmul(tmp, phase_T[j], psi_k[j]);
    }
    fftwf_execute(plan_inv);
    for (int i = 0; i < N; i++) {
        fftwf_complex tmp = {psi[i][0], psi[i][1]};
        cmul(tmp, phase_V[i], psi[i]);
    }
    sim_step++;
}

/* -------------------------------------------------------------------------
 * Rendering
 * ---------------------------------------------------------------------- */

/**
 * @brief  Draw the physics plot (probability curve + barrier).
 *
 * @param ren  Active SDL renderer.
 */
static void render_plot(SDL_Renderer *ren) {
    /* Clear plot area */
    SDL_Rect plot_rect = {0, 0, WIDTH, PLOT_HEIGHT};
    SDL_SetRenderDrawColor(ren, 10, 10, 22, 255);
    SDL_RenderFillRect(ren, &plot_rect);

    /* Compute |ψ|² */
    float max_prob = 1e-12f;
    for (int i = 0; i < N; i++) {
        prob[i] = psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1];
        if (prob[i] > max_prob) max_prob = prob[i];
    }

    int baseline = PLOT_HEIGHT - 30;
    int plot_h   = baseline - 20;

    /* Baseline reference line */
    SDL_SetRenderDrawColor(ren, 35, 38, 55, 255);
    SDL_RenderDrawLine(ren, 0, baseline, WIDTH, baseline);

    /* Potential barrier — orange-red filled */
    for (int i = 0; i < N; i++) {
        if (V[i] > 0.0f) {
            int x  = i * WIDTH / N;
            /* Scale barrier so full height occupies 1/3 of plot area */
            int bh = (int)(120.0f * V[i] / fmaxf(barrier_height, 1.0f));
            SDL_SetRenderDrawColor(ren, 220, 90, 30, 200);
            SDL_RenderDrawLine(ren, x, baseline, x, baseline - bh);
        }
    }

    /* |ψ|² — cyan filled area */
    for (int i = 1; i < N; i++) {
        int x1 = (i-1) * WIDTH / N;
        int x2 =  i    * WIDTH / N;
        int y1 = baseline - (int)(prob[i-1] / max_prob * plot_h);
        int y2 = baseline - (int)(prob[i]   / max_prob * plot_h);
        /* Fill area */
        SDL_SetRenderDrawColor(ren, 0, 100, 95, 50);
        SDL_RenderDrawLine(ren, x2, y2, x2, baseline);
        /* Bright outline */
        SDL_SetRenderDrawColor(ren, 0, 220, 200, 255);
        SDL_RenderDrawLine(ren, x1, y1, x2, y2);
    }

    /* Status line at top of plot */
    if (font_sm) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Barrier: %.0f   Step: %d", barrier_height, sim_step);
        draw_text(ren, font_sm, buf, 10, 6, 100, 110, 130);
    }
}

/**
 * @brief  Draw the explanatory legend panel below the plot.
 *
 * Explains what the cyan curve and orange barrier are, and what
 * tunneling means — in plain language.
 *
 * @param ren  Active SDL renderer.
 */
static void render_legend(SDL_Renderer *ren) {
    SDL_Rect panel = {0, PLOT_HEIGHT, WIDTH, LEGEND_H};
    SDL_SetRenderDrawColor(ren, 16, 18, 30, 255);
    SDL_RenderFillRect(ren, &panel);
    SDL_SetRenderDrawColor(ren, 45, 50, 70, 255);
    SDL_RenderDrawLine(ren, 0, PLOT_HEIGHT, WIDTH, PLOT_HEIGHT);

    if (!font_md) return;

    int y = PLOT_HEIGHT + 10;
    int col1 = 16, col2 = 320, col3 = 640;

    /* --- Left column: cyan curve --- */
    /* Swatch */
    SDL_SetRenderDrawColor(ren, 0, 220, 200, 255);
    SDL_Rect cs = {col1, y+2, 16, 12};
    SDL_RenderFillRect(ren, &cs);
    draw_text(ren, font_md, "Probability density |?|²", col1+22, y, 0, 220, 200);
    y += 22;
    draw_text(ren, font_sm, "Height = how likely the particle is", col1, y, 130, 140, 160);
    y += 16;
    draw_text(ren, font_sm, "to be found at that position.", col1, y, 130, 140, 160);
    y += 16;
    draw_text(ren, font_sm, "Total area is always = 1.", col1, y, 90, 100, 120);
    y += 22;
    draw_text(ren, font_sm, "Controls:", col1, y, 100, 110, 140);
    y += 16;
    draw_text(ren, font_sm, "R = reset   Space = pause", col1, y, 80, 90, 110);

    /* --- Middle column: barrier --- */
    int y2 = PLOT_HEIGHT + 10;
    SDL_SetRenderDrawColor(ren, 220, 90, 30, 255);
    SDL_Rect os = {col2, y2+2, 16, 12};
    SDL_RenderFillRect(ren, &os);
    draw_text(ren, font_md, "Energy barrier", col2+22, y2, 220, 90, 30);
    y2 += 22;
    draw_text(ren, font_sm, "A region that costs energy to enter", col2, y2, 130, 140, 160);
    y2 += 16;
    draw_text(ren, font_sm, "(like a wall for the particle).", col2, y2, 130, 140, 160);
    y2 += 16;
    draw_text(ren, font_sm, "Height = energy needed to pass.", col2, y2, 90, 100, 120);
    y2 += 22;
    char bline[48];
    snprintf(bline, sizeof(bline), "Current height: %.0f  (K0^2/2=8)", barrier_height);
    draw_text(ren, font_sm, bline, col2, y2, 200, 140, 80);
    y2 += 16;
    draw_text(ren, font_sm, "+/- to raise or lower it.", col2, y2, 80, 90, 110);

    /* --- Right column: tunneling --- */
    int y3 = PLOT_HEIGHT + 10;
    draw_text(ren, font_md, "Quantum tunneling", col3, y3, 170, 150, 255);
    y3 += 22;
    draw_text(ren, font_sm, "Classically: particle bounces if", col3, y3, 130, 140, 160);
    y3 += 16;
    draw_text(ren, font_sm, "it lacks energy to clear the wall.", col3, y3, 130, 140, 160);
    y3 += 16;
    draw_text(ren, font_sm, "Quantum: part of the wave leaks", col3, y3, 130, 140, 160);
    y3 += 16;
    draw_text(ren, font_sm, "through anyway (tunneling).", col3, y3, 130, 140, 160);
    y3 += 16;
    draw_text(ren, font_sm, "Watch the wave split at the wall:", col3, y3, 90, 100, 120);
    y3 += 14;
    draw_text(ren, font_sm, "reflected left + transmitted right.", col3, y3, 90, 100, 120);
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

/**
 * @brief  Entry point — allocate, initialise, run event loop.
 * @return 0 on clean exit.
 */
int main(void) {
    psi     = fftwf_alloc_complex(N);
    psi_k   = fftwf_alloc_complex(N);
    V       = malloc(N * sizeof(float));
    prob    = malloc(N * sizeof(float));
    phase_V = fftwf_alloc_complex(N);
    phase_T = fftwf_alloc_complex(N);

    plan_fwd = fftwf_plan_dft_1d(N, psi, psi_k, FFTW_FORWARD,  FFTW_MEASURE);
    plan_inv = fftwf_plan_dft_1d(N, psi_k, psi,  FFTW_BACKWARD, FFTW_MEASURE);

    build_potential();
    build_phases();
    init_psi();

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    font_md = TTF_OpenFont(FONT_PATH, 15);
    font_sm = TTF_OpenFont(FONT_PATH, 12);
    if (!font_md) font_md = TTF_OpenFont("/Library/Fonts/Arial.ttf", 15);
    if (!font_sm) font_sm = TTF_OpenFont("/Library/Fonts/Arial.ttf", 12);

    SDL_Window   *win = SDL_CreateWindow(
        "Wave Packet — Quantum Tunneling",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int paused = 0;
    int steps_per_frame = 8;
    SDL_Event ev;

    while (1) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) goto done;
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE: goto done;
                    case SDLK_r:
                        init_psi();
                        break;
                    case SDLK_SPACE:
                        paused = !paused;
                        break;
                    case SDLK_EQUALS:
                    case SDLK_PLUS:
                        barrier_height += 1.0f;
                        build_potential(); build_phases(); init_psi();
                        break;
                    case SDLK_MINUS:
                        barrier_height = fmaxf(0.0f, barrier_height - 1.0f);
                        build_potential(); build_phases(); init_psi();
                        break;
                }
            }
        }

        if (!paused)
            for (int s = 0; s < steps_per_frame; s++)
                step();

        render_plot(ren);
        render_legend(ren);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

done:
    if (font_md) TTF_CloseFont(font_md);
    if (font_sm) TTF_CloseFont(font_sm);
    TTF_Quit();
    fftwf_destroy_plan(plan_fwd);
    fftwf_destroy_plan(plan_inv);
    fftwf_free(psi); fftwf_free(psi_k);
    fftwf_free(phase_V); fftwf_free(phase_T);
    free(V); free(prob);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
