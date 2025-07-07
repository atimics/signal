// src/system/memory.c
// Memory management and optimization system implementation - Sprint 19 Task 2.2

#include "memory.h"
#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// ============================================================================
// INTERNAL STATE
// ============================================================================

// Forward declarations
static void memory_update_asset_distances(struct World* world, AssetRegistry* registry);
static uint32_t memory_automatic_cleanup(AssetRegistry* registry);

static MemoryManager memory_state = { 0 };

// Standard memory pool IDs
static uint32_t MESH_POOL_ID = UINT32_MAX;
static uint32_t TEXTURE_POOL_ID = UINT32_MAX;
static uint32_t MATERIAL_POOL_ID = UINT32_MAX;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static double get_time(void) {
    return (double)clock() / CLOCKS_PER_SEC;
}

static MemoryPool* get_pool(uint32_t pool_id) {
    if (pool_id >= memory_state.pool_count) {
        return NULL;
    }
    return &memory_state.pools[pool_id];
}

static TrackedAsset* find_tracked_asset(const char* asset_name) {
    for (uint32_t i = 0; i < memory_state.tracked_asset_count; i++) {
        if (strcmp(memory_state.tracked_assets[i].asset_name, asset_name) == 0) {
            return &memory_state.tracked_assets[i];
        }
    }
    return NULL;
}

static bool track_allocation(void* ptr, size_t size, uint32_t pool_id) {
    if (memory_state.allocation_count >= MAX_ALLOCATION_TRACKING) {
        return false; // No space to track more allocations
    }
    
    AllocationMetadata* metadata = &memory_state.allocations[memory_state.allocation_count];
    metadata->ptr = ptr;
    metadata->size = size;
    metadata->pool_id = pool_id;
    memory_state.allocation_count++;
    return true;
}

static AllocationMetadata* find_allocation(void* ptr) {
    for (uint32_t i = 0; i < memory_state.allocation_count; i++) {
        if (memory_state.allocations[i].ptr == ptr) {
            return &memory_state.allocations[i];
        }
    }
    return NULL;
}

static void remove_allocation_tracking(void* ptr) {
    for (uint32_t i = 0; i < memory_state.allocation_count; i++) {
        if (memory_state.allocations[i].ptr == ptr) {
            // Move the last allocation to this slot to maintain compact array
            if (i < memory_state.allocation_count - 1) {
                memory_state.allocations[i] = memory_state.allocations[memory_state.allocation_count - 1];
            }
            memory_state.allocation_count--;
            return;
        }
    }
}

static size_t calculate_mesh_memory(const Mesh* mesh) {
    if (!mesh || !mesh->loaded) return 0;
    
    size_t vertex_memory = mesh->vertex_count * sizeof(Vertex);
    size_t index_memory = mesh->index_count * sizeof(int);
    
    return vertex_memory + index_memory;
}

static size_t calculate_texture_memory(const Texture* texture) {
    if (!texture || !texture->loaded) return 0;
    
    // Estimate texture memory: width * height * 4 bytes per pixel (RGBA)
    return texture->width * texture->height * 4;
}

// ============================================================================
// CORE MEMORY MANAGEMENT FUNCTIONS
// ============================================================================

