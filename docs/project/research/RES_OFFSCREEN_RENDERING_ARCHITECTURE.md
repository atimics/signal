# Offscreen Rendering and Compositing Architecture

**Document Type**: Technical Research & Architecture Design
**Date**: January 5, 2025
**Status**: Active Research
**Category**: Rendering Architecture
**Priority**: HIGH - Solves current rendering conflicts

## Executive Summary

This document presents a comprehensive architecture for implementing offscreen rendering and compositing in SIGNAL. The proposed system solves current pipeline state conflicts by rendering different content types (3D scenes, UI, effects) to separate offscreen targets, then compositing them in a final pass. This approach provides complete isolation between rendering pipelines while enabling advanced visual effects.

## Problem Analysis

### Current Issues
1. **Pipeline State Conflicts**: UI and 3D pipelines conflict within single render pass
2. **Context Invalidation**: App context becomes invalid during rendering
3. **Limited Flexibility**: Cannot apply post-processing to individual layers
4. **No Render-to-Texture**: Cannot use rendered content as textures elsewhere

### Root Cause
The current architecture attempts to render all content types in a single pass to the swapchain, causing:
- State management complexity
- Pipeline compatibility issues
- Limited compositing options
- Debugging difficulties

## Proposed Architecture

### Overview
```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   3D Scene      │     │       UI        │     │    Effects      │
│ Render Target   │     │ Render Target   │     │ Render Target   │
└────────┬────────┘     └────────┬────────┘     └────────┬────────┘
         │                       │                         │
         └───────────────────────┴─────────────────────────┘
                                 │
                        ┌────────▼────────┐
                        │   Compositor    │
                        │  Render Pass    │
                        └────────┬────────┘
                                 │
                        ┌────────▼────────┐
                        │   Swapchain     │
                        │  (Final Output) │
                        └─────────────────┘
```

### Layer System Design

```c
// Render layer definition
typedef struct RenderLayer {
    const char* name;
    sg_image color_target;
    sg_image depth_target;
    sg_sampler sampler;
    sg_pipeline pipeline;
    sg_pass pass;
    bool enabled;
    int order;  // Rendering order (0 = first)
    float opacity;  // For compositing
    BlendMode blend_mode;
} RenderLayer;

// Layer manager
typedef struct LayerManager {
    RenderLayer* layers;
    int layer_count;
    int max_layers;
    
    // Compositor resources
    sg_pipeline compositor_pipeline;
    sg_buffer fullscreen_quad;
    sg_shader compositor_shader;
} LayerManager;
```

## Implementation Details

### 1. Render Target Creation

```c
RenderLayer* create_render_layer(const char* name, int width, int height, bool needs_depth) {
    RenderLayer* layer = calloc(1, sizeof(RenderLayer));
    layer->name = strdup(name);
    
    // Create color target
    layer->color_target = sg_make_image(&(sg_image_desc){
        .render_target = true,
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .sample_count = 1,
        .label = name
    });
    
    // Create depth target if needed
    if (needs_depth) {
        layer->depth_target = sg_make_image(&(sg_image_desc){
            .render_target = true,
            .width = width,
            .height = height,
            .pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .label = name
        });
    }
    
    // Create sampler for composition
    layer->sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    });
    
    // Create render pass
    layer->pass = sg_make_pass(&(sg_pass_desc){
        .color_attachments[0] = {
            .image = layer->color_target,
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.0f, 0.0f, 0.0f }
        },
        .depth_stencil_attachment = needs_depth ? (sg_pass_attachment_desc){
            .image = layer->depth_target,
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { .depth = 1.0f }
        } : (sg_pass_attachment_desc){0},
        .label = name
    });
    
    return layer;
}
```

### 2. Layer Rendering Pipeline

```c
void render_layers(LayerManager* manager, World* world, float dt) {
    // Sort layers by order
    qsort(manager->layers, manager->layer_count, sizeof(RenderLayer), compare_layer_order);
    
    // Render each layer to its offscreen target
    for (int i = 0; i < manager->layer_count; i++) {
        RenderLayer* layer = &manager->layers[i];
        if (!layer->enabled) continue;
        
        // Begin offscreen pass
        sg_begin_pass(&layer->pass);
        
        // Apply layer-specific pipeline
        sg_apply_pipeline(layer->pipeline);
        
        // Render layer content
        if (strcmp(layer->name, "3d_scene") == 0) {
            render_3d_scene(world, dt);
        } else if (strcmp(layer->name, "ui") == 0) {
            render_ui_layer(world, dt);
        } else if (strcmp(layer->name, "effects") == 0) {
            render_effects_layer(world, dt);
        }
        
        sg_end_pass();
    }
}
```

