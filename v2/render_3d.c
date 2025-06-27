#include "render.h"
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
    
    config->window = SDL_CreateWindow("V2 Solar System - Data-Driven Engine",
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
    
    // Initialize 3D camera for zoomed-out solar system view
    config->camera.position = (Vector3){0, 100, 300};    // Above and back from sun
    config->camera.target = (Vector3){0, 0, 0};          // Looking at sun
    config->camera.up = (Vector3){0, 1, 0};              // Y is up
    config->camera.fov = 60.0f;
    config->camera.near_plane = 1.0f;
    config->camera.far_plane = 1000.0f;
    config->camera.aspect_ratio = viewport_width / viewport_height;
    
    printf("✅ 3D Render system initialized (%dx%d)\n", config->screen_width, config->screen_height);
    printf("   Camera position: (%.1f, %.1f, %.1f)\n", 
           config->camera.position.x, config->camera.position.y, config->camera.position.z);
    printf("   Using data-driven asset system\n");
    
    return true;
}

void render_cleanup(RenderConfig* config) {
    if (!config) return;
    
    // Cleanup meshes
    for (int i = 0; i < VISUAL_TYPE_COUNT; i++) {
        cleanup_mesh(&config->meshes[i]);
    }
    
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

void render_wireframe_mesh(const SimpleMesh* mesh, Vector3 position, Vector3 rotation, 
                          Vector3 scale, RenderConfig* config, uint8_t r, uint8_t g, uint8_t b) {
    
    if (!mesh || !config || !config->renderer) return;
    
    SDL_SetRenderDrawColor(config->renderer, r, g, b, 255);
    
    // Simple wireframe rendering - draw lines between indexed vertices
    for (uint32_t i = 0; i < mesh->index_count; i += 2) {
        uint32_t idx1 = mesh->indices[i];
        uint32_t idx2 = mesh->indices[i + 1];
        
        if (idx1 >= mesh->vertex_count || idx2 >= mesh->vertex_count) continue;
        
        // Transform vertices (simplified - no proper matrix math)
        Vector3 v1 = mesh->vertices[idx1];
        Vector3 v2 = mesh->vertices[idx2];
        
        // Apply scale
        v1.x *= scale.x; v1.y *= scale.y; v1.z *= scale.z;
        v2.x *= scale.x; v2.y *= scale.y; v2.z *= scale.z;
        
        // Apply position
        v1.x += position.x; v1.y += position.y; v1.z += position.z;
        v2.x += position.x; v2.y += position.y; v2.z += position.z;
        
        // Project to 2D
        Point2D p1 = project_3d_to_2d(v1, &config->camera, config->screen_width, config->screen_height);
        Point2D p2 = project_3d_to_2d(v2, &config->camera, config->screen_width, config->screen_height);
        
        // Draw line if on screen
        if (p1.x >= 0 && p1.x < config->screen_width && p1.y >= 0 && p1.y < config->screen_height &&
            p2.x >= 0 && p2.x < config->screen_width && p2.y >= 0 && p2.y < config->screen_height) {
            SDL_RenderDrawLine(config->renderer, p1.x, p1.y, p2.x, p2.y);
        }
    }
}

void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config) {
    if (!world || entity_id == INVALID_ENTITY || !config) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Renderable* renderable = entity_get_renderable(world, entity_id);
    
    if (!transform || !renderable || !renderable->visible) return;
    
    // Get visual type and corresponding mesh
    VisualType visual_type = get_entity_visual_type(world, entity_id);
    const SimpleMesh* mesh = &config->meshes[visual_type];
    
    // Set color based on entity type
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
    if (transform) {
        // Simple follow: stay at a fixed distance behind and above the entity
        Vector3 offset = {0, 50, distance};
        
        Vector3 target_pos = {
            transform->position.x + offset.x,
            transform->position.y + offset.y,
            transform->position.z + offset.z
        };
        
        // Smooth camera movement
        float lerp = 0.05f;
        camera->position.x += (target_pos.x - camera->position.x) * lerp;
        camera->position.y += (target_pos.y - camera->position.y) * lerp;
        camera->position.z += (target_pos.z - camera->position.z) * lerp;
        
        // Always look at the entity
        camera->target = transform->position;
    }
}

// ============================================================================
// MAIN RENDER FRAME FUNCTION
// ============================================================================

void render_frame(struct World* world, RenderConfig* config, float delta_time) {
    if (!world || !config) return;
    
    // Handle SDL events (window close, etc.)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            // In a real game, you'd set a quit flag
            printf("🚪 Window close requested\n");
        }
    }
    
    // Clear the screen
    render_clear_screen(config);
    
    // Render all visible entities
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if ((entity->component_mask & COMPONENT_RENDERABLE) &&
            (entity->component_mask & COMPONENT_TRANSFORM)) {
            
            EntityID entity_id = i + 1;  // Entity IDs are 1-based
            render_entity_3d(world, entity_id, config);
        }
    }
    
    // Render debug information
    render_debug_info(world, config);
    
    // Present the frame
    render_present(config);
}
