/**
 * @file render_layers.h
 * @brief Offscreen rendering layer system for compositing 3D scenes, UI, and effects
 * 
 * This system provides complete isolation between different rendering pipelines
 * by rendering to separate offscreen targets and compositing them in a final pass.
 */

#ifndef RENDER_LAYERS_H
#define RENDER_LAYERS_H

#include "sokol_gfx.h"
#include <stdbool.h>

// Forward declarations
struct World;

// ============================================================================
// TYPES AND CONSTANTS
// ============================================================================

#define MAX_RENDER_LAYERS 8
#define LAYER_NAME_MAX_LEN 32

/**
 * Blend modes for layer compositing
 */
typedef enum BlendMode {
    BLEND_MODE_NORMAL = 0,     // Standard alpha blending
    BLEND_MODE_ADDITIVE = 1,   // Additive blending (for effects)
    BLEND_MODE_MULTIPLY = 2,   // Multiply blending
    BLEND_MODE_SCREEN = 3,     // Screen blending
    BLEND_MODE_COUNT
} BlendMode;

/**
 * Update frequency hints for layer optimization
 */
typedef enum UpdateFrequency {
    UPDATE_STATIC = 0,      // Rarely updates (e.g., background)
    UPDATE_DYNAMIC = 1,     // Updates every frame
    UPDATE_ON_DEMAND = 2    // Updates only when marked dirty
} UpdateFrequency;

/**
 * Render layer configuration
 */
typedef struct RenderLayerConfig {
    const char* name;
    int width;
    int height;
    bool needs_depth;
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    int sample_count;
    UpdateFrequency update_frequency;
} RenderLayerConfig;

/**
 * Individual render layer with offscreen target
 */
typedef struct RenderLayer {
    char name[LAYER_NAME_MAX_LEN];
    
    // Render targets
    sg_image color_target;
    sg_image depth_target;
    sg_sampler sampler;
    sg_attachments attachments;  // Render pass attachments
    
    // Layer properties
    int width;
    int height;
    bool enabled;
    int order;              // Rendering order (lower = rendered first)
    float opacity;          // For compositing (0.0 - 1.0)
    BlendMode blend_mode;
    
    // Optimization hints
    UpdateFrequency update_frequency;
    bool dirty;             // Needs redraw
    uint64_t last_update_frame;
    
    // Clear values
    sg_color clear_color;
    float clear_depth;
    uint8_t clear_stencil;
} RenderLayer;

/**
 * Compositor uniforms for fullscreen quad rendering
 */
typedef struct CompositorUniforms {
    float opacity;
    int blend_mode;
    float _pad[2];  // Padding for 16-byte alignment
} CompositorUniforms;

/**
 * Layer manager - handles all render layers and compositing
 */
typedef struct LayerManager {
    RenderLayer layers[MAX_RENDER_LAYERS];
    int layer_count;
    
    // Compositor resources
    sg_pipeline compositor_pipeline;
    sg_buffer fullscreen_quad_vbuf;
    sg_shader compositor_shader;
    
    // Frame tracking
    uint64_t current_frame;
    
    // Screen dimensions
    int screen_width;
    int screen_height;
} LayerManager;

// ============================================================================
// LAYER MANAGER FUNCTIONS
// ============================================================================

/**
 * Create and initialize a new layer manager
 */
LayerManager* layer_manager_create(int screen_width, int screen_height);

/**
 * Destroy layer manager and all its resources
 */
void layer_manager_destroy(LayerManager* manager);

/**
 * Update screen dimensions (recreates render targets if needed)
 */
void layer_manager_resize(LayerManager* manager, int screen_width, int screen_height);

// ============================================================================
// LAYER MANAGEMENT
// ============================================================================

/**
 * Create and add a new render layer
 */
RenderLayer* layer_manager_add_layer(LayerManager* manager, const RenderLayerConfig* config);

/**
 * Get layer by name
 */
RenderLayer* layer_manager_get_layer(LayerManager* manager, const char* name);

/**
 * Remove layer by name
 */
void layer_manager_remove_layer(LayerManager* manager, const char* name);

/**
 * Mark layer as needing redraw
 */
void layer_manager_mark_dirty(LayerManager* manager, const char* layer_name);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Begin rendering to a specific layer
 */
void layer_begin_render(RenderLayer* layer);

/**
 * End rendering to current layer
 */
void layer_end_render(void);

/**
 * Composite all layers to the swapchain
 */
void layer_manager_composite(LayerManager* manager);

/**
 * Get the color texture of a layer (for using as texture in other passes)
 */
sg_image layer_get_color_texture(RenderLayer* layer);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Set layer visibility
 */
void layer_set_enabled(RenderLayer* layer, bool enabled);

/**
 * Set layer opacity for compositing
 */
void layer_set_opacity(RenderLayer* layer, float opacity);

/**
 * Set layer blend mode
 */
void layer_set_blend_mode(RenderLayer* layer, BlendMode mode);

/**
 * Set layer render order
 */
void layer_set_order(RenderLayer* layer, int order);

/**
 * Check if layer needs update based on its frequency settings
 */
bool layer_should_update(LayerManager* manager, RenderLayer* layer);

#endif // RENDER_LAYERS_H