#include "render.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// ============================================================================
// RENDER SYSTEM IMPLEMENTATION
// ============================================================================

// Terminal control codes
#define CLEAR_SCREEN "\033[2J\033[H"
#define RESET_COLOR "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"

// Colors
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

// Screen buffer for ASCII rendering
#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 40
static char screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH + 1];
static char color_buffer[SCREEN_HEIGHT][SCREEN_WIDTH + 1];

// Visual representations
static const char* entity_symbols[] = {
    [VISUAL_TYPE_PLAYER] = "🚀",
    [VISUAL_TYPE_SUN] = "☀️",
    [VISUAL_TYPE_PLANET] = "🪐", 
    [VISUAL_TYPE_ASTEROID] = "🗿",
    [VISUAL_TYPE_AI_SHIP] = "🛸"
};

static const char* entity_ascii[] = {
    [VISUAL_TYPE_PLAYER] = "P",
    [VISUAL_TYPE_SUN] = "*",
    [VISUAL_TYPE_PLANET] = "O",
    [VISUAL_TYPE_ASTEROID] = "o",
    [VISUAL_TYPE_AI_SHIP] = "A"
};

static const char* entity_colors[] = {
    [VISUAL_TYPE_PLAYER] = COLOR_CYAN,
    [VISUAL_TYPE_SUN] = COLOR_YELLOW,
    [VISUAL_TYPE_PLANET] = COLOR_BLUE,
    [VISUAL_TYPE_ASTEROID] = COLOR_WHITE,
    [VISUAL_TYPE_AI_SHIP] = COLOR_GREEN
};

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

bool render_init(RenderConfig* config, float viewport_width, float viewport_height) {
    if (!config) return false;
    
    // Initialize config with defaults
    config->mode = RENDER_MODE_SYMBOLS;
    config->show_debug_info = true;
    config->show_velocities = false;
    config->show_collision_bounds = false;
    config->update_interval = 1.0f / 30.0f;  // 30 FPS for terminal rendering
    config->last_update = 0.0f;
    
    // Initialize camera for zoomed-out solar system view
    config->camera.position = (Vector3){0.0f, 0.0f, 0.0f};
    config->camera.zoom = 1.0f;
    config->camera.width = viewport_width;
    config->camera.height = viewport_height;
    
    // Clear screen buffer
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            screen_buffer[y][x] = ' ';
            color_buffer[y][x] = '0';  // Default color
        }
        screen_buffer[y][SCREEN_WIDTH] = '\0';
        color_buffer[y][SCREEN_WIDTH] = '\0';
    }
    
    printf("🎨 Render system initialized (%dx%d)\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    printf("   Mode: %s\n", config->mode == RENDER_MODE_SYMBOLS ? "Symbols" : "ASCII");
    printf("   Camera zoom: %.2f\n", config->camera.zoom);
    
    return true;
}

void render_cleanup(RenderConfig* config) {
    if (!config) return;
    printf(RESET_COLOR);  // Reset terminal colors
}

// ============================================================================
// COORDINATE CONVERSION
// ============================================================================

void world_to_screen(const Camera* camera, Vector3 world_pos, int* screen_x, int* screen_y) {
    if (!camera || !screen_x || !screen_y) return;
    
    // Calculate relative position from camera
    float rel_x = world_pos.x - camera->position.x;
    float rel_z = world_pos.z - camera->position.z;  // Using Z as "Y" for 2D view
    
    // Apply zoom and convert to screen coordinates
    float scale = camera->zoom * 2.0f;  // Scale factor for solar system view
    
    *screen_x = (int)(SCREEN_WIDTH / 2 + rel_x / scale);
    *screen_y = (int)(SCREEN_HEIGHT / 2 - rel_z / scale);  // Flip Y for screen coordinates
    
    // Clamp to screen bounds
    if (*screen_x < 0) *screen_x = 0;
    if (*screen_x >= SCREEN_WIDTH) *screen_x = SCREEN_WIDTH - 1;
    if (*screen_y < 0) *screen_y = 0;
    if (*screen_y >= SCREEN_HEIGHT) *screen_y = SCREEN_HEIGHT - 1;
}

// ============================================================================
// ENTITY CLASSIFICATION
// ============================================================================

