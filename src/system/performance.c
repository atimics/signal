// src/system/performance.c
// Performance profiling and monitoring system implementation - Sprint 19 Task 2

#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ============================================================================
// INTERNAL STRUCTURES AND GLOBALS
// ============================================================================

#define MAX_SYSTEM_TIMERS 16
#define PERFORMANCE_HISTORY_SIZE 300  // 5 seconds at 60 FPS

typedef struct {
    char name[32];
    clock_t start_time;
    float total_time_ms;
    uint32_t call_count;
    bool active;
} SystemTimer;

static struct {
    PerformanceSnapshot snapshots[PERFORMANCE_HISTORY_SIZE];
    uint32_t snapshot_index;
    uint32_t snapshot_count;
    
    SystemTimer system_timers[MAX_SYSTEM_TIMERS];
    uint32_t timer_count;
    
    clock_t frame_start_time;
    clock_t frame_end_time;
    
    bool initialized;
    bool overlay_visible;
    
    // Warning thresholds
    float frame_time_warning_ms;
    size_t memory_warning_mb;
    
    // Current frame data
    PerformanceSnapshot current_snapshot;
    
} performance_state = { 0 };

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static float clock_to_ms(clock_t clock_time) {
    return (float)clock_time / (CLOCKS_PER_SEC / 1000.0f);
}

static double get_timestamp(void) {
    return (double)clock() / CLOCKS_PER_SEC;
}

static SystemTimer* find_system_timer(const char* system_name) {
    for (uint32_t i = 0; i < performance_state.timer_count; i++) {
        if (strcmp(performance_state.system_timers[i].name, system_name) == 0) {
            return &performance_state.system_timers[i];
        }
    }
    return NULL;
}

static SystemTimer* create_system_timer(const char* system_name) {
    if (performance_state.timer_count >= MAX_SYSTEM_TIMERS) {
        printf("⚠️ Performance: Maximum system timers reached\n");
        return NULL;
    }
    
    SystemTimer* timer = &performance_state.system_timers[performance_state.timer_count++];
    strncpy(timer->name, system_name, sizeof(timer->name) - 1);
    timer->name[sizeof(timer->name) - 1] = '\0';
    timer->start_time = 0;
    timer->total_time_ms = 0.0f;
    timer->call_count = 0;
    timer->active = false;
    
    return timer;
}

// ============================================================================
// CORE PERFORMANCE MONITORING FUNCTIONS
// ============================================================================

void performance_init(void) {
    if (performance_state.initialized) {
        printf("⚠️ Performance monitoring already initialized\n");
        return;
    }
    
    memset(&performance_state, 0, sizeof(performance_state));
    
    // Set default warning thresholds
    performance_state.frame_time_warning_ms = 20.0f;  // 50 FPS
    performance_state.memory_warning_mb = 512;        // 512 MB
    performance_state.overlay_visible = false;
    performance_state.initialized = true;
    
    printf("⚡ Performance monitoring initialized\n");
    printf("   Frame time warning: %.1f ms\n", performance_state.frame_time_warning_ms);
    printf("   Memory warning: %zu MB\n", performance_state.memory_warning_mb);
}

void performance_cleanup(void) {
    if (!performance_state.initialized) {
        return;
    }
    
    printf("🔧 Cleaning up performance monitoring\n");
    performance_log_summary();
    
    memset(&performance_state, 0, sizeof(performance_state));
}

void performance_frame_begin(void) {
    if (!performance_state.initialized) {
        performance_init();
    }
    
    performance_state.frame_start_time = clock();
    
    // Reset current frame snapshot
    memset(&performance_state.current_snapshot, 0, sizeof(PerformanceSnapshot));
    performance_state.current_snapshot.timestamp = get_timestamp();
    performance_state.current_snapshot.valid = true;
}