### 3. Compositor Implementation

```c
void composite_layers(LayerManager* manager) {
    // Begin final swapchain pass
    sg_begin_pass(&(sg_pass){
        .swapchain = sglue_swapchain(),
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
            }
        }
    });
    
    // Apply compositor pipeline
    sg_apply_pipeline(manager->compositor_pipeline);
    
    // Composite each layer
    for (int i = 0; i < manager->layer_count; i++) {
        RenderLayer* layer = &manager->layers[i];
        if (!layer->enabled || layer->opacity <= 0.0f) continue;
        
        // Bind layer texture
        sg_bindings binds = {
            .vertex_buffers[0] = manager->fullscreen_quad,
            .images[0] = layer->color_target,
            .samplers[0] = layer->sampler
        };
        sg_apply_bindings(&binds);
        
        // Set compositor uniforms
        compositor_uniforms_t uniforms = {
            .opacity = layer->opacity,
            .blend_mode = layer->blend_mode
        };
        sg_apply_uniforms(0, &SG_RANGE(uniforms));
        
        // Draw fullscreen quad
        sg_draw(0, 6, 1);
    }
    
    sg_end_pass();
}
```

### 4. Compositor Shader

```glsl
// Vertex shader
#version 330
const vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),
    vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)
);

const vec2 texcoords[6] = vec2[](
    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0),
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0)
);

out vec2 uv;

void main() {
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    uv = texcoords[gl_VertexID];
}

// Fragment shader
#version 330
uniform sampler2D layer_texture;
uniform float opacity;
uniform int blend_mode;

in vec2 uv;
out vec4 frag_color;

vec4 blend_normal(vec4 src, vec4 dst) {
    return mix(dst, src, src.a * opacity);
}

vec4 blend_additive(vec4 src, vec4 dst) {
    return dst + src * opacity;
}

vec4 blend_multiply(vec4 src, vec4 dst) {
    return mix(dst, dst * src, opacity);
}

void main() {
    vec4 src = texture(layer_texture, uv);
    vec4 dst = frag_color;  // Current framebuffer content
    
    switch(blend_mode) {
        case 0: frag_color = blend_normal(src, dst); break;
        case 1: frag_color = blend_additive(src, dst); break;
        case 2: frag_color = blend_multiply(src, dst); break;
        default: frag_color = src;
    }
}
```

## Advanced Features

### 1. Post-Processing Effects

```c
typedef struct PostProcess {
    sg_pipeline pipeline;
    sg_shader shader;
    void* uniforms;
    size_t uniform_size;
} PostProcess;

void apply_post_process(RenderLayer* layer, PostProcess* effect) {
    // Create temporary target
    sg_image temp = sg_make_image(&(sg_image_desc){
        .render_target = true,
        .width = layer->width,
        .height = layer->height,
        .pixel_format = SG_PIXELFORMAT_RGBA8
    });
    
    // Render effect to temporary target
    sg_begin_pass(&(sg_pass){ .color_attachments[0].image = temp });
    sg_apply_pipeline(effect->pipeline);
    sg_apply_bindings(&(sg_bindings){
        .images[0] = layer->color_target,
        .samplers[0] = layer->sampler
    });
    sg_apply_uniforms(0, &(sg_range){ 
        .ptr = effect->uniforms, 
        .size = effect->uniform_size 
    });
    sg_draw(0, 6, 1);
    sg_end_pass();
    
    // Swap targets
    sg_destroy_image(layer->color_target);
    layer->color_target = temp;
}
```

### 2. Dynamic Resolution Scaling

```c
void update_layer_resolution(RenderLayer* layer, float scale) {
    int new_width = (int)(layer->base_width * scale);
    int new_height = (int)(layer->base_height * scale);
    
    // Recreate render targets at new resolution
    sg_destroy_image(layer->color_target);
    sg_destroy_image(layer->depth_target);
    sg_destroy_pass(layer->pass);
    
    // Recreate with new dimensions
    layer->color_target = sg_make_image(&(sg_image_desc){
        .render_target = true,
        .width = new_width,
        .height = new_height,
        .pixel_format = SG_PIXELFORMAT_RGBA8
    });
    
    // Update render pass
    layer->pass = sg_make_pass(/* ... */);
}
```

