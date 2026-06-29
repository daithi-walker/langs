/**
 * @file   main.c
 * @brief  N-body gravitational simulation visualizer with SDL2 UI.
 *
 * N=1000 point masses under pairwise Newtonian gravity.
 * The G slider scales gravity far above physical reality.
 *
 * Init modes:
 *   Galaxy  — rotating disk; watch spiral arms form and wind
 *   Static  — zero velocity; pure gravitational collapse
 *   Heavy   — one massive central body + light orbiters; solar system seed
 *
 * Mass modes:
 *   Equal   — all mass = 1
 *   Random  — log-uniform spread 0.1–10; heavy bodies shown larger
 *   Massive — one body at 100× mass in the center
 *
 * Body colour = speed: blue (slow) → cyan → orange → white (fast).
 * Dot size scales with mass so you can track heavy bodies.
 *
 * Keys: Space=pause/resume, R=reset, Escape=quit.
 *
 * @section deps
 *   SDL2:     brew install sdl2
 *   SDL2_ttf: brew install sdl2_ttf
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Layout
 * ---------------------------------------------------------------------- */

#define N_MAX     5000   /* maximum bodies; runtime n_bodies controls actual count */
#define SIM_W     800
#define SIM_H     800
#define PANEL_W   230
#define WIDTH     (SIM_W + PANEL_W)
#define HEIGHT    SIM_H

#define FONT_PATH    "/System/Library/Fonts/SFNSMono.ttf"
#define FONT_SIZE_LG 18
#define FONT_SIZE_SM 13

/* -------------------------------------------------------------------------
 * Physics constants
 * ---------------------------------------------------------------------- */

#define DT_BASE    0.0002
#define SOFTENING  0.02
#define G_MIN      0.05
#define G_MAX      50.0
#define G_DEFAULT  1.0
#define DISK_R     0.8
#define v_frac_DEFAULT 0.55

/* Hernquist dark matter halo: M(r) = M_halo * r² / (r + a)²
 * a = scale radius; enclosed mass → M_halo as r → ∞             */
#define HALO_MASS  800.0   /* total halo mass (relative to body masses) */
#define HALO_A     0.6     /* scale radius in world units */

/* -------------------------------------------------------------------------
 * Init / mass modes
 * ---------------------------------------------------------------------- */

typedef enum { INIT_GALAXY = 0, INIT_STATIC = 1, INIT_HEAVY = 2 } InitMode;
typedef enum { MASS_EQUAL  = 0, MASS_RANDOM = 1, MASS_MASSIVE = 2 } MassMode;

/* -------------------------------------------------------------------------
 * Global state
 * ---------------------------------------------------------------------- */

static double bx[N_MAX], by[N_MAX], bz[N_MAX];
static double bvx[N_MAX], bvy[N_MAX], bvz[N_MAX];
static double bmass[N_MAX];

static int    n_bodies    = 1000;
static double v_frac      = v_frac_DEFAULT;  /* 0=pure collapse, 1=stable orbit */
static double g_val       = G_DEFAULT;
static double slider_t    = 0.0;
static double view_radius = 1.5;
static int    paused      = 0;
static int    spf         = 5;
static long   total_steps = 0;

static InitMode init_mode   = INIT_GALAXY;
static MassMode mass_mode   = MASS_EQUAL;
static int      halo_on     = 0;   /* dark matter halo background potential */
static int      has_bh      = 0;   /* index 0 is the black hole when active */
static int      accretion   = 0;   /* whether BH eats nearby bodies */
static int      pin_bh      = 0;   /* freeze BH position each step */
#define BH_MASS        5000.0
#define BH_ACCRETE_R   0.04   /* world-units; bodies closer than this are eaten */

static SDL_Rect slider_rect   = {0, 0, 0, 0};   /* G slider */
static SDL_Rect vfrac_rect    = {0, 0, 0, 0};   /* v_frac slider */

static TTF_Font *font_lg = NULL;
static TTF_Font *font_sm = NULL;

/* -------------------------------------------------------------------------
 * LCG
 * ---------------------------------------------------------------------- */

static unsigned long long lcg_state;

