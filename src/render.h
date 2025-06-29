#ifndef RENDER_H
#define RENDER_H

#include "core.h"
#include "assets.h"
#include "ui.h"
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

// Light types
typedef enum {
    LIGHT_TYPE_DIRECTIONAL,  // Sun-like light (parallel rays)
    LIGHT_TYPE_POINT,        // Point light source
    LIGHT_TYPE_AMBIENT       // Global ambient light
} LightType;

// Light structure
typedef struct {
    LightType type;
    Vector3 position;        // For point lights
    Vector3 direction;       // For directional lights
    Vector3 color;           // RGB color (0.0-1.0)
    float intensity;         // Light intensity multiplier
    bool enabled;
} Light;

// Lighting system
typedef struct {
    Light lights[8];         // Max 8 lights
    uint32_t light_count;
    Vector3 ambient_color;   // Global ambient light
    float ambient_intensity;
} LightingSystem;

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
    
    // UI system
    CockpitUI ui;
    
    // Lighting system
    LightingSystem lighting;
    
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
void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time);

// Camera controls
void camera_set_position(Camera3D* camera, Vector3 position);
void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up);
void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance);

// Lighting controls
void lighting_init(LightingSystem* lighting);
void lighting_add_directional_light(LightingSystem* lighting, Vector3 direction, Vector3 color, float intensity);
void lighting_add_point_light(LightingSystem* lighting, Vector3 position, Vector3 color, float intensity);
void lighting_set_ambient(LightingSystem* lighting, Vector3 color, float intensity);

// Textured mesh rendering
Vector3 calculate_lighting(Vector3 surface_pos, Vector3 surface_normal, Vector3 material_color, LightingSystem* lighting);
void render_textured_triangle(SDL_Renderer* renderer, Vector3 v1, Vector3 v2, Vector3 v3, 
                             Vector3 n1, Vector3 n2, Vector3 n3, SDL_Texture* texture,
                             Camera3D* camera, LightingSystem* lighting, int screen_width, int screen_height);

// Enhanced mesh rendering with lighting
void render_solid_mesh_with_lighting(Mesh* mesh, struct Transform* transform, Material* material, RenderConfig* config);
void render_textured_mesh_with_lighting(Mesh* mesh, struct Transform* transform, Texture* texture, RenderConfig* config);
Vector3 transform_vertex(Vector3 vertex, struct Transform* transform);

// Triangle rasterization
void render_filled_triangle(SDL_Renderer* renderer, Point2D p1, Point2D p2, Point2D p3, Uint8 r, Uint8 g, Uint8 b);

// 3D to 2D projection
Point2D project_3d_to_2d(Vector3 world_pos, const Camera3D* camera, int screen_width, int screen_height);

// Get visual type for an entity
VisualType get_entity_visual_type(struct World* world, EntityID entity_id);

// UI access for game events
void render_add_comm_message(RenderConfig* config, const char* sender, const char* message, bool is_player);

// Rendering functions
void render_clear_screen(RenderConfig* config);
void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config);
void render_wireframe_mesh(const Mesh* mesh, Vector3 position, Vector3 rotation, 
                          Vector3 scale, RenderConfig* config, uint8_t r, uint8_t g, uint8_t b);
void render_debug_info(struct World* world, RenderConfig* config);
void render_present(RenderConfig* config);

// Screenshot functionality
bool render_take_screenshot(RenderConfig* config, const char* filename);
bool render_take_screenshot_from_position(struct World* world, RenderConfig* config, 
                                         Vector3 camera_pos, Vector3 look_at_pos, 
                                         const char* filename);

#endif // RENDER_H
