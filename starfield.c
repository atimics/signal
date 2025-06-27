#include "cgame.h"

// Generate random float between min and max
static float random_float(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

// Initialize star field
void init_stars(struct Game* game) {
    for (int i = 0; i < MAX_STARS; i++) {
        game->stars[i].position.x = random_float(-1000, 1000);
        game->stars[i].position.y = random_float(-1000, 1000);
        game->stars[i].position.z = random_float(10, 2000);
        game->stars[i].brightness = random_float(0.3f, 1.0f);
    }
}

// Update stars for parallax effect
void update_stars(struct Game* game, float deltaTime) {
    for (int i = 0; i < MAX_STARS; i++) {
        game->stars[i].position.z -= game->player.velocity.z * 0.1f;
        if (game->stars[i].position.z < -10) {
            game->stars[i].position.z = 2000;
            game->stars[i].position.x = random_float(-1000, 1000);
            game->stars[i].position.y = random_float(-1000, 1000);
        }
    }
}
