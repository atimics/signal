#include "render.h"
#include "render_camera.h"
#include "render_lighting.h"
#include "render_mesh.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

bool render_init(RenderConfig* config, AssetRegistry* assets, float viewport_width, float viewport_height) {
    if (!config || !assets) return false;
    
    printf("🎨 Initializing 3D SDL Render System...\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("❌ SDL could not initialize: %s\n", SDL_GetError());
        return false;
    }
    
    // Create window
    config->screen_width = (int)viewport_width;
    config->screen_height = (int)viewport_height;
    config->assets = assets;
    
    config->window = SDL_CreateWindow("CGGame - Spaceport",
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     config->screen_width,
                                     config->screen_height,
                                     SDL_WINDOW_SHOWN);
    
    if (!config->window) {
        printf("❌ Window could not be created: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    config->renderer = SDL_CreateRenderer(config->window, -1, 
                                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!config->renderer) {
        printf("❌ Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(config->window);
        SDL_Quit();
        return false;
    }
    
    // Initialize render settings
    config->mode = RENDER_MODE_TEXTURED;  // Use textured mode by default
    config->show_debug_info = true;
    config->show_velocities = true;
    config->show_collision_bounds = false;
    config->show_orbits = true;
    config->update_interval = 1.0f / 60.0f;  // 60 FPS
    config->last_update = 0.0f;
    config->frame_count = 0;
    
    // Initialize cockpit UI
    if (!cockpit_ui_init(&config->ui, config->renderer, config->screen_width, config->screen_height)) {
        printf("❌ Failed to initialize cockpit UI\n");
        SDL_DestroyRenderer(config->renderer);
        SDL_DestroyWindow(config->window);
        SDL_Quit();
        return false;
    }
    
    // Initialize 3D camera for first-person view
    config->camera.position = (Vector3){-20, 5, -20};    // Start at player position (elevated)
    config->camera.target = (Vector3){0, 5, 0};          // Looking toward spaceport center
    config->camera.up = (Vector3){0, 1, 0};              // Y is up
    config->camera.fov = 75.0f;                          // Wider FOV for FPS
    config->camera.near_plane = 0.1f;                    // Closer near plane for FPS
    config->camera.far_plane = 500.0f;
    config->camera.aspect_ratio = viewport_width / viewport_height;
    
    // Initialize lighting system
    lighting_init(&config->lighting);
    
    printf("✅ 3D Render system initialized (%dx%d)\n", config->screen_width, config->screen_height);
    printf("   Camera position: (%.1f, %.1f, %.1f)\n", 
           config->camera.position.x, config->camera.position.y, config->camera.position.z);
    printf("   Using enhanced material system\n");
    
    return true;
}

void render_cleanup(RenderConfig* config) {
    if (!config) return;
    
    // Cleanup UI
    cockpit_ui_cleanup(&config->ui);
    
    // Asset cleanup is handled by the asset system
    
    // Cleanup SDL
    if (config->renderer) {
        SDL_DestroyRenderer(config->renderer);
        config->renderer = NULL;
    }
    
    if (config->window) {
        SDL_DestroyWindow(config->window);
        config->window = NULL;
    }
    
    SDL_Quit();
    printf("🎨 Render system cleaned up\n");
}

// ============================================================================
// MAIN RENDERING PIPELINE
// ============================================================================

void render_clear_screen(RenderConfig* config) {
    if (!config || !config->renderer) return;
    
    // Set background to space black
    SDL_SetRenderDrawColor(config->renderer, 0, 0, 0, 255);
    SDL_RenderClear(config->renderer);
}

void render_debug_info(struct World* world, RenderConfig* config) {
    if (!config || !config->show_debug_info) return;
    
    // For now, just count entities - we'd need a font system for proper text
    // This is where you'd render FPS, entity count, camera position, etc.
    
    static uint32_t debug_counter = 0;
    if (++debug_counter % 60 == 0) {  // Print debug info every second
        printf("🎮 Frame %d | Entities: %d | Camera: (%.1f,%.1f,%.1f)\n", 
               config->frame_count, world->entity_count,
               config->camera.position.x, config->camera.position.y, config->camera.position.z);
    }
}

void render_present(RenderConfig* config) {
    if (!config || !config->renderer) return;
    
    SDL_RenderPresent(config->renderer);
    config->frame_count++;
}

void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time) {
    if (!world || !config) return;
    
    // Suppress unused parameter warning
    (void)delta_time;
    (void)player_id;  // Camera system now handles camera positioning
    
    // Clear the screen
    render_clear_screen(config);
    
    // Camera position is now managed by camera system
    
    // Render all entities using enhanced material system
    for (uint32_t i = 0; i < world->entity_count; i++) {
        EntityID entity_id = i + 1; // Entity IDs are 1-based
        render_entity_3d(world, entity_id, config);
    }
    
    // Render debug information
    render_debug_info(world, config);
    
    // Render UI
    cockpit_ui_render(&config->ui);
    
    // Present the frame
    render_present(config);
}

// ============================================================================
// UI MESSAGE API
// ============================================================================

void render_add_comm_message(RenderConfig* config, const char* sender, const char* message, bool is_player) {
    if (!config) return;
    cockpit_ui_add_message(&config->ui, sender, message, is_player);
}

// ============================================================================
// SCREENSHOT FUNCTIONALITY
// ============================================================================

bool render_take_screenshot(RenderConfig* config, const char* filename) {
    if (!config || !config->renderer || !filename) return false;
    
    // Get the current render target surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, config->screen_width, config->screen_height, 32,
                                               0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        printf("❌ Failed to create surface for screenshot: %s\n", SDL_GetError());
        return false;
    }
    
    // Read pixels from renderer
    if (SDL_RenderReadPixels(config->renderer, NULL, surface->format->format,
                           surface->pixels, surface->pitch) != 0) {
        printf("❌ Failed to read pixels for screenshot: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    
    // Save as BMP file
    if (SDL_SaveBMP(surface, filename) != 0) {
        printf("❌ Failed to save screenshot: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    
    SDL_FreeSurface(surface);
    printf("📸 Screenshot saved: %s (%dx%d)\n", filename, config->screen_width, config->screen_height);
    return true;
}

bool render_take_screenshot_from_position(struct World* world, RenderConfig* config, 
                                         Vector3 camera_pos, Vector3 look_at_pos, 
                                         const char* filename) {
    if (!world || !config || !filename) return false;
    
    // Save current camera state
    Camera3D original_camera = config->camera;
    
    // Set up camera at specified position
    camera_look_at(&config->camera, camera_pos, look_at_pos, (Vector3){0, 1, 0});
    
    printf("📸 Taking screenshot from (%.1f, %.1f, %.1f) looking at (%.1f, %.1f, %.1f)\n",
           camera_pos.x, camera_pos.y, camera_pos.z,
           look_at_pos.x, look_at_pos.y, look_at_pos.z);
    
    // Clear screen with space background
    SDL_SetRenderDrawColor(config->renderer, 8, 8, 20, 255);  // Dark space blue
    SDL_RenderClear(config->renderer);
    
    // Render all entities from this viewpoint
    uint32_t rendered_count = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if ((entity->component_mask & COMPONENT_RENDERABLE) &&
            (entity->component_mask & COMPONENT_TRANSFORM)) {
            
            EntityID entity_id = i + 1;  // Entity IDs are 1-based
            render_entity_3d(world, entity_id, config);
            rendered_count++;
        }
    }
    
    printf("📸 Rendered %d entities for screenshot\n", rendered_count);
    
    // Present the frame (this updates the renderer's back buffer)
    SDL_RenderPresent(config->renderer);
    
    // Small delay to ensure rendering is complete
    SDL_Delay(100);
    
    // Take the screenshot
    bool success = render_take_screenshot(config, filename);
    
    // Restore original camera
    config->camera = original_camera;
    
    return success;
}
