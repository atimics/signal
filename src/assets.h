#ifndef ASSETS_H
#define ASSETS_H

#include "core.h"
#include "gpu_resources.h"
#include <stdint.h>
#include <stdbool.h>

// Forward-declare the opaque structs. The header knows nothing about them.
struct MeshGpuResources;
struct TextureGpuResources;

// Asset registry constants
#define MAX_MESHES 32
#define MAX_TEXTURES 32
#define MAX_MATERIALS 32

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
    Vector3 aabb_min;   // Axis-aligned bounding box minimum
    Vector3 aabb_max;   // Axis-aligned bounding box maximum
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
    Mesh meshes[MAX_MESHES];        // Max meshes
    Texture textures[MAX_TEXTURES]; // Max textures  
    Material materials[MAX_MATERIALS]; // Max materials
    
    uint32_t mesh_count;
    uint32_t texture_count;
    uint32_t material_count;
    
    char asset_root[256];   // Root directory for assets
} AssetRegistry;

// ============================================================================
// ASSET SYSTEM API
// ============================================================================

/**
 * @brief Initialize the asset system
 * @param registry The asset registry to initialize
 * @param asset_root Path to the root directory containing assets
 * @return true if initialization successful, false otherwise
 */
bool assets_init(AssetRegistry* registry, const char* asset_root);

/**
 * @brief Cleanup the asset system and free all resources
 * @param registry The asset registry to cleanup
 */
void assets_cleanup(AssetRegistry* registry);

/**
 * @brief Load a material from material definition
 * @param registry The asset registry to load into
 * @param material Name or path of the material to load
 * @return true if material loaded successfully, false otherwise
 */
bool load_material(AssetRegistry* registry, const char* material);

/**
 * @brief Load a texture from file
 * @param registry The asset registry to load into
 * @param texture Path to the texture file
 * @param texture_name Name to assign to the loaded texture
 * @return true if texture loaded successfully, false otherwise
 */
bool load_texture(AssetRegistry* registry, const char* texture, const char* texture_name);

/**
 * @brief Load a mesh from an absolute file path
 * @param registry The asset registry to load into
 * @param absolute_filepath Full path to the mesh file
 * @param mesh_name Name to assign to the loaded mesh
 * @return true if mesh loaded successfully, false otherwise
 */
bool load_mesh_from_file(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name);

/**
 * @brief Get mesh asset path from index file
 * @param index_path Path to the index.json file
 * @param asset_name Name of the asset to find
 * @param out_path Buffer to store the resolved path
 * @param out_size Size of the output buffer
 * @return true if asset was found and path resolved, false otherwise
 */
bool assets_get_mesh_path_from_index(const char* index_path, const char* asset_name, char* out_path, size_t out_size);

/**
 * @brief Load a mesh from a binary .cobj file format
 * @param registry The asset registry to load into
 * @param absolute_filepath Full path to the .cobj file
 * @param mesh_name Name to assign to the loaded mesh
 * @return true if mesh loaded successfully, false otherwise
 */
bool load_cobj_binary(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name);

/**
 * @brief Get a mesh by name from the asset registry
 * @param registry The asset registry to search
 * @param name Name of the mesh to find
 * @return Pointer to the mesh if found, NULL otherwise
 */
Mesh* assets_get_mesh(AssetRegistry* registry, const char* name);

/**
 * @brief Get a material by name from the asset registry
 * @param registry The asset registry to search
 * @param name Name of the material to find
 * @return Pointer to the material if found, NULL otherwise
 */
Material* assets_get_material(AssetRegistry* registry, const char* name);

/**
 * @brief Get a texture by name from the asset registry
 * @param registry The asset registry to search
 * @param name Name of the texture to find
 * @return Pointer to the texture if found, NULL otherwise
 */
Texture* assets_get_texture(AssetRegistry* registry, const char* name);

/**
 * @brief List all loaded assets to console for debugging
 * @param registry The asset registry to list
 */
void assets_list_loaded(AssetRegistry* registry);

/**
 * @brief Load all assets from the default asset directory
 * @param registry The asset registry to load into
 * @return true if all assets loaded successfully, false otherwise
 */
bool assets_load_all_in_directory(AssetRegistry* registry);

/**
 * @brief Get GPU buffer handles for a mesh
 * @param mesh The mesh to get buffers for
 * @param out_vbuf Output vertex buffer handle
 * @param out_ibuf Output index buffer handle
 */
void mesh_get_gpu_buffers(const Mesh* mesh, void* out_vbuf, void* out_ibuf);

/**
 * @brief Initialize GPU resources for all loaded assets
 * @param registry The asset registry containing assets to initialize
 * @return true if all GPU resources initialized successfully, false otherwise
 */
bool assets_initialize_gpu_resources(AssetRegistry* registry);

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
// Note: mesh_get_gpu_buffers is available in the main API with void* parameters
// Tests should cast to sg_buffer* as needed
void texture_get_gpu_image(const Texture* texture, sg_image* out_image);
#endif

// ============================================================================
// BINARY ASSET FORMAT (Sprint 12 - Task 2)
// ============================================================================

// Binary .cobj file header for high-performance asset loading
typedef struct {
    char magic[4];           // "COBJ" magic number
    uint32_t version;        // Format version (1)
    uint32_t vertex_count;   // Number of vertices
    uint32_t index_count;    // Number of indices
    Vector3 aabb_min;        // Bounding box minimum
    Vector3 aabb_max;        // Bounding box maximum
    uint32_t reserved[4];    // Reserved for future use
} COBJHeader;

// Enhanced vertex structure with tangent data for advanced lighting
typedef struct {
    Vector3 position;        // Vertex position
    Vector3 normal;          // Surface normal
    Vector2 tex_coord;       // Texture coordinates
    Vector3 tangent;         // Tangent for normal mapping
    float padding;           // Padding to 48 bytes total
} VertexEnhanced;

#endif // ASSETS_H
