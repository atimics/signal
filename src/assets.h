#ifndef ASSETS_H
#define ASSETS_H

#include "core.h"
#include <stdint.h>

// ============================================================================
// ASSET SYSTEM
// ============================================================================

// Asset types
typedef enum {
    ASSET_TYPE_MESH,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MATERIAL
} AssetType;

// Mesh data structure (replaces SimpleMesh)
typedef struct {
    char name[64];
    Vector3* vertices;
    Vector3* normals;      // For future lighting
    uint32_t* indices;
    uint32_t vertex_count;
    uint32_t index_count;
    bool loaded;
} Mesh;

// Texture data (for future use)
typedef struct {
    char name[64];
    char filepath[256];
    uint32_t width, height;
    bool loaded;
} Texture;

// Material data
typedef struct {
    char name[64];
    Vector3 diffuse_color;  // RGB color
    Vector3 ambient_color;
    Vector3 specular_color;
    float shininess;
    char texture_name[64];  // Reference to texture
    bool loaded;
} Material;

// Asset registry
typedef struct {
    Mesh meshes[32];        // Max 32 meshes
    Texture textures[32];   // Max 32 textures  
    Material materials[32]; // Max 32 materials
    
    uint32_t mesh_count;
    uint32_t texture_count;
    uint32_t material_count;
    
    char asset_root[256];   // Root directory for assets
} AssetRegistry;

// ============================================================================
// ASSET SYSTEM API
// ============================================================================

// Initialize the asset system
bool assets_init(AssetRegistry* registry, const char* asset_root);

// Cleanup the asset system
void assets_cleanup(AssetRegistry* registry);

// Load individual assets
bool load_obj_mesh(AssetRegistry* registry, const char* filename, const char* mesh_name);
bool load_material(AssetRegistry* registry, const char* filename);
bool load_texture(AssetRegistry* registry, const char* filename, const char* texture_name);

// Asset lookup
Mesh* assets_get_mesh(AssetRegistry* registry, const char* name);
Material* assets_get_material(AssetRegistry* registry, const char* name);
Texture* assets_get_texture(AssetRegistry* registry, const char* name);

// Utility functions
void assets_list_loaded(AssetRegistry* registry);
bool assets_load_all_in_directory(AssetRegistry* registry);
bool load_assets_from_metadata(AssetRegistry* registry);

// OBJ file parsing helpers
bool parse_obj_file(const char* filepath, Mesh* mesh);
bool parse_mtl_file(const char* filepath, AssetRegistry* registry);

// Mesh generation (fallback for missing assets)
void generate_fallback_meshes(AssetRegistry* registry);

#endif // ASSETS_H
