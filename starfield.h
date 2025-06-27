#ifndef STARFIELD_H
#define STARFIELD_H

#include "math3d.h"

// Forward declaration to avoid circular dependency
struct Game;

#define MAX_STARS 200

// Star field for background
struct Star {
    Vector3D position;
    float brightness;
};

// Star field functions
void init_stars(struct Game* game);
void update_stars(struct Game* game, float deltaTime);

#endif
