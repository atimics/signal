#include "cgame.h"

// Generate random float between min and max
static float random_float(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

// Initialize particle system
void init_particles(struct Game* game) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        game->particles[i].life = 0;
        game->particles[i].maxLife = 0;
    }
}

// Add engine particles
void add_engine_particle(struct Game* game) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game->particles[i].life <= 0) {
            // Create particle behind the spaceship
            game->particles[i].position = game->player.position;
            game->particles[i].position.z -= 5; // Behind spaceship
            game->particles[i].position.x += random_float(-2, 2);
            game->particles[i].position.y += random_float(-2, 2);
            
            game->particles[i].velocity.x = random_float(-1, 1);
            game->particles[i].velocity.y = random_float(-1, 1);
            game->particles[i].velocity.z = -random_float(5, 10); // Moving backward
            
            game->particles[i].maxLife = random_float(0.5f, 1.5f);
            game->particles[i].life = game->particles[i].maxLife;
            break;
        }
    }
}

// Update particle system
void update_particles_system(struct Game* game, float deltaTime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game->particles[i].life > 0) {
            game->particles[i].life -= deltaTime;
            game->particles[i].position = vector3d_add(
                game->particles[i].position,
                vector3d_multiply(game->particles[i].velocity, deltaTime)
            );
        }
    }
}