bool memory_system_init(size_t memory_limit_mb) {
    if (memory_state.initialized) {
        printf("⚠️ Memory system already initialized\n");
        return false;
    }
    
    memset(&memory_state, 0, sizeof(memory_state));
    
    memory_state.memory_limit_bytes = memory_limit_mb * 1024 * 1024;  // Convert MB to bytes
    memory_state.streaming_enabled = true;
    memory_state.unloading_enabled = true;
    memory_state.last_cleanup_time = get_time();
    memory_state.last_check_time = get_time();
    
    // Create standard memory pools
    MESH_POOL_ID = memory_create_pool("Meshes", memory_limit_mb * 1024 * 1024 / 2);  // 50% for meshes
    TEXTURE_POOL_ID = memory_create_pool("Textures", memory_limit_mb * 1024 * 1024 / 4);  // 25% for textures
    MATERIAL_POOL_ID = memory_create_pool("Materials", 1024 * 1024);  // 1MB for materials
    
    if (MESH_POOL_ID == UINT32_MAX || TEXTURE_POOL_ID == UINT32_MAX || MATERIAL_POOL_ID == UINT32_MAX) {
        printf("❌ Failed to create standard memory pools\n");
        return false;
    }
    
    memory_state.initialized = true;
    
    printf("✅ Memory system initialized:\n");
    printf("   Memory limit: %zu MB\n", memory_limit_mb);
    printf("   Streaming: %s\n", memory_state.streaming_enabled ? "enabled" : "disabled");
    printf("   Unloading: %s\n", memory_state.unloading_enabled ? "enabled" : "disabled");
    
    return true;
}

void memory_system_shutdown(void) {
    if (!memory_state.initialized) {
        return;
    }
    
    printf("📊 Memory System Shutdown Statistics:\n");
    printf("   Total assets tracked: %u\n", memory_state.tracked_asset_count);
    printf("   Assets loaded: %u\n", memory_state.assets_loaded_count);
    printf("   Assets unloaded: %u\n", memory_state.assets_unloaded_count);
    printf("   Memory freed: %.2f MB\n", (float)memory_state.bytes_freed_total / (1024.0f * 1024.0f));
    
    memory_state.initialized = false;
}

void memory_system_update(struct World* world, AssetRegistry* registry, float delta_time) {
    (void)delta_time;  // Suppress unused parameter warning
    
    if (!memory_state.initialized || !registry) {
        return;
    }
    
    double current_time = get_time();
    
    // Update asset usage based on camera distance
    if (current_time - memory_state.last_check_time > MEMORY_CHECK_INTERVAL) {
        memory_update_asset_distances(world, registry);
        memory_state.last_check_time = current_time;
    }
    
    // Perform automatic cleanup if enabled
    if (memory_state.unloading_enabled && 
        current_time - memory_state.last_cleanup_time > MEMORY_CHECK_INTERVAL * 2) {
        
        uint32_t unloaded = memory_automatic_cleanup(registry);
        if (unloaded > 0) {
            printf("🧹 Memory cleanup: unloaded %u assets\n", unloaded);
        }
        
        memory_state.last_cleanup_time = current_time;
    }
    
    // Check memory limits and warn if exceeded
    if (memory_is_over_limit()) {
        printf("⚠️ Memory limit exceeded: %.2f/%.2f MB\n", 
               (float)memory_state.total_allocated_bytes / (1024.0f * 1024.0f),
               (float)memory_state.memory_limit_bytes / (1024.0f * 1024.0f));
        
        // Force cleanup if over limit
        memory_force_cleanup(registry);
    }
}

// ============================================================================
// MEMORY POOL MANAGEMENT
// ============================================================================

uint32_t memory_create_pool(const char* name, size_t max_bytes) {
    if (memory_state.pool_count >= MAX_MEMORY_POOLS) {
        printf("❌ Maximum memory pools reached\n");
        return UINT32_MAX;
    }
    
    MemoryPool* pool = &memory_state.pools[memory_state.pool_count];
    
    strncpy(pool->name, name, sizeof(pool->name) - 1);
    pool->name[sizeof(pool->name) - 1] = '\0';
    pool->allocated_bytes = 0;
    pool->peak_bytes = 0;
    pool->max_bytes = max_bytes;
    pool->allocation_count = 0;
    pool->enabled = true;
    
    uint32_t pool_id = memory_state.pool_count++;
    
    printf("📦 Created memory pool '%s' (ID: %u, limit: %.2f MB)\n", 
           name, pool_id, (float)max_bytes / (1024.0f * 1024.0f));
    
    return pool_id;
}

// ============================================================================
// ASSET TRACKING
// ============================================================================

