#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // For mkdir

#include "assets.h"
#include "gpu_resources.h"
#include "graphics_api.h"
#include "render.h"
#include "render_camera.h"
#include "render_lighting.h"
#include "render_mesh.h"
#include "sokol_gfx.h"  // Direct inclusion for rendering implementation
#include "ui.h"

// Internal conversion functions for PIMPL pattern
// These are only used within the rendering implementation
static sg_buffer gpu_buffer_to_sg(gpu_buffer_t buf)
{
    return (sg_buffer){ .id = buf.id };
}

static sg_image gpu_image_to_sg(gpu_image_t img)
{
    return (sg_image){ .id = img.id };
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

// Performance monitoring structure (Sprint 08 Review Action Item)
static struct
{
    uint32_t entities_processed;   // Total entities examined this frame
    uint32_t entities_rendered;    // Successfully rendered entities
    uint32_t entities_culled;      // Entities skipped due to visibility/validation
    uint32_t draw_calls;           // Number of sg_draw() calls made
    uint32_t validation_failures;  // Entities that failed validation
    float frame_time_ms;           // Time taken for this frame
    uint32_t frame_count;          // Total frames processed
} render_performance = { 0 };

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Forward declarations for static functions
static bool validate_entity_for_rendering(struct Entity* entity, struct Transform* transform,
                                          struct Renderable* renderable, uint32_t frame_count);
static void report_render_performance(void);

// ============================================================================
// SOKOL RENDERING DEFINITIONS
// ============================================================================

// Uniform data structures
typedef struct
{
    float mvp[16];  // Model-View-Projection matrix
} vs_uniforms_t;

typedef struct
{
    float light_dir[3];  // Light direction
    float _pad;          // Padding for alignment
} fs_uniforms_t;

// Global rendering state
static struct
{
    sg_pipeline pipeline;
    sg_shader shader;
    sg_sampler sampler;
    sg_buffer uniform_buffer;  // Uniform buffer for dynamic updates
    sg_image default_texture;
    bool initialized;
    char* vertex_shader_source;
    char* fragment_shader_source;
} render_state = { 0 };

// ============================================================================
// SOKOL INITIALIZATION
// ============================================================================

static bool render_sokol_init(void)
{
    if (render_state.initialized) return true;

    // Load shader sources from files
    const char* vs_path = get_shader_path("basic_3d", "vert");
    const char* fs_path = get_shader_path("basic_3d", "frag");  // Use textured fragment shader

    printf("📂 Loading vertex shader: %s\n", vs_path);
    render_state.vertex_shader_source = load_shader_source(vs_path);

    printf("📂 Loading fragment shader: %s\n", fs_path);
    render_state.fragment_shader_source = load_shader_source(fs_path);

    if (!render_state.vertex_shader_source || !render_state.fragment_shader_source)
    {
        printf("❌ Failed to load shader sources\n");
        if (render_state.vertex_shader_source)
        {
            free_shader_source(render_state.vertex_shader_source);
            render_state.vertex_shader_source = NULL;
        }
        if (render_state.fragment_shader_source)
        {
            free_shader_source(render_state.fragment_shader_source);
            render_state.fragment_shader_source = NULL;
        }
        return false;
    }

    // Create shader - using loaded shader sources and entry points
    sg_shader_desc shader_desc = {
        .vertex_func = { .source = render_state.vertex_shader_source,
#ifdef SOKOL_METAL
                         .entry = "vs_main"
#else
                         .entry = "main"
#endif
        },
        .fragment_func = { .source = render_state.fragment_shader_source,
#ifdef SOKOL_METAL
                           .entry = "fs_main"
#else
                           .entry = "main"
#endif
        },
        .uniform_blocks = { [0] = { .stage = SG_SHADERSTAGE_VERTEX,
                                    .size = sizeof(vs_uniforms_t),
                                    .layout = SG_UNIFORMLAYOUT_NATIVE },
                            [1] = { .stage = SG_SHADERSTAGE_FRAGMENT,
                                    .size = sizeof(fs_uniforms_t),
                                    .layout = SG_UNIFORMLAYOUT_NATIVE } },
        .images = { [0] = { .stage = SG_SHADERSTAGE_FRAGMENT,
                            .image_type = SG_IMAGETYPE_2D,
                            .sample_type = SG_IMAGESAMPLETYPE_FLOAT } },
        .samplers = { [0] = { .stage = SG_SHADERSTAGE_FRAGMENT,
                              .sampler_type = SG_SAMPLERTYPE_FILTERING } },
        .image_sampler_pairs = { [0] = { .stage = SG_SHADERSTAGE_FRAGMENT,
                                         .image_slot = 0,
                                         .sampler_slot = 0 } },
        .label = "basic_3d_shader"
    };

    printf("🔧 Creating shader with entry points: vs=%s, fs=%s\n", shader_desc.vertex_func.entry,
           shader_desc.fragment_func.entry);

    render_state.shader = sg_make_shader(&shader_desc);

    // Check for shader creation errors
    sg_resource_state shader_state = sg_query_shader_state(render_state.shader);
    printf("🔍 Shader state: %d (VALID=%d, FAILED=%d, INVALID=%d)\n", shader_state,
           SG_RESOURCESTATE_VALID, SG_RESOURCESTATE_FAILED, SG_RESOURCESTATE_INVALID);

    if (render_state.shader.id == SG_INVALID_ID || shader_state == SG_RESOURCESTATE_FAILED)
    {
        printf("❌ Failed to create shader\n");
        free_shader_source(render_state.vertex_shader_source);
        free_shader_source(render_state.fragment_shader_source);
        render_state.vertex_shader_source = NULL;
        render_state.fragment_shader_source = NULL;
        return false;
    }

    // Create sampler
    render_state.sampler = sg_make_sampler(&(sg_sampler_desc){ .min_filter = SG_FILTER_LINEAR,
                                                               .mag_filter = SG_FILTER_LINEAR,
                                                               .wrap_u = SG_WRAP_REPEAT,
                                                               .wrap_v = SG_WRAP_REPEAT,
                                                               .label = "default_sampler" });

    // Create pipeline - use default formats for swapchain compatibility
    printf("🔧 Creating pipeline with default swapchain-compatible formats\n");
    printf("🔧 Creating pipeline with default swapchain-compatible formats\n");

    render_state.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = render_state.shader,
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,  // position
                [1].format = SG_VERTEXFORMAT_FLOAT3,  // normal
                [2].format = SG_VERTEXFORMAT_FLOAT2   // texcoord
            }
        },
        .index_type = SG_INDEXTYPE_UINT32,  // Match our int indices (4 bytes)
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,  // Standard depth testing
            .write_enabled = true                  // Write to depth buffer
            // Don't specify pixel_format - let it default to match swapchain
        },
        .colors[0] = {
            // Don't specify pixel_format - let it default to match swapchain
        },
        // Don't specify sample_count - let it default to match swapchain
        .cull_mode = SG_CULLMODE_NONE,  // Disable culling for debugging
        .face_winding = SG_FACEWINDING_CCW,  // Try counter-clockwise (standard)
        .label = "basic_3d_pipeline"
    });

    if (render_state.pipeline.id == SG_INVALID_ID)
    {
        printf("❌ Failed to create pipeline\n");
        return false;
    }

    printf("🔍 Pipeline created with default formats\n");

    // Create uniform buffer (dynamic to allow updates)
    render_state.uniform_buffer = sg_make_buffer(
        &(sg_buffer_desc){ .size = sizeof(vs_uniforms_t),
                           .usage = { .dynamic_update = true },  // Allow dynamic updates
                           .label = "uniforms" });

    // Create default white texture (1x1 white pixel)
    uint32_t white_pixel = 0xFFFFFFFF;
    render_state.default_texture =
        sg_make_image(&(sg_image_desc){ .width = 1,
                                        .height = 1,
                                        .pixel_format = SG_PIXELFORMAT_RGBA8,
                                        .data.subimage[0][0] = SG_RANGE(white_pixel),
                                        .label = "default_white_texture" });

    // Check pipeline state
    sg_resource_state pipeline_state = sg_query_pipeline_state(render_state.pipeline);
    printf("🔍 Pipeline state: %d (VALID=%d, FAILED=%d, INVALID=%d)\n", pipeline_state,
           SG_RESOURCESTATE_VALID, SG_RESOURCESTATE_FAILED, SG_RESOURCESTATE_INVALID);

    if (pipeline_state == SG_RESOURCESTATE_FAILED)
    {
        printf("❌ Pipeline creation failed - invalid resource state\n");
        return false;
    }

    render_state.initialized = true;
    printf("✅ Sokol rendering pipeline initialized with external shaders\n");
    return true;
}

