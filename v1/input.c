#include "cgame.h"

// Handle input events
void handle_events(struct Game* game) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            game->running = false;
        }
        else if (e.type == SDL_KEYDOWN) {
            game->keys[e.key.keysym.scancode] = true;
            
            // Toggle mouse capture with M key
            if (e.key.keysym.scancode == SDL_SCANCODE_M) {
                game->mouse_captured = !game->mouse_captured;
                if (game->mouse_captured) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                } else {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
            }
        }
        else if (e.type == SDL_KEYUP) {
            game->keys[e.key.keysym.scancode] = false;
        }
        else if (e.type == SDL_MOUSEMOTION && game->mouse_captured) {
            // Mouse look controls
            float sensitivity = 0.002f;
            game->player.angular_velocity.y += e.motion.xrel * sensitivity;
            game->player.angular_velocity.x += e.motion.yrel * sensitivity;
        }
    }

    // Throttle control (smooth acceleration)
    if (game->keys[SDL_SCANCODE_W] || game->keys[SDL_SCANCODE_UP]) {
        game->player.throttle += 0.02f;
        if (game->player.throttle > 1.0f) game->player.throttle = 1.0f;
    } else if (game->keys[SDL_SCANCODE_S] || game->keys[SDL_SCANCODE_DOWN]) {
        game->player.throttle -= 0.02f;
        if (game->player.throttle < -0.5f) game->player.throttle = -0.5f;
    } else {
        game->player.throttle *= 0.95f; // Gradual throttle decay
    }
    
    // Afterburner
    game->player.afterburner = (game->keys[SDL_SCANCODE_LSHIFT] || game->keys[SDL_SCANCODE_RSHIFT]) && 
                               game->player.energy > 0;
    
    // Lateral movement (strafe)
    if (game->keys[SDL_SCANCODE_A] || game->keys[SDL_SCANCODE_LEFT]) {
        game->player.velocity.x -= game->player.speed * 0.05f;
    }
    if (game->keys[SDL_SCANCODE_D] || game->keys[SDL_SCANCODE_RIGHT]) {
        game->player.velocity.x += game->player.speed * 0.05f;
    }
    
    // Vertical movement
    if (game->keys[SDL_SCANCODE_SPACE]) {
        game->player.velocity.y += game->player.speed * 0.05f;
    }
    if (game->keys[SDL_SCANCODE_C] || game->keys[SDL_SCANCODE_LCTRL]) {
        game->player.velocity.y -= game->player.speed * 0.05f;
    }
    
    // Roll controls (fixed Q/E)
    if (game->keys[SDL_SCANCODE_Q]) {
        game->player.angular_velocity.z += game->player.rotSpeed * 0.02f;
    }
    if (game->keys[SDL_SCANCODE_E]) {
        game->player.angular_velocity.z -= game->player.rotSpeed * 0.02f;
    }
    
    // Pitch controls (additional to mouse)
    if (game->keys[SDL_SCANCODE_R]) {
        game->player.angular_velocity.x -= game->player.rotSpeed * 0.02f;
    }
    if (game->keys[SDL_SCANCODE_F]) {
        game->player.angular_velocity.x += game->player.rotSpeed * 0.02f;
    }
    
    // Yaw controls (additional to mouse)
    if (game->keys[SDL_SCANCODE_T]) {
        game->player.angular_velocity.y -= game->player.rotSpeed * 0.02f;
    }
    if (game->keys[SDL_SCANCODE_G]) {
        game->player.angular_velocity.y += game->player.rotSpeed * 0.02f;
    }
    
    // Weapons
    game->player.firing = game->keys[SDL_SCANCODE_LALT] || game->keys[SDL_SCANCODE_RALT];
}