void performance_frame_end(void) {
    if (!performance_state.initialized) {
        return;
    }
    
    performance_state.frame_end_time = clock();
    
    // Calculate frame timing
    clock_t frame_duration = performance_state.frame_end_time - performance_state.frame_start_time;
    float frame_time_ms = clock_to_ms(frame_duration);
    
    // Update current snapshot
    performance_state.current_snapshot.frame.frame_time_ms = frame_time_ms;
    performance_state.current_snapshot.frame.frame_count++;
    
    if (frame_time_ms > 0.0f) {
        performance_state.current_snapshot.frame.fps = 1000.0f / frame_time_ms;
    }
    
    // Calculate system timing breakdown
    float total_system_time = 0.0f;
    for (uint32_t i = 0; i < performance_state.timer_count; i++) {
        SystemTimer* timer = &performance_state.system_timers[i];
        total_system_time += timer->total_time_ms;
        
        // Map specific systems to snapshot fields
        if (strcmp(timer->name, "Physics") == 0) {
            performance_state.current_snapshot.systems.physics_time_ms = timer->total_time_ms;
        } else if (strcmp(timer->name, "Collision") == 0) {
            performance_state.current_snapshot.systems.collision_time_ms = timer->total_time_ms;
        } else if (strcmp(timer->name, "AI") == 0) {
            performance_state.current_snapshot.systems.ai_time_ms = timer->total_time_ms;
        } else if (strcmp(timer->name, "LOD") == 0) {
            performance_state.current_snapshot.systems.lod_time_ms = timer->total_time_ms;
        } else if (strcmp(timer->name, "Camera") == 0) {
            performance_state.current_snapshot.systems.camera_time_ms = timer->total_time_ms;
        }
        
        // Reset timer for next frame
        timer->total_time_ms = 0.0f;
        timer->call_count = 0;
    }
    
    performance_state.current_snapshot.frame.system_time_ms = total_system_time;
    performance_state.current_snapshot.frame.render_time_ms = 
        frame_time_ms - total_system_time;
    
    // Store snapshot in history
    uint32_t index = performance_state.snapshot_index;
    performance_state.snapshots[index] = performance_state.current_snapshot;
    performance_state.snapshot_index = (index + 1) % PERFORMANCE_HISTORY_SIZE;
    if (performance_state.snapshot_count < PERFORMANCE_HISTORY_SIZE) {
        performance_state.snapshot_count++;
    }
    
    // Check for performance warnings
    if (frame_time_ms > performance_state.frame_time_warning_ms) {
        static uint32_t warning_count = 0;
        warning_count++;
        if (warning_count % 60 == 1) {  // Warn once per second
            printf("⚠️ Performance: Frame time %.1f ms (target: %.1f ms)\n", 
                   frame_time_ms, performance_state.frame_time_warning_ms);
        }
    }
}

void performance_system_begin(const char* system_name) {
    if (!performance_state.initialized || !system_name) {
        return;
    }
    
    SystemTimer* timer = find_system_timer(system_name);
    if (!timer) {
        timer = create_system_timer(system_name);
        if (!timer) {
            return;
        }
    }
    
    if (timer->active) {
        printf("⚠️ Performance: System '%s' already active\n", system_name);
        return;
    }
    
    timer->start_time = clock();
    timer->active = true;
}

void performance_system_end(const char* system_name) {
    if (!performance_state.initialized || !system_name) {
        return;
    }
    
    SystemTimer* timer = find_system_timer(system_name);
    if (!timer || !timer->active) {
        return;
    }
    
    clock_t end_time = clock();
    clock_t duration = end_time - timer->start_time;
    float duration_ms = clock_to_ms(duration);
    
    timer->total_time_ms += duration_ms;
    timer->call_count++;
    timer->active = false;
}

void performance_record_render_metrics(const RenderMetrics* metrics) {
    if (!performance_state.initialized || !metrics) {
        return;
    }
    
    performance_state.current_snapshot.render = *metrics;
    
    // Calculate derived metrics
    if (metrics->entities_processed > 0) {
        performance_state.current_snapshot.render.culling_efficiency = 
            (float)metrics->entities_culled / metrics->entities_processed * 100.0f;
    }
}

