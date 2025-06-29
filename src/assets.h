#ifndef ASSETS_H
#define ASSETS_H

#include "core.h"
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

// ============================================================================
// ASSET DEFINITIONS
// ============================================================================

// A single vertex with all its attributes
typedef struct {
    Vector3 position;
    Vector3 normal;
    Vector2 tex_coord;
} Vertex;

// Mesh structure with all necessary data for rendering
typedef struct {
    char name[64];
    char material_name[64];  // Material name from usemtl directive
    Vertex* vertices; // Unified vertex data
    int vertex_count;
    int* indices;
    int index_count;
    bool loaded;
} Mesh;

// Forward declare SDL_Texture for header
struct SDL_Texture;
struct SDL_Renderer;

// Texture data with SDL support
typedef struct {
    char name[64];
    char filepath[256];
    uint32_t width, height;
    struct SDL_Texture* sdl_texture;  // SDL texture for rendering
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
bool load_compiled_mesh(AssetRegistry* registry, const char* filename, const char* mesh_name);
bool load_material(AssetRegistry* registry, const char* filename);
bool load_texture(AssetRegistry* registry, const char* filename, const char* texture_name, struct SDL_Renderer* renderer);

// Asset lookup
Mesh* assets_get_mesh(AssetRegistry* registry, const char* name);
Material* assets_get_material(AssetRegistry* registry, const char* name);
Texture* assets_get_texture(AssetRegistry* registry, const char* name);

// Utility functions
void assets_list_loaded(AssetRegistry* registry);
bool assets_load_all_in_directory(AssetRegistry* registry, struct SDL_Renderer* renderer);
bool load_assets_from_metadata(AssetRegistry* registry, struct SDL_Renderer* renderer);
bool load_legacy_metadata(AssetRegistry* registry, struct SDL_Renderer* renderer);
bool load_single_mesh_metadata(AssetRegistry* registry, struct SDL_Renderer* renderer, const char* metadata_path);

// OBJ file parsing helpers
bool parse_obj_file(const char* filepath, Mesh* mesh);
bool parse_mtl_file(const char* filepath, AssetRegistry* registry);

#endif // ASSETS_H
