#ifndef PARTICLES_H
#define PARTICLES_H

#include "math3d.h"

// Forward declaration to avoid circular dependency
struct Game;

#define MAX_PARTICLES 100

// Particle system for engine trail
struct Particle {
    Vector3D position;
    Vector3D velocity;
    float life;
    float maxLife;
};

// Particle functions
void init_particles(struct Game* game);
void add_engine_particle(struct Game* game);
void update_particles_system(struct Game* game, float deltaTime);

#endif
