#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

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
    
    printf("🎨 Initializing Sokol Render System...\n");
    
    // Create window
    config->screen_width = (int)viewport_width;
    config->screen_height = (int)viewport_height;
    config->assets = assets;
    
    // Initialize render settings
    config->mode = RENDER_MODE_TEXTURED;  // Use textured mode by default
    config->show_debug_info = true;
    config->show_velocities = true;
    config->show_collision_bounds = false;
    config->show_orbits = true;
    config->update_interval = 1.0f / 60.0f;  // 60 FPS
    config->last_update = 0.0f;
    config->frame_count = 0;
    
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
    
    sg_shutdown();
    printf("🎨 Render system cleaned up\n");
}

// ============================================================================
// MAIN RENDERING PIPELINE
// ============================================================================

void render_clear_screen(RenderConfig* config) {
    // This will be handled by the sokol_gfx pass action
    (void)config;
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
    // This will be handled by sg_commit()
    (void)config;
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
    // cockpit_ui_render(&config->ui);
    
    // Present the frame
    render_present(config);
}

// ============================================================================
// UI MESSAGE API
// ============================================================================

void render_add_comm_message(RenderConfig* config, const char* sender, const char* message, bool is_player) {
    if (!config) return;
    // cockpit_ui_add_message(&config->ui, sender, message, is_player);
}

// ============================================================================
// SCREENSHOT FUNCTIONALITY
// ============================================================================

bool render_take_screenshot(RenderConfig* config, const char* filename) {
    // This will need to be re-implemented with sokol_gfx
    (void)config;
    (void)filename;
    return false;
}

bool render_take_screenshot_from_position(struct World* world, RenderConfig* config, 
                                         Vector3 camera_pos, Vector3 look_at_pos, 
                                         const char* filename) {
    // This will need to be re-implemented with sokol_gfx
    (void)world;
    (void)config;
    (void)camera_pos;
    (void)look_at_pos;
    (void)filename;
    return false;
}
