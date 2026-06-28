/**
 * @file   main.c
 * @brief  Hydrogen atom orbital probability density renderer with on-screen UI.
 *
 * Renders |ψ_{nlm}(x,y,0)|² — the probability of finding the electron at
 * each point in the z=0 cross-section through the hydrogen atom.
 *
 * The wavefunction is:
 *   ψ_{nlm}(r,θ,φ) = R_{nl}(r) · Y_l^m(θ,φ)
 *
 * Quantum numbers:
 *   n = principal (energy level: 1=ground state, higher=more energy)
 *   l = angular momentum (0=s sphere, 1=p dumbbell, 2=d cloverleaf, 3=f)
 *   m = magnetic (orientation of the orbital, -l to +l)
 *
 * Colour: black=zero probability, white=highest probability.
 * Bright spots show where the electron is most likely to be found.
 *
 * @section deps Dependencies
 *   SDL2:     brew install sdl2
 *   SDL2_ttf: brew install sdl2_ttf
 *   GSL:      brew install gsl
 *
 * @section compile Compile
 *   make
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <gsl/gsl_sf_laguerre.h>
#include <gsl/gsl_sf_legendre.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Window layout: orbital rendered in left square, UI panel on the right */
#define ORBITAL_SIZE  700          /* square canvas for the orbital */
#define PANEL_W       220          /* right-hand UI panel width */
#define WIDTH         (ORBITAL_SIZE + PANEL_W)
#define HEIGHT        ORBITAL_SIZE
#define VIEW_RANGE_BASE  20.0f     /* ±Bohr radii at n=1; scales as n² */

#define FONT_PATH "/System/Library/Fonts/SFNSMono.ttf"
#define FONT_SIZE_LG 18
#define FONT_SIZE_SM 13

/* -------------------------------------------------------------------------
 * Wavefunction math (unchanged from physics — do not modify)
 * ---------------------------------------------------------------------- */

/**
 * @brief  Radial wavefunction R_{nl}(r) for hydrogen (atomic units, a₀=1).
 * @param n  Principal quantum number (≥1).
 * @param l  Angular quantum number (0..n-1).
 * @param r  Radial distance in Bohr radii.
 * @return   R_{nl}(r).
 */
static double radial(int n, int l, double r) {
    double rho     = 2.0 * r / n;
    double norm    = sqrt(pow(2.0/n, 3.0));
    double fac_num = 1.0, fac_den = 1.0;
    for (int k = 1; k <= n-l-1; k++) fac_num *= k;
    for (int k = 1; k <= n+l;   k++) fac_den *= k;
    norm *= sqrt(fac_num / (2.0 * n * fac_den * fac_den * fac_den));
    return norm * exp(-r/n) * pow(rho, l) * gsl_sf_laguerre_n(n-l-1, 2*l+1, rho);
}

/**
 * @brief  Real spherical harmonic Y_l^m at the z=0 plane (θ=π/2, cos θ=0).
 * @param l    Angular quantum number.
 * @param m    Magnetic quantum number (-l..l).
 * @param phi  Azimuthal angle atan2(y,x).
 * @return     Real Y_l^m value.
 */
static double sph_harmonic(int l, int m, double phi) {
    int    am  = abs(m);
    double Plm = gsl_sf_legendre_sphPlm(l, am, 0.0);   /* cos(π/2)=0 */
    if (m == 0) return Plm;
    if (m > 0)  return M_SQRT2 * pow(-1.0, m) * Plm * cos(am * phi);
    return           M_SQRT2 * pow(-1.0, m) * Plm * sin(am * phi);
}

/**
 * @brief  Probability density |ψ_{nlm}(x,y,0)|² at a point in the z=0 plane.
 * @param n, l, m  Quantum numbers.
 * @param x, y     Position in Bohr radii.
 * @return         |ψ|² ≥ 0.
 */
static double psi2(int n, int l, int m, double x, double y) {
    double r = sqrt(x*x + y*y), phi = atan2(y, x);
    double R = radial(n, l, r), Y = sph_harmonic(l, m, phi);
    return R*R * Y*Y;
}

/* -------------------------------------------------------------------------
 * Text / UI helpers
 * ---------------------------------------------------------------------- */

static TTF_Font    *font_lg     = NULL;
static TTF_Font    *font_sm     = NULL;
static SDL_Texture *orbital_tex = NULL;  /* cached orbital, rebuilt only on quantum number change */

/**
 * @brief  Draw a UTF-8 string at (x,y) with the given colour.
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

/**
 * @brief  Draw a labelled button; returns 1 if the mouse click (mx,my) hit it.
 *
 * @param active  Non-zero = highlight as the current selection.
 */
