#ifndef MATERIAL_SYSTEM_H
#define MATERIAL_SYSTEM_H

#include "../core.h"
#include <stdint.h>

// Material registry configuration
#define MAX_MATERIAL_REGISTRY 64

// Vector3 type for consistency
typedef struct {
    float x, y, z;
} float3;

// Material properties
typedef struct {
    // Base material properties
    float metallic;         // 0.0 = dielectric, 1.0 = metallic
    float roughness;        // 0.0 = mirror, 1.0 = completely rough
    float3 albedo;          // Base color (RGB)
    float3 emissive;        // Self-illumination color
    float emissive_strength; // Emissive intensity multiplier
    
    // Special effects
    float glow_intensity;   // For special glow effects
    bool is_emissive;       // Whether material should emit light
    bool ground_effect;     // Whether to apply ground effect visuals
    
    // Lighting interaction
    float ambient_factor;   // How much ambient light to receive
    float specular_factor;  // Specular reflection strength
} MaterialProperties;

// Predefined material types
typedef enum {
    MATERIAL_TYPE_DEFAULT,
    MATERIAL_TYPE_METAL,
    MATERIAL_TYPE_PLASTIC,
    MATERIAL_TYPE_EMISSIVE,  // Self-illuminating materials
    MATERIAL_TYPE_RACING_SHIP, // Special racing ships with ground effects
    MATERIAL_TYPE_LOGO_GLOW,  // Special logo cube material
    MATERIAL_TYPE_COUNT
} MaterialType;

// Material system functions
void material_system_init(void);
void material_system_shutdown(void);

// Material creation and management
MaterialProperties* material_create(MaterialType type);
MaterialProperties* material_get_by_name(const char* name);
MaterialProperties* material_get_by_id(uint32_t material_id);
void material_set_properties(MaterialProperties* material, MaterialType type);

// Predefined material configurations
MaterialProperties material_get_default(void);
MaterialProperties material_get_metal(float3 color, float roughness);
MaterialProperties material_get_plastic(float3 color, float roughness);
MaterialProperties material_get_emissive(float3 color, float strength);
MaterialProperties material_get_racing_ship(float3 color);
MaterialProperties material_get_logo_glow(void);

// Material property utilities
void material_set_albedo(MaterialProperties* material, float r, float g, float b);
void material_set_emissive(MaterialProperties* material, float r, float g, float b, float strength);
void material_set_metallic_roughness(MaterialProperties* material, float metallic, float roughness);

#endif // MATERIAL_SYSTEM_H
