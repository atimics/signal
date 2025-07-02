#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

#include "../core.h"
#include "material.h"
#include <stdint.h>

// Use the float3 type from material.h
// typedef struct { float x, y, z; } float3; // Already defined in material.h

// Light types
typedef enum {
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_SPOT,
    LIGHT_TYPE_AMBIENT
} LightType;

// Light structure
typedef struct {
    LightType type;
    float3 position;      // For point/spot lights
    float3 direction;     // For directional/spot lights
    float3 color;         // RGB color
    float intensity;      // Light intensity
    float range;          // For point/spot lights
    float spot_angle;     // For spot lights (in radians)
    bool enabled;
} Light;

// Lighting system state
typedef struct {
    Light lights[16];     // Maximum 16 lights
    uint32_t light_count;
    float3 ambient_color;
    float ambient_intensity;
    
    // Advanced lighting settings
    bool pbr_enabled;     // Physically Based Rendering
    bool ground_lighting; // Special ground effect lighting
    float time;           // For animated lighting effects
} LightingSystem;

// Global lighting system
extern LightingSystem g_lighting;

// Lighting system functions
void lighting_system_init(void);
void lighting_system_shutdown(void);
void lighting_system_update(float delta_time);

// Light management
uint32_t lighting_add_directional(float3 direction, float3 color, float intensity);
uint32_t lighting_add_point(float3 position, float3 color, float intensity, float range);
uint32_t lighting_add_spot(float3 position, float3 direction, float3 color, float intensity, float range, float angle);
void lighting_remove(uint32_t light_id);
void lighting_set_enabled(uint32_t light_id, bool enabled);

// Ambient lighting
void lighting_set_ambient(float3 color, float intensity);

// Advanced lighting features
void lighting_enable_pbr(bool enable);
void lighting_enable_ground_effects(bool enable);

// Lighting calculation for materials
float3 lighting_calculate_material(const MaterialProperties* material, float3 world_pos, float3 normal, float3 view_dir);
float lighting_calculate_ground_effect(float altitude);

// Preset lighting configurations
void lighting_setup_outdoor_scene(void);
void lighting_setup_racing_scene(void);
void lighting_setup_logo_scene(void);

// System scheduler compatibility wrapper
void lighting_system_update_wrapper(struct World* world, RenderConfig* render_config, float delta_time);

#endif // LIGHTING_SYSTEM_H