static int draw_button(SDL_Renderer *ren, int x, int y, int w, int h,
                       const char *label, int active,
                       int mx, int my, int clicked) {
    SDL_Rect r = {x, y, w, h};
    if (active) {
        SDL_SetRenderDrawColor(ren, 0, 160, 180, 255);
    } else {
        SDL_SetRenderDrawColor(ren, 50, 55, 70, 255);
    }
    SDL_RenderFillRect(ren, &r);
    SDL_SetRenderDrawColor(ren, 100, 110, 130, 255);
    SDL_RenderDrawRect(ren, &r);

    /* Centre the label */
    int tw, th;
    TTF_SizeUTF8(font_sm, label, &tw, &th);
    draw_text(ren, font_sm, label,
              x + (w - tw)/2, y + (h - th)/2,
              active ? 255 : 200, active ? 255 : 200, active ? 255 : 200);

    if (clicked && mx >= x && mx < x+w && my >= y && my < y+h)
        return 1;
    return 0;
}

/* -------------------------------------------------------------------------
 * Orbital rendering
 * ---------------------------------------------------------------------- */

/**
 * @brief  Recompute the |ψ|² heatmap and cache it in orbital_tex.
 *
 * Rebuilds the texture only when quantum numbers change (the `dirty` flag).
 * Every frame, main() blits orbital_tex to the left side of the window —
 * this keeps both SDL back buffers complete and eliminates flicker.
 *
 * Colour map: black(0) → deep blue → cyan → white(peak).
 *
 * @param ren      Renderer (needed to create the texture).
 * @param n, l, m  Quantum numbers.
 */
static void rebuild_orbital_texture(SDL_Renderer *ren, int n, int l, int m) {
    float *buf  = malloc(ORBITAL_SIZE * HEIGHT * sizeof(float));
    Uint32 *pix = malloc(ORBITAL_SIZE * HEIGHT * sizeof(Uint32));
    float   peak = 1e-30f;
    /* Scale view so the orbital fits: most probable radius ≈ n² Bohr radii.
     * Add 50% margin so the tails are visible too. */
    double view = n * n * 1.5;
    if (view < VIEW_RANGE_BASE) view = VIEW_RANGE_BASE;

    for (int py = 0; py < HEIGHT; py++) {
        for (int px = 0; px < ORBITAL_SIZE; px++) {
            double bx = ((double)px / ORBITAL_SIZE - 0.5) * 2.0 * view;
            double by = ((double)py / HEIGHT       - 0.5) * 2.0 * view;
            float  v  = (float)psi2(n, l, m, bx, by);
            buf[py * ORBITAL_SIZE + px] = v;
            if (v > peak) peak = v;
        }
    }

    for (int py = 0; py < HEIGHT; py++) {
        for (int px = 0; px < ORBITAL_SIZE; px++) {
            float t = buf[py * ORBITAL_SIZE + px] / peak;
            Uint8 r, g, b;
            if (t < 0.3f) {
                float s = t / 0.3f;
                r = 0; g = 0; b = (Uint8)(s * 200);
            } else if (t < 0.6f) {
                float s = (t - 0.3f) / 0.3f;
                r = 0; g = (Uint8)(s * 220); b = 200;
            } else {
                float s = (t - 0.6f) / 0.4f;
                r = (Uint8)(s * 255); g = 220; b = 200;
            }
            pix[py * ORBITAL_SIZE + px] = ((Uint32)0xFF << 24) |
                                          ((Uint32)r   << 16) |
                                          ((Uint32)g   <<  8) |
                                          ((Uint32)b);
        }
    }

    if (orbital_tex) SDL_DestroyTexture(orbital_tex);
    orbital_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STATIC,
                                    ORBITAL_SIZE, HEIGHT);
    SDL_UpdateTexture(orbital_tex, NULL, pix, ORBITAL_SIZE * sizeof(Uint32));

    free(buf);
    free(pix);
}

/* -------------------------------------------------------------------------
 * UI panel
 * ---------------------------------------------------------------------- */

/* Preset list shown as clickable buttons */
static const struct { int n, l, m; const char *label; } PRESETS[] = {
    {1,0, 0, "1s"},
    {2,0, 0, "2s"},
    {2,1, 0, "2p  m=0"},
    {2,1, 1, "2p  m=1"},
    {2,1,-1, "2p  m=-1"},
    {3,0, 0, "3s"},
    {3,1, 0, "3p  m=0"},
    {3,2, 0, "3d  m=0"},
    {3,2, 1, "3d  m=1"},
    {3,2, 2, "3d  m=2"},
    {4,3, 0, "4f  m=0"},
    {4,3, 1, "4f  m=1"},
};
static const int N_PRESETS = (int)(sizeof(PRESETS)/sizeof(PRESETS[0]));