void performance_record_memory_usage(size_t allocated_bytes, const char* category) {
    if (!performance_state.initialized) {
        return;
    }
    
    size_t allocated_mb = allocated_bytes / (1024 * 1024);
    
    // Update total allocated memory
    performance_state.current_snapshot.memory.total_allocated_mb += allocated_mb;
    
    // Update category-specific memory if specified
    if (category) {
        if (strcmp(category, "mesh") == 0) {
            performance_state.current_snapshot.memory.mesh_memory_mb += allocated_mb;
        } else if (strcmp(category, "texture") == 0) {
            performance_state.current_snapshot.memory.texture_memory_mb += allocated_mb;
        } else if (strcmp(category, "system") == 0) {
            performance_state.current_snapshot.memory.system_memory_mb += allocated_mb;
        }
    }
    
    // Update peak memory
    if (performance_state.current_snapshot.memory.total_allocated_mb > 
        performance_state.current_snapshot.memory.peak_memory_mb) {
        performance_state.current_snapshot.memory.peak_memory_mb = 
            performance_state.current_snapshot.memory.total_allocated_mb;
    }
    
    // Check for memory warnings
    if (allocated_mb > performance_state.memory_warning_mb) {
        static uint32_t warning_count = 0;
        warning_count++;
        if (warning_count % 300 == 1) {  // Warn once every 5 seconds
            printf("⚠️ Performance: High memory usage %zu MB (warning: %zu MB)\n", 
                   allocated_mb, performance_state.memory_warning_mb);
        }
    }
}

PerformanceSnapshot performance_get_snapshot(void) {
    if (!performance_state.initialized) {
        PerformanceSnapshot empty = { 0 };
        return empty;
    }
    
    return performance_state.current_snapshot;
}

PerformanceSnapshot performance_get_average(float window_seconds) {
    PerformanceSnapshot avg = { 0 };
    
    if (!performance_state.initialized || performance_state.snapshot_count == 0) {
        return avg;
    }
    
    // Calculate how many snapshots to average (assuming 60 FPS)
    uint32_t window_frames = (uint32_t)(window_seconds * 60.0f);
    if (window_frames > performance_state.snapshot_count) {
        window_frames = performance_state.snapshot_count;
    }
    
    float frame_time_sum = 0.0f;
    uint32_t render_entities_sum = 0;
    uint32_t draw_calls_sum = 0;
    
    // Sum up the values
    for (uint32_t i = 0; i < window_frames; i++) {
        uint32_t index = (performance_state.snapshot_index - 1 - i + PERFORMANCE_HISTORY_SIZE) 
                        % PERFORMANCE_HISTORY_SIZE;
        const PerformanceSnapshot* snapshot = &performance_state.snapshots[index];
        
        frame_time_sum += snapshot->frame.frame_time_ms;
        render_entities_sum += snapshot->render.entities_rendered;
        draw_calls_sum += snapshot->render.draw_calls;
    }
    
    // Calculate averages
    avg.frame.frame_time_ms = frame_time_sum / window_frames;
    avg.frame.fps = (avg.frame.frame_time_ms > 0.0f) ? 1000.0f / avg.frame.frame_time_ms : 0.0f;
    avg.render.entities_rendered = render_entities_sum / window_frames;
    avg.render.draw_calls = draw_calls_sum / window_frames;
    avg.valid = true;
    
    return avg;
}

bool performance_is_below_target(float target_fps) {
    if (!performance_state.initialized) {
        return false;
    }
    
    return performance_state.current_snapshot.frame.fps < target_fps;
}