static double lcg_01(void) {
    lcg_state = lcg_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)(lcg_state >> 33) / (double)(1ULL << 31);
}

/* returns a value in [lo, hi] */
static double lcg_range(double lo, double hi) { return lo + lcg_01() * (hi - lo); }

/* log-uniform in [lo, hi] */
static double lcg_log(double lo, double hi) {
    return exp(lcg_range(log(lo), log(hi)));
}

/* -------------------------------------------------------------------------
 * Mass assignment
 * ---------------------------------------------------------------------- */

static void assign_masses(void) {
    lcg_state = 99991;   /* separate seed so mass doesn't affect positions */
    switch (mass_mode) {
    case MASS_EQUAL:
        for (int i = 0; i < n_bodies; i++) bmass[i] = 1.0;
        break;
    case MASS_RANDOM:
        for (int i = 0; i < n_bodies; i++) bmass[i] = lcg_log(0.1, 10.0);
        break;
    case MASS_MASSIVE:
        bmass[0] = 100.0;
        for (int i = 1; i < n_bodies; i++) bmass[i] = 1.0;
        break;
    }
}

/* -------------------------------------------------------------------------
 * Simulation init
 * ---------------------------------------------------------------------- */

static void reset_sim(void) {
    assign_masses();
    lcg_state = 12345;
    total_steps = 0;
    has_bh      = 0;
    pin_bh      = 0;
    accretion   = 0;

    switch (init_mode) {

    case INIT_GALAXY: {
        /* Rotating disk — circular velocities give a flat galaxy */
        double total_mass = 0.0;
        for (int i = 0; i < n_bodies; i++) total_mass += bmass[i];

        for (int i = 0; i < n_bodies; i++) {
            double r_norm = sqrt(lcg_01());
            double theta  = lcg_01() * 2.0 * M_PI;
            double r      = r_norm * DISK_R;

            bx[i]  = r * cos(theta);
            by[i]  = r * sin(theta);
            bz[i]  = lcg_range(-0.02, 0.02);

            double M_enc  = total_mass * r_norm * r_norm;
            /* Add halo circular speed: v²_halo = G*M_halo*r / (r+a)² */
            double halo_contrib = halo_on ? g_val * HALO_MASS * r / ((r + HALO_A) * (r + HALO_A)) : 0.0;
            double v_circ = (r > 0.001) ? sqrt(g_val * M_enc / r + halo_contrib) : 0.0;
            bvx[i] = -sin(theta) * v_circ * v_frac;
            bvy[i] =  cos(theta) * v_circ * v_frac;
            bvz[i] = 0.0;
        }
        /* Massive mode: heavy body in the center */
        if (mass_mode == MASS_MASSIVE) {
            bx[0] = 0.0; by[0] = 0.0; bz[0] = 0.0;
            bvx[0] = 0.0; bvy[0] = 0.0; bvz[0] = 0.0;
        }
        break;
    }

    case INIT_STATIC: {
        /* Random positions, zero velocity — pure collapse */
        for (int i = 0; i < n_bodies; i++) {
            double r_norm = sqrt(lcg_01());
            double theta  = lcg_01() * 2.0 * M_PI;
            double r      = r_norm * DISK_R;
            bx[i]  = r * cos(theta);
            by[i]  = r * sin(theta);
            bz[i]  = lcg_range(-0.3, 0.3);
            bvx[i] = bvy[i] = bvz[i] = 0.0;
        }
        if (mass_mode == MASS_MASSIVE) {
            bx[0] = 0.0; by[0] = 0.0; bz[0] = 0.0;
        }
        break;
    }

    case INIT_HEAVY: {
        /* Heavy central body + light bodies in roughly circular orbits */
        /* Make sure massive mode / equal both work — heavy body is always body 0 */
        double M_center = (mass_mode == MASS_MASSIVE) ? bmass[0] : 100.0;
        if (mass_mode != MASS_MASSIVE) {
            bmass[0] = M_center;
            for (int i = 1; i < n_bodies; i++) {
                if (mass_mode == MASS_RANDOM) bmass[i] = lcg_log(0.1, 2.0);
                else                          bmass[i] = 1.0;
            }
        }
        bx[0] = 0.0; by[0] = 0.0; bz[0] = 0.0;
        bvx[0] = bvy[0] = bvz[0] = 0.0;

        for (int i = 1; i < n_bodies; i++) {
            double r_norm = sqrt(lcg_01()) * 0.9 + 0.05;
            double theta  = lcg_01() * 2.0 * M_PI;
            double r      = r_norm * DISK_R;
            bx[i]  = r * cos(theta);
            by[i]  = r * sin(theta);
            bz[i]  = lcg_range(-0.02, 0.02);

            double v_circ = sqrt(g_val * M_center / r);
            /* small random eccentricity */
            double jitter = lcg_range(0.85, 1.05);
            bvx[i] = -sin(theta) * v_circ * jitter;
            bvy[i] =  cos(theta) * v_circ * jitter;
            bvz[i] = lcg_range(-0.01, 0.01) * v_circ;
        }
        break;
    }
    }
}