### 3. Layer Caching and Optimization

```c
typedef struct LayerCache {
    bool dirty;
    uint64_t last_update_frame;
    uint32_t content_hash;
} LayerCache;

bool should_update_layer(RenderLayer* layer, LayerCache* cache) {
    // Skip static layers that haven't changed
    if (!cache->dirty && layer->update_frequency == UPDATE_STATIC) {
        return false;
    }
    
    // Update at specified frequency
    uint64_t frame_delta = current_frame - cache->last_update_frame;
    if (frame_delta < layer->update_interval) {
        return false;
    }
    
    return true;
}
```

## Integration Strategy

### Phase 1: Core Implementation (Week 1)
1. Implement basic layer system
2. Create offscreen render targets
3. Basic compositor with normal blending
4. Migrate 3D scene to offscreen target

### Phase 2: UI Integration (Week 2)
1. Create dedicated UI layer
2. Implement UI-specific pipeline
3. Add transparency support
4. Test with MicroUI

### Phase 3: Advanced Features (Week 3)
1. Multiple blend modes
2. Post-processing effects
3. Dynamic resolution
4. Performance optimizations

### Phase 4: Polish (Week 4)
1. Debug visualization tools
2. Performance profiling
3. Memory optimization
4. Documentation

## Performance Considerations

### Memory Usage
- **Per Layer**: ~16MB for 1920x1080 RGBA + Depth
- **Optimization**: Share depth buffers between compatible layers
- **Dynamic**: Release unused layers, reduce resolution when needed

### GPU Performance
- **Fill Rate**: Multiple passes increase fill rate requirements
- **Bandwidth**: Offscreen targets increase memory bandwidth
- **Optimization**: Render lower resolution for some layers

### Optimization Strategies
1. **Conditional Updates**: Only update changed layers
2. **Resolution Scaling**: Render UI at native, 3D at lower resolution
3. **Format Optimization**: Use appropriate pixel formats per layer
4. **Depth Sharing**: Share depth buffers when possible

## Benefits

### 1. **Complete Pipeline Isolation**
- No state conflicts between rendering systems
- Each layer has independent pipeline configuration
- Easier debugging and maintenance

### 2. **Advanced Visual Effects**
- Post-processing per layer
- Complex blending modes
- Motion blur, DOF, bloom per layer

### 3. **Performance Flexibility**
- Render different layers at different rates
- Dynamic resolution per layer
- Skip unchanged static layers

### 4. **Debugging Capabilities**
- Visualize individual layers
- Save layer contents to disk
- Performance metrics per layer

## Implementation Example

```c
// Initialize layer system
LayerManager* manager = create_layer_manager();

// Create layers
RenderLayer* scene_layer = create_render_layer("3d_scene", 1920, 1080, true);
scene_layer->pipeline = create_3d_pipeline();
scene_layer->order = 0;

RenderLayer* ui_layer = create_render_layer("ui", 1920, 1080, false);
ui_layer->pipeline = create_ui_pipeline();
ui_layer->order = 1;
ui_layer->blend_mode = BLEND_NORMAL;

add_layer(manager, scene_layer);
add_layer(manager, ui_layer);

// Main render loop
void frame() {
    // Update and render all layers
    render_layers(manager, world, dt);
    
    // Composite to screen
    composite_layers(manager);
    
    // Present
    sg_commit();
}
```

## Conclusion

Implementing offscreen rendering and compositing solves the current pipeline conflict issues while providing a flexible, extensible architecture for future rendering needs. The layer-based approach enables advanced visual effects, better performance optimization, and cleaner code organization.

The proposed architecture provides:
- Complete isolation between rendering pipelines
- Flexibility for post-processing and effects
- Better debugging and profiling capabilities
- Foundation for advanced rendering techniques

This approach transforms the rendering architecture from a monolithic single-pass system to a modular, composable pipeline that can evolve with the project's needs.

## Related Documents
- [UI Render Systems Improvements](RES_UI_RENDER_SYSTEMS_IMPROVEMENTS.md)
- [UI Render Crash Solution](RES_UI_RENDER_CRASH_SOLUTION.md)
- [C Architecture Analysis](RES_C_ARCHITECTURE_ANALYSIS.md)