// ============================================================================
// LEGACY SDL COMPATIBILITY LAYER
// ============================================================================

bool render_init(RenderConfig* config, AssetRegistry* assets, float viewport_width,
                 float viewport_height)
{
    printf("🚀 Initializing Sokol-based renderer (%.0fx%.0f)\n", viewport_width, viewport_height);
    (void)config;
    (void)assets;  // Unused for now

    // Note: sg_setup() is already called in main.c init(), so we don't call it again here

    if (!sg_isvalid())
    {
        printf("❌ Sokol graphics not initialized - sg_setup() must be called first\n");
        return false;
    }

    // Initialize our rendering pipeline
    if (!render_sokol_init())
    {
        printf("❌ Failed to initialize Sokol rendering pipeline\n");
        return false;
    }

    printf("✅ Render system initialized successfully\n");
    return true;
}

void render_clear(float r, float g, float b, float a)
{
    // Clear is now handled by render pass action in render_frame()
    // This function is kept for API compatibility but does nothing
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}

void render_present(RenderConfig* config)
{
    // Present is now handled by sg_commit() in render_frame()
    // This function is kept for API compatibility but does nothing
    (void)config;
}

// ============================================================================
// ECS INTEGRATION
// ============================================================================

// ============================================================================
// MAIN RENDER INTERFACE FUNCTIONS
// ============================================================================

