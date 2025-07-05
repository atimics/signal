#ifndef PIPELINE_MANAGER_H
#define PIPELINE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "graphics_api.h"

// Pipeline types
typedef enum {
    PIPELINE_TYPE_3D_MAIN,
    PIPELINE_TYPE_3D_OFFSCREEN,
    PIPELINE_TYPE_UI_MAIN,
    PIPELINE_TYPE_UI_OFFSCREEN,
    PIPELINE_TYPE_COMPOSITOR,
    PIPELINE_TYPE_DEBUG_LINES,
    PIPELINE_TYPE_MAX
} PipelineType;

// Handle with generation to detect stale references
typedef struct {
    uint32_t id;
    uint32_t generation;
} PipelineHandle;

// Pipeline manager functions
bool pipeline_manager_init(void);
void pipeline_manager_shutdown(void);

// Create or get existing pipeline
PipelineHandle pipeline_manager_get(PipelineType type);

// Recreate pipeline (e.g., after resize or MSAA change)
PipelineHandle pipeline_manager_recreate(PipelineType type);

// Validate handle is still valid
bool pipeline_manager_is_valid(PipelineHandle handle);

// Get actual Sokol pipeline from handle
sg_pipeline pipeline_manager_get_pipeline(PipelineHandle handle);

// Notify manager of context changes
void pipeline_manager_on_resize(int width, int height);
void pipeline_manager_on_msaa_change(int sample_count);

// Get current swapchain sample count
int pipeline_manager_get_swapchain_samples(void);

#endif // PIPELINE_MANAGER_H