static const char *lname(int l) {
    static const char *names[] = {"s","p","d","f","g","h","i","k"};
    return (l < 8) ? names[l] : "?";
}

/**
 * @brief  Render the right-hand UI panel and handle button clicks.
 *
 * Draws:
 *   - n / l / m labels with +/- arrows
 *   - Explanation of what each quantum number means
 *   - Preset orbital buttons
 *
 * @param ren      Renderer.
 * @param n, l, m  Current quantum numbers (modified via pointers on click).
 * @param mx, my   Mouse position.
 * @param clicked  Non-zero if a mouse button was just released.
 * @return         Non-zero if any value changed (orbital needs redraw).
 */
static int render_panel(SDL_Renderer *ren, int *n, int *l, int *m,
                        int mx, int my, int clicked) {
    int changed = 0;
    int px0 = ORBITAL_SIZE + 8;   /* panel left edge + margin */
    int pw  = PANEL_W - 16;       /* usable panel width */

    /* Panel background */
    SDL_Rect panel = {ORBITAL_SIZE, 0, PANEL_W, HEIGHT};
    SDL_SetRenderDrawColor(ren, 22, 24, 34, 255);
    SDL_RenderFillRect(ren, &panel);
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, ORBITAL_SIZE, 0, ORBITAL_SIZE, HEIGHT);

    int y = 12;

    /* Title + reset button on the same line */
    draw_text(ren, font_lg, "Orbital", px0, y, 200, 220, 255);
    if (draw_button(ren, px0 + pw - 48, y - 2, 48, 22, "Reset", 0, mx, my, clicked)) {
        *n = 1; *l = 0; *m = 0; changed = 1;
    }
    y += 28;

    /* Current orbital name */
    char name_buf[32];
    snprintf(name_buf, sizeof(name_buf), "n=%d  l=%d(%s)  m=%d",
             *n, *l, lname(*l), *m);
    draw_text(ren, font_sm, name_buf, px0, y, 120, 200, 180);
    y += 24;

    /* Separator */
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, px0, y, px0 + pw, y);
    y += 10;

    /* --- n control --- */
    draw_text(ren, font_sm, "n  (energy level)", px0, y, 160, 160, 200);
    y += 18;
    draw_text(ren, font_sm, "higher n = more",  px0, y, 80, 80, 110);
    y += 14;
    draw_text(ren, font_sm, "energy, larger cloud", px0, y, 80, 80, 110);
    y += 20;

    int bw = 36, bh = 26;
    /* n- button */
    if (draw_button(ren, px0, y, bw, bh, "-", 0, mx, my, clicked) && *n > 1) {
        (*n)--; if (*l >= *n) *l = *n-1; if (*m > *l) *m = *l; if (*m < -*l) *m = -*l;
        changed = 1;
    }
    char nval[8]; snprintf(nval, sizeof(nval), "n=%d", *n);
    draw_text(ren, font_lg, nval, px0 + bw + 6, y + 4, 255, 255, 255);
    /* n+ button */
    if (draw_button(ren, px0 + pw - bw, y, bw, bh, "+", 0, mx, my, clicked) && *n < 7) {
        (*n)++; changed = 1;
    }
    y += bh + 14;

    /* --- l control --- */
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, px0, y, px0 + pw, y);
    y += 8;
    draw_text(ren, font_sm, "l  (shape)", px0, y, 160, 160, 200);
    y += 18;
    draw_text(ren, font_sm, "0=s  1=p  2=d  3=f", px0, y, 80, 80, 110);
    y += 20;

    if (draw_button(ren, px0, y, bw, bh, "-", 0, mx, my, clicked) && *l > 0) {
        (*l)--; if (*m > *l) *m = *l; if (*m < -*l) *m = -*l; changed = 1;
    }
    char lval[16]; snprintf(lval, sizeof(lval), "l=%d(%s)", *l, lname(*l));
    draw_text(ren, font_sm, lval, px0 + bw + 6, y + 6, 255, 255, 255);
    if (draw_button(ren, px0 + pw - bw, y, bw, bh, "+", 0, mx, my, clicked) && *l < *n-1) {
        (*l)++; changed = 1;
    }
    y += bh + 14;

    /* --- m control --- */
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, px0, y, px0 + pw, y);
    y += 8;
    draw_text(ren, font_sm, "m  (orientation)", px0, y, 160, 160, 200);
    y += 18;
    draw_text(ren, font_sm, "rotates the orbital", px0, y, 80, 80, 110);
    y += 20;

    if (draw_button(ren, px0, y, bw, bh, "-", 0, mx, my, clicked) && *m > -*l) {
        (*m)--; changed = 1;
    }
    char mval[8]; snprintf(mval, sizeof(mval), "m=%d", *m);
    draw_text(ren, font_lg, mval, px0 + bw + 6, y + 4, 255, 255, 255);
    if (draw_button(ren, px0 + pw - bw, y, bw, bh, "+", 0, mx, my, clicked) && *m < *l) {
        (*m)++; changed = 1;
    }
    y += bh + 14;

    /* --- Preset buttons --- */
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, px0, y, px0 + pw, y);
    y += 8;
    draw_text(ren, font_sm, "Common orbitals:", px0, y, 160, 160, 200);
    y += 20;

    for (int i = 0; i < N_PRESETS; i++) {
        int is_active = (PRESETS[i].n == *n && PRESETS[i].l == *l && PRESETS[i].m == *m);
        if (draw_button(ren, px0, y, pw, 22, PRESETS[i].label, is_active, mx, my, clicked)) {
            *n = PRESETS[i].n; *l = PRESETS[i].l; *m = PRESETS[i].m;
            changed = 1;
        }
        y += 25;
        if (y > HEIGHT - 30) break;
    }

    /* Colour scale legend at bottom */
    y = HEIGHT - 44;
    draw_text(ren, font_sm, "Probability:", px0, y, 120, 120, 150);
    y += 16;
    /* gradient bar */
    for (int i = 0; i < pw; i++) {
        float t = (float)i / pw;
        Uint8 r2, g2, b2;
        if (t < 0.3f) { float s=t/0.3f; r2=0; g2=0; b2=(Uint8)(s*200); }
        else if (t < 0.6f) { float s=(t-0.3f)/0.3f; r2=0; g2=(Uint8)(s*220); b2=200; }
        else { float s=(t-0.6f)/0.4f; r2=(Uint8)(s*255); g2=220; b2=200; }
        SDL_SetRenderDrawColor(ren, r2, g2, b2, 255);
        SDL_RenderDrawLine(ren, px0+i, y, px0+i, y+10);
    }
    draw_text(ren, font_sm, "low", px0, y+12, 80, 80, 110);
    draw_text(ren, font_sm, "high", px0+pw-28, y+12, 200, 200, 200);

    return changed;
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

