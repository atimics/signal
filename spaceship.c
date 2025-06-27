#include "cgame.h"

// Initialize spaceship
void init_spaceship(struct Spaceship* ship) {
    ship->position = (Vector3D){0, 0, 0};
    ship->velocity = (Vector3D){0, 0, 0};
    ship->rotation = (Vector3D){0, 0, 0};
    ship->angular_velocity = (Vector3D){0, 0, 0};
    ship->speed = 3.0f;
    ship->rotSpeed = 3.0f;
    ship->throttle = 0.0f;
    ship->shields = 100.0f;
    ship->energy = 100.0f;
    ship->afterburner = false;
    ship->firing = false;
    ship->mesh = &starship_mesh;
}

// Update spaceship physics
void update_spaceship(struct Spaceship* ship, float deltaTime) {
    // Apply throttle to forward velocity
    float thrust = ship->throttle * ship->speed;
    if (ship->afterburner && ship->energy > 0) {
        thrust *= 2.0f; // Afterburner doubles thrust
        ship->energy -= 30.0f * deltaTime; // Drain energy
        if (ship->energy < 0) ship->energy = 0;
    } else {
        // Regenerate energy when not using afterburner
        ship->energy += 20.0f * deltaTime;
        if (ship->energy > 100.0f) ship->energy = 100.0f;
    }
    
    // Apply thrust in forward direction (Z-axis)
    ship->velocity.z += thrust * deltaTime;
    
    // Apply angular velocity to rotation
    ship->rotation = vector3d_add(ship->rotation, 
                                vector3d_multiply(ship->angular_velocity, deltaTime));
    
    // Apply velocity to position
    ship->position = vector3d_add(ship->position, 
                                vector3d_multiply(ship->velocity, deltaTime));
    
    // Apply damping to linear velocity (space friction)
    ship->velocity = vector3d_multiply(ship->velocity, 0.985f);
    
    // Apply damping to angular velocity (rotational friction)
    ship->angular_velocity = vector3d_multiply(ship->angular_velocity, 0.95f);
}
