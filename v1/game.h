#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "spaceship.h"
#include "starfield.h"
#include "particles.h"
#include "camera.h"
#include "universe.h"
#include "dialog_system.h"
#include "entity.h"

// Screen dimensions
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

// Maximum entities
#define MAX_STARS 200
#define MAX_PARTICLES 100

// Game state structure
struct Game {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    struct Spaceship player;
    struct Star stars[MAX_STARS];
    struct Particle particles[MAX_PARTICLES];
    struct Camera camera;
    float time;
    bool mouse_captured;
    int last_mouse_x, last_mouse_y;
    bool keys[SDL_NUM_SCANCODES];
    
    // AI-driven universe system
    struct Universe* universe;
    struct DialogSession* active_dialog;
    
    // Demo AI entities
    struct Entity* demo_entities[5];
    int demo_entity_count;
};

// Game initialization and cleanup
bool init_game(struct Game* game);
void cleanup_game(struct Game* game);

// Main game loop functions
void update_game(struct Game* game);
void render_game(struct Game* game);

#endif
