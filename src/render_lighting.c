#include "render_lighting.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// ============================================================================
// LIGHTING SYSTEM IMPLEMENTATION
// ============================================================================

void lighting_init(LightingSystem* lighting) {
    if (!lighting) return;
    
    memset(lighting, 0, sizeof(LightingSystem));
    
    // Set default ambient lighting
    lighting->ambient_color = (Vector3){0.2f, 0.2f, 0.3f};  // Slight blue tint
    lighting->ambient_intensity = 0.3f;
    
    // Add a default sun light
    lighting_add_directional_light(lighting, 
                                  (Vector3){-0.5f, -1.0f, -0.3f},  // From upper left
                                  (Vector3){1.0f, 0.95f, 0.8f},    // Warm white
                                  0.8f);                           // Strong intensity
    
    printf("💡 Lighting system initialized with %d lights\n", lighting->light_count);
}

void lighting_add_directional_light(LightingSystem* lighting, Vector3 direction, Vector3 color, float intensity) {
    if (!lighting || lighting->light_count >= 8) return;
    
    Light* light = &lighting->lights[lighting->light_count++];
    light->type = LIGHT_TYPE_DIRECTIONAL;
    light->direction = direction;
    light->color = color;
    light->intensity = intensity;
    light->enabled = true;
    
    // Normalize direction
    float len = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len > 0.001f) {
        light->direction.x /= len;
        light->direction.y /= len;
        light->direction.z /= len;
    }
    
    printf("💡 Added directional light: dir(%.2f,%.2f,%.2f) color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           direction.x, direction.y, direction.z, color.x, color.y, color.z, intensity);
}

void lighting_add_point_light(LightingSystem* lighting, Vector3 position, Vector3 color, float intensity) {
    if (!lighting || lighting->light_count >= 8) return;
    
    Light* light = &lighting->lights[lighting->light_count++];
    light->type = LIGHT_TYPE_POINT;
    light->position = position;
    light->color = color;
    light->intensity = intensity;
    light->enabled = true;
    
    printf("💡 Added point light: pos(%.1f,%.1f,%.1f) color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           position.x, position.y, position.z, color.x, color.y, color.z, intensity);
}

void lighting_set_ambient(LightingSystem* lighting, Vector3 color, float intensity) {
    if (!lighting) return;
    
    lighting->ambient_color = color;
    lighting->ambient_intensity = intensity;
    
    printf("💡 Set ambient light: color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           color.x, color.y, color.z, intensity);
}

Vector3 calculate_lighting(Vector3 surface_pos, Vector3 surface_normal, Vector3 material_color, LightingSystem* lighting) {
    if (!lighting) return material_color;
    
    // Start with ambient lighting
    Vector3 final_color = {
        material_color.x * lighting->ambient_color.x * lighting->ambient_intensity,
        material_color.y * lighting->ambient_color.y * lighting->ambient_intensity,
        material_color.z * lighting->ambient_color.z * lighting->ambient_intensity
    };
    
    // Normalize surface normal
    float normal_len = sqrtf(surface_normal.x * surface_normal.x + 
                            surface_normal.y * surface_normal.y + 
                            surface_normal.z * surface_normal.z);
    if (normal_len > 0.001f) {
        surface_normal.x /= normal_len;
        surface_normal.y /= normal_len;
        surface_normal.z /= normal_len;
    }
    
    // Process each light
    for (uint32_t i = 0; i < lighting->light_count; i++) {
        Light* light = &lighting->lights[i];
        if (!light->enabled) continue;
        
        Vector3 light_dir = {0, 0, 0};
        float attenuation = 1.0f;
        
        if (light->type == LIGHT_TYPE_DIRECTIONAL) {
            // Directional light - direction is constant
            light_dir = light->direction;
        } else if (light->type == LIGHT_TYPE_POINT) {
            // Point light - calculate direction from surface to light
            light_dir.x = light->position.x - surface_pos.x;
            light_dir.y = light->position.y - surface_pos.y;
            light_dir.z = light->position.z - surface_pos.z;
            
            // Calculate distance for attenuation
            float distance = sqrtf(light_dir.x * light_dir.x + 
                                  light_dir.y * light_dir.y + 
                                  light_dir.z * light_dir.z);
            
            if (distance > 0.001f) {
                light_dir.x /= distance;
                light_dir.y /= distance;
                light_dir.z /= distance;
                
                // Simple distance attenuation
                attenuation = 1.0f / (1.0f + 0.01f * distance + 0.001f * distance * distance);
            }
        }
        
        // Calculate dot product for diffuse lighting (Lambert's cosine law)
        float dot_product = -(surface_normal.x * light_dir.x + 
                             surface_normal.y * light_dir.y + 
                             surface_normal.z * light_dir.z);
        
        // Clamp to positive values (no negative lighting)
        if (dot_product > 0.0f) {
            float light_contribution = dot_product * light->intensity * attenuation;
            
            final_color.x += material_color.x * light->color.x * light_contribution;
            final_color.y += material_color.y * light->color.y * light_contribution;
            final_color.z += material_color.z * light->color.z * light_contribution;
        }
    }
    
    // Clamp final color to [0.0, 1.0] range
    if (final_color.x > 1.0f) final_color.x = 1.0f;
    if (final_color.y > 1.0f) final_color.y = 1.0f;
    if (final_color.z > 1.0f) final_color.z = 1.0f;
    
    return final_color;
}
