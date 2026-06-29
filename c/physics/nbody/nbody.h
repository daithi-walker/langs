#pragma once

#define N_BODIES 1000

typedef struct { double x, y, z, vx, vy, vz, mass; } Body;

void nbody_init(Body *bodies, int n);
void nbody_step(Body *bodies, int n, double dt);