VisualType get_entity_visual_type(struct World* world, EntityID entity_id) {
    if (!world || entity_id == INVALID_ENTITY) return VISUAL_TYPE_AI_SHIP;
    
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return VISUAL_TYPE_AI_SHIP;
    
    // Check component combinations to determine type
    if (entity->component_mask & COMPONENT_PLAYER) {
        return VISUAL_TYPE_PLAYER;
    }
    
    if (entity->component_mask & COMPONENT_AI) {
        return VISUAL_TYPE_AI_SHIP;
    }
    
    // Check if it's a large, stationary object (sun/planet)
    if ((entity->component_mask & COMPONENT_PHYSICS) && 
        (entity->component_mask & COMPONENT_COLLISION)) {
        
        struct Physics* physics = entity_get_physics(world, entity_id);
        struct Collision* collision = entity_get_collision(world, entity_id);
        
        if (physics && collision) {
            // Large radius suggests sun or planet
            if (collision->radius > 15.0f) {
                return VISUAL_TYPE_SUN;
            } else if (collision->radius > 5.0f) {
                return VISUAL_TYPE_PLANET;
            } else {
                return VISUAL_TYPE_ASTEROID;
            }
        }
    }
    
    return VISUAL_TYPE_ASTEROID;  // Default
}

// ============================================================================
// RENDERING FUNCTIONS
// ============================================================================

void render_clear_screen(void) {
    // Clear the screen buffer
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            screen_buffer[y][x] = ' ';
            color_buffer[y][x] = '0';
        }
    }
}