uint32_t memory_track_allocation(uint32_t pool_id, const char* asset_name, 
                                const char* asset_type, size_t bytes) {
    if (memory_state.tracked_asset_count >= MAX_TRACKED_ASSETS) {
        printf("❌ Maximum tracked assets reached\n");
        return UINT32_MAX;
    }
    
    MemoryPool* pool = get_pool(pool_id);
    if (!pool) {
        printf("❌ Invalid memory pool ID: %u\n", pool_id);
        return UINT32_MAX;
    }
    
    // Check if already tracking this asset
    TrackedAsset* existing = find_tracked_asset(asset_name);
    if (existing) {
        printf("⚠️ Asset '%s' already tracked\n", asset_name);
        return UINT32_MAX;
    }
    
    TrackedAsset* asset = &memory_state.tracked_assets[memory_state.tracked_asset_count];
    
    strncpy(asset->asset_name, asset_name, sizeof(asset->asset_name) - 1);
    asset->asset_name[sizeof(asset->asset_name) - 1] = '\0';
    
    strncpy(asset->asset_type, asset_type, sizeof(asset->asset_type) - 1);
    asset->asset_type[sizeof(asset->asset_type) - 1] = '\0';
    
    asset->asset_id = memory_state.tracked_asset_count;
    asset->memory_bytes = bytes;
    asset->last_used_time = get_time();
    asset->distance_from_camera = 0.0f;
    asset->loaded = true;
    asset->can_unload = true;
    
    // Update pool statistics
    pool->allocated_bytes += bytes;
    pool->allocation_count++;
    
    if (pool->allocated_bytes > pool->peak_bytes) {
        pool->peak_bytes = pool->allocated_bytes;
    }
    
    // Update global statistics
    memory_state.total_allocated_bytes += bytes;
    memory_state.assets_loaded_count++;
    
    uint32_t tracking_id = memory_state.tracked_asset_count++;
    
    // Report to performance system
    performance_record_memory_usage(bytes, asset_type);
    
    return tracking_id;
}

void memory_untrack_allocation(uint32_t tracking_id) {
    if (tracking_id >= memory_state.tracked_asset_count) {
        return;
    }
    
    TrackedAsset* asset = &memory_state.tracked_assets[tracking_id];
    
    if (!asset->loaded) {
        return;  // Already unloaded
    }
    
    size_t bytes = asset->memory_bytes;
    
    // Update global statistics
    memory_state.total_allocated_bytes -= bytes;
    memory_state.bytes_freed_total += bytes;
    memory_state.assets_unloaded_count++;
    
    asset->loaded = false;
    asset->memory_bytes = 0;
    
    printf("🗑️ Untracked asset '%s' (freed %.2f MB)\n", 
           asset->asset_name, (float)bytes / (1024.0f * 1024.0f));
}

// ============================================================================
// ASSET USAGE TRACKING
// ============================================================================

void memory_update_asset_usage(const char* asset_name, float distance_from_camera, bool used) {
    TrackedAsset* asset = find_tracked_asset(asset_name);
    if (!asset) {
        return;
    }
    
    asset->distance_from_camera = distance_from_camera;
    
    if (used) {
        asset->last_used_time = get_time();
    }
}

static void memory_update_asset_distances(struct World* world, AssetRegistry* registry) {
    if (!world || !registry) {
        return;
    }
    
    // Get active camera position (simplified - assumes camera at origin for now)
    Vector3 camera_pos = { 0.0f, 0.0f, 0.0f };
    
    // TODO: Get actual camera position from camera system
    // camera_pos = camera_system_get_position();
    
    // Update distances for all meshes
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        if (!mesh->loaded) continue;
        
        // Calculate distance from camera to mesh center
        Vector3 mesh_center = {
            (mesh->aabb_min.x + mesh->aabb_max.x) * 0.5f,
            (mesh->aabb_min.y + mesh->aabb_max.y) * 0.5f,
            (mesh->aabb_min.z + mesh->aabb_max.z) * 0.5f
        };
        
        float distance = sqrtf(
            (camera_pos.x - mesh_center.x) * (camera_pos.x - mesh_center.x) +
            (camera_pos.y - mesh_center.y) * (camera_pos.y - mesh_center.y) +
            (camera_pos.z - mesh_center.z) * (camera_pos.z - mesh_center.z)
        );
        
        memory_update_asset_usage(mesh->name, distance, true);
    }
}