/**
 * @brief  Program entry point.
 * @return 0 on clean exit.
 */
int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    font_lg = TTF_OpenFont(FONT_PATH, FONT_SIZE_LG);
    font_sm = TTF_OpenFont(FONT_PATH, FONT_SIZE_SM);
    if (!font_lg || !font_sm) {
        /* Fallback font paths */
        font_lg = TTF_OpenFont("/Library/Fonts/Arial.ttf", FONT_SIZE_LG);
        font_sm = TTF_OpenFont("/Library/Fonts/Arial.ttf", FONT_SIZE_SM);
    }

    SDL_Window   *win = SDL_CreateWindow(
        "Hydrogen Orbitals",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int n = 1, l = 0, m = 0;
    int dirty = 1;
    int mx = 0, my = 0, clicked = 0;

    SDL_Event ev;
    while (1) {
        clicked = 0;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) goto done;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) goto done;
            if (ev.type == SDL_MOUSEMOTION) { mx = ev.motion.x; my = ev.motion.y; }
            if (ev.type == SDL_MOUSEBUTTONUP) {
                mx = ev.button.x; my = ev.button.y; clicked = 1;
            }
        }

        /* Rebuild the orbital texture only when quantum numbers change */
        if (dirty) {
            rebuild_orbital_texture(ren, n, l, m);
            dirty = 0;
        }

        /* Blit cached orbital texture every frame (keeps both back buffers complete) */
        SDL_Rect orbital_dst = {0, 0, ORBITAL_SIZE, HEIGHT};
        SDL_RenderCopy(ren, orbital_tex, NULL, &orbital_dst);

        /* Panel redrawn every frame so button highlights stay responsive */
        if (render_panel(ren, &n, &l, &m, mx, my, clicked))
            dirty = 1;

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

done:
    if (orbital_tex) SDL_DestroyTexture(orbital_tex);
    if (font_lg) TTF_CloseFont(font_lg);
    if (font_sm) TTF_CloseFont(font_sm);
    TTF_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
