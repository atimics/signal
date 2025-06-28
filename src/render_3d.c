#include "render.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// 3D PROJECTION
// ============================================================================

Point2D project_3d_to_2d(Vector3 world_pos, const Camera3D* camera, int screen_width, int screen_height) {
    // Simple perspective projection
    
    // Transform world position relative to camera
    Vector3 cam_pos = {
        world_pos.x - camera->position.x,
        world_pos.y - camera->position.y,
        world_pos.z - camera->position.z
    };
    
    // Simple rotation toward target (simplified - should use proper matrix)
    // For now, just use the position directly
    
    // Perspective division
    float z = cam_pos.z;
    if (z <= 0.1f) z = 0.1f;  // Prevent division by zero
    
    float scale = 200.0f / z;  // Simple perspective scale
    
    Point2D result = {
        (int)(screen_width / 2 + cam_pos.x * scale),
        (int)(screen_height / 2 - cam_pos.y * scale)  // Flip Y for screen coordinates
    };
    
    return result;
}

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
    config->mode = RENDER_MODE_WIREFRAME;
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
    
    printf("✅ 3D Render system initialized (%dx%d)\n", config->screen_width, config->screen_height);
    printf("   Camera position: (%.1f, %.1f, %.1f)\n", 
           config->camera.position.x, config->camera.position.y, config->camera.position.z);
    printf("   Using data-driven asset system\n");
    
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

void render_clear_screen(RenderConfig* config) {
    if (!config || !config->renderer) return;
    
    // Set background to space black
    SDL_SetRenderDrawColor(config->renderer, 0, 0, 0, 255);
    SDL_RenderClear(config->renderer);
}

void render_wireframe_mesh(const Mesh* mesh, Vector3 position, Vector3 rotation, 
                          Vector3 scale, RenderConfig* config, uint8_t r, uint8_t g, uint8_t b) {
    
    if (!mesh || !config || !config->renderer || !mesh->loaded) return;
    
    SDL_SetRenderDrawColor(config->renderer, r, g, b, 255);
    
    // For OBJ files, indices are for triangles (3 per face)
    // We'll draw wireframe by drawing triangle edges
    for (uint32_t i = 0; i < mesh->index_count; i += 3) {
        if (i + 2 >= mesh->index_count) break;
        
        uint32_t idx1 = mesh->indices[i];
        uint32_t idx2 = mesh->indices[i + 1];
        uint32_t idx3 = mesh->indices[i + 2];
        
        if (idx1 >= mesh->vertex_count || idx2 >= mesh->vertex_count || idx3 >= mesh->vertex_count) continue;
        
        // Get triangle vertices
        Vector3 v1 = mesh->vertices[idx1];
        Vector3 v2 = mesh->vertices[idx2];
        Vector3 v3 = mesh->vertices[idx3];
        
        // Apply scale
        v1.x *= scale.x; v1.y *= scale.y; v1.z *= scale.z;
        v2.x *= scale.x; v2.y *= scale.y; v2.z *= scale.z;
        v3.x *= scale.x; v3.y *= scale.y; v3.z *= scale.z;
        
        // Apply position
        v1.x += position.x; v1.y += position.y; v1.z += position.z;
        v2.x += position.x; v2.y += position.y; v2.z += position.z;
        v3.x += position.x; v3.y += position.y; v3.z += position.z;
        
        // Project to 2D
        Point2D p1 = project_3d_to_2d(v1, &config->camera, config->screen_width, config->screen_height);
        Point2D p2 = project_3d_to_2d(v2, &config->camera, config->screen_width, config->screen_height);
        Point2D p3 = project_3d_to_2d(v3, &config->camera, config->screen_width, config->screen_height);
        
        // Draw triangle edges
        SDL_RenderDrawLine(config->renderer, p1.x, p1.y, p2.x, p2.y);
        SDL_RenderDrawLine(config->renderer, p2.x, p2.y, p3.x, p3.y);
        SDL_RenderDrawLine(config->renderer, p3.x, p3.y, p1.x, p1.y);
    }
}