// ============================================================================
// ASSET UNLOADING
// ============================================================================

static uint32_t memory_automatic_cleanup(AssetRegistry* registry) {
    uint32_t unloaded_count = 0;
    double current_time = get_time();
    
    for (uint32_t i = 0; i < memory_state.tracked_asset_count; i++) {
        TrackedAsset* asset = &memory_state.tracked_assets[i];
        
        if (!asset->loaded || !asset->can_unload) {
            continue;
        }
        
        // Check if asset should be unloaded based on distance and time
        bool should_unload = false;
        
        // Unload if too far from camera
        if (asset->distance_from_camera > MEMORY_UNLOAD_DISTANCE) {
            should_unload = true;
        }
        
        // Unload if not used for a long time
        if (current_time - asset->last_used_time > 30.0) {  // 30 seconds
            should_unload = true;
        }
        
        if (should_unload) {
            if (strcmp(asset->asset_type, "mesh") == 0) {
                if (asset_unload_mesh(registry, asset->asset_name)) {
                    unloaded_count++;
                }
            } else if (strcmp(asset->asset_type, "texture") == 0) {
                if (asset_unload_texture(registry, asset->asset_name)) {
                    unloaded_count++;
                }
            }
        }
    }
    
    return unloaded_count;
}

uint32_t memory_force_cleanup(AssetRegistry* registry) {
    printf("🧹 Forcing memory cleanup...\n");
    return memory_automatic_cleanup(registry);
}

bool asset_unload_mesh(AssetRegistry* registry, const char* mesh_name) {
    if (!registry || !mesh_name) {
        return false;
    }
    
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        
        if (strcmp(mesh->name, mesh_name) == 0 && mesh->loaded) {
            size_t freed_bytes = calculate_mesh_memory(mesh);
            
            // Free CPU memory
            if (mesh->vertices) {
                free(mesh->vertices);
                mesh->vertices = NULL;
            }
            if (mesh->indices) {
                free(mesh->indices);
                mesh->indices = NULL;
            }
            
            // TODO: Free GPU resources
            // gpu_free_mesh_resources(mesh->gpu_resources);
            
            mesh->loaded = false;
            
            // Update tracking
            TrackedAsset* tracked = find_tracked_asset(mesh_name);
            if (tracked) {
                memory_untrack_allocation(tracked->asset_id);
            }
            
            printf("🗑️ Unloaded mesh '%s' (freed %.2f MB)\n", 
                   mesh_name, (float)freed_bytes / (1024.0f * 1024.0f));
            
            return true;
        }
    }
    
    return false;
}

bool asset_unload_texture(AssetRegistry* registry, const char* texture_name) {
    if (!registry || !texture_name) {
        return false;
    }
    
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        Texture* texture = &registry->textures[i];
        
        if (strcmp(texture->name, texture_name) == 0 && texture->loaded) {
            size_t freed_bytes = calculate_texture_memory(texture);
            
            // TODO: Free GPU resources
            // gpu_free_texture_resources(texture->gpu_resources);
            
            texture->loaded = false;
            
            // Update tracking
            TrackedAsset* tracked = find_tracked_asset(texture_name);
            if (tracked) {
                memory_untrack_allocation(tracked->asset_id);
            }
            
            printf("🗑️ Unloaded texture '%s' (freed %.2f MB)\n", 
                   texture_name, (float)freed_bytes / (1024.0f * 1024.0f));
            
            return true;
        }
    }
    
    return false;
}

