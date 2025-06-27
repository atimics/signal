#ifndef SPACESHIP_H
#define SPACESHIP_H

#include <stdbool.h>
#include "math3d.h"
#include "mesh.h"

// Forward declaration
struct Universe;

// 3D Spaceship structure
struct Spaceship {
    Vector3D position;
    Vector3D velocity;
    Vector3D rotation;
    Vector3D angular_velocity;
    float speed;
    float rotSpeed;
    float throttle;
    float shields;
    float energy;
    bool afterburner;
    bool firing;
    struct Mesh* mesh;
    
    // Collision properties for spaceship
    float collision_radius;
    bool collision_enabled;
};

// Spaceship functions
void init_spaceship(struct Spaceship* ship);
void update_spaceship(struct Spaceship* ship, float deltaTime);
void update_spaceship_with_universe(struct Spaceship* ship, struct Universe* universe, float deltaTime);

#endif

