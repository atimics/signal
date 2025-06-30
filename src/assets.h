#ifndef ASSETS_H
#define ASSETS_H

#include "core.h"
#include "gpu_resources.h"
#include <stdint.h>
#include <stdbool.h>

// Forward-declare the opaque structs. The header knows nothing about them.
struct MeshGpuResources;
struct TextureGpuResources;

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
    struct MeshGpuResources* gpu_resources; // Opaque pointer
} Mesh;

// Texture data with Sokol support
typedef struct {
    char name[64];
    char filepath[256];
    uint32_t width, height;
    bool loaded;
    struct TextureGpuResources* gpu_resources; // Opaque pointer
} Texture;

// Material data with multi-texture support
typedef struct {
    char name[64];
    
    // Basic material properties
    Vector3 diffuse_color;   // RGB color
    Vector3 ambient_color;
    Vector3 specular_color;
    Vector3 emission_color;  // For glowing materials
    float shininess;
    float roughness;         // Surface roughness (0.0 = mirror, 1.0 = rough)
    float metallic;          // Metallic factor (0.0 = dielectric, 1.0 = metallic)
    
    // Multi-texture support
    char diffuse_texture[64];   // Base color map (map_Kd)
    char normal_texture[64];    // Surface detail/normal map
    char specular_texture[64];  // Specular/reflectivity map
    char emission_texture[64];  // Glow/emission map
    
    // Legacy single texture support (for compatibility)
    char texture_name[64];      // Reference to primary texture
    
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
bool load_material(AssetRegistry* registry, const char* material);
bool load_texture(AssetRegistry* registry, const char* texture, const char* texture_name);

// Asset lookup
Mesh* assets_get_mesh(AssetRegistry* registry, const char* name);
Material* assets_get_material(AssetRegistry* registry, const char* name);
Texture* assets_get_texture(AssetRegistry* registry, const char* name);

// Path resolution from index.json
bool assets_get_mesh_path_from_index(const char* index_path, const char* asset_name, char* out_path, size_t out_size);

// Utility functions
void assets_list_loaded(AssetRegistry* registry);
bool assets_load_all_in_directory(AssetRegistry* registry);
bool load_assets_from_metadata(AssetRegistry* registry);
bool load_single_mesh_metadata(AssetRegistry* registry, const char* metadata_path);

// OBJ file parsing helpers
bool parse_obj_file(const char* filepath, Mesh* mesh);
bool parse_mtl_file(const char* filepath, AssetRegistry* registry);

// Mesh loading functions
bool load_mesh_from_file(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name);

// ============================================================================
// MATERIAL REPOSITORY FUNCTIONS
// ============================================================================

// Material management
bool materials_load_library(AssetRegistry* registry, const char* materials_dir);
Material* materials_get_by_name(AssetRegistry* registry, const char* name);
bool materials_create_variant(AssetRegistry* registry, const char* base_name, const char* variant_name, 
                             Vector3 color_tint, float roughness_modifier);
void materials_list_loaded(AssetRegistry* registry);

// Multi-texture material support
bool materials_load_texture_set(AssetRegistry* registry, Material* material, 
                               const char* texture_dir);
bool materials_bind_textures(Material* material);

// ============================================================================
// SHADER LOADING FUNCTIONS
// ============================================================================

// Load shader source from file
char* load_shader_source(const char* shader_path);

// Free shader source
void free_shader_source(char* source);

// Get platform-specific shader path
const char* get_shader_path(const char* base_name, const char* stage); // stage: "vert" or "frag"

// ============================================================================
// GPU RESOURCE CREATION FUNCTIONS
// ============================================================================

// Create GPU resources from loaded mesh data and populate Renderable component
bool assets_create_renderable_from_mesh(AssetRegistry* registry, const char* mesh_name, struct Renderable* renderable);

// Upload mesh data to GPU and validate before buffer creation
bool assets_upload_mesh_to_gpu(Mesh* mesh);

// Create GPU texture from loaded texture data  
gpu_image_t assets_create_gpu_texture(AssetRegistry* registry, const char* texture_name);

// Helper function to create a default white texture
gpu_image_t assets_create_default_texture(void);

// Load all loaded textures to GPU (batch operation)
bool assets_load_all_textures_to_gpu(AssetRegistry* registry);

// Initialize all GPU resources from loaded asset data
bool assets_initialize_gpu_resources(AssetRegistry* registry);

// Accessor functions for tests (Task 4: PIMPL compliance)
// These expose underlying GPU handles in a controlled way for testing
#ifdef CGAME_TESTING
#include "sokol_gfx.h"
void mesh_get_gpu_buffers(const Mesh* mesh, sg_buffer* out_vbuf, sg_buffer* out_ibuf);
void texture_get_gpu_image(const Texture* texture, sg_image* out_image);
#endif

#endif // ASSETS_H
