#include "cgame.h"

// External screen dimensions
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

// Draw a line in 3D space
void draw_line_3d(struct Game* game, Vector3D start, Vector3D end, Uint8 r, Uint8 g, Uint8 b) {
    Point2D p1 = project_3d_to_2d(start, &game->camera);
    Point2D p2 = project_3d_to_2d(end, &game->camera);
    
    // Only draw if points are on screen
    if (p1.x >= 0 && p1.x < SCREEN_WIDTH && p1.y >= 0 && p1.y < SCREEN_HEIGHT &&
        p2.x >= 0 && p2.x < SCREEN_WIDTH && p2.y >= 0 && p2.y < SCREEN_HEIGHT) {
        SDL_SetRenderDrawColor(game->renderer, r, g, b, 255);
        SDL_RenderDrawLine(game->renderer, p1.x, p1.y, p2.x, p2.y);
    }
}

// Draw spaceship as detailed X-wing style fighter
void draw_spaceship(struct Game* game) {
    struct Spaceship* ship = &game->player;
    struct Mesh* mesh = ship->mesh;

    for (int i = 0; i < mesh->num_edges; ++i) {
        Vector3D v1 = mesh->vertices[mesh->edges[i * 2]];
        Vector3D v2 = mesh->vertices[mesh->edges[i * 2 + 1]];

        v1 = apply_rotation(v1, ship->rotation);
        v2 = apply_rotation(v2, ship->rotation);

        v1 = vector3d_add(v1, ship->position);
        v2 = vector3d_add(v2, ship->position);

        draw_line_3d(game, v1, v2, 255, 255, 255);
    }
}

// Draw star field
void draw_stars(struct Game* game) {
    for (int i = 0; i < MAX_STARS; i++) {
        Point2D star_pos = project_3d_to_2d(game->stars[i].position, &game->camera);
        
        if (star_pos.x >= 0 && star_pos.x < SCREEN_WIDTH && 
            star_pos.y >= 0 && star_pos.y < SCREEN_HEIGHT) {
            
            Uint8 brightness = (Uint8)(game->stars[i].brightness * 255);
            SDL_SetRenderDrawColor(game->renderer, brightness, brightness, brightness, 255);
            
            // Draw star as a small cross
            SDL_RenderDrawPoint(game->renderer, star_pos.x, star_pos.y);
            SDL_RenderDrawPoint(game->renderer, star_pos.x + 1, star_pos.y);
            SDL_RenderDrawPoint(game->renderer, star_pos.x - 1, star_pos.y);
            SDL_RenderDrawPoint(game->renderer, star_pos.x, star_pos.y + 1);
            SDL_RenderDrawPoint(game->renderer, star_pos.x, star_pos.y - 1);
        }
    }
}

// Draw engine particles
void draw_particles(struct Game* game) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game->particles[i].life > 0) {
            Point2D particle_pos = project_3d_to_2d(game->particles[i].position, &game->camera);
            
            if (particle_pos.x >= 0 && particle_pos.x < SCREEN_WIDTH && 
                particle_pos.y >= 0 && particle_pos.y < SCREEN_HEIGHT) {
                
                float life_ratio = game->particles[i].life / game->particles[i].maxLife;
                Uint8 alpha = (Uint8)(life_ratio * 255);
                
                SDL_SetRenderDrawColor(game->renderer, 255, 100, 0, alpha); // Orange color
                SDL_RenderDrawPoint(game->renderer, particle_pos.x, particle_pos.y);
            }
        }
    }
}