void render_shutdown(RenderConfig* config)
{
    (void)config;  // Unused for now

    if (render_state.initialized)
    {
        sg_destroy_buffer(render_state.uniform_buffer);  // Clean up uniform buffer
        sg_destroy_image(render_state.default_texture);
        sg_destroy_pipeline(render_state.pipeline);
        sg_destroy_shader(render_state.shader);
        sg_destroy_sampler(render_state.sampler);

        // Free loaded shader sources
        if (render_state.vertex_shader_source)
        {
            free_shader_source(render_state.vertex_shader_source);
            render_state.vertex_shader_source = NULL;
        }
        if (render_state.fragment_shader_source)
        {
            free_shader_source(render_state.fragment_shader_source);
            render_state.fragment_shader_source = NULL;
        }

        render_state.initialized = false;
    }

    printf("🔄 Render system cleaned up\n");
}

void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time)
{
    (void)config;      // Unused for now
    (void)player_id;   // Unused for now
    (void)delta_time;  // Unused for now

    // Performance monitoring (Sprint 08 Review Action Item)
    render_performance.frame_count++;
    render_performance.entities_processed = 0;
    render_performance.entities_rendered = 0;
    render_performance.entities_culled = 0;
    render_performance.draw_calls = 0;
    render_performance.validation_failures = 0;

    // Debug counters
    static bool first_frame = true;
    static int frame_count = 0;
    frame_count++;

    if (!render_state.initialized)
    {
        printf("⚠️ Render state not initialized, skipping frame\n");
        return;
    }

    // Check pipeline validity before using it (Enhanced error handling)
    sg_resource_state pipeline_state = sg_query_pipeline_state(render_state.pipeline);
    if (pipeline_state != SG_RESOURCESTATE_VALID)
    {
        printf("⚠️ Pipeline not valid (state=%d), skipping frame\n", pipeline_state);
        return;
    }

    // Pipeline is already set up in main.c render pass
    // Just apply the rendering pipeline here

    // Apply the rendering pipeline
    sg_apply_pipeline(render_state.pipeline);

    // Count entities to render
    int renderable_count = 0;
    int rendered_count = 0;

    // Debug total entity count
    if (first_frame)
    {
        printf("🔍 Total entities in world: %d\n", world->entity_count);
        for (uint32_t j = 0; j < world->entity_count; j++)
        {
            struct Entity* debug_entity = &world->entities[j];
            printf("🔍 Entity %d: components=0x%X (R:%d T:%d)\n", debug_entity->id,
                   debug_entity->component_mask,
                   !!(debug_entity->component_mask & COMPONENT_RENDERABLE),
                   !!(debug_entity->component_mask & COMPONENT_TRANSFORM));
        }
    }

    // Iterate through renderable entities (Engineering Brief pattern)
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        render_performance.entities_processed++;

        // Debug first few frames
        if (frame_count < 3)
        {
            printf("🔍 Processing Entity %d: components=0x%X (R:%d T:%d)\n", entity->id,
                   entity->component_mask, !!(entity->component_mask & COMPONENT_RENDERABLE),
                   !!(entity->component_mask & COMPONENT_TRANSFORM));
        }

        if (!(entity->component_mask & COMPONENT_RENDERABLE) ||
            !(entity->component_mask & COMPONENT_TRANSFORM))
        {
            continue;
        }

        renderable_count++;

        // Get components
        struct Transform* transform = entity_get_transform(world, entity->id);
        struct Renderable* renderable = entity_get_renderable(world, entity->id);

        // Enhanced validation (Sprint 08 Review Action Item)
        if (!validate_entity_for_rendering(entity, transform, renderable, frame_count))
        {
            render_performance.entities_culled++;
            continue;
        }

        // Apply bindings (VBO, IBO, textures)
        sg_bindings binds = {
            .vertex_buffers[0] =
                gpu_buffer_to_sg(gpu_resources_get_vertex_buffer(renderable->gpu_resources)),
            .index_buffer =
                gpu_buffer_to_sg(gpu_resources_get_index_buffer(renderable->gpu_resources)),
            .images[0] = gpu_resources_is_texture_valid(renderable->gpu_resources)
                             ? gpu_image_to_sg(gpu_resources_get_texture(renderable->gpu_resources))
                             : render_state.default_texture,
            .samplers[0] = render_state.sampler
        };
        sg_apply_bindings(&binds);

        // Calculate MVP matrix from transform component
        float model[16], mvp[16];

        // Create proper model matrix from transform (include scale and rotation)
        // Use normal scale values - the meshes should be properly sized
        Vector3 render_scale = { transform->scale.x,  // Use original scale
                                 transform->scale.y, transform->scale.z };
        mat4_compose_transform(model, transform->position, transform->rotation, render_scale);

        // Get active camera matrices
        EntityID active_camera_id = world_get_active_camera(world);
        struct Camera* active_camera = NULL;

        if (active_camera_id != INVALID_ENTITY)
        {
            active_camera = entity_get_camera(world, active_camera_id);
        }

        if (active_camera)
        {
            // Use cached camera matrices
            mat4_multiply(mvp, active_camera->view_projection_matrix, model);

            // Debug camera info occasionally
            static int debug_counter = 0;
            debug_counter++;
            if (debug_counter % 180 == 0)
            {  // Every 3 seconds
                printf("📷 Camera pos:(%.1f,%.1f,%.1f) target:(%.1f,%.1f,%.1f) fov:%.1f\n",
                       active_camera->position.x, active_camera->position.y,
                       active_camera->position.z, active_camera->target.x, active_camera->target.y,
                       active_camera->target.z, active_camera->fov);

                // Debug first few elements of view-projection matrix
                printf("📷 VP matrix [0-3]: [%.2f %.2f %.2f %.2f]\n",
                       active_camera->view_projection_matrix[0],
                       active_camera->view_projection_matrix[1],
                       active_camera->view_projection_matrix[2],
                       active_camera->view_projection_matrix[3]);
            }
        }
        else
        {
            // Fallback: create matrices on the fly with better positioning
            float view[16], proj[16], temp[16];

            Vector3 camera_pos = { 0.0f, 15.0f, 50.0f };  // Further back for better overview
            Vector3 camera_target = { 0.0f, 0.0f, 0.0f };
            Vector3 camera_up = { 0.0f, 1.0f, 0.0f };
            float fov = 60.0f;  // Match camera system FOV
            float aspect = 16.0f / 9.0f;
            float near_plane = 0.1f;
            float far_plane = 1000.0f;

            // Create view and projection matrices
            mat4_lookat(view, camera_pos, camera_target, camera_up);
            mat4_perspective(proj, fov, aspect, near_plane, far_plane);

            // Combine matrices: MVP = Projection * View * Model
            mat4_multiply(temp, view, model);
            mat4_multiply(mvp, proj, temp);
        }

        // Apply uniforms (MVP matrix)
        vs_uniforms_t vs_params;
        memcpy(vs_params.mvp, mvp, sizeof(mvp));
        sg_apply_uniforms(0, &SG_RANGE(vs_params));

        // Apply fragment shader uniforms (lighting)
        fs_uniforms_t fs_params;
        fs_params.light_dir[0] = 0.3f;
        fs_params.light_dir[1] = -0.7f;
        fs_params.light_dir[2] = 0.2f;
        fs_params._pad = 0.0f;
        sg_apply_uniforms(1, &SG_RANGE(fs_params));

        // Debug first entity's matrix in first few frames
        if (frame_count < 3 && entity->id == 1)
        {
            printf("🎯 Entity %d transform: pos:(%.2f,%.2f,%.2f) scale:(%.2f,%.2f,%.2f)\n",
                   entity->id, transform->position.x, transform->position.y, transform->position.z,
                   transform->scale.x, transform->scale.y, transform->scale.z);

            printf("🎯 Entity %d model matrix:\n", entity->id);
            printf("   [%.2f %.2f %.2f %.2f]\n", model[0], model[1], model[2], model[3]);
            printf("   [%.2f %.2f %.2f %.2f]\n", model[4], model[5], model[6], model[7]);
            printf("   [%.2f %.2f %.2f %.2f]\n", model[8], model[9], model[10], model[11]);
            printf("   [%.2f %.2f %.2f %.2f]\n", model[12], model[13], model[14], model[15]);

            printf("🎯 Entity %d MVP matrix:\n", entity->id);
            printf("   [%.2f %.2f %.2f %.2f]\n", mvp[0], mvp[1], mvp[2], mvp[3]);
            printf("   [%.2f %.2f %.2f %.2f]\n", mvp[4], mvp[5], mvp[6], mvp[7]);
            printf("   [%.2f %.2f %.2f %.2f]\n", mvp[8], mvp[9], mvp[10], mvp[11]);
            printf("   [%.2f %.2f %.2f %.2f]\n", mvp[12], mvp[13], mvp[14], mvp[15]);
        }

        // Draw
        sg_draw(0, renderable->index_count, 1);
        render_performance.draw_calls++;
        render_performance.entities_rendered++;
        rendered_count++;
    }

    // Debug first frame and periodically
    if (first_frame)
    {
        printf("🎨 Sokol ECS rendering active: pipeline_state=%d, renderable_entities=%d\n",
               pipeline_state, renderable_count);
        first_frame = false;
    }

    if (frame_count % 300 == 0 && rendered_count > 0)
    {  // Print every 5 seconds
        printf("🎨 Rendered %d/%d entities (frame %d)\n", rendered_count, renderable_count,
               frame_count);
    }

    // Performance reporting (Sprint 08 Review Action Item)
    report_render_performance();

    // Render pass is ended in main.c
}