const char* performance_get_bottleneck(void) {
    if (!performance_state.initialized) {
        return "Performance monitoring not initialized";
    }
    
    const PerformanceSnapshot* snapshot = &performance_state.current_snapshot;
    
    // Check frame time first
    if (snapshot->frame.frame_time_ms > performance_state.frame_time_warning_ms) {
        // Determine what's taking the most time
        float max_time = 0.0f;
        const char* bottleneck = "Unknown";
        
        if (snapshot->frame.render_time_ms > max_time) {
            max_time = snapshot->frame.render_time_ms;
            bottleneck = "Rendering";
        }
        
        if (snapshot->systems.physics_time_ms > max_time) {
            max_time = snapshot->systems.physics_time_ms;
            bottleneck = "Physics";
        }
        
        if (snapshot->systems.collision_time_ms > max_time) {
            max_time = snapshot->systems.collision_time_ms;
            bottleneck = "Collision";
        }
        
        if (snapshot->systems.ai_time_ms > max_time) {
            max_time = snapshot->systems.ai_time_ms;
            bottleneck = "AI";
        }
        
        return bottleneck;
    }
    
    return "Performance OK";
}

void performance_log_summary(void) {
    if (!performance_state.initialized) {
        printf("⚡ Performance monitoring not initialized\n");
        return;
    }
    
    PerformanceSnapshot avg = performance_get_average(5.0f);
    
    printf("\n⚡ Performance Summary (5-second average):\n");
    printf("   Frame Time: %.2f ms (%.1f FPS)\n", avg.frame.frame_time_ms, avg.frame.fps);
    printf("   Entities Rendered: %d\n", avg.render.entities_rendered);
    printf("   Draw Calls: %d\n", avg.render.draw_calls);
    printf("   Bottleneck: %s\n", performance_get_bottleneck());
    
    if (performance_state.snapshot_count > 0) {
        printf("   Total Snapshots: %d\n", performance_state.snapshot_count);
    }
    
    printf("\n");
}

bool performance_export_to_file(const char* filename) {
    if (!performance_state.initialized || !filename) {
        return false;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("❌ Failed to open performance export file: %s\n", filename);
        return false;
    }
    
    fprintf(file, "# CGame Performance Data Export\n");
    fprintf(file, "# Timestamp,FrameTime(ms),FPS,EntitiesRendered,DrawCalls\n");
    
    for (uint32_t i = 0; i < performance_state.snapshot_count; i++) {
        const PerformanceSnapshot* snapshot = &performance_state.snapshots[i];
        fprintf(file, "%.3f,%.2f,%.1f,%d,%d\n",
                snapshot->timestamp,
                snapshot->frame.frame_time_ms,
                snapshot->frame.fps,
                snapshot->render.entities_rendered,
                snapshot->render.draw_calls);
    }
    
    fclose(file);
    printf("✅ Performance data exported to %s\n", filename);
    return true;
}

void performance_render_overlay(RenderConfig* render_config) {
    if (!performance_state.initialized || !performance_state.overlay_visible || !render_config) {
        return;
    }
    
    // This would integrate with the UI system to render performance overlay
    // For now, we'll just log to console periodically
    static uint32_t overlay_counter = 0;
    overlay_counter++;
    
    if (overlay_counter % 60 == 0) {  // Update once per second
        const PerformanceSnapshot* snapshot = &performance_state.current_snapshot;
        printf("⚡ FPS: %.1f | Frame: %.1fms | Entities: %d | DrawCalls: %d\n",
               snapshot->frame.fps,
               snapshot->frame.frame_time_ms,
               snapshot->render.entities_rendered,
               snapshot->render.draw_calls);
    }
}

void performance_toggle_overlay(void) {
    if (!performance_state.initialized) {
        performance_init();
    }
    
    performance_state.overlay_visible = !performance_state.overlay_visible;
    printf("⚡ Performance overlay: %s\n", 
           performance_state.overlay_visible ? "ON" : "OFF");
}

void performance_set_warning_thresholds(float frame_time_ms, size_t memory_mb) {
    if (!performance_state.initialized) {
        performance_init();
    }
    
    performance_state.frame_time_warning_ms = frame_time_ms;
    performance_state.memory_warning_mb = memory_mb;
    
    printf("⚡ Performance thresholds updated: %.1fms frame time, %zuMB memory\n",
           frame_time_ms, memory_mb);
}

void performance_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)world;      // Unused
    (void)delta_time; // Unused
    
    // Render performance overlay if enabled
    performance_render_overlay(render_config);
}