bool asset_reload(AssetRegistry* registry, const char* asset_name, const char* asset_type) {
    (void)registry;    // Suppress unused parameter warning
    (void)asset_name;  // Suppress unused parameter warning
    (void)asset_type;  // Suppress unused parameter warning
    
    // TODO: Implement asset reloading
    printf("🔄 Asset reload requested for '%s' (type: %s)\n", asset_name, asset_type);
    return false;
}

bool asset_should_unload(const char* asset_name) {
    TrackedAsset* asset = find_tracked_asset(asset_name);
    if (!asset || !asset->loaded) {
        return false;
    }
    
    // Unload if far from camera (primary check)
    if (asset->distance_from_camera > MEMORY_UNLOAD_DISTANCE) {
        return true;
    }
    
    // Unload if not used for a long time (secondary check)
    #ifndef CGAME_TESTING
    double current_time = get_time();
    if (current_time - asset->last_used_time > 30.0) {  // 30 seconds
        return true;
    }
    #endif
    
    return false;
}

// ============================================================================
// STATISTICS AND REPORTING
// ============================================================================

void memory_get_stats(size_t* out_total_mb, size_t* out_peak_mb, uint32_t* out_asset_count) {
    if (out_total_mb) {
        *out_total_mb = memory_state.total_allocated_bytes / (1024 * 1024);
    }
    
    if (out_peak_mb) {
        size_t peak_bytes = 0;
        for (uint32_t i = 0; i < memory_state.pool_count; i++) {
            peak_bytes += memory_state.pools[i].peak_bytes;
        }
        *out_peak_mb = peak_bytes / (1024 * 1024);
    }
    
    if (out_asset_count) {
        *out_asset_count = memory_state.tracked_asset_count;
    }
}

bool memory_is_over_limit(void) {
    if (memory_state.memory_limit_bytes == 0) {
        return false;  // No limit set
    }
    
    return memory_state.total_allocated_bytes > memory_state.memory_limit_bytes;
}

void memory_set_streaming_enabled(bool enabled) {
    memory_state.streaming_enabled = enabled;
    printf("💾 Asset streaming %s\n", enabled ? "enabled" : "disabled");
}

void memory_set_unloading_enabled(bool enabled) {
    memory_state.unloading_enabled = enabled;
    printf("🗑️ Asset unloading %s\n", enabled ? "enabled" : "disabled");
}

void memory_print_report(void) {
    printf("\n📊 Memory Usage Report\n");
    printf("=======================\n");
    printf("Total allocated: %.2f MB\n", (float)memory_state.total_allocated_bytes / (1024.0f * 1024.0f));
    printf("Memory limit: %.2f MB\n", (float)memory_state.memory_limit_bytes / (1024.0f * 1024.0f));
    printf("Tracked assets: %u\n", memory_state.tracked_asset_count);
    printf("Assets loaded: %u\n", memory_state.assets_loaded_count);
    printf("Assets unloaded: %u\n", memory_state.assets_unloaded_count);
    printf("Memory freed: %.2f MB\n", (float)memory_state.bytes_freed_total / (1024.0f * 1024.0f));
    
    printf("\nMemory Pools:\n");
    for (uint32_t i = 0; i < memory_state.pool_count; i++) {
        MemoryPool* pool = &memory_state.pools[i];
        printf("  %s: %.2f/%.2f MB (%u allocations)\n", 
               pool->name,
               (float)pool->allocated_bytes / (1024.0f * 1024.0f),
               (float)pool->max_bytes / (1024.0f * 1024.0f),
               pool->allocation_count);
    }
    
    printf("\nStreaming: %s\n", memory_state.streaming_enabled ? "enabled" : "disabled");
    printf("Unloading: %s\n", memory_state.unloading_enabled ? "enabled" : "disabled");
    printf("\n");
}

// ============================================================================
// SCHEDULER COMPATIBILITY WRAPPER
// ============================================================================

