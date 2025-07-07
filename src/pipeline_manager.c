#include "pipeline_manager.h"
#include <string.h>
#include <stdio.h>

// Pipeline info structure
typedef struct {
    sg_pipeline pipeline;
    uint32_t generation;
    PipelineType type;
    bool needs_depth;
    bool is_offscreen;
    const char* label;
} PipelineInfo;

// Pipeline manager state
static struct {
    PipelineInfo pipelines[PIPELINE_TYPE_MAX];
    int swapchain_width;
    int swapchain_height;
    int swapchain_samples;
    bool initialized;
} g_manager = {0};

// Forward declarations
static bool create_3d_pipeline(PipelineInfo* info, bool offscreen);
static bool create_ui_pipeline(PipelineInfo* info, bool offscreen);
static bool create_compositor_pipeline(PipelineInfo* info);

bool pipeline_manager_init(void) {
    if (g_manager.initialized) {
        return true;
    }
    
    memset(&g_manager, 0, sizeof(g_manager));
    
    // Initialize with default swapchain values
    g_manager.swapchain_samples = 1;  // Default to no MSAA
    g_manager.swapchain_width = 1280;
    g_manager.swapchain_height = 720;
    
    // Set up pipeline info
    g_manager.pipelines[PIPELINE_TYPE_3D_MAIN] = (PipelineInfo){
        .type = PIPELINE_TYPE_3D_MAIN,
        .needs_depth = true,
        .is_offscreen = false,
        .label = "3d_main_pipeline"
    };
    
    g_manager.pipelines[PIPELINE_TYPE_3D_OFFSCREEN] = (PipelineInfo){
        .type = PIPELINE_TYPE_3D_OFFSCREEN,
        .needs_depth = true,
        .is_offscreen = true,
        .label = "3d_offscreen_pipeline"
    };
    
    g_manager.pipelines[PIPELINE_TYPE_UI_MAIN] = (PipelineInfo){
        .type = PIPELINE_TYPE_UI_MAIN,
        .needs_depth = false,
        .is_offscreen = false,
        .label = "ui_main_pipeline"
    };
    
    g_manager.pipelines[PIPELINE_TYPE_UI_OFFSCREEN] = (PipelineInfo){
        .type = PIPELINE_TYPE_UI_OFFSCREEN,
        .needs_depth = false,
        .is_offscreen = true,
        .label = "ui_offscreen_pipeline"
    };
    
    g_manager.pipelines[PIPELINE_TYPE_COMPOSITOR] = (PipelineInfo){
        .type = PIPELINE_TYPE_COMPOSITOR,
        .needs_depth = false,
        .is_offscreen = false,
        .label = "compositor_pipeline"
    };
    
    g_manager.initialized = true;
    printf("✅ Pipeline manager initialized\n");
    return true;
}

void pipeline_manager_shutdown(void) {
    if (!g_manager.initialized) {
        return;
    }
    
    // Destroy all pipelines
    for (int i = 0; i < PIPELINE_TYPE_MAX; i++) {
        if (g_manager.pipelines[i].pipeline.id != SG_INVALID_ID) {
            sg_destroy_pipeline(g_manager.pipelines[i].pipeline);
        }
    }
    
    memset(&g_manager, 0, sizeof(g_manager));
    printf("✅ Pipeline manager shut down\n");
}

PipelineHandle pipeline_manager_get(PipelineType type) {
    if (!g_manager.initialized || type >= PIPELINE_TYPE_MAX) {
        return (PipelineHandle){0, 0};
    }
    
    PipelineInfo* info = &g_manager.pipelines[type];
    
    // Create pipeline if it doesn't exist
    if (info->pipeline.id == SG_INVALID_ID) {
        bool success = false;
        
        switch (type) {
            case PIPELINE_TYPE_3D_MAIN:
            case PIPELINE_TYPE_3D_OFFSCREEN:
                success = create_3d_pipeline(info, info->is_offscreen);
                break;
                
            case PIPELINE_TYPE_UI_MAIN:
            case PIPELINE_TYPE_UI_OFFSCREEN:
                success = create_ui_pipeline(info, info->is_offscreen);
                break;
                
            case PIPELINE_TYPE_COMPOSITOR:
                success = create_compositor_pipeline(info);
                break;
                
            default:
                break;
        }
        
        if (!success) {
            printf("❌ Failed to create pipeline type %d\n", type);
            return (PipelineHandle){0, 0};
        }
        
        info->generation++;
    }
    
    return (PipelineHandle){
        .id = info->pipeline.id,
        .generation = info->generation
    };
}

