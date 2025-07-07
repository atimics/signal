#include "graphics_health.h"
#include "graphics_api.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Draw call history for diagnostics
#define MAX_DRAW_HISTORY 64

typedef struct {
    char description[128];
    int vertex_count;
    uint32_t frame;
    time_t timestamp;
} DrawCallRecord;

static struct {
    GraphicsHealthStats stats;
    DrawCallRecord draw_history[MAX_DRAW_HISTORY];
    int draw_history_index;
    bool initialized;
    uint32_t current_frame;
} g_health = {0};

void gfx_health_init(void) {
    memset(&g_health, 0, sizeof(g_health));
    g_health.initialized = true;
    printf("✅ Graphics health monitoring initialized\n");
}

bool gfx_health_check(const char* system_name) {
    if (!g_health.initialized) {
        return true;  // Don't fail if not initialized
    }
    
    g_health.current_frame++;
    
    // Check if Sokol context is valid
    if (!sg_isvalid()) {
        g_health.stats.context_invalidations++;
        g_health.stats.last_error_frame = g_health.current_frame;
        snprintf(g_health.stats.last_error_msg, sizeof(g_health.stats.last_error_msg),
                 "Context invalid after %s", system_name);
        
        printf("❌ HEALTH CHECK FAILED: %s\n", g_health.stats.last_error_msg);
        
        // In debug builds, attempt recovery
        #ifdef DEBUG
        if (g_health.stats.context_invalidations > 3) {
            printf("🔧 Attempting graphics recovery...\n");
            return gfx_health_attempt_recovery();
        }
        #endif
        
        return false;
    }
    
    return true;
}

const GraphicsHealthStats* gfx_health_get_stats(void) {
    return &g_health.stats;
}

void gfx_health_reset_stats(void) {
    memset(&g_health.stats, 0, sizeof(g_health.stats));
    g_health.draw_history_index = 0;
    printf("📊 Graphics health stats reset\n");
}

void gfx_health_log_draw_call(const char* desc, int vertex_count) {
    if (!g_health.initialized) {
        return;
    }
    
    DrawCallRecord* record = &g_health.draw_history[g_health.draw_history_index];
    strncpy(record->description, desc, sizeof(record->description) - 1);
    record->vertex_count = vertex_count;
    record->frame = g_health.current_frame;
    record->timestamp = time(NULL);
    
    g_health.draw_history_index = (g_health.draw_history_index + 1) % MAX_DRAW_HISTORY;
}

bool gfx_health_attempt_recovery(void) {
    printf("🚨 GRAPHICS RECOVERY: Attempting to restart graphics backend...\n");
    
    // Log current state
    gfx_health_dump_diagnostics();
    
    // In a real implementation, this would:
    // 1. Save current render state
    // 2. Call sg_shutdown()
    // 3. Call sg_setup() with same parameters
    // 4. Recreate all resources
    // 5. Restore render state
    
    // For now, just log the attempt
    printf("⚠️ Graphics recovery not fully implemented - manual restart required\n");
    
    return false;
}

void gfx_health_dump_diagnostics(void) {
    printf("\n=== GRAPHICS HEALTH DIAGNOSTICS ===\n");
    printf("Current Frame: %u\n", g_health.current_frame);
    printf("Context Invalidations: %u\n", g_health.stats.context_invalidations);
    printf("Pipeline Failures: %u\n", g_health.stats.pipeline_failures);
    printf("Buffer Overflows: %u\n", g_health.stats.buffer_overflows);
    printf("Draw Calls Dropped: %u\n", g_health.stats.draw_calls_dropped);
    printf("Frames Rendered: %u\n", g_health.stats.frames_rendered);
    
    if (g_health.stats.last_error_frame > 0) {
        printf("Last Error: Frame %u - %s\n", 
               g_health.stats.last_error_frame, 
               g_health.stats.last_error_msg);
    }
    
    printf("\n--- Recent Draw Calls ---\n");
    int start = (g_health.draw_history_index - 10 + MAX_DRAW_HISTORY) % MAX_DRAW_HISTORY;
    for (int i = 0; i < 10; i++) {
        int idx = (start + i) % MAX_DRAW_HISTORY;
        DrawCallRecord* record = &g_health.draw_history[idx];
        if (record->frame > 0) {
            printf("  Frame %u: %s (%d vertices)\n", 
                   record->frame, record->description, record->vertex_count);
        }
    }
    
    printf("\n--- Sokol Graphics Info ---\n");
    sg_backend backend = sg_query_backend();
    printf("Backend: ");
    switch (backend) {
        case SG_BACKEND_GLCORE: printf("OpenGL Core\n"); break;
        case SG_BACKEND_GLES3: printf("OpenGL ES3\n"); break;
        case SG_BACKEND_D3D11: printf("Direct3D 11\n"); break;
        case SG_BACKEND_METAL_IOS: printf("Metal iOS\n"); break;
        case SG_BACKEND_METAL_MACOS: printf("Metal macOS\n"); break;
        case SG_BACKEND_METAL_SIMULATOR: printf("Metal Simulator\n"); break;
        case SG_BACKEND_WGPU: printf("WebGPU\n"); break;
        case SG_BACKEND_DUMMY: printf("Dummy\n"); break;
        default: printf("Unknown\n");
    }
    
    sg_features features = sg_query_features();
    printf("Features:\n");
    printf("  Origin top-left: %s\n", features.origin_top_left ? "yes" : "no");
    printf("  Image clamp to border: %s\n", features.image_clamp_to_border ? "yes" : "no");
    printf("  MRT independent blend: %s\n", features.mrt_independent_blend_state ? "yes" : "no");
    printf("  MRT independent mask: %s\n", features.mrt_independent_write_mask ? "yes" : "no");
    printf("  Compute shaders: %s\n", features.compute ? "yes" : "no");
    printf("  MSAA image bindings: %s\n", features.msaa_image_bindings ? "yes" : "no");
    
    printf("=================================\n\n");
}