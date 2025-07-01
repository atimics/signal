// src/system/performance.h
// Performance profiling and monitoring system - Sprint 19 Task 2

#ifndef SYSTEM_PERFORMANCE_H
#define SYSTEM_PERFORMANCE_H

#include "core.h"
#include "render.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// PERFORMANCE METRICS STRUCTURES
// ============================================================================

/** @brief Frame timing metrics */
typedef struct {
    float frame_time_ms;        /**< Total frame time in milliseconds */
    float system_time_ms;       /**< Time spent in ECS systems */
    float render_time_ms;       /**< Time spent in rendering */
    float gpu_time_ms;          /**< Estimated GPU time (if available) */
    float idle_time_ms;         /**< Time spent waiting/idle */
    uint32_t frame_count;       /**< Total frames processed */
    float fps;                  /**< Current frames per second */
    float avg_fps;              /**< Average FPS over time window */
} FrameMetrics;

/** @brief Rendering performance metrics */
typedef struct {
    uint32_t entities_processed;    /**< Total entities processed for rendering */
    uint32_t entities_rendered;     /**< Successfully rendered entities */
    uint32_t entities_culled;       /**< Entities culled (including LOD) */
    uint32_t draw_calls;            /**< Number of draw calls issued */
    uint32_t triangles_rendered;    /**< Total triangles rendered */
    uint32_t vertices_processed;    /**< Total vertices processed */
    float culling_efficiency;      /**< Percentage of entities culled */
} RenderMetrics;

/** @brief Memory usage metrics */
typedef struct {
    size_t total_allocated_mb;      /**< Total memory allocated in MB */
    size_t mesh_memory_mb;          /**< Memory used by mesh data */
    size_t texture_memory_mb;       /**< Memory used by textures */
    size_t system_memory_mb;        /**< Memory used by ECS systems */
    size_t peak_memory_mb;          /**< Peak memory usage */
    uint32_t allocation_count;      /**< Number of active allocations */
} MemoryMetrics;

/** @brief System-specific performance metrics */
typedef struct {
    float physics_time_ms;      /**< Time spent in physics system */
    float collision_time_ms;    /**< Time spent in collision detection */
    float ai_time_ms;           /**< Time spent in AI system */
    float lod_time_ms;          /**< Time spent in LOD system */
    float camera_time_ms;       /**< Time spent in camera system */
    uint32_t physics_entities;  /**< Entities processed by physics */
    uint32_t collision_checks;  /**< Number of collision checks performed */
    uint32_t ai_entities;       /**< Entities processed by AI */
} SystemMetrics;

/** @brief Complete performance snapshot */
typedef struct {
    FrameMetrics frame;
    RenderMetrics render;
    MemoryMetrics memory;
    SystemMetrics systems;
    double timestamp;           /**< When this snapshot was taken */
    bool valid;                 /**< Whether this snapshot contains valid data */
} PerformanceSnapshot;

// ============================================================================
// PERFORMANCE MONITORING API
// ============================================================================

/**
 * @brief Initialize the performance monitoring system
 */
void performance_init(void);

/**
 * @brief Cleanup performance monitoring resources
 */
void performance_cleanup(void);

/**
 * @brief Begin frame timing measurement
 */
void performance_frame_begin(void);

/**
 * @brief End frame timing measurement
 */
void performance_frame_end(void);

/**
 * @brief Begin timing a specific system
 * @param system_name Name of the system being timed
 */
void performance_system_begin(const char* system_name);

/**
 * @brief End timing a specific system
 * @param system_name Name of the system being timed
 */
void performance_system_end(const char* system_name);

/**
 * @brief Record rendering metrics
 * @param metrics Rendering performance data
 */
void performance_record_render_metrics(const RenderMetrics* metrics);

/**
 * @brief Record memory usage
 * @param allocated_bytes Number of bytes currently allocated
 * @param category Memory category (mesh, texture, system, etc.)
 */
void performance_record_memory_usage(size_t allocated_bytes, const char* category);

/**
 * @brief Get current performance snapshot
 * @return Complete performance metrics for current frame
 */
PerformanceSnapshot performance_get_snapshot(void);

/**
 * @brief Get average performance over time window
 * @param window_seconds Time window in seconds for averaging
 * @return Averaged performance metrics
 */
PerformanceSnapshot performance_get_average(float window_seconds);

// ============================================================================
// PERFORMANCE ANALYSIS API
// ============================================================================

/**
 * @brief Check if frame rate is below target
 * @param target_fps Target frames per second
 * @return True if performance is below target
 */
bool performance_is_below_target(float target_fps);

/**
 * @brief Get performance bottleneck information
 * @return String describing the primary performance bottleneck
 */
const char* performance_get_bottleneck(void);

/**
 * @brief Log performance summary to console
 */
void performance_log_summary(void);

/**
 * @brief Export performance data to file
 * @param filename File to write performance data to
 * @return True if export was successful
 */
bool performance_export_to_file(const char* filename);

// ============================================================================
// DEBUG AND VISUALIZATION API
// ============================================================================

/**
 * @brief Render performance overlay on screen
 * @param render_config Render configuration for drawing
 */
void performance_render_overlay(RenderConfig* render_config);

/**
 * @brief Toggle performance overlay visibility
 */
void performance_toggle_overlay(void);

/**
 * @brief Set performance warning thresholds
 * @param frame_time_ms Warning threshold for frame time
 * @param memory_mb Warning threshold for memory usage
 */
void performance_set_warning_thresholds(float frame_time_ms, size_t memory_mb);

/**
 * @brief Performance system update function (for scheduler compatibility)
 * @param world The world (unused)
 * @param render_config Render configuration
 * @param delta_time Delta time (unused)
 */
void performance_system_update(struct World* world, RenderConfig* render_config, float delta_time);

#endif // SYSTEM_PERFORMANCE_H
