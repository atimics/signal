// src/system/memory.h
// Memory management and optimization system - Sprint 19 Task 2.2

#ifndef SYSTEM_MEMORY_H
#define SYSTEM_MEMORY_H

#include "../core.h"
#include "../assets.h"
#include "../render.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// MEMORY MANAGEMENT CONSTANTS
// ============================================================================

#define MAX_MEMORY_POOLS 16
#define MAX_TRACKED_ASSETS 512
#define MAX_ALLOCATION_TRACKING 4096   // Maximum number of tracked allocations
#define MEMORY_UNLOAD_DISTANCE 100.0f  // Distance threshold for asset unloading
#define MEMORY_LOAD_DISTANCE 50.0f     // Distance threshold for asset loading
#define MEMORY_CHECK_INTERVAL 2.0f     // Check memory usage every 2 seconds

// ============================================================================
// MEMORY TRACKING STRUCTURES
// ============================================================================

/** @brief Allocation metadata for tracking allocation sizes */
typedef struct {
    void* ptr;                  /**< Pointer to the allocated memory */
    size_t size;               /**< Size of the allocation */
    uint32_t pool_id;          /**< Pool that owns this allocation */
} AllocationMetadata;

/** @brief Memory pool for different asset types */
typedef struct {
    char name[32];              /**< Name of the memory pool */
    size_t allocated_bytes;     /**< Currently allocated bytes */
    size_t peak_bytes;          /**< Peak memory usage */
    size_t max_bytes;           /**< Maximum allowed bytes */
    uint32_t allocation_count;  /**< Number of active allocations */
    bool enabled;               /**< Whether pool is active */
} MemoryPool;

/** @brief Asset memory tracking information */
typedef struct {
    uint32_t asset_id;          /**< ID of the tracked asset */
    char asset_name[64];        /**< Name of the asset */
    char asset_type[16];        /**< Type: "mesh", "texture", "material" */
    size_t memory_bytes;        /**< Memory used by this asset */
    float last_used_time;       /**< Last time asset was accessed */
    float distance_from_camera; /**< Current distance from active camera */
    bool loaded;                /**< Whether asset is currently loaded */
    bool can_unload;            /**< Whether asset can be safely unloaded */
} TrackedAsset;

/** @brief Memory management state */
typedef struct {
    MemoryPool pools[MAX_MEMORY_POOLS];
    TrackedAsset tracked_assets[MAX_TRACKED_ASSETS];
    AllocationMetadata allocations[MAX_ALLOCATION_TRACKING];
    
    uint32_t pool_count;
    uint32_t tracked_asset_count;
    uint32_t allocation_count;
    
    size_t total_allocated_bytes;
    size_t memory_limit_bytes;
    
    float last_cleanup_time;
    float last_check_time;
    
    bool streaming_enabled;
    bool unloading_enabled;
    bool initialized;
    
    // Statistics
    uint32_t assets_unloaded_count;
    uint32_t assets_loaded_count;
    size_t bytes_freed_total;
    
} MemoryManager;

// ============================================================================
// MEMORY MANAGEMENT API
// ============================================================================

/**
 * @brief Initialize the memory management system
 * @param memory_limit_mb Maximum memory limit in megabytes (0 = unlimited)
 * @return true if initialization successful
 */
bool memory_system_init(size_t memory_limit_mb);

/**
 * @brief Shutdown the memory management system
 */
void memory_system_shutdown(void);

/**
 * @brief Update memory management (call each frame or periodically)
 * @param world The game world for camera and entity access
 * @param registry Asset registry for asset management
 * @param delta_time Time since last update
 */
void memory_system_update(struct World* world, AssetRegistry* registry, float delta_time);

/**
 * @brief Update memory management (wrapper for scheduler compatibility)
 * @param world The world containing entities to process
 * @param render_config Render configuration (unused but required for consistency)
 * @param delta_time Time since last update
 */
void memory_system_update_wrapper(struct World* world, RenderConfig* render_config, float delta_time);

/**
 * @brief Create a new memory pool for asset tracking
 * @param name Name of the memory pool
 * @param max_bytes Maximum bytes for this pool (0 = unlimited)
 * @return Pool ID, or UINT32_MAX on failure
 */