void memory_system_update_wrapper(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config;  // Suppress unused parameter warning
    
    // In test mode, use a NULL registry (system will handle gracefully)
    #ifdef CGAME_TESTING
        memory_system_update(world, NULL, delta_time);
    #else
        // Get asset registry (global access for now)
        extern AssetRegistry g_asset_registry;
        memory_system_update(world, &g_asset_registry, delta_time);
    #endif
}

// ============================================================================
// MEMORY POOL API (for testing and advanced allocation)
// ============================================================================

void* memory_pool_alloc(uint32_t pool_id, size_t size) {
    if (!memory_state.initialized) {
        return NULL;
    }
    
    MemoryPool* pool = get_pool(pool_id);
    if (!pool) {
        return NULL;
    }
    
    // Check if pool has space for this allocation
    if (pool->allocated_bytes + size > pool->max_bytes && pool->max_bytes > 0) {
        return NULL;
    }
    
    // Allocate memory
    void* ptr = malloc(size);
    if (ptr) {
        // Track the allocation metadata
        if (track_allocation(ptr, size, pool_id)) {
            pool->allocated_bytes += size;
            pool->allocation_count++;
            memory_state.total_allocated_bytes += size;
            
            // Update peak usage
            if (pool->allocated_bytes > pool->peak_bytes) {
                pool->peak_bytes = pool->allocated_bytes;
            }
        } else {
            // Failed to track allocation - free the memory and return NULL
            free(ptr);
            return NULL;
        }
    }
    
    return ptr;
}

void memory_pool_free(uint32_t pool_id, void* ptr) {
    if (!memory_state.initialized || !ptr) {
        return;
    }
    
    MemoryPool* pool = get_pool(pool_id);
    if (!pool) {
        return;
    }
    
    // Find the allocation metadata to get the size
    AllocationMetadata* metadata = find_allocation(ptr);
    if (metadata && metadata->pool_id == pool_id) {
        // Update pool statistics
        pool->allocated_bytes -= metadata->size;
        pool->allocation_count--;
        memory_state.total_allocated_bytes -= metadata->size;
        memory_state.bytes_freed_total += metadata->size;
        
        // Remove from tracking
        remove_allocation_tracking(ptr);
    } else {
        // Fallback: just decrement count if we can't find the metadata
        // This should not happen in normal operation
        if (pool->allocation_count > 0) {
            pool->allocation_count--;
        }
    }
    
    free(ptr);
}

void memory_destroy_pool(uint32_t pool_id) {
    if (!memory_state.initialized) {
        return;
    }
    
    MemoryPool* pool = get_pool(pool_id);
    if (!pool) {
        return;
    }
    
    // Reset pool state
    pool->allocated_bytes = 0;
    pool->peak_bytes = 0;
    pool->max_bytes = 0;
    pool->allocation_count = 0;
    pool->name[0] = '\0';
    pool->enabled = false;
}

void memory_track_asset_allocation(void* ptr, size_t size, const char* asset_name) {
    if (!memory_state.initialized || !ptr || !asset_name) {
        return;
    }
    
    // Find or create tracked asset entry
    TrackedAsset* asset = find_tracked_asset(asset_name);
    if (!asset && memory_state.tracked_asset_count < MAX_TRACKED_ASSETS) {
        asset = &memory_state.tracked_assets[memory_state.tracked_asset_count++];
        asset->asset_id = memory_state.tracked_asset_count;
        strncpy(asset->asset_name, asset_name, sizeof(asset->asset_name) - 1);
        asset->asset_name[sizeof(asset->asset_name) - 1] = '\0';
        strncpy(asset->asset_type, "test", sizeof(asset->asset_type) - 1);
        asset->asset_type[sizeof(asset->asset_type) - 1] = '\0';
        asset->memory_bytes = 0;
        asset->last_used_time = get_time();
        asset->distance_from_camera = 0.0f;
        asset->loaded = true;
        asset->can_unload = true;
    }
    
    if (asset) {
        asset->memory_bytes += size;
        asset->last_used_time = get_time();
        asset->loaded = true;
    }
}