/* Drop a supermassive body at the center of mass, mid-simulation. */
static void drop_bh(void) {
    if (has_bh) return;
    /* Compute current centre of mass so the BH lands there */
    double cx = 0.0, cy = 0.0, total_m = 0.0;
    for (int i = 0; i < n_bodies; i++) {
        cx += bx[i] * bmass[i]; cy += by[i] * bmass[i]; total_m += bmass[i];
    }
    cx /= total_m; cy /= total_m;
    /* Shift body 0 to be the BH; preserve it in the loop */
    bx[0]  = cx;  by[0]  = cy;  bz[0]  = 0.0;
    bvx[0] = 0.0; bvy[0] = 0.0; bvz[0] = 0.0;
    bmass[0] = BH_MASS;
    has_bh = 1;
}

/* -------------------------------------------------------------------------
 * Physics step
 * ---------------------------------------------------------------------- */

static void step_sim(double G, double dt) {
    for (int i = 0; i < n_bodies; i++) {
        double ax = 0.0, ay = 0.0, az = 0.0;
        for (int j = 0; j < n_bodies; j++) {
            if (i == j) continue;
            double dx = bx[j] - bx[i];
            double dy = by[j] - by[i];
            double dz = bz[j] - bz[i];
            double r2 = dx*dx + dy*dy + dz*dz + SOFTENING*SOFTENING;
            double r3 = r2 * sqrt(r2);
            double gm = G * bmass[j];
            ax += gm * dx / r3;
            ay += gm * dy / r3;
            az += gm * dz / r3;
        }
        bvx[i] += ax * dt;
        bvy[i] += ay * dt;
        bvz[i] += az * dt;
    }
    /* Dark matter halo: analytic Hernquist potential, no extra particles */
    if (halo_on) {
        for (int i = 0; i < n_bodies; i++) {
            double r = sqrt(bx[i]*bx[i] + by[i]*by[i] + bz[i]*bz[i]);
            if (r < 1e-6) continue;
            double ra   = r + HALO_A;
            double amag = G * HALO_MASS / (ra * ra);  /* |a| = GM(r)/r² for Hernquist */
            bvx[i] -= amag * (bx[i] / r) * dt;
            bvy[i] -= amag * (by[i] / r) * dt;
            bvz[i] -= amag * (bz[i] / r) * dt;
        }
    }

    if (has_bh && pin_bh) { bvx[0] = 0.0; bvy[0] = 0.0; bvz[0] = 0.0; }

    for (int i = 0; i < n_bodies; i++) {
        bx[i] += bvx[i] * dt;
        by[i] += bvy[i] * dt;
        bz[i] += bvz[i] * dt;
    }
    total_steps++;

    /* Accretion: swallow bodies that cross inside the BH radius */
    if (has_bh && accretion) {
        double r2_acc = BH_ACCRETE_R * BH_ACCRETE_R;
        int i = 1;
        while (i < n_bodies) {
            double dx = bx[i] - bx[0];
            double dy = by[i] - by[0];
            double dz = bz[i] - bz[0];
            if (dx*dx + dy*dy + dz*dz < r2_acc) {
                bmass[0] += bmass[i];          /* BH eats the body's mass */
                /* Swap with last active body and shrink the array */
                int last = n_bodies - 1;
                bx[i]    = bx[last];    by[i]    = by[last];    bz[i]    = bz[last];
                bvx[i]   = bvx[last];   bvy[i]   = bvy[last];   bvz[i]   = bvz[last];
                bmass[i] = bmass[last];
                n_bodies--;
                /* don't increment i — recheck the swapped body */
            } else {
                i++;
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * Colour mapping: speed → colour
 * ---------------------------------------------------------------------- */

static void speed_color(double t, Uint8 *r, Uint8 *g, Uint8 *b) {
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    if (t < 0.4) {
        double s = t / 0.4;
        *r = 20; *g = (Uint8)(20 + s * 80); *b = (Uint8)(120 + s * 135);
    } else if (t < 0.75) {
        double s = (t - 0.4) / 0.35;
        *r = (Uint8)(s * 255); *g = (Uint8)(100 + s * 130); *b = (Uint8)(255 * (1.0 - s));
    } else {
        double s = (t - 0.75) / 0.25;
        *r = 255; *g = (Uint8)(230 + s * 25); *b = (Uint8)(s * 255);
    }
}

/* -------------------------------------------------------------------------
 * Simulation render
 * ---------------------------------------------------------------------- */

static void render_sim(SDL_Renderer *ren) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_Rect bg = {0, 0, SIM_W, SIM_H};
    SDL_RenderFillRect(ren, &bg);

    /* Compute max speed and max mass for normalisation */
    double max_spd  = 1e-10;
    double max_mass = 1e-10;
    for (int i = 0; i < n_bodies; i++) {
        double spd = sqrt(bvx[i]*bvx[i] + bvy[i]*bvy[i] + bvz[i]*bvz[i]);
        if (spd  > max_spd)  max_spd  = spd;
        if (bmass[i] > max_mass) max_mass = bmass[i];
    }

    for (int i = 0; i < n_bodies; i++) {
        int px = (int)((bx[i] / view_radius + 1.0) * 0.5 * SIM_W);
        int py = (int)((-by[i] / view_radius + 1.0) * 0.5 * SIM_H);
        if (px < 2 || px >= SIM_W-2 || py < 2 || py >= SIM_H-2) continue;

        double spd = sqrt(bvx[i]*bvx[i] + bvy[i]*bvy[i] + bvz[i]*bvz[i]);
        Uint8 r, g, b;
        speed_color(spd / max_spd, &r, &g, &b);

        /* dot radius scales logarithmically with mass */
        int rad = 1;
        if (max_mass > 2.0) {
            double mrel = bmass[i] / max_mass;
            rad = (int)(1.0 + 3.0 * log1p(mrel * 9.0) / log(10.0));
        }

        SDL_SetRenderDrawColor(ren, r, g, b, 255);
        SDL_Rect dot = {px - rad, py - rad, 2*rad+1, 2*rad+1};
        SDL_RenderFillRect(ren, &dot);
    }

    /* Draw BH — disk grows linearly with accreted mass, occludes stars behind it */
    if (has_bh) {
        int px = (int)((bx[0] / view_radius + 1.0) * 0.5 * SIM_W);
        int py = (int)((-by[0] / view_radius + 1.0) * 0.5 * SIM_H);

        /* Radius in pixels: starts at 5px for BH_MASS, grows 1px per 200 eaten mass */
        int disk_r = 5 + (int)((bmass[0] - BH_MASS) / 200.0);
        if (disk_r > 60) disk_r = 60;

        if (px >= disk_r+4 && px < SIM_W-disk_r-4 && py >= disk_r+4 && py < SIM_H-disk_r-4) {
            /* Outer glow — faint orange halo one step wider */
            SDL_SetRenderDrawColor(ren, 120, 60, 10, 255);
            for (int r2 = disk_r+3; r2 <= disk_r+5; r2++) {
                SDL_Rect ring = {px-r2, py-r2, 2*r2+1, 2*r2+1};
                SDL_RenderDrawRect(ren, &ring);
            }
            /* Bright orange accretion ring */
            SDL_SetRenderDrawColor(ren, 255, 160, 40, 255);
            for (int r2 = disk_r+1; r2 <= disk_r+2; r2++) {
                SDL_Rect ring = {px-r2, py-r2, 2*r2+1, 2*r2+1};
                SDL_RenderDrawRect(ren, &ring);
            }
            /* Black disk — swallows all star pixels beneath it */
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_Rect disk = {px - disk_r, py - disk_r, 2*disk_r+1, 2*disk_r+1};
            SDL_RenderFillRect(ren, &disk);
            /* Bright white centre point */
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_Rect core = {px-1, py-1, 3, 3};
            SDL_RenderFillRect(ren, &core);
        }
    }
}

/* -------------------------------------------------------------------------
 * UI helpers
 * ---------------------------------------------------------------------- */

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

static int draw_button(SDL_Renderer *ren, int x, int y, int w, int h,
                       const char *label, int active,
                       int mx, int my, int clicked) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(ren, active ? 0 : 50, active ? 160 : 55, active ? 180 : 70, 255);
    SDL_RenderFillRect(ren, &rect);
    SDL_SetRenderDrawColor(ren, 100, 110, 130, 255);
    SDL_RenderDrawRect(ren, &rect);

    int tw, th;
    TTF_SizeUTF8(font_sm, label, &tw, &th);
    draw_text(ren, font_sm, label,
              x + (w-tw)/2, y + (h-th)/2,
              active ? 255 : 200, active ? 255 : 200, active ? 255 : 200);

    return clicked && mx >= x && mx < x+w && my >= y && my < y+h;
}

static void draw_slider(SDL_Renderer *ren, int x, int y, int w, int h,
                        double t, SDL_Rect *out_rect) {
    int ty = y + (h-6)/2;
    SDL_Rect track = {x, ty, w, 6};
    SDL_SetRenderDrawColor(ren, 40, 45, 60, 255);
    SDL_RenderFillRect(ren, &track);
    SDL_SetRenderDrawColor(ren, 80, 90, 110, 255);
    SDL_RenderDrawRect(ren, &track);

    int fill_w = (int)(t * w);
    if (fill_w > 0) {
        SDL_Rect fill = {x, ty, fill_w, 6};
        SDL_SetRenderDrawColor(ren, 0, 160, 180, 255);
        SDL_RenderFillRect(ren, &fill);
    }
    int hx = x + (int)(t * w) - h/2;
    SDL_Rect handle = {hx, y, h, h};
    SDL_SetRenderDrawColor(ren, 0, 200, 220, 255);
    SDL_RenderFillRect(ren, &handle);
    SDL_SetRenderDrawColor(ren, 200, 240, 255, 255);
    SDL_RenderDrawRect(ren, &handle);

    *out_rect = (SDL_Rect){x, y, w, h};
}

static void sep(SDL_Renderer *ren, int px0, int pw, int y) {
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, px0, y, px0+pw, y);
}

/* -------------------------------------------------------------------------
 * Panel
 * ---------------------------------------------------------------------- */

static void render_panel(SDL_Renderer *ren, int mx, int my, int clicked) {
    int px0 = SIM_W + 8;
    int pw  = PANEL_W - 16;

    SDL_Rect panel = {SIM_W, 0, PANEL_W, HEIGHT};
    SDL_SetRenderDrawColor(ren, 22, 24, 34, 255);
    SDL_RenderFillRect(ren, &panel);
    SDL_SetRenderDrawColor(ren, 55, 60, 80, 255);
    SDL_RenderDrawLine(ren, SIM_W, 0, SIM_W, HEIGHT);

    int y = 12;
    int tw, th;

    /* Title + Drop BH + Reset */
    draw_text(ren, font_lg, "N-Body", px0, y, 200, 220, 255);
    if (draw_button(ren, px0 + pw - 52, y - 2, 52, 22, "Reset", 0, mx, my, clicked))
        reset_sim();
    if (draw_button(ren, px0 + pw - 110, y - 2, 54, 22,
                    has_bh ? "BH on" : "Drop BH", has_bh, mx, my, clicked) && !has_bh)
        drop_bh();
    y += 30;

    /* BH options — only shown when BH is active */
    if (has_bh) {
        if (draw_button(ren, px0, y, (pw-4)/2, 22,
                        pin_bh ? "Pinned" : "Pin BH", pin_bh, mx, my, clicked))
            pin_bh = !pin_bh;
        if (draw_button(ren, px0 + (pw-4)/2 + 4, y, (pw-4)/2, 22,
                        accretion ? "Eating" : "Accrete", accretion, mx, my, clicked))
            accretion = !accretion;
        y += 30;
    }

    /* Pause / Resume */
    if (draw_button(ren, px0, y, pw, 26, paused ? "Resume" : "Pause", paused, mx, my, clicked))
        paused = !paused;
    y += 34;

    /* ---- N (body count) ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Bodies N:", px0, y, 160, 160, 200);
    y += 18;
    static const int n_opts[] = {100, 500, 1000, 2000, 5000};
    int bw5 = (pw - 8) / 5;
    for (int i = 0; i < 5; i++) {
        char s[8]; snprintf(s, sizeof(s), "%d", n_opts[i]);
        if (draw_button(ren, px0 + i*(bw5+2), y, bw5, 24, s, n_bodies == n_opts[i], mx, my, clicked)) {
            n_bodies = n_opts[i];
            reset_sim();
        }
    }
    y += 32;

    /* ---- Init mode ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Init:", px0, y, 160, 160, 200);
    y += 18;
    static const struct { InitMode mode; const char *label; } INITS[] = {
        {INIT_GALAXY, "Galaxy"}, {INIT_STATIC, "Static"}, {INIT_HEAVY, "Solar Sys"},
    };
    int bw3 = (pw - 4) / 3;
    for (int i = 0; i < 3; i++) {
        if (draw_button(ren, px0 + i*(bw3+2), y, bw3, 24,
                        INITS[i].label, init_mode == INITS[i].mode, mx, my, clicked)) {
            init_mode = INITS[i].mode;
            reset_sim();
        }
    }
    y += 26;

    /* Mode hint */
    const char *hints[] = {
        "Rotating disk",
        "Zero vel - pure collapse",
        "Sun + orbiting bodies",
    };
    draw_text(ren, font_sm, hints[init_mode], px0, y, 70, 90, 130);
    y += 20;

    /* Dark matter halo toggle */
    if (draw_button(ren, px0, y, pw, 22,
                    halo_on ? "Dark Halo: ON" : "Dark Halo: OFF", halo_on, mx, my, clicked)) {
        halo_on = !halo_on;
        reset_sim();
    }
    y += 28;

    /* ---- Mass mode ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Mass:", px0, y, 160, 160, 200);
    y += 18;
    static const struct { MassMode mode; const char *label; } MASSES[] = {
        {MASS_EQUAL,   "Equal"},
        {MASS_RANDOM,  "Random"},
        {MASS_MASSIVE, "Massive"},
    };
    for (int i = 0; i < 3; i++) {
        if (draw_button(ren, px0 + i*(bw3+2), y, bw3, 24,
                        MASSES[i].label, mass_mode == MASSES[i].mode, mx, my, clicked)) {
            mass_mode = MASSES[i].mode;
            reset_sim();
        }
    }
    y += 26;
    const char *mhints[] = {
        "All m=1",
        "0.1-10  size=mass",
        "One body 100\xc3\x97",
    };
    draw_text(ren, font_sm, mhints[mass_mode], px0, y, 70, 90, 130);
    y += 22;

    /* ---- Steps / frame ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Steps/frame:", px0, y, 160, 160, 200);
    y += 18;
    static const int spf_opts[] = {1, 5, 10, 20};
    int bw4 = (pw - 6) / 4;
    for (int i = 0; i < 4; i++) {
        char s[4]; snprintf(s, sizeof(s), "%d", spf_opts[i]);
        if (draw_button(ren, px0 + i*(bw4+2), y, bw4, 24, s, spf == spf_opts[i], mx, my, clicked))
            spf = spf_opts[i];
    }
    y += 32;

    /* ---- G slider ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Gravity  G", px0, y, 160, 160, 200);
    char gstr[20]; snprintf(gstr, sizeof(gstr), "%.3f", g_val);
    TTF_SizeUTF8(font_sm, gstr, &tw, &th);
    draw_text(ren, font_sm, gstr, px0 + pw - tw, y, 255, 220, 100);
    y += 20;

    draw_slider(ren, px0, y, pw, 18, slider_t, &slider_rect);
    y += 24;
    draw_text(ren, font_sm, "0.05", px0, y, 60, 70, 90);
    TTF_SizeUTF8(font_sm, "50", &tw, &th);
    draw_text(ren, font_sm, "50", px0+pw-tw, y, 60, 70, 90);
    y += 22;

    /* ---- v_frac slider (Galaxy / Solar Sys only) ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Orbit fraction", px0, y, 160, 160, 200);
    char vstr[16]; snprintf(vstr, sizeof(vstr), "%.2f", v_frac);
    TTF_SizeUTF8(font_sm, vstr, &tw, &th);
    draw_text(ren, font_sm, vstr, px0+pw-tw, y, 180, 255, 180);
    y += 20;
    draw_slider(ren, px0, y, pw, 18, v_frac, &vfrac_rect);
    y += 24;
    draw_text(ren, font_sm, "0=collapse", px0, y, 60, 70, 90);
    TTF_SizeUTF8(font_sm, "1=orbit", &tw, &th);
    draw_text(ren, font_sm, "1=orbit", px0+pw-tw, y, 60, 70, 90);
    y += 22;

    /* ---- Zoom ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Zoom", px0, y, 160, 160, 200);
    y += 18;
    int zbw = 32;
    if (draw_button(ren, px0, y, zbw, 26, "-", 0, mx, my, clicked) && view_radius < 8.0)
        view_radius *= 1.4;
    char zstr[16]; snprintf(zstr, sizeof(zstr), "+-%.2f", view_radius);
    draw_text(ren, font_sm, zstr, px0 + zbw + 6, y + 6, 200, 200, 200);
    if (draw_button(ren, px0+pw-zbw, y, zbw, 26, "+", 0, mx, my, clicked) && view_radius > 0.05)
        view_radius /= 1.4;
    y += 34;

    /* ---- Status ---- */
    sep(ren, px0, pw, y); y += 8;
    char step_s[32];
    snprintf(step_s, sizeof(step_s), "Step  %ld", total_steps);
    draw_text(ren, font_sm, step_s, px0, y, 100, 120, 180); y += 18;
    snprintf(step_s, sizeof(step_s), "N     %d", has_bh ? n_bodies - 1 : n_bodies);
    draw_text(ren, font_sm, step_s, px0, y, 100, 120, 180); y += 18;
    if (has_bh) {
        snprintf(step_s, sizeof(step_s), "BH M  %.0f", bmass[0]);
        draw_text(ren, font_sm, step_s, px0, y, 255, 180, 60); y += 18;
    }
    double dt_disp = DT_BASE / sqrt(g_val / G_DEFAULT > 1.0 ? g_val / G_DEFAULT : 1.0);
    snprintf(step_s, sizeof(step_s), "dt    %.5f", dt_disp);
    draw_text(ren, font_sm, step_s, px0, y, 60, 70, 100); y += 18;
    snprintf(step_s, sizeof(step_s), "soft  %.2f", SOFTENING);
    draw_text(ren, font_sm, step_s, px0, y, 60, 70, 100); y += 26;

    /* ---- Colour legend ---- */
    sep(ren, px0, pw, y); y += 8;
    draw_text(ren, font_sm, "Colour = speed", px0, y, 160, 160, 200); y += 18;
    for (int i = 0; i < pw; i++) {
        Uint8 r2, g2, b2;
        speed_color((double)i / pw, &r2, &g2, &b2);
        SDL_SetRenderDrawColor(ren, r2, g2, b2, 255);
        SDL_RenderDrawLine(ren, px0+i, y, px0+i, y+10);
    }
    y += 12;
    draw_text(ren, font_sm, "slow", px0, y, 60, 70, 130);
    TTF_SizeUTF8(font_sm, "fast", &tw, &th);
    draw_text(ren, font_sm, "fast", px0+pw-tw, y, 220, 220, 220);
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void) {
    slider_t = log10(G_DEFAULT / G_MIN) / log10(G_MAX / G_MIN);

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    font_lg = TTF_OpenFont(FONT_PATH, FONT_SIZE_LG);
    font_sm = TTF_OpenFont(FONT_PATH, FONT_SIZE_SM);
    if (!font_lg || !font_sm) {
        font_lg = TTF_OpenFont("/Library/Fonts/Arial.ttf", FONT_SIZE_LG);
        font_sm = TTF_OpenFont("/Library/Fonts/Arial.ttf", FONT_SIZE_SM);
    }

    SDL_Window   *win = SDL_CreateWindow(
        "N-Body Gravity — drag G slider, try all modes",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    reset_sim();

    int mx = 0, my = 0, clicked = 0, slider_dragging = 0, vfrac_dragging = 0;

    SDL_Event ev;
    while (1) {
        clicked = 0;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: goto done;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym) {
                case SDLK_ESCAPE: goto done;
                case SDLK_SPACE:  paused = !paused; break;
                case SDLK_r:      reset_sim(); break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                mx = ev.button.x; my = ev.button.y;
                if (slider_rect.w > 0 &&
                    mx >= slider_rect.x && mx < slider_rect.x + slider_rect.w &&
                    my >= slider_rect.y - 4 && my < slider_rect.y + slider_rect.h + 4) {
                    slider_dragging = 1;
                    slider_t = (double)(mx - slider_rect.x) / slider_rect.w;
                    if (slider_t < 0.0) slider_t = 0.0;
                    if (slider_t > 1.0) slider_t = 1.0;
                    g_val = G_MIN * pow(G_MAX / G_MIN, slider_t);
                } else if (vfrac_rect.w > 0 &&
                    mx >= vfrac_rect.x && mx < vfrac_rect.x + vfrac_rect.w &&
                    my >= vfrac_rect.y - 4 && my < vfrac_rect.y + vfrac_rect.h + 4) {
                    vfrac_dragging = 1;
                    v_frac = (double)(mx - vfrac_rect.x) / vfrac_rect.w;
                    if (v_frac < 0.0) v_frac = 0.0;
                    if (v_frac > 1.0) v_frac = 1.0;
                    reset_sim();
                }
                break;
            case SDL_MOUSEMOTION:
                mx = ev.motion.x; my = ev.motion.y;
                if (slider_dragging) {
                    slider_t = (double)(mx - slider_rect.x) / slider_rect.w;
                    if (slider_t < 0.0) slider_t = 0.0;
                    if (slider_t > 1.0) slider_t = 1.0;
                    g_val = G_MIN * pow(G_MAX / G_MIN, slider_t);
                } else if (vfrac_dragging) {
                    v_frac = (double)(mx - vfrac_rect.x) / vfrac_rect.w;
                    if (v_frac < 0.0) v_frac = 0.0;
                    if (v_frac > 1.0) v_frac = 1.0;
                    reset_sim();
                }
                break;
            case SDL_MOUSEBUTTONUP:
                mx = ev.button.x; my = ev.button.y;
                slider_dragging = 0;
                vfrac_dragging = 0;
                clicked = 1;
                break;
            }
        }

        /* Scale dt down with G so high gravity doesn't cause numerical blow-up.
         * Force scales as G, so the orbital period scales as 1/sqrt(G).
         * Keeping dt * sqrt(G) constant preserves orbit resolution at any G. */
        double dt_eff = DT_BASE / sqrt(g_val / G_DEFAULT > 1.0 ? g_val / G_DEFAULT : 1.0);
        if (!paused)
            for (int s = 0; s < spf; s++) step_sim(g_val, dt_eff);

        render_sim(ren);
        render_panel(ren, mx, my, clicked);
        SDL_RenderPresent(ren);
    }

done:
    if (font_lg) TTF_CloseFont(font_lg);
    if (font_sm) TTF_CloseFont(font_sm);
    TTF_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
