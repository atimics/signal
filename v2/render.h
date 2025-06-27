#ifndef RENDER_H
#define RENDER_H

#include "core.h"
#include "assets.h"
#include <stdint.h>
#include <SDL.h>

// ============================================================================
// RENDER SYSTEM DEFINITIONS
// ============================================================================

// 3D Camera for the solar system view
typedef struct {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fov;
    float near_plane;
    float far_plane;
    float aspect_ratio;
} Camera3D;

// Render modes
typedef enum {
    RENDER_MODE_WIREFRAME,
    RENDER_MODE_SOLID,
    RENDER_MODE_TEXTURED
} RenderMode;

// Entity visual types
typedef enum {
    VISUAL_TYPE_PLAYER,
    VISUAL_TYPE_SUN,
    VISUAL_TYPE_PLANET,
    VISUAL_TYPE_ASTEROID,
    VISUAL_TYPE_AI_SHIP,
    VISUAL_TYPE_COUNT
} VisualType;

// Simple mesh data
typedef struct {
    Vector3* vertices;
    uint32_t vertex_count;
    uint32_t* indices;
    uint32_t index_count;
} SimpleMesh;

// Render configuration
typedef struct {
    // SDL/Graphics
    SDL_Window* window;
    SDL_Renderer* renderer;
    int screen_width;
    int screen_height;
    
    // Asset system
    AssetRegistry* assets;
    
    // Render settings
    RenderMode mode;
    Camera3D camera;
    bool show_debug_info;
    bool show_velocities;
    bool show_collision_bounds;
    bool show_orbits;
    
    // Timing
    float update_interval;
    float last_update;
    uint32_t frame_count;
} RenderConfig;

// 2D point for projection
typedef struct {
    int x, y;
} Point2D;

// ============================================================================
// RENDER SYSTEM API
// ============================================================================

// Initialize the render system
bool render_init(RenderConfig* config, AssetRegistry* assets, float viewport_width, float viewport_height);

// Cleanup render system
void render_cleanup(RenderConfig* config);

// Main render function - called by systems.c
void render_frame(struct World* world, RenderConfig* config, float delta_time);

// Camera controls
void camera_set_position(Camera3D* camera, Vector3 position);
void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up);
void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance);

// 3D to 2D projection
Point2D project_3d_to_2d(Vector3 world_pos, const Camera3D* camera, int screen_width, int screen_height);

// Get visual type for an entity
VisualType get_entity_visual_type(struct World* world, EntityID entity_id);

// Rendering functions
void render_clear_screen(RenderConfig* config);
void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config);
void render_wireframe_mesh(const Mesh* mesh, Vector3 position, Vector3 rotation, 
                          Vector3 scale, RenderConfig* config, uint8_t r, uint8_t g, uint8_t b);
void render_debug_info(struct World* world, RenderConfig* config);
void render_present(RenderConfig* config);

#endif // RENDER_H
