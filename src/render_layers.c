/**
 * @file render_layers.c
 * @brief Implementation of the offscreen rendering layer system
 */

#include "render_layers.h"
#include "sokol_app.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// ============================================================================
// COMPOSITOR SHADER SOURCE
// ============================================================================

#ifndef __APPLE__
static const char* compositor_vs_source = 
    "#version 330\n"
    "const vec2 positions[6] = vec2[](\n"
    "    vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),\n"
    "    vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)\n"
    ");\n"
    "const vec2 texcoords[6] = vec2[](\n"
    "    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0),\n"
    "    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0)\n"
    ");\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);\n"
    "    uv = texcoords[gl_VertexID];\n"
    "}\n";

static const char* compositor_fs_source = 
    "#version 330\n"
    "uniform sampler2D layer_texture;\n"
    "uniform vec4 compositor_params;\n"  // x=opacity, y=blend_mode
    "in vec2 uv;\n"
    "out vec4 frag_color;\n"
    "\n"
    "vec4 blend_normal(vec4 src, vec4 dst, float opacity) {\n"
    "    return mix(dst, src, src.a * opacity);\n"
    "}\n"
    "\n"
    "vec4 blend_additive(vec4 src, vec4 dst, float opacity) {\n"
    "    return dst + src * opacity;\n"
    "}\n"
    "\n"
    "vec4 blend_multiply(vec4 src, vec4 dst, float opacity) {\n"
    "    return mix(dst, dst * src, opacity);\n"
    "}\n"
    "\n"
    "vec4 blend_screen(vec4 src, vec4 dst, float opacity) {\n"
    "    vec4 result = vec4(1.0) - (vec4(1.0) - dst) * (vec4(1.0) - src);\n"
    "    return mix(dst, result, opacity);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec4 src = texture(layer_texture, uv);\n"
    "    float opacity = compositor_params.x;\n"
    "    \n"
    "    // Apply opacity and return\n"
    "    src.rgb *= src.a;  // Premultiply alpha\n"
    "    src *= opacity;\n"
    "    frag_color = src;\n"
    "}\n";
#endif