void render_debug_info(struct World* world, RenderConfig* config)
{
    (void)config;  // Unused for now

    // Print basic debug info occasionally
    static int debug_counter = 0;
    debug_counter++;

    if (debug_counter % 180 == 0)
    {  // Every 3 seconds at 60fps
        printf("🔧 Debug: %d entities, %dx%d viewport\n", world->entity_count, config->screen_width,
               config->screen_height);
    }
}

void render_clear_screen(RenderConfig* config)
{
    (void)config;  // Unused - clearing is handled by Sokol pass action
}

bool render_take_screenshot(RenderConfig* config, const char* filename)
{
    (void)config;  // Config not needed for now
    
    // Ensure screenshots directory exists
    mkdir("screenshots", 0755);
    
    // Use our graphics API screenshot function
    return graphics_capture_screenshot(filename);
}

bool render_take_screenshot_from_position(struct World* world, RenderConfig* config,
                                          Vector3 position, Vector3 target, const char* filename)
{
    (void)world;
    (void)config;
    (void)position;
    (void)target;
    (void)filename;
    printf("📸 Positioned screenshot requested: %s (not implemented yet)\n", filename);
    return false;
}

// ============================================================================
// SPRINT 08 REVIEW: ENHANCED ERROR HANDLING AND PERFORMANCE MONITORING
// ============================================================================