uint32_t memory_create_pool(const char* name, size_t max_bytes);

/**
 * @brief Track memory allocation for an asset
 * @param pool_id Memory pool ID
 * @param asset_name Name of the asset
 * @param asset_type Type of asset ("mesh", "texture", "material")
 * @param bytes Number of bytes allocated
 * @return Tracking ID, or UINT32_MAX on failure
 */
uint32_t memory_track_allocation(uint32_t pool_id, const char* asset_name, 
                                const char* asset_type, size_t bytes);

/**
 * @brief Remove memory tracking for an asset
 * @param tracking_id ID returned from memory_track_allocation
 */
void memory_untrack_allocation(uint32_t tracking_id);

/**
 * @brief Update asset usage information (for streaming decisions)
 * @param asset_name Name of the asset
 * @param distance_from_camera Current distance from camera
 * @param used Whether asset was used this frame
 */
void memory_update_asset_usage(const char* asset_name, float distance_from_camera, bool used);

/**
 * @brief Force unload unused assets beyond distance threshold
 * @param registry Asset registry for unloading assets
 * @return Number of assets unloaded
 */
uint32_t memory_force_cleanup(AssetRegistry* registry);

/**
 * @brief Get current memory usage statistics
 * @param out_total_mb Total allocated memory in MB
 * @param out_peak_mb Peak memory usage in MB
 * @param out_asset_count Number of tracked assets
 */
void memory_get_stats(size_t* out_total_mb, size_t* out_peak_mb, uint32_t* out_asset_count);

/**
 * @brief Check if memory limit is exceeded
 * @return true if over memory limit
 */
bool memory_is_over_limit(void);

/**
 * @brief Enable/disable automatic asset streaming
 * @param enabled Whether to enable streaming
 */
void memory_set_streaming_enabled(bool enabled);

/**
 * @brief Enable/disable automatic asset unloading
 * @param enabled Whether to enable unloading
 */
void memory_set_unloading_enabled(bool enabled);

/**
 * @brief Print detailed memory usage report
 */
void memory_print_report(void);

// ============================================================================
// MEMORY POOL API (for testing and advanced allocation)
// ============================================================================

/**
 * @brief Allocate memory from a specific pool
 * @param pool_id Pool ID returned from memory_create_pool
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* memory_pool_alloc(uint32_t pool_id, size_t size);

/**
 * @brief Free memory back to a specific pool
 * @param pool_id Pool ID where memory was allocated
 * @param ptr Pointer to memory to free
 */
void memory_pool_free(uint32_t pool_id, void* ptr);

/**
 * @brief Destroy a memory pool and free all its memory
 * @param pool_id Pool ID to destroy
 */
void memory_destroy_pool(uint32_t pool_id);

/**
 * @brief Track asset allocation for testing
 * @param ptr Pointer to allocated memory
 * @param size Size of allocation
 * @param asset_name Name of the asset
 */
void memory_track_asset_allocation(void* ptr, size_t size, const char* asset_name);

// ============================================================================
// ASSET UNLOADING API
// ============================================================================

/**
 * @brief Unload a mesh from memory
 * @param registry Asset registry
 * @param mesh_name Name of mesh to unload
 * @return true if unloaded successfully
 */
bool asset_unload_mesh(AssetRegistry* registry, const char* mesh_name);

/**
 * @brief Unload a texture from memory
 * @param registry Asset registry
 * @param texture_name Name of texture to unload
 * @return true if unloaded successfully
 */
bool asset_unload_texture(AssetRegistry* registry, const char* texture_name);

/**
 * @brief Reload a previously unloaded asset
 * @param registry Asset registry
 * @param asset_name Name of asset to reload
 * @param asset_type Type of asset ("mesh", "texture")
 * @return true if reloaded successfully
 */
bool asset_reload(AssetRegistry* registry, const char* asset_name, const char* asset_type);

/**
 * @brief Check if an asset should be unloaded based on usage patterns
 * @param asset_name Name of the asset
 * @return true if asset should be unloaded
 */
bool asset_should_unload(const char* asset_name);

#endif // SYSTEM_MEMORY_H
