// src/system/lod.c
// Level of Detail (LOD) system implementation - Sprint 19 Task 1

#include "lod.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================================
// INTERNAL STRUCTURES AND GLOBALS
// ============================================================================

// Default LOD configuration
static const LODConfig DEFAULT_LOD_CONFIG = {
    .high_detail_distance = 20.0f,    // High detail within 20 units
    .medium_detail_distance = 50.0f,  // Medium detail within 50 units
    .low_detail_distance = 100.0f,    // Low detail within 100 units
    .cull_distance = 200.0f           // Cull beyond 200 units
};

// Current LOD system state
static struct {
    LODConfig config;
    LODPerformanceMetrics metrics;
    bool initialized;
    clock_t frame_start_time;
} lod_state = { 0 };

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get current time in milliseconds
 */
static float get_time_ms(void) {
    return (float)clock() / (CLOCKS_PER_SEC / 1000.0f);
}

// ============================================================================
// LOD SYSTEM CORE FUNCTIONS
// ============================================================================

void lod_system_init(void) {
    if (lod_state.initialized) {
        printf("⚠️ LOD system already initialized\n");
        return;
    }
    
    // Initialize with default configuration
    lod_state.config = DEFAULT_LOD_CONFIG;
    memset(&lod_state.metrics, 0, sizeof(LODPerformanceMetrics));
    lod_state.initialized = true;
    
    printf("✅ LOD system initialized - distances: H:%.1f M:%.1f L:%.1f C:%.1f\n",
           lod_state.config.high_detail_distance,
           lod_state.config.medium_detail_distance,
           lod_state.config.low_detail_distance,
           lod_state.config.cull_distance);
}

void lod_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config; // Unused parameter
    (void)delta_time; // LOD updates are not time-dependent, only distance-dependent
    
    if (!lod_state.initialized) {
        lod_system_init();
    }
    
    // Start performance timing
    lod_state.frame_start_time = clock();
    
    // Reset frame metrics
    memset(&lod_state.metrics, 0, sizeof(LODPerformanceMetrics));
    
    // Get active camera
    EntityID camera_id = world_get_active_camera(world);
    if (camera_id == INVALID_ENTITY) {
        // No active camera - can't calculate LOD
        lod_state.metrics.update_time_ms = get_time_ms() - (float)lod_state.frame_start_time / (CLOCKS_PER_SEC / 1000.0f);
        return;
    }
    
    // Process all renderable entities
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Skip non-renderable entities
        if (!(entity->component_mask & COMPONENT_RENDERABLE) ||
            !(entity->component_mask & COMPONENT_TRANSFORM)) {
            continue;
        }
        
        lod_state.metrics.entities_processed++;
        
        // Calculate distance to camera
        float distance = lod_calculate_distance(world, camera_id, entity->id);
        if (distance < 0.0f) {
            continue; // Error calculating distance
        }
        
        // Determine new LOD level
        LODLevel new_lod_level = lod_get_level_for_distance(distance);
        
        // Get renderable component and update LOD
        struct Renderable* renderable = entity_get_renderable(world, entity->id);
        if (renderable) {
            // Track LOD level changes
            if (renderable->lod_level != new_lod_level) {
                lod_state.metrics.lod_switches++;
            }
            
            // Update LOD level and visibility
            renderable->lod_level = new_lod_level;
            renderable->lod_distance = distance;
            
            // Set visibility based on LOD level
            renderable->visible = (new_lod_level != LOD_CULLED);
            
            // Update performance counters
            switch (new_lod_level) {
                case LOD_HIGH:
                    lod_state.metrics.entities_high_lod++;
                    break;
                case LOD_MEDIUM:
                    lod_state.metrics.entities_medium_lod++;
                    break;
                case LOD_LOW:
                    lod_state.metrics.entities_low_lod++;
                    break;
                case LOD_CULLED:
                    lod_state.metrics.entities_culled++;
                    break;
            }
        }
    }
    
    // Calculate update time
    clock_t end_time = clock();
    lod_state.metrics.update_time_ms = (float)(end_time - lod_state.frame_start_time) / (CLOCKS_PER_SEC / 1000.0f);
}

void lod_system_cleanup(void) {
    if (!lod_state.initialized) {
        return;
    }
    
    printf("🔧 Cleaning up LOD system\n");
    memset(&lod_state, 0, sizeof(lod_state));
}

// ============================================================================
// LOD CALCULATION FUNCTIONS
// ============================================================================

float lod_calculate_distance(struct World* world, EntityID camera_id, EntityID entity_id) {
    if (!world || camera_id == INVALID_ENTITY || entity_id == INVALID_ENTITY) {
        return -1.0f;
    }
    
    // Get camera transform
    struct Transform* camera_transform = entity_get_transform(world, camera_id);
    if (!camera_transform) {
        return -1.0f;
    }
    
    // Get entity transform
    struct Transform* entity_transform = entity_get_transform(world, entity_id);
    if (!entity_transform) {
        return -1.0f;
    }
    
    // Calculate 3D distance
    return vector3_distance(camera_transform->position, entity_transform->position);
}

LODLevel lod_get_level_for_distance(float distance) {
    if (!lod_state.initialized) {
        lod_system_init();
    }
    
    if (distance < 0.0f) {
        return LOD_CULLED;
    }
    
    if (distance <= lod_state.config.high_detail_distance) {
        return LOD_HIGH;
    } else if (distance <= lod_state.config.medium_detail_distance) {
        return LOD_MEDIUM;
    } else if (distance <= lod_state.config.low_detail_distance) {
        return LOD_LOW;
    } else {
        return LOD_CULLED;
    }
}

// ============================================================================
// LOD CONFIGURATION API
// ============================================================================

LODConfig lod_get_config(void) {
    if (!lod_state.initialized) {
        return DEFAULT_LOD_CONFIG;
    }
    return lod_state.config;
}

void lod_set_config(LODConfig config) {
    if (!lod_state.initialized) {
        lod_system_init();
    }
    
    // Validate configuration (distances must be in ascending order)
    if (config.high_detail_distance >= config.medium_detail_distance ||
        config.medium_detail_distance >= config.low_detail_distance ||
        config.low_detail_distance >= config.cull_distance ||
        config.high_detail_distance <= 0.0f) {
        printf("⚠️ Invalid LOD configuration - distances must be positive and ascending\n");
        return;
    }
    
    lod_state.config = config;
    printf("📐 LOD configuration updated - H:%.1f M:%.1f L:%.1f C:%.1f\n",
           config.high_detail_distance, config.medium_detail_distance,
           config.low_detail_distance, config.cull_distance);
}

void lod_reset_config(void) {
    lod_state.config = DEFAULT_LOD_CONFIG;
    printf("🔄 LOD configuration reset to defaults\n");
}

// ============================================================================
// LOD PERFORMANCE MONITORING
// ============================================================================

LODPerformanceMetrics lod_get_performance_metrics(void) {
    return lod_state.metrics;
}

void lod_reset_performance_metrics(void) {
    memset(&lod_state.metrics, 0, sizeof(LODPerformanceMetrics));
}