// Enhanced entity validation function (Sprint 08 Review Action Item)
// Provides comprehensive error reporting and graceful degradation
static bool validate_entity_for_rendering(struct Entity* entity, struct Transform* transform,
                                          struct Renderable* renderable, uint32_t frame_count)
{
    // Basic pointer validation
    if (!entity || !transform || !renderable)
    {
        if (frame_count < 10)
        {  // Limit error spam to first 10 frames
            printf("❌ Entity validation failed: NULL pointers (E:%p T:%p R:%p)\n", (void*)entity,
                   (void*)transform, (void*)renderable);
        }
        render_performance.validation_failures++;
        return false;
    }

    // Visibility check (not an error, just filtered out)
    if (!renderable->visible)
    {
        return false;
    }

    // GPU resource validation with detailed error reporting
    bool gpu_resources_valid = true;

    if (!gpu_resources_is_vertex_buffer_valid(renderable->gpu_resources))
    {
        if (frame_count < 10)
        {
            printf("❌ Entity %d: Invalid vertex buffer (ID: %d)\n", entity->id,
                   gpu_resources_get_vertex_buffer(renderable->gpu_resources).id);
        }
        gpu_resources_valid = false;
    }

    if (!gpu_resources_is_index_buffer_valid(renderable->gpu_resources))
    {
        if (frame_count < 10)
        {
            printf("❌ Entity %d: Invalid index buffer (ID: %d)\n", entity->id,
                   gpu_resources_get_index_buffer(renderable->gpu_resources).id);
        }
        gpu_resources_valid = false;
    }

