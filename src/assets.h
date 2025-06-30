/**
 * @file assets.h
 * @brief Defines the structures and API for managing all game assets.
 *
 * This file contains the definitions for core asset types such as Meshes,
 * Textures, and Materials, as well as the AssetRegistry that manages them.
 * It also declares the functions for loading, querying, and managing these assets.
 */

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

/**
 * @brief A single vertex with all its attributes for rendering.
 */
typedef struct {
    Vector3 position;   /**< 3D position of the vertex. */
    Vector3 normal;     /**< Surface normal vector for lighting. */
    Vector2 tex_coord;  /**< UV coordinates for texture mapping. */
} Vertex;

/**
 * @brief Represents a 3D mesh, including its geometry and material info.
 */
typedef struct {
    char name[64];              /**< Unique name of the mesh. */
    char material_name[64];     /**< Name of the material used by this mesh. */
    Vertex* vertices;           /**< Pointer to the vertex data on the CPU. */
    int vertex_count;           /**< Number of vertices in the mesh. */
    int* indices;               /**< Pointer to the index data on the CPU. */
    int index_count;            /**< Number of indices in the mesh. */
    Vector3 aabb_min;           /**< The minimum corner of the mesh's axis-aligned bounding box. */
    Vector3 aabb_max;           /**< The maximum corner of the mesh's axis-aligned bounding box. */
    bool loaded;                /**< Flag indicating if the mesh data is loaded. */
    struct MeshGpuResources* gpu_resources; /**< Opaque pointer to GPU-specific resource handles. */
} Mesh;

/**
 * @brief Represents a 2D texture asset.
 */
typedef struct {
    char name[64];              /**< Unique name of the texture. */
    char filepath[256];         /**< Full path to the source image file. */
    uint32_t width, height;     /**< Dimensions of the texture in pixels. */
    bool loaded;                /**< Flag indicating if the texture data is loaded. */
    struct TextureGpuResources* gpu_resources; /**< Opaque pointer to GPU-specific resource handles. */
} Texture;

/**
 * @brief Represents the material properties of a surface for PBR rendering.
 */
typedef struct {
    char name[64];              /**< Unique name of the material. */
    
    // Basic material properties
    Vector3 diffuse_color;      /**< Base color of the material (albedo). */
    Vector3 ambient_color;      /**< Ambient color component. */
    Vector3 specular_color;     /**< Specular color component. */
    Vector3 emission_color;     /**< Emissive color for glowing surfaces. */
    float shininess;            /**< Shininess factor for specular highlights. */
    float roughness;            /**< Surface roughness (0.0 = smooth, 1.0 = rough). */
    float metallic;             /**< Metallic factor (0.0 = dielectric, 1.0 = metal). */
    
    // Multi-texture support
    char diffuse_texture[64];   /**< Filename of the diffuse (albedo) texture map. */
    char normal_texture[64];    /**< Filename of the normal map. */
    char specular_texture[64];  /**< Filename of the specular map. */
    char emission_texture[64];  /**< Filename of the emissive map. */
    
    bool loaded;                /**< Flag indicating if the material data is loaded. */
} Material;

/**
 * @brief A central registry that holds all loaded game assets.
 */
typedef struct {
    Mesh meshes[MAX_MESHES];            /**< Array of loaded meshes. */
    Texture textures[MAX_TEXTURES];     /**< Array of loaded textures. */
    Material materials[MAX_MATERIALS];  /**< Array of loaded materials. */
    
    uint32_t mesh_count;                /**< Current number of loaded meshes. */
    uint32_t texture_count;             /**< Current number of loaded textures. */
    uint32_t material_count;            /**< Current number of loaded materials. */
    
    char asset_root[256];               /**< The root directory for all game assets. */
} AssetRegistry;

// ============================================================================
// ASSET SYSTEM API
// ============================================================================

/**
 * @brief Initializes the asset system and sets the root asset path.
 * @param registry Pointer to the AssetRegistry to initialize.
 * @param asset_root The absolute path to the root assets directory.
 * @return True on success, false on failure.
 */
bool assets_init(AssetRegistry* registry, const char* asset_root);

/**
 * @brief Cleans up the asset system, freeing all associated memory and GPU resources.
 * @param registry The asset registry to cleanup.
 */
