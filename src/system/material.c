#include "material.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Material registry for named materials
#define MAX_MATERIAL_REGISTRY 64
static MaterialProperties material_registry[MAX_MATERIAL_REGISTRY];
static char material_names[MAX_MATERIAL_REGISTRY][32];
static uint32_t material_count = 0;

void material_system_init(void) {
    material_count = 0;
    memset(material_registry, 0, sizeof(material_registry));
    memset(material_names, 0, sizeof(material_names));
    
    // Create default materials
    material_create(MATERIAL_TYPE_DEFAULT);
    strcpy(material_names[material_count - 1], "default");
    
    material_create(MATERIAL_TYPE_METAL);
    strcpy(material_names[material_count - 1], "metal");
    
    material_create(MATERIAL_TYPE_RACING_SHIP);
    strcpy(material_names[material_count - 1], "racing_ship");
    
    material_create(MATERIAL_TYPE_LOGO_GLOW);
    strcpy(material_names[material_count - 1], "logo_glow");
    
    printf("✅ Material system initialized with %d default materials\n", material_count);
}

void material_system_shutdown(void) {
    material_count = 0;
    printf("🔧 Material system shutdown\n");
}

MaterialProperties* material_create(MaterialType type) {
    if (material_count >= MAX_MATERIAL_REGISTRY) {
        printf("❌ Material registry full!\n");
        return NULL;
    }
    
    MaterialProperties* material = &material_registry[material_count];
    material_set_properties(material, type);
    material_count++;
    
    return material;
}

MaterialProperties* material_get_by_name(const char* name) {
    for (uint32_t i = 0; i < material_count; i++) {
        if (strcmp(material_names[i], name) == 0) {
            return &material_registry[i];
        }
    }
    return NULL;
}

MaterialProperties* material_get_by_id(uint32_t material_id) {
    if (material_id >= material_count) {
        return NULL;
    }
    return &material_registry[material_id];
}

void material_set_properties(MaterialProperties* material, MaterialType type) {
    // Initialize with defaults
    material->metallic = 0.0f;
    material->roughness = 0.5f;
    material->albedo = (float3){0.8f, 0.8f, 0.8f};
    material->emissive = (float3){0.0f, 0.0f, 0.0f};
    material->emissive_strength = 0.0f;
    material->glow_intensity = 0.0f;
    material->is_emissive = false;
    material->ground_effect = false;
    material->ambient_factor = 0.3f;
    material->specular_factor = 0.5f;
    
    // Configure based on type
    switch (type) {
        case MATERIAL_TYPE_DEFAULT:
            // Already set to defaults
            break;
            
        case MATERIAL_TYPE_METAL:
            material->metallic = 1.0f;
            material->roughness = 0.2f;
            material->albedo = (float3){0.7f, 0.7f, 0.8f};
            material->specular_factor = 0.9f;
            break;
            
        case MATERIAL_TYPE_PLASTIC:
            material->metallic = 0.0f;
            material->roughness = 0.7f;
            material->albedo = (float3){0.8f, 0.2f, 0.2f};
            material->specular_factor = 0.3f;
            break;
            
        case MATERIAL_TYPE_EMISSIVE:
            material->is_emissive = true;
            material->emissive = (float3){1.0f, 0.8f, 0.6f};
            material->emissive_strength = 2.0f;
            material->ambient_factor = 0.1f; // Emissive materials need less ambient
            break;
            
        case MATERIAL_TYPE_RACING_SHIP:
            material->metallic = 0.8f;
            material->roughness = 0.3f;
            material->albedo = (float3){0.9f, 0.4f, 0.1f}; // Orange racing colors
            material->ground_effect = true;
            material->specular_factor = 0.8f;
            break;
            
        case MATERIAL_TYPE_LOGO_GLOW:
            material->is_emissive = true;
            material->emissive = (float3){1.0f, 0.9f, 0.7f}; // Warm white glow
            material->emissive_strength = 1.5f;
            material->glow_intensity = 1.0f; // Special glow effect
            material->albedo = (float3){1.0f, 1.0f, 1.0f}; // White base
            material->ambient_factor = 0.1f;
            break;
            
        default:
            break;
    }
}

MaterialProperties material_get_default(void) {
    MaterialProperties mat;
    material_set_properties(&mat, MATERIAL_TYPE_DEFAULT);
    return mat;
}

MaterialProperties material_get_metal(float3 color, float roughness) {
    MaterialProperties mat;
    material_set_properties(&mat, MATERIAL_TYPE_METAL);
    mat.albedo = color;
    mat.roughness = roughness;
    return mat;
}

MaterialProperties material_get_plastic(float3 color, float roughness) {
    MaterialProperties mat;
    material_set_properties(&mat, MATERIAL_TYPE_PLASTIC);
    mat.albedo = color;
    mat.roughness = roughness;
    return mat;
}

MaterialProperties material_get_emissive(float3 color, float strength) {
    MaterialProperties mat;
    material_set_properties(&mat, MATERIAL_TYPE_EMISSIVE);
    mat.emissive = color;
    mat.emissive_strength = strength;
    return mat;
}

MaterialProperties material_get_racing_ship(float3 color) {
    MaterialProperties mat;
    material_set_properties(&mat, MATERIAL_TYPE_RACING_SHIP);
    mat.albedo = color;
    return mat;
}

MaterialProperties material_get_logo_glow(void) {
    MaterialProperties mat;
    material_set_properties(&mat, MATERIAL_TYPE_LOGO_GLOW);
    return mat;
}

void material_set_albedo(MaterialProperties* material, float r, float g, float b) {
    material->albedo = (float3){r, g, b};
}

void material_set_emissive(MaterialProperties* material, float r, float g, float b, float strength) {
    material->emissive = (float3){r, g, b};
    material->emissive_strength = strength;
    material->is_emissive = (strength > 0.0f);
}

void material_set_metallic_roughness(MaterialProperties* material, float metallic, float roughness) {
    material->metallic = metallic;
    material->roughness = roughness;
}