    if (renderable->index_count == 0)
    {
        if (frame_count < 10)
        {
            printf("❌ Entity %d: Zero index count\n", entity->id);
        }
        gpu_resources_valid = false;
    }

    if (!gpu_resources_valid)
    {
        render_performance.validation_failures++;
        return false;
    }

    // Transform validation with warnings (non-fatal)
    if (transform->scale.x <= 0.0f || transform->scale.y <= 0.0f || transform->scale.z <= 0.0f)
    {
        if (frame_count < 10)
        {
            printf("⚠️ Entity %d: Invalid scale (%.2f,%.2f,%.2f) - continuing with clamped values\n",
                   entity->id, transform->scale.x, transform->scale.y, transform->scale.z);
        }
        // Clamp to minimum scale to prevent rendering issues
        if (transform->scale.x <= 0.0f) transform->scale.x = 0.001f;
        if (transform->scale.y <= 0.0f) transform->scale.y = 0.001f;
        if (transform->scale.z <= 0.0f) transform->scale.z = 0.001f;
    }

    // Additional mesh validation
    if (renderable->index_count > 65536)
    {  // Reasonable upper limit
        if (frame_count < 10)
        {
            printf("⚠️ Entity %d: Very high index count (%d) - performance may be affected\n",
                   entity->id, renderable->index_count);
        }
    }

    return true;
}

// Performance reporting function (Sprint 08 Review Action Item)
static void report_render_performance(void)
{
    static uint32_t last_report_frame = 0;

    // Report every 5 seconds (assuming 60fps)
    if (render_performance.frame_count - last_report_frame >= 300)
    {
        printf("📊 Render Performance (Frame %d):\n", render_performance.frame_count);
        printf("   Processed: %d | Rendered: %d | Culled: %d | Failures: %d\n",
               render_performance.entities_processed, render_performance.entities_rendered,
               render_performance.entities_culled, render_performance.validation_failures);
        printf("   Draw Calls: %d | Frame Time: %.2fms\n", render_performance.draw_calls,
               render_performance.frame_time_ms);

        last_report_frame = render_performance.frame_count;
    }
}

// ============================================================================
// RENDER SYSTEM CLEANUP
// ============================================================================

void render_cleanup(RenderConfig* config)
{
    (void)config;  // Unused parameter

    if (!render_state.initialized)
    {
        return;
    }

    // Cleanup Sokol resources
    if (render_state.pipeline.id != SG_INVALID_ID)
    {
        sg_destroy_pipeline(render_state.pipeline);
    }
    if (render_state.shader.id != SG_INVALID_ID)
    {
        sg_destroy_shader(render_state.shader);
    }
    if (render_state.default_texture.id != SG_INVALID_ID)
    {
        sg_destroy_image(render_state.default_texture);
    }
    if (render_state.sampler.id != SG_INVALID_ID)
    {
        sg_destroy_sampler(render_state.sampler);
    }

    // Free shader source memory
    if (render_state.vertex_shader_source)
    {
        free(render_state.vertex_shader_source);
    }
    if (render_state.fragment_shader_source)
    {
        free(render_state.fragment_shader_source);
    }

    // Reset state
    memset(&render_state, 0, sizeof(render_state));

    printf("🧹 Render system cleaned up\n");
}

// Global render config getter for UI system
static RenderConfig* g_render_config_ptr = NULL;

void set_render_config(RenderConfig* config)
{
    g_render_config_ptr = config;
}

RenderConfig* get_render_config(void)
{
    return g_render_config_ptr;
}