void assets_cleanup(AssetRegistry* registry);

/**
 * @brief Loads a texture from a file and uploads it to the GPU.
 * @param registry The asset registry to load into.
 * @param texture_path Path to the texture file, relative to the asset root's 'textures' subdirectory.
 * @param texture_name A unique name to assign to the loaded texture.
 * @return True if the texture was loaded successfully, false otherwise.
 */
bool load_texture(AssetRegistry* registry, const char* texture_path, const char* texture_name);

/**
 * @brief Loads a mesh from a file, automatically detecting its format (binary .cobj or text).
 * @param registry The asset registry to load into.
 * @param absolute_filepath The full, absolute path to the mesh file.
 * @param mesh_name A unique name to assign to the loaded mesh.
 * @return True if the mesh was loaded successfully, false otherwise.
 */
bool load_mesh_from_file(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name);

/**
 * @brief Gets a pointer to a loaded mesh by its unique name.
 * @param registry The asset registry to search.
 * @param name The name of the mesh to find.
 * @return A pointer to the Mesh struct if found, otherwise NULL.
 */
Mesh* assets_get_mesh(AssetRegistry* registry, const char* name);

/**
 * @brief Gets a pointer to a loaded material by its unique name.
 * @param registry The asset registry to search.
 * @param name The name of the material to find.
 * @return A pointer to the Material struct if found, otherwise NULL.
 */
Material* assets_get_material(AssetRegistry* registry, const char* name);

/**
 * @brief Gets a pointer to a loaded texture by its unique name.
 * @param registry The asset registry to search.
 * @param name The name of the texture to find.
 * @return A pointer to the Texture struct if found, otherwise NULL.
 */
Texture* assets_get_texture(AssetRegistry* registry, const char* name);

/**
 * @brief Prints a list of all currently loaded assets to the console for debugging.
 * @param registry The asset registry to list.
 */
void assets_list_loaded(AssetRegistry* registry);

/**
 * @brief Loads all assets defined in the `index.json` file.
 * @param registry The asset registry to load into.
 * @return True if all assets were loaded successfully, false otherwise.
 */
bool assets_load_all_in_directory(AssetRegistry* registry);

/**
 * @brief Creates a Renderable component from a loaded mesh.
 * @param registry The asset registry containing the mesh.
 * @param mesh_name The name of the mesh to use.
 * @param renderable A pointer to the Renderable component to populate.
 * @return True on success, false if the mesh is not found or has invalid GPU resources.
 */
bool assets_create_renderable_from_mesh(AssetRegistry* registry, const char* mesh_name, struct Renderable* renderable);

// ============================================================================
// SHADER LOADING FUNCTIONS
// ============================================================================

/**
 * @brief Loads shader source code from a file into a string.
 * @param shader_path The path to the shader file.
 * @return A dynamically allocated string with the shader code, or NULL on failure. The caller is responsible for freeing this memory.
 */
char* load_shader_source(const char* shader_path);

/**
 * @brief Frees the memory allocated for a shader source string.
 * @param source The shader source string to free.
 */
void free_shader_source(char* source);

/**
 * @brief Gets the platform-specific path for a shader.
 * @param base_name The base name of the shader (e.g., "basic_3d").
 * @param stage The shader stage ("vert" or "frag").
 * @return A string with the full shader path (e.g., "assets/shaders/basic_3d.vert.metal").
 */
const char* get_shader_path(const char* base_name, const char* stage);

// ============================================================================
// BINARY ASSET FORMAT
// ============================================================================

/**
 * @brief Header for the binary .cobj file format.
 * This struct defines the exact binary layout of the file's header,
 * allowing for direct, high-performance loading.
 */
typedef struct {
    char magic[4];           /**< Magic number to identify the file type ("CGMF"). */
    uint32_t version;        /**< Format version number. */
    uint32_t vertex_count;   /**< The total number of vertices in the mesh. */
    uint32_t index_count;    /**< The total number of indices in the mesh. */
    Vector3 aabb_min;        /**< The minimum corner of the mesh's axis-aligned bounding box. */
    Vector3 aabb_max;        /**< The maximum corner of the mesh's axis-aligned bounding box. */
    uint32_t reserved[8];    /**< Reserved for future use, ensures header alignment. */
} COBJHeader;

#endif // ASSETS_H
