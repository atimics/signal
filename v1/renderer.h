#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "game.h"
#include "math3d.h"

// Rendering functions
void draw_line_3d(struct Game* game, Vector3D start, Vector3D end, Uint8 r, Uint8 g, Uint8 b);
void draw_spaceship(struct Game* game);
void draw_stars(struct Game* game);
void draw_particles(struct Game* game);
void draw_hud(struct Game* game);
void render_game(struct Game* game);

#endif
