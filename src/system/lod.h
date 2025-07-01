// src/system/lod.h
// Level of Detail (LOD) system for performance optimization - Sprint 19 Task 1

#ifndef SYSTEM_LOD_H
#define SYSTEM_LOD_H

#include "core.h"
#include "render.h"

// ============================================================================
// LOD CONFIGURATION
// ============================================================================

/** @brief Configuration for LOD distance thresholds */
typedef struct {
    float high_detail_distance;   /**< Below this distance: high detail */
    float medium_detail_distance; /**< Below this distance: medium detail */
    float low_detail_distance;    /**< Below this distance: low detail */
    float cull_distance;          /**< Beyond this distance: culled */
} LODConfig;

/** @brief Performance metrics for LOD system */
typedef struct {
    uint32_t entities_processed;   /**< Total entities processed this frame */
    uint32_t entities_high_lod;    /**< Entities at high LOD */
    uint32_t entities_medium_lod;  /**< Entities at medium LOD */
    uint32_t entities_low_lod;     /**< Entities at low LOD */
    uint32_t entities_culled;      /**< Entities culled */
    float update_time_ms;          /**< Time taken to update LOD this frame */
    uint32_t lod_switches;         /**< Number of LOD level changes this frame */
} LODPerformanceMetrics;

// ============================================================================
// LOD SYSTEM API
// ============================================================================

/**
 * @brief Initialize the LOD system with default configuration
 */
void lod_system_init(void);

/**
 * @brief Update LOD levels for all renderable entities based on camera distance
 * @param world The world containing entities to process
 * @param render_config Render configuration (unused but required for consistency)
 * @param delta_time Time since last update (unused but maintained for consistency)
 */
void lod_system_update(struct World* world, RenderConfig* render_config, float delta_time);

/**
 * @brief Cleanup LOD system resources
 */
void lod_system_cleanup(void);

// ============================================================================
// LOD CALCULATION FUNCTIONS
// ============================================================================

/**
 * @brief Calculate distance between camera and entity
 * @param world The world containing both entities
 * @param camera_id ID of the camera entity
 * @param entity_id ID of the entity to measure distance to
 * @return Distance between camera and entity, or -1.0f on error
 */
float lod_calculate_distance(struct World* world, EntityID camera_id, EntityID entity_id);

/**
 * @brief Determine LOD level for a given distance
 * @param distance Distance from camera to entity
 * @return Appropriate LOD level for this distance
 */
LODLevel lod_get_level_for_distance(float distance);

// ============================================================================
// LOD CONFIGURATION API
// ============================================================================

/**
 * @brief Get current LOD configuration
 * @return Current LOD distance thresholds
 */
LODConfig lod_get_config(void);

/**
 * @brief Set new LOD configuration
 * @param config New LOD distance thresholds
 */
void lod_set_config(LODConfig config);

/**
 * @brief Reset LOD configuration to defaults
 */
void lod_reset_config(void);

// ============================================================================
// LOD PERFORMANCE MONITORING
// ============================================================================

/**
 * @brief Get performance metrics for the LOD system
 * @return Current frame's LOD performance metrics
 */
LODPerformanceMetrics lod_get_performance_metrics(void);

/**
 * @brief Reset performance metrics counters
 */
void lod_reset_performance_metrics(void);

#endif // SYSTEM_LOD_H