// Draw advanced starfighter cockpit HUD
void draw_hud(struct Game* game) {
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;
    
    // === COCKPIT FRAME ===
    SDL_SetRenderDrawColor(game->renderer, 100, 100, 100, 255);
    
    // Top cockpit frame
    SDL_RenderDrawLine(game->renderer, 0, 0, SCREEN_WIDTH, 0);
    SDL_RenderDrawLine(game->renderer, 0, 0, 0, 100);
    SDL_RenderDrawLine(game->renderer, SCREEN_WIDTH-1, 0, SCREEN_WIDTH-1, 100);
    SDL_RenderDrawLine(game->renderer, 0, 100, 200, 100);
    SDL_RenderDrawLine(game->renderer, SCREEN_WIDTH-200, 100, SCREEN_WIDTH, 100);
    
    // Bottom cockpit frame
    SDL_RenderDrawLine(game->renderer, 0, SCREEN_HEIGHT-1, SCREEN_WIDTH, SCREEN_HEIGHT-1);
    SDL_RenderDrawLine(game->renderer, 0, SCREEN_HEIGHT-100, 0, SCREEN_HEIGHT);
    SDL_RenderDrawLine(game->renderer, SCREEN_WIDTH-1, SCREEN_HEIGHT-100, SCREEN_WIDTH-1, SCREEN_HEIGHT);
    SDL_RenderDrawLine(game->renderer, 0, SCREEN_HEIGHT-100, 300, SCREEN_HEIGHT-100);
    SDL_RenderDrawLine(game->renderer, SCREEN_WIDTH-300, SCREEN_HEIGHT-100, SCREEN_WIDTH, SCREEN_HEIGHT-100);
    
    // === PRIMARY CROSSHAIR (rotates with ship roll) ===
    SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 200);
    
    // Calculate roll rotation for crosshair
    float roll_angle = game->player.rotation.z;
    float cos_roll = cos(roll_angle);
    float sin_roll = sin(roll_angle);
    
    // Rotate crosshair lines
    int cross_size = 20;
    int cross_x1 = center_x + (int)(cross_size * cos_roll);
    int cross_y1 = center_y + (int)(cross_size * sin_roll);
    int cross_x2 = center_x - (int)(cross_size * cos_roll);
    int cross_y2 = center_y - (int)(cross_size * sin_roll);
    
    int cross_x3 = center_x + (int)(cross_size * -sin_roll);
    int cross_y3 = center_y + (int)(cross_size * cos_roll);
    int cross_x4 = center_x - (int)(cross_size * -sin_roll);
    int cross_y4 = center_y - (int)(cross_size * cos_roll);
    
    SDL_RenderDrawLine(game->renderer, cross_x1, cross_y1, cross_x2, cross_y2);
    SDL_RenderDrawLine(game->renderer, cross_x3, cross_y3, cross_x4, cross_y4);
    
    // Corner brackets (also rotated)
    int bracket_size = 15;
    int bracket_offset = 10;
    for (int i = 0; i < 4; i++) {
        float angle = roll_angle + i * M_PI / 2.0f;
        int bx = center_x + (int)(bracket_offset * cos(angle));
        int by = center_y + (int)(bracket_offset * sin(angle));
        
        int bx1 = bx + (int)(bracket_size * cos(angle + M_PI/4));
        int by1 = by + (int)(bracket_size * sin(angle + M_PI/4));
        int bx2 = bx + (int)(bracket_size * cos(angle - M_PI/4));
        int by2 = by + (int)(bracket_size * sin(angle - M_PI/4));
        
        SDL_RenderDrawLine(game->renderer, bx, by, bx1, by1);
        SDL_RenderDrawLine(game->renderer, bx, by, bx2, by2);
    }
    
    // === THROTTLE INDICATOR ===
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
    int throttle_height = (int)(game->player.throttle * 80);
    int throttle_x = 50;
    int throttle_y = center_y;
    
    // Throttle frame
    SDL_RenderDrawLine(game->renderer, throttle_x - 10, throttle_y - 90, throttle_x + 10, throttle_y - 90);
    SDL_RenderDrawLine(game->renderer, throttle_x - 10, throttle_y + 90, throttle_x + 10, throttle_y + 90);
    SDL_RenderDrawLine(game->renderer, throttle_x - 10, throttle_y - 90, throttle_x - 10, throttle_y + 90);
    SDL_RenderDrawLine(game->renderer, throttle_x + 10, throttle_y - 90, throttle_x + 10, throttle_y + 90);
    
    // Throttle bar
    if (throttle_height > 0) {
        SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
        for (int i = 0; i < throttle_height; i += 5) {
            SDL_RenderDrawLine(game->renderer, throttle_x - 8, throttle_y - i, throttle_x + 8, throttle_y - i);
        }
    } else if (throttle_height < 0) {
        SDL_SetRenderDrawColor(game->renderer, 255, 100, 0, 255);
        for (int i = 0; i > throttle_height; i -= 5) {
            SDL_RenderDrawLine(game->renderer, throttle_x - 8, throttle_y - i, throttle_x + 8, throttle_y - i);
        }
    }
    
    // === ENERGY/SHIELDS DISPLAY ===
    int energy_x = SCREEN_WIDTH - 100;
    int energy_y = 150;
    
    // Energy bar
    SDL_SetRenderDrawColor(game->renderer, 0, 100, 255, 255);
    SDL_RenderDrawLine(game->renderer, energy_x - 40, energy_y, energy_x + 40, energy_y);
    SDL_RenderDrawLine(game->renderer, energy_x - 40, energy_y + 20, energy_x + 40, energy_y + 20);
    SDL_RenderDrawLine(game->renderer, energy_x - 40, energy_y, energy_x - 40, energy_y + 20);
    SDL_RenderDrawLine(game->renderer, energy_x + 40, energy_y, energy_x + 40, energy_y + 20);
    
    int energy_width = (int)(game->player.energy * 0.8f);
    SDL_SetRenderDrawColor(game->renderer, 0, 200, 255, 255);
    for (int i = 0; i < energy_width; i++) {
        SDL_RenderDrawLine(game->renderer, energy_x - 38 + i, energy_y + 2, energy_x - 38 + i, energy_y + 18);
    }
    
    // Shields bar
    SDL_SetRenderDrawColor(game->renderer, 0, 255, 100, 255);
    SDL_RenderDrawLine(game->renderer, energy_x - 40, energy_y + 40, energy_x + 40, energy_y + 40);
    SDL_RenderDrawLine(game->renderer, energy_x - 40, energy_y + 60, energy_x + 40, energy_y + 60);
    SDL_RenderDrawLine(game->renderer, energy_x - 40, energy_y + 40, energy_x - 40, energy_y + 60);
    SDL_RenderDrawLine(game->renderer, energy_x + 40, energy_y + 40, energy_x + 40, energy_y + 60);
    
    int shields_width = (int)(game->player.shields * 0.8f);
    SDL_SetRenderDrawColor(game->renderer, 0, 255, 150, 255);
    for (int i = 0; i < shields_width; i++) {
        SDL_RenderDrawLine(game->renderer, energy_x - 38 + i, energy_y + 42, energy_x - 38 + i, energy_y + 58);
    }
    
    // === VELOCITY VECTOR DISPLAY ===
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 180);
    int vel_scale = 20;
    int vel_center_x = center_x + 150;
    int vel_center_y = center_y - 150;
    
    // Velocity vector circle
    for (int angle = 0; angle < 360; angle += 10) {
        float rad = deg_to_rad(angle);
        int x1 = vel_center_x + (int)(cos(rad) * 40);
        int y1 = vel_center_y + (int)(sin(rad) * 40);
        int x2 = vel_center_x + (int)(cos(rad + 0.1f) * 40);
        int y2 = vel_center_y + (int)(sin(rad + 0.1f) * 40);
        SDL_RenderDrawLine(game->renderer, x1, y1, x2, y2);
    }
    
    // Velocity indicator
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    int vel_x_indicator = vel_center_x + (int)(game->player.velocity.x * vel_scale);
    int vel_y_indicator = vel_center_y - (int)(game->player.velocity.y * vel_scale);
    SDL_RenderDrawLine(game->renderer, vel_center_x, vel_center_y, vel_x_indicator, vel_y_indicator);
    SDL_RenderDrawLine(game->renderer, vel_x_indicator - 3, vel_y_indicator - 3, vel_x_indicator + 3, vel_y_indicator + 3);
    SDL_RenderDrawLine(game->renderer, vel_x_indicator - 3, vel_y_indicator + 3, vel_x_indicator + 3, vel_y_indicator - 3);
    
    // === AFTERBURNER INDICATOR ===
    if (game->player.afterburner) {
        SDL_SetRenderDrawColor(game->renderer, 255, 100, 0, 255);
        for (int i = 0; i < 10; i++) {
            SDL_RenderDrawLine(game->renderer, center_x - 50 + i * 10, center_y + 200, 
                             center_x - 50 + i * 10, center_y + 220);
        }
    }
    
    // === ROLL INDICATOR ===
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 180);
    int roll_center_x = center_x;
    int roll_center_y = 80;
    int roll_radius = 60;
    
    // Draw roll scale (horizon line with tick marks)
    for (int angle = -90; angle <= 90; angle += 15) {
        float rad = deg_to_rad(angle);
        int tick_x1 = roll_center_x + (int)(roll_radius * cos(rad));
        int tick_y1 = roll_center_y + (int)(roll_radius * sin(rad));
        int tick_x2 = roll_center_x + (int)((roll_radius - 8) * cos(rad));
        int tick_y2 = roll_center_y + (int)((roll_radius - 8) * sin(rad));
        
        SDL_RenderDrawLine(game->renderer, tick_x1, tick_y1, tick_x2, tick_y2);
    }
    
    // Draw current roll indicator
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
    float current_roll = -game->player.rotation.z; // Negative for correct orientation
    int roll_indicator_x = roll_center_x + (int)(roll_radius * cos(current_roll + M_PI/2));
    int roll_indicator_y = roll_center_y + (int)(roll_radius * sin(current_roll + M_PI/2));
    
    // Draw roll indicator triangle
    SDL_RenderDrawLine(game->renderer, roll_indicator_x, roll_indicator_y, 
                     roll_indicator_x - 5, roll_indicator_y + 10);
    SDL_RenderDrawLine(game->renderer, roll_indicator_x, roll_indicator_y, 
                     roll_indicator_x + 5, roll_indicator_y + 10);
    SDL_RenderDrawLine(game->renderer, roll_indicator_x - 5, roll_indicator_y + 10, 
                     roll_indicator_x + 5, roll_indicator_y + 10);
    
    // === FIRING INDICATOR ===
    if (game->player.firing) {
        SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(game->renderer, center_x - 100, center_y - 200, center_x + 100, center_y - 200);
        SDL_RenderDrawLine(game->renderer, center_x - 100, center_y - 190, center_x + 100, center_y - 190);
    }
    
    // === MOUSE CAPTURE INDICATOR ===
    if (game->mouse_captured) {
        SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 150);
        SDL_RenderDrawLine(game->renderer, 10, SCREEN_HEIGHT - 30, 30, SCREEN_HEIGHT - 30);
        SDL_RenderDrawLine(game->renderer, 10, SCREEN_HEIGHT - 20, 30, SCREEN_HEIGHT - 20);
    } else {
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 150);
        SDL_RenderDrawLine(game->renderer, 10, SCREEN_HEIGHT - 30, 30, SCREEN_HEIGHT - 30);
        SDL_RenderDrawLine(game->renderer, 10, SCREEN_HEIGHT - 20, 30, SCREEN_HEIGHT - 20);
    }
}

// Render everything
void render_game(struct Game* game) {
    // Clear screen with dark space color
    SDL_SetRenderDrawColor(game->renderer, 5, 5, 15, 255);
    SDL_RenderClear(game->renderer);
    
    // Draw stars first (background)
    draw_stars(game);
    
    // Draw engine particles
    draw_particles(game);
    
    // Draw spaceship
    draw_spaceship(game);
    
    // Draw HUD
    draw_hud(game);
    
    // Present the rendered frame
    SDL_RenderPresent(game->renderer);
}