// Metal shader source for macOS
#ifdef __APPLE__
static const char* compositor_vs_source_metal = 
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct vs_out {\n"
    "    float4 pos [[position]];\n"
    "    float2 uv;\n"
    "};\n"
    "vertex vs_out vs_main(uint vid [[vertex_id]]) {\n"
    "    const float2 positions[6] = {\n"
    "        float2(-1.0, -1.0), float2( 1.0, -1.0), float2( 1.0,  1.0),\n"
    "        float2(-1.0, -1.0), float2( 1.0,  1.0), float2(-1.0,  1.0)\n"
    "    };\n"
    "    const float2 texcoords[6] = {\n"
    "        float2(0.0, 1.0), float2(1.0, 1.0), float2(1.0, 0.0),\n"
    "        float2(0.0, 1.0), float2(1.0, 0.0), float2(0.0, 0.0)\n"
    "    };\n"
    "    vs_out out;\n"
    "    out.pos = float4(positions[vid], 0.0, 1.0);\n"
    "    out.uv = texcoords[vid];\n"
    "    return out;\n"
    "}\n";

static const char* compositor_fs_source_metal = 
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct fs_in {\n"
    "    float2 uv;\n"
    "};\n"
    "fragment float4 fs_main(\n"
    "    fs_in in [[stage_in]],\n"
    "    texture2d<float> layer_texture [[texture(0)]],\n"
    "    sampler layer_sampler [[sampler(0)]],\n"
    "    constant float4& compositor_params [[buffer(0)]]\n"
    ") {\n"
    "    float4 src = layer_texture.sample(layer_sampler, in.uv);\n"
    "    float opacity = compositor_params.x;\n"
    "    int blend_mode = int(compositor_params.y);\n"
    "    \n"
    "    // Apply opacity and return\n"
    "    src.rgb *= src.a;  // Premultiply alpha\n"
    "    src *= opacity;\n"
    "    return src;\n"
    "}\n";
#endif

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

static void create_compositor_resources(LayerManager* manager) {
    // Create compositor shader
    sg_shader_desc shader_desc = {
        .label = "compositor_shader"
    };
    
#ifdef __APPLE__
    shader_desc.vertex_func = (sg_shader_function){
        .source = compositor_vs_source_metal,
        .entry = "vs_main"
    };
    shader_desc.fragment_func = (sg_shader_function){
        .source = compositor_fs_source_metal,
        .entry = "fs_main"
    };
    shader_desc.images[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.images[0].image_type = SG_IMAGETYPE_2D;
    shader_desc.images[0].sample_type = SG_IMAGESAMPLETYPE_FLOAT;
    
    shader_desc.samplers[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.samplers[0].sampler_type = SG_SAMPLERTYPE_FILTERING;
    
    shader_desc.image_sampler_pairs[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.image_sampler_pairs[0].image_slot = 0;
    shader_desc.image_sampler_pairs[0].sampler_slot = 0;
    
    shader_desc.uniform_blocks[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.uniform_blocks[0].size = sizeof(float) * 4;
    shader_desc.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_NATIVE;
#else
    shader_desc.vertex_func = (sg_shader_function){
        .source = compositor_vs_source,
        .entry = "main"
    };
    shader_desc.fragment_func = (sg_shader_function){
        .source = compositor_fs_source,
        .entry = "main"
    };
    shader_desc.images[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.images[0].image_type = SG_IMAGETYPE_2D;
    shader_desc.images[0].sample_type = SG_IMAGESAMPLETYPE_FLOAT;
    
    shader_desc.samplers[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.samplers[0].sampler_type = SG_SAMPLERTYPE_FILTERING;
    
    shader_desc.image_sampler_pairs[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.image_sampler_pairs[0].glsl_name = "layer_texture";
    shader_desc.image_sampler_pairs[0].image_slot = 0;
    shader_desc.image_sampler_pairs[0].sampler_slot = 0;
    
    shader_desc.uniform_blocks[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shader_desc.uniform_blocks[0].size = sizeof(float) * 4;
    shader_desc.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_NATIVE;
    shader_desc.uniform_blocks[0].glsl_name = "compositor_params";
#endif
    
    manager->compositor_shader = sg_make_shader(&shader_desc);
    
    if (manager->compositor_shader.id == SG_INVALID_ID) {
        printf("❌ ERROR: Failed to create compositor shader!\n");
        return;
    }
    
    printf("✅ Created compositor shader (id=%u)\n", manager->compositor_shader.id);
    
    // Create compositor pipeline
    manager->compositor_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = manager->compositor_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .cull_mode = SG_CULLMODE_NONE,
        .face_winding = SG_FACEWINDING_CCW,
        .colors[0] = {
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
            }
        },
        .label = "compositor_pipeline"
    });
    
    if (manager->compositor_pipeline.id == SG_INVALID_ID) {
        printf("❌ ERROR: Failed to create compositor pipeline!\n");
        return;
    }
    
    printf("✅ Created compositor pipeline (id=%u)\n", manager->compositor_pipeline.id);
    
    // Create fullscreen quad vertex buffer (empty - we generate vertices in shader)
    float dummy_data[6] = {0};  // Dummy data for immutable buffer
    manager->fullscreen_quad_vbuf = sg_make_buffer(&(sg_buffer_desc){
        .size = 6 * sizeof(float),  // Dummy buffer
        .data = SG_RANGE(dummy_data),
        .label = "compositor_quad"
    });
}

static void destroy_layer(RenderLayer* layer) {
    if (layer->color_target.id != SG_INVALID_ID) {
        sg_destroy_image(layer->color_target);
    }
    if (layer->depth_target.id != SG_INVALID_ID) {
        sg_destroy_image(layer->depth_target);
    }
    if (layer->sampler.id != SG_INVALID_ID) {
        sg_destroy_sampler(layer->sampler);
    }
    if (layer->attachments.id != SG_INVALID_ID) {
        sg_destroy_attachments(layer->attachments);
    }
    memset(layer, 0, sizeof(RenderLayer));
}

static int compare_layer_order(const void* a, const void* b) {
    const RenderLayer* layer_a = (const RenderLayer*)a;
    const RenderLayer* layer_b = (const RenderLayer*)b;
    return layer_a->order - layer_b->order;
}

// ============================================================================
// LAYER MANAGER FUNCTIONS
// ============================================================================

LayerManager* layer_manager_create(int screen_width, int screen_height) {
    LayerManager* manager = calloc(1, sizeof(LayerManager));
    if (!manager) {
        return NULL;
    }
    
    manager->screen_width = screen_width;
    manager->screen_height = screen_height;
    manager->layer_count = 0;
    manager->current_frame = 0;
    
    // Create compositor resources
    create_compositor_resources(manager);
    
    return manager;
}

void layer_manager_destroy(LayerManager* manager) {
    if (!manager) return;
    
    // Destroy all layers
    for (int i = 0; i < manager->layer_count; i++) {
        destroy_layer(&manager->layers[i]);
    }
    
    // Destroy compositor resources
    if (manager->compositor_pipeline.id != SG_INVALID_ID) {
        sg_destroy_pipeline(manager->compositor_pipeline);
    }
    if (manager->compositor_shader.id != SG_INVALID_ID) {
        sg_destroy_shader(manager->compositor_shader);
    }
    if (manager->fullscreen_quad_vbuf.id != SG_INVALID_ID) {
        sg_destroy_buffer(manager->fullscreen_quad_vbuf);
    }
    
    free(manager);
}

void layer_manager_resize(LayerManager* manager, int screen_width, int screen_height) {
    if (!manager) return;
    
    manager->screen_width = screen_width;
    manager->screen_height = screen_height;
    
    // Recreate all layers at new resolution
    for (int i = 0; i < manager->layer_count; i++) {
        RenderLayer* layer = &manager->layers[i];
        
        // Store layer config
        RenderLayerConfig config = {
            .name = layer->name,
            .width = screen_width,
            .height = screen_height,
            .needs_depth = (layer->depth_target.id != SG_INVALID_ID),
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = layer->update_frequency
        };
        
        // Store properties
        bool enabled = layer->enabled;
        int order = layer->order;
        float opacity = layer->opacity;
        BlendMode blend_mode = layer->blend_mode;
        sg_color clear_color = layer->clear_color;
        
        // Destroy old resources
        destroy_layer(layer);
        
        // Recreate layer
        *layer = (RenderLayer){0};
        strncpy(layer->name, config.name, LAYER_NAME_MAX_LEN - 1);
        layer->width = config.width;
        layer->height = config.height;
        layer->enabled = enabled;
        layer->order = order;
        layer->opacity = opacity;
        layer->blend_mode = blend_mode;
        layer->clear_color = clear_color;
        layer->update_frequency = config.update_frequency;
        layer->dirty = true;
        
        // Create color target
        layer->color_target = sg_make_image(&(sg_image_desc){
            .usage = { .render_attachment = true, .immutable = true },
            .width = config.width,
            .height = config.height,
            .pixel_format = config.color_format,
            .label = config.name
        });
        
        // Create depth target if needed
        if (config.needs_depth) {
            layer->depth_target = sg_make_image(&(sg_image_desc){
                .usage = { .render_attachment = true, .immutable = true },
                .width = config.width,
                .height = config.height,
                .pixel_format = config.depth_format,
                .label = config.name
            });
        }
        
        // Create sampler
        layer->sampler = sg_make_sampler(&(sg_sampler_desc){
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
            .label = config.name
        });
        
        // Create attachments
        layer->attachments = sg_make_attachments(&(sg_attachments_desc){
            .colors[0] = {
                .image = layer->color_target
            },
            .depth_stencil = config.needs_depth ? (sg_attachment_desc){
                .image = layer->depth_target
            } : (sg_attachment_desc){0},
            .label = config.name
        });
    }
}

// ============================================================================
// LAYER MANAGEMENT
// ============================================================================

RenderLayer* layer_manager_add_layer(LayerManager* manager, const RenderLayerConfig* config) {
    if (!manager || !config || manager->layer_count >= MAX_RENDER_LAYERS) {
        return NULL;
    }
    
    RenderLayer* layer = &manager->layers[manager->layer_count];
    memset(layer, 0, sizeof(RenderLayer));
    
    // Copy name
    strncpy(layer->name, config->name, LAYER_NAME_MAX_LEN - 1);
    layer->name[LAYER_NAME_MAX_LEN - 1] = '\0';
    
    // Set properties
    layer->width = config->width;
    layer->height = config->height;
    layer->enabled = true;
    layer->order = manager->layer_count;  // Default order
    layer->opacity = 1.0f;
    layer->blend_mode = BLEND_MODE_NORMAL;
    layer->update_frequency = config->update_frequency;
    layer->dirty = true;
    layer->clear_color = (sg_color){ 0.0f, 0.0f, 0.0f, 0.0f };
    layer->clear_depth = 1.0f;
    
    // Create color target
    layer->color_target = sg_make_image(&(sg_image_desc){
        .usage = { .render_attachment = true, .immutable = true },
        .width = config->width,
        .height = config->height,
        .pixel_format = config->color_format,
        .label = config->name
    });
    
    // Create depth target if needed
    if (config->needs_depth) {
        layer->depth_target = sg_make_image(&(sg_image_desc){
            .usage = { .render_attachment = true, .immutable = true },
            .width = config->width,
            .height = config->height,
            .pixel_format = config->depth_format,
            .label = config->name
        });
    }
    
    // Create sampler
    layer->sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .label = config->name
    });
    
    // Create attachments
    layer->attachments = sg_make_attachments(&(sg_attachments_desc){
        .colors[0] = {
            .image = layer->color_target
        },
        .depth_stencil = config->needs_depth ? (sg_attachment_desc){
            .image = layer->depth_target
        } : (sg_attachment_desc){0},
        .label = config->name
    });
    
    manager->layer_count++;
    
    printf("✅ Created render layer '%s' (%dx%d, depth=%s)\n", 
           config->name, config->width, config->height, 
           config->needs_depth ? "yes" : "no");
    
    return layer;
}

RenderLayer* layer_manager_get_layer(LayerManager* manager, const char* name) {
    if (!manager || !name) return NULL;
    
    for (int i = 0; i < manager->layer_count; i++) {
        if (strcmp(manager->layers[i].name, name) == 0) {
            return &manager->layers[i];
        }
    }
    
    return NULL;
}

void layer_manager_remove_layer(LayerManager* manager, const char* name) {
    if (!manager || !name) return;
    
    for (int i = 0; i < manager->layer_count; i++) {
        if (strcmp(manager->layers[i].name, name) == 0) {
            // Destroy layer resources
            destroy_layer(&manager->layers[i]);
            
            // Shift remaining layers
            for (int j = i; j < manager->layer_count - 1; j++) {
                manager->layers[j] = manager->layers[j + 1];
            }
            
            manager->layer_count--;
            printf("✅ Removed render layer '%s'\n", name);
            return;
        }
    }
}

void layer_manager_mark_dirty(LayerManager* manager, const char* layer_name) {
    RenderLayer* layer = layer_manager_get_layer(manager, layer_name);
    if (layer) {
        layer->dirty = true;
    }
}

// ============================================================================
// RENDERING
// ============================================================================

// Track render pass state
static struct {
    bool pass_active;
    const char* layer_name;
} g_pass_state = {false, NULL};

void layer_begin_render(RenderLayer* layer) {
    if (!layer || !layer->enabled) return;
    
    if (!sg_isvalid()) {
        printf("⚠️ WARNING: Skipping layer_begin_render - context invalid\n");
        return;
    }
    
    if (g_pass_state.pass_active) {
        printf("⚠️ ERROR: Attempting to begin pass for layer '%s' while pass for '%s' is active!\n", 
               layer->name, g_pass_state.layer_name ? g_pass_state.layer_name : "unknown");
        return;
    }
    
    g_pass_state.pass_active = true;
    g_pass_state.layer_name = layer->name;
    
    sg_begin_pass(&(sg_pass){
        .attachments = layer->attachments,
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = layer->clear_color
            },
            .depth = {
                .load_action = layer->depth_target.id != SG_INVALID_ID ? SG_LOADACTION_CLEAR : SG_LOADACTION_DONTCARE,
                .clear_value = layer->clear_depth
            },
            .stencil = {
                .load_action = layer->depth_target.id != SG_INVALID_ID ? SG_LOADACTION_CLEAR : SG_LOADACTION_DONTCARE,
                .clear_value = layer->clear_stencil
            }
        }
    });
    
    layer->last_update_frame = layer->last_update_frame + 1;  // Track updates
    layer->dirty = false;
}

void layer_end_render(void) {
    if (!g_pass_state.pass_active) {
        printf("⚠️ ERROR: layer_end_render called but no pass is active!\n");
        return;
    }
    
    if (sg_isvalid()) {
        sg_end_pass();
    } else {
        printf("⚠️ WARNING: Context became invalid during rendering of '%s' - NOT calling sg_end_pass\n",
               g_pass_state.layer_name ? g_pass_state.layer_name : "unknown");
    }
    
    // Clear the state
    g_pass_state.pass_active = false;
    g_pass_state.layer_name = NULL;
}

void layer_manager_composite(LayerManager* manager) {
    if (!manager || manager->layer_count == 0) {
        printf("⚠️ layer_manager_composite: No manager or no layers\n");
        return;
    }
    
    // Sort layers by order
    qsort(manager->layers, manager->layer_count, sizeof(RenderLayer), compare_layer_order);
    
    // Apply compositor pipeline
    sg_apply_pipeline(manager->compositor_pipeline);
    
    // Track if any layers were composited
    int layers_composited = 0;
    
    // Composite each layer
    for (int i = 0; i < manager->layer_count; i++) {
        RenderLayer* layer = &manager->layers[i];
        if (!layer->enabled || layer->opacity <= 0.0f) {
            printf("🔍 Layer '%s': skipped (enabled=%d, opacity=%.2f)\n", 
                   layer->name, layer->enabled, layer->opacity);
            continue;
        }
        
        // Debug: Compositing layer (commented out to reduce spam)
        // printf("🎨 Compositing layer '%s': order=%d, opacity=%.2f, blend=%d\n",
        //        layer->name, layer->order, layer->opacity, layer->blend_mode);
        
        // Validate layer texture
        if (layer->color_target.id == SG_INVALID_ID) {
            printf("❌ ERROR: Layer '%s' has invalid color target!\n", layer->name);
            continue;
        }
        
        // Bind layer texture
        sg_bindings binds = {
            .vertex_buffers[0] = manager->fullscreen_quad_vbuf,
            .images[0] = layer->color_target,
            .samplers[0] = layer->sampler
        };
        sg_apply_bindings(&binds);
        
        // Set compositor uniforms
        float params[4] = {
            layer->opacity,
            (float)layer->blend_mode,
            0.0f, 0.0f
        };
        sg_apply_uniforms(0, &(sg_range){ .ptr = params, .size = sizeof(params) });
        
        // Draw fullscreen quad
        sg_draw(0, 6, 1);
        layers_composited++;
    }
    
    if (layers_composited == 0) {
        printf("⚠️ WARNING: No layers were composited!\n");
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

sg_image layer_get_color_texture(RenderLayer* layer) {
    return layer ? layer->color_target : (sg_image){0};
}

void layer_set_enabled(RenderLayer* layer, bool enabled) {
    if (layer) {
        layer->enabled = enabled;
    }
}

void layer_set_opacity(RenderLayer* layer, float opacity) {
    if (layer) {
        layer->opacity = opacity < 0.0f ? 0.0f : (opacity > 1.0f ? 1.0f : opacity);
    }
}

void layer_set_blend_mode(RenderLayer* layer, BlendMode mode) {
    if (layer && mode < BLEND_MODE_COUNT) {
        layer->blend_mode = mode;
    }
}

void layer_set_order(RenderLayer* layer, int order) {
    if (layer) {
        layer->order = order;
    }
}

bool layer_should_update(LayerManager* manager, RenderLayer* layer) {
    (void)manager;  // Unused for now, but may be used for frame-based updates
    if (!layer || !layer->enabled) return false;
    
    switch (layer->update_frequency) {
        case UPDATE_STATIC:
            return layer->dirty;
            
        case UPDATE_DYNAMIC:
            return true;
            
        case UPDATE_ON_DEMAND:
            return layer->dirty;
            
        default:
            return true;
    }
}