PipelineHandle pipeline_manager_recreate(PipelineType type) {
    if (!g_manager.initialized || type >= PIPELINE_TYPE_MAX) {
        return (PipelineHandle){0, 0};
    }
    
    PipelineInfo* info = &g_manager.pipelines[type];
    
    // Destroy existing pipeline
    if (info->pipeline.id != SG_INVALID_ID) {
        sg_destroy_pipeline(info->pipeline);
        info->pipeline.id = SG_INVALID_ID;
    }
    
    // Create new one
    return pipeline_manager_get(type);
}

bool pipeline_manager_is_valid(PipelineHandle handle) {
    if (!g_manager.initialized || handle.id == 0) {
        return false;
    }
    
    // Find pipeline by ID
    for (int i = 0; i < PIPELINE_TYPE_MAX; i++) {
        if (g_manager.pipelines[i].pipeline.id == handle.id) {
            // Check generation matches
            return g_manager.pipelines[i].generation == handle.generation;
        }
    }
    
    return false;
}

sg_pipeline pipeline_manager_get_pipeline(PipelineHandle handle) {
    if (!pipeline_manager_is_valid(handle)) {
        return (sg_pipeline){SG_INVALID_ID};
    }
    
    // Find and return pipeline
    for (int i = 0; i < PIPELINE_TYPE_MAX; i++) {
        if (g_manager.pipelines[i].pipeline.id == handle.id) {
            return g_manager.pipelines[i].pipeline;
        }
    }
    
    return (sg_pipeline){SG_INVALID_ID};
}

void pipeline_manager_on_resize(int width, int height) {
    g_manager.swapchain_width = width;
    g_manager.swapchain_height = height;
    
    // Recreate all pipelines that might be affected by resize
    printf("🔄 Pipeline manager: Handling resize to %dx%d\n", width, height);
}

void pipeline_manager_on_msaa_change(int sample_count) {
    if (g_manager.swapchain_samples == sample_count) {
        return;
    }
    
    g_manager.swapchain_samples = sample_count;
    
    // Recreate all pipelines affected by MSAA change
    printf("🔄 Pipeline manager: MSAA changed to %dx, recreating pipelines\n", sample_count);
    
    for (int i = 0; i < PIPELINE_TYPE_MAX; i++) {
        if (g_manager.pipelines[i].pipeline.id != SG_INVALID_ID) {
            pipeline_manager_recreate(i);
        }
    }
}

int pipeline_manager_get_swapchain_samples(void) {
    return g_manager.swapchain_samples;
}

// Pipeline creation helpers
static bool create_3d_pipeline(PipelineInfo* info, bool offscreen) {
    // This would normally load shaders from the asset system
    // For now, creating a placeholder
    printf("🔧 Creating 3D pipeline (offscreen=%d)\n", offscreen);
    
    // Note: Real implementation would create actual pipeline here
    // with proper shader, vertex layout, etc.
    info->pipeline = (sg_pipeline){1000 + info->type};  // Fake ID for testing
    
    return true;
}

static bool create_ui_pipeline(PipelineInfo* info, bool offscreen) {
    printf("🔧 Creating UI pipeline (offscreen=%d)\n", offscreen);
    
    // Note: Real implementation would create actual pipeline here
    info->pipeline = (sg_pipeline){2000 + info->type};  // Fake ID for testing
    
    return true;
}

static bool create_compositor_pipeline(PipelineInfo* info) {
    printf("🔧 Creating compositor pipeline\n");
    
    // Note: Real implementation would create actual pipeline here
    info->pipeline = (sg_pipeline){3000 + info->type};  // Fake ID for testing
    
    return true;
}