void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config) {
    if (!world || entity_id == INVALID_ENTITY || !config || !config->assets) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Renderable* renderable = entity_get_renderable(world, entity_id);
    
    if (!transform || !renderable || !renderable->visible) return;
    
    // Get visual type and corresponding mesh
    VisualType visual_type = get_entity_visual_type(world, entity_id);
    
    // Map visual types to mesh names
    const char* mesh_name = "fallback";
    switch (visual_type) {
        case VISUAL_TYPE_PLAYER:
        case VISUAL_TYPE_AI_SHIP:
            mesh_name = "player_ship";
            break;
        case VISUAL_TYPE_SUN:
            mesh_name = "sun";
            break;
        case VISUAL_TYPE_PLANET:
            mesh_name = "planet";
            break;
        case VISUAL_TYPE_ASTEROID:
            mesh_name = "asteroid";
            break;
        default:
            mesh_name = "player_ship";  // Fallback
    }
    
    // Get mesh from asset system
    Mesh* mesh = assets_get_mesh(config->assets, mesh_name);
    if (!mesh) {
        // Try fallback mesh
        mesh = assets_get_mesh(config->assets, "player_ship");
        if (!mesh) return;  // No mesh available
    }
    
    // Set color based on entity type (could be driven by materials in the future)
    uint8_t r, g, b;
    switch (visual_type) {
        case VISUAL_TYPE_PLAYER:
            r = 0; g = 255; b = 255;  // Cyan
            break;
        case VISUAL_TYPE_SUN:
            r = 255; g = 255; b = 0;  // Yellow
            break;
        case VISUAL_TYPE_PLANET:
            r = 100; g = 150; b = 255;  // Blue
            break;
        case VISUAL_TYPE_ASTEROID:
            r = 128; g = 128; b = 128;  // Gray
            break;
        case VISUAL_TYPE_AI_SHIP:
            r = 0; g = 255; b = 0;    // Green
            break;
        default:
            r = 255; g = 255; b = 255;  // White
    }
    
    // Render the mesh
    Vector3 rotation = {0, 0, 0};  // TODO: Use actual rotation from transform
    render_wireframe_mesh(mesh, transform->position, rotation, transform->scale, config, r, g, b);
    
    // Draw velocity vector if requested
    if (config->show_velocities) {
        struct Physics* physics = entity_get_physics(world, entity_id);
        if (physics && (physics->velocity.x != 0 || physics->velocity.y != 0 || physics->velocity.z != 0)) {
            Vector3 vel_end = {
                transform->position.x + physics->velocity.x * 5.0f,
                transform->position.y + physics->velocity.y * 5.0f,
                transform->position.z + physics->velocity.z * 5.0f
            };
            
            Point2D p1 = project_3d_to_2d(transform->position, &config->camera, 
                                         config->screen_width, config->screen_height);
            Point2D p2 = project_3d_to_2d(vel_end, &config->camera, 
                                         config->screen_width, config->screen_height);
            
            SDL_SetRenderDrawColor(config->renderer, 255, 255, 255, 128);  // White velocity lines
            SDL_RenderDrawLine(config->renderer, p1.x, p1.y, p2.x, p2.y);
        }
    }
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

// ============================================================================
// CAMERA CONTROLS
// ============================================================================

void camera_set_position(Camera3D* camera, Vector3 position) {
    if (!camera) return;
    camera->position = position;
}

void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up) {
    if (!camera) return;
    camera->position = eye;
    camera->target = target;
    camera->up = up;
}

void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance) {
    if (!camera || !world || entity_id == INVALID_ENTITY) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Physics* physics = entity_get_physics(world, entity_id);
    
    if (transform) {
        // First-person camera: position camera AT the entity (slightly elevated)
        Vector3 fps_offset = {0, 2, 0};  // Elevated cockpit view
        
        Vector3 target_pos = {
            transform->position.x + fps_offset.x,
            transform->position.y + fps_offset.y,
            transform->position.z + fps_offset.z
        };
        
        // Smooth camera movement
        float lerp = 0.08f;  // Slightly faster for FPS feel
        camera->position.x += (target_pos.x - camera->position.x) * lerp;
        camera->position.y += (target_pos.y - camera->position.y) * lerp;
        camera->position.z += (target_pos.z - camera->position.z) * lerp;
        
        // Look ahead based on movement direction
        if (physics && (physics->velocity.x != 0 || physics->velocity.z != 0)) {
            // Look in direction of movement
            Vector3 look_ahead = {
                camera->position.x + physics->velocity.x * 5.0f,
                camera->position.y + physics->velocity.y * 2.0f,
                camera->position.z + physics->velocity.z * 5.0f
            };
            camera->target = look_ahead;
        } else {
            // Look forward when stationary
            Vector3 forward_look = {
                camera->position.x + 0,
                camera->position.y + 0,
                camera->position.z - 10  // Look forward (negative Z)
            };
            camera->target = forward_look;
        }
    }
}

// ============================================================================
// UI MESSAGE API
// ============================================================================

void render_add_comm_message(RenderConfig* config, const char* sender, const char* message, bool is_player) {
    if (!config) return;
    cockpit_ui_add_message(&config->ui, sender, message, is_player);
}

// ============================================================================
// MAIN RENDER FRAME FUNCTION
// ============================================================================

void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time) {
    if (!world || !config) return;
    
    // Update UI data (but don't render it)
    cockpit_ui_update(&config->ui, world, player_id, delta_time);
    
    // Clear the screen
    render_clear_screen(config);
    
    // Use full screen for 3D rendering (no UI borders)
    SDL_RenderSetViewport(config->renderer, NULL);
    
    // Render all visible entities in the full viewport
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if ((entity->component_mask & COMPONENT_RENDERABLE) &&
            (entity->component_mask & COMPONENT_TRANSFORM)) {
            
            EntityID entity_id = i + 1;  // Entity IDs are 1-based
            render_entity_3d(world, entity_id, config);
        }
    }
    
    // Render debug information (if enabled) - but no cockpit UI
    if (config->show_debug_info) {
        render_debug_info(world, config);
    }
    
    // Present the frame
    render_present(config);
}
