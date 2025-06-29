#ifndef RENDER_LIGHTING_H
#define RENDER_LIGHTING_H

#include "core.h"
#include "render.h"

// ============================================================================
// LIGHTING SYSTEM API
// ============================================================================

// Lighting system management
void lighting_init(LightingSystem* lighting);
void lighting_add_directional_light(LightingSystem* lighting, Vector3 direction, Vector3 color, float intensity);
void lighting_add_point_light(LightingSystem* lighting, Vector3 position, Vector3 color, float intensity);
void lighting_set_ambient(LightingSystem* lighting, Vector3 color, float intensity);

// Lighting calculations
Vector3 calculate_lighting(Vector3 surface_pos, Vector3 surface_normal, Vector3 material_color, LightingSystem* lighting);

#endif // RENDER_LIGHTING_H