void render_entity(struct World* world, EntityID entity_id, const Camera* camera, RenderConfig* config) {
    if (!world || entity_id == INVALID_ENTITY || !camera || !config) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Renderable* renderable = entity_get_renderable(world, entity_id);
    
    if (!transform || !renderable || !renderable->visible) return;
    
    // Convert world position to screen coordinates
    int screen_x, screen_y;
    world_to_screen(camera, transform->position, &screen_x, &screen_y);
    
    // Get visual type and representation
    VisualType visual_type = get_entity_visual_type(world, entity_id);
    
    const char* symbol;
    char color_code;
    
    if (config->mode == RENDER_MODE_SYMBOLS) {
        symbol = entity_symbols[visual_type];
        // For symbols, we'll just use the first character for simplicity in buffer
        screen_buffer[screen_y][screen_x] = entity_ascii[visual_type][0];
    } else {
        symbol = entity_ascii[visual_type];
        screen_buffer[screen_y][screen_x] = symbol[0];
    }
    
    // Set color code (simplified)
    color_buffer[screen_y][screen_x] = '0' + visual_type;
    
    // Draw collision bounds if requested
    if (config->show_collision_bounds) {
        struct Collision* collision = entity_get_collision(world, entity_id);
        if (collision && collision->shape == COLLISION_SPHERE) {
            // Draw a simple circle around the entity
            float radius_screen = collision->radius / (camera->zoom * 2.0f);
            for (int dy = -radius_screen; dy <= radius_screen; dy++) {
                for (int dx = -radius_screen; dx <= radius_screen; dx++) {
                    if (dx*dx + dy*dy <= radius_screen*radius_screen) {
                        int cx = screen_x + dx;
                        int cy = screen_y + dy;
                        if (cx >= 0 && cx < SCREEN_WIDTH && cy >= 0 && cy < SCREEN_HEIGHT) {
                            if (screen_buffer[cy][cx] == ' ') {
                                screen_buffer[cy][cx] = '.';
                                color_buffer[cy][cx] = '8';  // Dim color
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Draw velocity vector if requested
    if (config->show_velocities) {
        struct Physics* physics = entity_get_physics(world, entity_id);
        if (physics) {
            Vector3 vel_end = {
                transform->position.x + physics->velocity.x * 0.5f,
                transform->position.y + physics->velocity.y * 0.5f,
                transform->position.z + physics->velocity.z * 0.5f
            };
            
            int vel_x, vel_y;
            world_to_screen(camera, vel_end, &vel_x, &vel_y);
            
            // Draw simple line
            int dx = vel_x - screen_x;
            int dy = vel_y - screen_y;
            int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
            
            for (int i = 1; i <= steps && i < 5; i++) {  // Limit line length
                int lx = screen_x + (dx * i) / steps;
                int ly = screen_y + (dy * i) / steps;
                
                if (lx >= 0 && lx < SCREEN_WIDTH && ly >= 0 && ly < SCREEN_HEIGHT) {
                    if (screen_buffer[ly][lx] == ' ') {
                        screen_buffer[ly][lx] = '-';
                        color_buffer[ly][lx] = '7';  // White
                    }
                }
            }
        }
    }
}

void render_draw_hud(struct World* world, const Camera* camera, RenderConfig* config) {
    if (!world || !camera || !config) return;
    
    // Draw border
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        screen_buffer[0][x] = '-';
        screen_buffer[SCREEN_HEIGHT-1][x] = '-';
    }
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        screen_buffer[y][0] = '|';
        screen_buffer[y][SCREEN_WIDTH-1] = '|';
    }
    
    // Corners
    screen_buffer[0][0] = '+';
    screen_buffer[0][SCREEN_WIDTH-1] = '+';
    screen_buffer[SCREEN_HEIGHT-1][0] = '+';
    screen_buffer[SCREEN_HEIGHT-1][SCREEN_WIDTH-1] = '+';
    
    // Add title
    const char* title = " SOLAR SYSTEM VIEW ";
    int title_x = (SCREEN_WIDTH - strlen(title)) / 2;
    for (int i = 0; title[i] && (title_x + i) < SCREEN_WIDTH-1; i++) {
        screen_buffer[0][title_x + i] = title[i];
    }
    
    if (config->show_debug_info) {
        // Show camera position
        char cam_info[50];
        snprintf(cam_info, sizeof(cam_info), "Cam:(%.1f,%.1f) Zoom:%.2f", 
                camera->position.x, camera->position.z, camera->zoom);
        
        int info_len = strlen(cam_info);
        for (int i = 0; i < info_len && i < SCREEN_WIDTH-4; i++) {
            screen_buffer[SCREEN_HEIGHT-1][2 + i] = cam_info[i];
        }
    }
}

void render_present(void) {
    // Clear terminal and move cursor to top
    printf(CLEAR_SCREEN);
    
    // Render the buffer with colors
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            char ch = screen_buffer[y][x];
            char color_code = color_buffer[y][x];
            
            // Apply color based on color code
            switch (color_code) {
                case '0': printf(COLOR_CYAN); break;      // Player
                case '1': printf(COLOR_YELLOW BOLD); break; // Sun
                case '2': printf(COLOR_BLUE); break;      // Planet
                case '3': printf(COLOR_WHITE DIM); break; // Asteroid
                case '4': printf(COLOR_GREEN); break;     // AI Ship
                case '7': printf(COLOR_WHITE); break;     // Velocity lines
                case '8': printf(COLOR_WHITE DIM); break; // Collision bounds
                default: printf(RESET_COLOR); break;
            }
            
            printf("%c", ch);
        }
        printf(RESET_COLOR "\n");
    }
    
    fflush(stdout);
}

// ============================================================================
// CAMERA CONTROLS
// ============================================================================

void camera_set_position(Camera* camera, Vector3 position) {
    if (!camera) return;
    camera->position = position;
}

void camera_set_zoom(Camera* camera, float zoom) {
    if (!camera) return;
    camera->zoom = zoom;
    if (camera->zoom < 0.1f) camera->zoom = 0.1f;
    if (camera->zoom > 10.0f) camera->zoom = 10.0f;
}

void camera_follow_entity(Camera* camera, struct World* world, EntityID entity_id) {
    if (!camera || !world || entity_id == INVALID_ENTITY) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    if (transform) {
        // Smooth camera following (simple lerp)
        float lerp_factor = 0.1f;
        camera->position.x += (transform->position.x - camera->position.x) * lerp_factor;
        camera->position.y += (transform->position.y - camera->position.y) * lerp_factor;
        camera->position.z += (transform->position.z - camera->position.z) * lerp_factor;
    }
}

// ============================================================================
// MAIN RENDER FRAME FUNCTION
// ============================================================================

void render_frame(struct World* world, RenderConfig* config, float delta_time) {
    if (!world || !config) return;
    
    // Update timing
    config->last_update += delta_time;
    if (config->last_update < config->update_interval) {
        return;  // Skip this frame
    }
    config->last_update = 0.0f;
    
    // Clear screen buffer
    render_clear_screen();
    
    // Draw HUD/UI elements first
    render_draw_hud(world, &config->camera, config);
    
    // Render all visible entities
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if ((entity->component_mask & COMPONENT_RENDERABLE) &&
            (entity->component_mask & COMPONENT_TRANSFORM)) {
            
            EntityID entity_id = i + 1;  // Entity IDs are 1-based
            render_entity(world, entity_id, &config->camera, config);
        }
    }
    
    // Present the frame to terminal
    render_present();
}
