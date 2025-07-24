#include "lighting.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Global lighting system instance
LightingSystem g_lighting;

void lighting_system_init(void) {
    memset(&g_lighting, 0, sizeof(LightingSystem));
    
    // Default ambient lighting - made brighter
    g_lighting.ambient_color = (float3){0.4f, 0.5f, 0.6f};  // Brighter blueish ambient
    g_lighting.ambient_intensity = 0.6f;  // Doubled ambient intensity for brighter scenes
    g_lighting.pbr_enabled = true;
    g_lighting.ground_lighting = true;
    g_lighting.time = 0.0f;
    
    printf("✅ Lighting system initialized\n");
}

void lighting_system_shutdown(void) {
    g_lighting.light_count = 0;
    printf("🔧 Lighting system shutdown\n");
}

void lighting_system_update(float delta_time) {
    g_lighting.time += delta_time;
    
    // Update any animated lighting effects here
    // For example, pulsing lights, day/night cycles, etc.
}

uint32_t lighting_add_directional(float3 direction, float3 color, float intensity) {
    if (g_lighting.light_count >= 16) {
        printf("❌ Maximum lights reached!\n");
        return UINT32_MAX;
    }
    
    uint32_t id = g_lighting.light_count++;
    Light* light = &g_lighting.lights[id];
    
    light->type = LIGHT_TYPE_DIRECTIONAL;
    light->direction = direction;
    light->color = color;
    light->intensity = intensity;
    light->enabled = true;
    
    printf("💡 Added directional light %d: dir(%.2f,%.2f,%.2f) color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           id, direction.x, direction.y, direction.z, color.x, color.y, color.z, intensity);
    
    return id;
}

uint32_t lighting_add_point(float3 position, float3 color, float intensity, float range) {
    if (g_lighting.light_count >= 16) {
        printf("❌ Maximum lights reached!\n");
        return UINT32_MAX;
    }
    
    uint32_t id = g_lighting.light_count++;
    Light* light = &g_lighting.lights[id];
    
    light->type = LIGHT_TYPE_POINT;
    light->position = position;
    light->color = color;
    light->intensity = intensity;
    light->range = range;
    light->enabled = true;
    
    printf("💡 Added point light %d: pos(%.2f,%.2f,%.2f) range=%.2f\n",
           id, position.x, position.y, position.z, range);
    
    return id;
}

uint32_t lighting_add_spot(float3 position, float3 direction, float3 color, float intensity, float range, float angle) {
    if (g_lighting.light_count >= 16) {
        printf("❌ Maximum lights reached!\n");
        return UINT32_MAX;
    }
    
    uint32_t id = g_lighting.light_count++;
    Light* light = &g_lighting.lights[id];
    
    light->type = LIGHT_TYPE_SPOT;
    light->position = position;
    light->direction = direction;
    light->color = color;
    light->intensity = intensity;
    light->range = range;
    light->spot_angle = angle;
    light->enabled = true;
    
    printf("💡 Added spot light %d: pos(%.2f,%.2f,%.2f) angle=%.2f\n",
           id, position.x, position.y, position.z, angle);
    
    return id;
}

void lighting_remove(uint32_t light_id) {
    if (light_id >= g_lighting.light_count) return;
    g_lighting.lights[light_id].enabled = false;
}

void lighting_set_enabled(uint32_t light_id, bool enabled) {
    if (light_id >= g_lighting.light_count) return;
    g_lighting.lights[light_id].enabled = enabled;
}

void lighting_set_ambient(float3 color, float intensity) {
    g_lighting.ambient_color = color;
    g_lighting.ambient_intensity = intensity;
    printf("💡 Set ambient light: color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           color.x, color.y, color.z, intensity);
}

void lighting_enable_pbr(bool enable) {
    g_lighting.pbr_enabled = enable;
    printf("🔧 PBR lighting %s\n", enable ? "enabled" : "disabled");
}

void lighting_enable_ground_effects(bool enable) {
    g_lighting.ground_lighting = enable;
    printf("🔧 Ground effect lighting %s\n", enable ? "enabled" : "disabled");
}

float3 lighting_calculate_material(const MaterialProperties* material, float3 world_pos, float3 normal, float3 view_dir) {
    // This is a simplified PBR-style calculation
    // In a full implementation, this would be much more complex
    
    float3 final_color = {0.0f, 0.0f, 0.0f};
    
    // Add ambient contribution
    float3 ambient = {
        g_lighting.ambient_color.x * g_lighting.ambient_intensity * material->ambient_factor,
        g_lighting.ambient_color.y * g_lighting.ambient_intensity * material->ambient_factor,
        g_lighting.ambient_color.z * g_lighting.ambient_intensity * material->ambient_factor
    };
    final_color.x += ambient.x * material->albedo.x;
    final_color.y += ambient.y * material->albedo.y;
    final_color.z += ambient.z * material->albedo.z;
    
    // Add emissive contribution
    if (material->is_emissive) {
        final_color.x += material->emissive.x * material->emissive_strength;
        final_color.y += material->emissive.y * material->emissive_strength;
        final_color.z += material->emissive.z * material->emissive_strength;
    }
    
    // Process each light
    for (uint32_t i = 0; i < g_lighting.light_count; i++) {
        const Light* light = &g_lighting.lights[i];
        if (!light->enabled) continue;
        
        float3 light_dir = {0.0f, 0.0f, 0.0f};
        float attenuation = 1.0f;
        
        switch (light->type) {
            case LIGHT_TYPE_DIRECTIONAL:
                light_dir = light->direction;
                break;
                
            case LIGHT_TYPE_POINT: {
                float3 to_light = {
                    light->position.x - world_pos.x,
                    light->position.y - world_pos.y,
                    light->position.z - world_pos.z
                };
                float distance = sqrtf(to_light.x * to_light.x + to_light.y * to_light.y + to_light.z * to_light.z);
                if (distance > light->range) continue;
                
                light_dir.x = to_light.x / distance;
                light_dir.y = to_light.y / distance;
                light_dir.z = to_light.z / distance;
                attenuation = 1.0f - (distance / light->range);
                break;
            }
            
            default:
                continue;
        }
        
        // Calculate diffuse lighting
        float ndotl = fmaxf(0.0f, normal.x * (-light_dir.x) + normal.y * (-light_dir.y) + normal.z * (-light_dir.z));
        
        float3 diffuse = {
            light->color.x * light->intensity * ndotl * attenuation,
            light->color.y * light->intensity * ndotl * attenuation,
            light->color.z * light->intensity * ndotl * attenuation
        };
        
        final_color.x += diffuse.x * material->albedo.x;
        final_color.y += diffuse.y * material->albedo.y;
        final_color.z += diffuse.z * material->albedo.z;
    }
    
    return final_color;
}

float lighting_calculate_ground_effect(float altitude) {
    if (!g_lighting.ground_lighting || altitude > 50.0f) return 1.0f;
    
    // Ground effect lighting boost
    return 1.0f + (50.0f - altitude) / 50.0f * 0.5f; // Up to 1.5x lighting near ground
}

void lighting_setup_outdoor_scene(void) {
    // Clear existing lights
    g_lighting.light_count = 0;
    
    // Sun light - made brighter
    lighting_add_directional((float3){0.3f, -0.7f, 0.2f}, (float3){1.0f, 0.95f, 0.8f}, 1.2f);
    
    // Ambient sky light - much brighter
    lighting_set_ambient((float3){0.5f, 0.7f, 0.9f}, 0.5f);
    
    printf("💡 Outdoor scene lighting configured\n");
}

void lighting_setup_racing_scene(void) {
    // Clear existing lights
    g_lighting.light_count = 0;
    
    // Strong sun for racing visibility - made even brighter
    lighting_add_directional((float3){0.2f, -0.8f, 0.3f}, (float3){1.0f, 0.9f, 0.7f}, 1.4f);
    
    // Additional fill light - brighter
    lighting_add_directional((float3){-0.4f, -0.3f, -0.2f}, (float3){0.6f, 0.7f, 0.9f}, 0.5f);
    
    // Warm ambient for racing atmosphere - increased 
    lighting_set_ambient((float3){0.6f, 0.5f, 0.4f}, 0.4f);
    
    // Enable ground effects for racing ships
    lighting_enable_ground_effects(true);
    
    printf("🏁 Racing scene lighting configured\n");
}

void lighting_setup_logo_scene(void) {
    // Clear existing lights
    g_lighting.light_count = 0;
    
    // Dramatic directional light - made brighter
    lighting_add_directional((float3){0.3f, -0.7f, 0.2f}, (float3){0.6f, 0.7f, 0.8f}, 0.8f);
    
    // Cool ambient - made brighter
    lighting_set_ambient((float3){0.3f, 0.4f, 0.5f}, 0.5f);
    
    printf("🎨 Logo scene lighting configured\n");
}

// System scheduler compatibility wrapper
void lighting_system_update_wrapper(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)world;         // Unused
    (void)render_config; // Unused
    lighting_system_update(delta_time);
}
