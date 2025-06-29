#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "render.h"
#include "render_camera.h"
#include "render_lighting.h"
#include "render_mesh.h"
#include "assets.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// SOKOL RENDERING DEFINITIONS
// ============================================================================

// Uniform data structures
typedef struct {
    float mvp[16];  // Model-View-Projection matrix
} vs_uniforms_t;

typedef struct {
    float light_dir[3];  // Light direction
    float _pad;          // Padding for alignment
} fs_uniforms_t;

// Global rendering state
static struct {
    sg_pipeline pipeline;
    sg_shader shader;
    sg_sampler sampler;
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    sg_buffer uniform_buffer;  // Add uniform buffer
    sg_image default_texture;
    bool initialized;
    char* vertex_shader_source;
    char* fragment_shader_source;
} render_state = {0};

// Test triangle vertices (position, normal, texcoord)
static float test_vertices[] = {
    // Position        Normal          TexCoord
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 0.0f,  // Top
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // Bottom-left  
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f   // Bottom-right
};

static uint16_t test_indices[] = { 0, 1, 2 };

// ============================================================================
// MATRIX MATH HELPERS
// ============================================================================

// Math helper functions
static void mat4_identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    mat4_identity(m);
    float f = 1.0f / tanf(fov * 0.5f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}

static void mat4_lookat(float* m, Vector3 eye, Vector3 target, Vector3 up) {
    // Calculate camera basis vectors
    Vector3 f = {target.x - eye.x, target.y - eye.y, target.z - eye.z};
    float len = sqrtf(f.x*f.x + f.y*f.y + f.z*f.z);
    if (len > 0) { f.x /= len; f.y /= len; f.z /= len; }
    
    Vector3 s = {f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x};
    len = sqrtf(s.x*s.x + s.y*s.y + s.z*s.z);
    if (len > 0) { s.x /= len; s.y /= len; s.z /= len; }
    
    Vector3 u = {s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x};
    
    // Build matrix
    mat4_identity(m);
    m[0] = s.x; m[4] = s.y; m[8] = s.z; m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
    m[1] = u.x; m[5] = u.y; m[9] = u.z; m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
    m[2] = -f.x; m[6] = -f.y; m[10] = -f.z; m[14] = (f.x*eye.x + f.y*eye.y + f.z*eye.z);
}

static void mat4_multiply(float* result, const float* a, const float* b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i*4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result[i*4 + j] += a[i*4 + k] * b[k*4 + j];
            }
        }
    }
}

// ============================================================================
// SOKOL INITIALIZATION
// ============================================================================

static bool render_sokol_init(void) {
    if (render_state.initialized) return true;
    
    // Load shader sources from files
    const char* vs_path = get_shader_path("basic_3d", "vert");
    const char* fs_path = get_shader_path("basic_3d_simple", "frag");
    
    printf("📂 Loading vertex shader: %s\n", vs_path);
    render_state.vertex_shader_source = load_shader_source(vs_path);
    
    printf("📂 Loading fragment shader: %s\n", fs_path);
    render_state.fragment_shader_source = load_shader_source(fs_path);
    
    if (!render_state.vertex_shader_source || !render_state.fragment_shader_source) {
        printf("❌ Failed to load shader sources\n");
        if (render_state.vertex_shader_source) {
            free_shader_source(render_state.vertex_shader_source);
            render_state.vertex_shader_source = NULL;
        }
        if (render_state.fragment_shader_source) {
            free_shader_source(render_state.fragment_shader_source);
            render_state.fragment_shader_source = NULL;
        }
        return false;
    }
    
    // Create shader - using loaded shader sources and entry points
    sg_shader_desc shader_desc = {
        .vertex_func = {
            .source = render_state.vertex_shader_source,
#ifdef SOKOL_METAL
            .entry = "vs_main"
#else
            .entry = "main"
#endif
        },
        .fragment_func = {
            .source = render_state.fragment_shader_source,
#ifdef SOKOL_METAL
            .entry = "fs_main"
#else
            .entry = "main"
#endif
        },
        .uniform_blocks = {
            [0] = {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = sizeof(vs_uniforms_t),
                .layout = SG_UNIFORMLAYOUT_NATIVE
            }
            // Fragment shader uniform temporarily disabled
            // [1] = {
            //     .stage = SG_SHADERSTAGE_FRAGMENT,
            //     .size = sizeof(fs_uniforms_t),
            //     .layout = SG_UNIFORMLAYOUT_NATIVE
            // }
        },
        .images = {
            [0] = { .stage = SG_SHADERSTAGE_FRAGMENT, .image_type = SG_IMAGETYPE_2D, .sample_type = SG_IMAGESAMPLETYPE_FLOAT }
        },
        .samplers = {
            [0] = { .stage = SG_SHADERSTAGE_FRAGMENT, .sampler_type = SG_SAMPLERTYPE_FILTERING }
        },
        .image_sampler_pairs = {
            [0] = { .stage = SG_SHADERSTAGE_FRAGMENT, .image_slot = 0, .sampler_slot = 0 }
        },
        .label = "basic_3d_shader"
    };
    
    printf("🔧 Creating shader with entry points: vs=%s, fs=%s\n", 
           shader_desc.vertex_func.entry, shader_desc.fragment_func.entry);
    
    render_state.shader = sg_make_shader(&shader_desc);
    
    // Check for shader creation errors
    sg_resource_state shader_state = sg_query_shader_state(render_state.shader);
    printf("🔍 Shader state: %d (VALID=%d, FAILED=%d, INVALID=%d)\n", 
           shader_state, SG_RESOURCESTATE_VALID, SG_RESOURCESTATE_FAILED, SG_RESOURCESTATE_INVALID);
    
    if (render_state.shader.id == SG_INVALID_ID || shader_state == SG_RESOURCESTATE_FAILED) {
        printf("❌ Failed to create shader\n");
        free_shader_source(render_state.vertex_shader_source);
        free_shader_source(render_state.fragment_shader_source);
        render_state.vertex_shader_source = NULL;
        render_state.fragment_shader_source = NULL;
        return false;
    }
    
    // Create sampler
    render_state.sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
        .label = "default_sampler"
    });
    
    // Create pipeline with swapchain-compatible formats
    // Query swapchain to get the actual formats
    sg_swapchain swapchain = sglue_swapchain();
    printf("🔍 Swapchain info: sample_count=%d, color_fmt=%d, depth_fmt=%d\n", 
           swapchain.sample_count, swapchain.color_format, swapchain.depth_format);
    
    render_state.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = render_state.shader,
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,  // position
                [1].format = SG_VERTEXFORMAT_FLOAT3,  // normal
                [2].format = SG_VERTEXFORMAT_FLOAT2   // texcoord
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
            .pixel_format = swapchain.depth_format  // Match swapchain depth format
        },
        .colors[0] = {
            .pixel_format = swapchain.color_format  // Match swapchain color format
        },
        .sample_count = swapchain.sample_count,  // Match swapchain sample count
        .cull_mode = SG_CULLMODE_BACK,
        .label = "basic_3d_pipeline"
    });
    
    if (render_state.pipeline.id == SG_INVALID_ID) {
        printf("❌ Failed to create pipeline\n");
        return false;
    }
    
    printf("🔍 Pipeline created with default formats\n");
    
    // Create test geometry buffers
    render_state.vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(test_vertices),
        .usage = { .vertex_buffer = true },
        .label = "test_vertices"
    });
    
    render_state.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(test_indices),
        .usage = { .index_buffer = true },  // Explicitly set as index buffer
        .label = "test_indices"
    });
    
    // Create uniform buffer (dynamic to allow updates)
    render_state.uniform_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(vs_uniforms_t),
        .usage = { .dynamic_update = true }, // Allow dynamic updates
        .label = "uniforms"
    });
    
    // Create default white texture (1x1 white pixel)
    uint32_t white_pixel = 0xFFFFFFFF;
    render_state.default_texture = sg_make_image(&(sg_image_desc){
        .width = 1,
        .height = 1,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(white_pixel),
        .label = "default_white_texture"
    });
    
    // Check pipeline state
    sg_resource_state pipeline_state = sg_query_pipeline_state(render_state.pipeline);
    printf("🔍 Pipeline state: %d (VALID=%d, FAILED=%d, INVALID=%d)\n", 
           pipeline_state, SG_RESOURCESTATE_VALID, SG_RESOURCESTATE_FAILED, SG_RESOURCESTATE_INVALID);
    
    if (pipeline_state == SG_RESOURCESTATE_FAILED) {
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

bool render_init(RenderConfig* config, AssetRegistry* assets, float viewport_width, float viewport_height) {
    printf("🚀 Initializing Sokol-based renderer (%.0fx%.0f)\n", viewport_width, viewport_height);
    (void)config; (void)assets; // Unused for now
    
    // Note: sg_setup() is already called in main.c init(), so we don't call it again here
    
    if (!sg_isvalid()) {
        printf("❌ Sokol graphics not initialized - sg_setup() must be called first\n");
        return false;
    }
    
    // Initialize our rendering pipeline
    if (!render_sokol_init()) {
        printf("❌ Failed to initialize Sokol rendering pipeline\n");
        return false;
    }
    
    printf("✅ Render system initialized successfully\n");
    return true;
}

void render_shutdown() {
    if (render_state.initialized) {
        sg_destroy_buffer(render_state.vertex_buffer);
        sg_destroy_buffer(render_state.index_buffer);
        sg_destroy_buffer(render_state.uniform_buffer);  // Clean up uniform buffer
        sg_destroy_image(render_state.default_texture);
        sg_destroy_pipeline(render_state.pipeline);
        sg_destroy_shader(render_state.shader);
        sg_destroy_sampler(render_state.sampler);
        
        // Free loaded shader sources
        if (render_state.vertex_shader_source) {
            free_shader_source(render_state.vertex_shader_source);
            render_state.vertex_shader_source = NULL;
        }
        if (render_state.fragment_shader_source) {
            free_shader_source(render_state.fragment_shader_source);
            render_state.fragment_shader_source = NULL;
        }
        
        render_state.initialized = false;
    }
    sg_shutdown();
    printf("🔄 Render system shut down\n");
}

void render_clear(float r, float g, float b, float a) {
    // Clear is now handled by render pass action in render_frame()
    // This function is kept for API compatibility but does nothing
    (void)r; (void)g; (void)b; (void)a;
}

void render_present(RenderConfig* config) {
    // Present is now handled by sg_commit() in render_frame()
    // This function is kept for API compatibility but does nothing
    (void)config;
}

// ============================================================================
// ECS INTEGRATION
// ============================================================================

void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config) {
    if (!render_state.initialized) {
        printf("⚠️  Sokol rendering not initialized, skipping entity render\n");
        return;
    }
    
    // Get entity transform component
    struct Transform* transform = entity_get_transform(world, entity_id);
    if (!transform) return;
    
    // TODO: Implement individual entity rendering
    // For now, this is handled in render_frame() to avoid multiple pipeline applications
    
    printf("🎨 Entity %d ready for render (transform available)\n", entity_id);
}

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS  
// ============================================================================

void render_set_camera(Vector3 position, Vector3 target) {
    // Legacy function - camera is now handled through render_entity_3d config
    printf("📷 Camera set: pos(%.1f,%.1f,%.1f) target(%.1f,%.1f,%.1f)\n", 
           position.x, position.y, position.z, target.x, target.y, target.z);
}

void render_set_lighting(Vector3 direction, uint8_t r, uint8_t g, uint8_t b) {
    // Legacy function - lighting is now handled in shaders
    printf("💡 Lighting set: dir(%.2f,%.2f,%.2f) color(%d,%d,%d)\n", 
           direction.x, direction.y, direction.z, r, g, b);
}

void render_add_mesh(Mesh* mesh) {
    // Legacy function - meshes are now handled through ECS
    if (mesh) {
        printf("🔺 Mesh added: %s (%d vertices, %d indices)\n", 
               mesh->name, mesh->vertex_count, mesh->index_count);
    }
}

void render_apply_material(Material* material) {
    // Legacy function - materials are now handled through uniform bindings
    if (material) {
        printf("🎨 Material applied: %s\n", material->name);
    }
}

// ============================================================================
// UI INTEGRATION
// ============================================================================

void render_add_comm_message(RenderConfig* config, const char* sender, const char* message, bool is_player) {
    // Legacy UI function - preserved for compatibility
    (void)config; (void)sender; (void)message; (void)is_player;
    printf("💬 Comm message: %s: %s\n", sender, message);
}

// ============================================================================
// MAIN RENDER INTERFACE FUNCTIONS
// ============================================================================

void render_cleanup(RenderConfig* config) {
    (void)config; // Unused for now
    
    if (render_state.initialized) {
        sg_destroy_buffer(render_state.vertex_buffer);
        sg_destroy_buffer(render_state.index_buffer);
        sg_destroy_buffer(render_state.uniform_buffer);  // Clean up uniform buffer
        sg_destroy_image(render_state.default_texture);
        sg_destroy_pipeline(render_state.pipeline);
        sg_destroy_shader(render_state.shader);
        sg_destroy_sampler(render_state.sampler);
        
        // Free loaded shader sources
        if (render_state.vertex_shader_source) {
            free_shader_source(render_state.vertex_shader_source);
            render_state.vertex_shader_source = NULL;
        }
        if (render_state.fragment_shader_source) {
            free_shader_source(render_state.fragment_shader_source);
            render_state.fragment_shader_source = NULL;
        }
        
        render_state.initialized = false;
    }
    
    printf("🔄 Render system cleaned up\n");
}

void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time) {
    (void)delta_time; // Unused for now
    
    if (!render_state.initialized) {
        printf("⚠️ Render state not initialized, skipping frame\n");
        return;
    }
    
    // Check pipeline validity before using it
    sg_resource_state pipeline_state = sg_query_pipeline_state(render_state.pipeline);
    if (pipeline_state != SG_RESOURCESTATE_VALID) {
        printf("⚠️ Pipeline not valid (state=%d), skipping frame\n", pipeline_state);
        return;
    }
    
    // Count entities to render
    int renderable_count = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_RENDERABLE) {
            renderable_count++;
        }
    }
    
    // Debug first frame
    static bool first_frame = true;
    if (first_frame) {
        printf("🎨 First render frame: pipeline_state=%d, renderable_count=%d\n", 
               pipeline_state, renderable_count);
        
        // Get swapchain info to debug sample count (first frame only)
        sg_swapchain swapchain = sglue_swapchain();
        printf("🔍 Swapchain sample_count: %d\n", swapchain.sample_count);
        
        first_frame = false;
    }
    
    // Note: Render pass is managed by main.c, we just do the rendering here
    
    // If we have entities to render, draw a simple test triangle
    if (renderable_count > 0) {
        // Debug: Check if we're in a render pass
        printf("🔧 About to apply pipeline - checking pass state...\n");
        
        // Apply pipeline (should be within render pass from main.c)
        printf("🔧 Applying pipeline within render pass...\n");
        sg_apply_pipeline(render_state.pipeline);
        
        // Apply vertex buffer
        sg_apply_bindings(&(sg_bindings){
            .vertex_buffers[0] = render_state.vertex_buffer,
            .index_buffer = render_state.index_buffer,
            .images[0] = render_state.default_texture,
            .samplers[0] = render_state.sampler
        });
        
        // Create simple identity MVP matrix
        vs_uniforms_t uniforms;
        mat4_identity(uniforms.mvp);
        
        // Apply uniforms
        sg_range uniform_data = SG_RANGE(uniforms);
        sg_apply_uniforms(0, &uniform_data);
        
        // Draw the triangle
        printf("🎨 Drawing triangle (3 indices)...\n");
        sg_draw(0, 3, 1);
        
        printf("✅ Triangle drawn successfully\n");
    } else {
        // Print once that we're not rendering
        static bool no_render_warned = false;
        if (!no_render_warned) {
            printf("🎨 Sokol render frame initialized - no renderable entities\n");
            no_render_warned = true;
        }
    }
    
    // Note: Render pass end and commit are handled by main.c
    
    // Render debug info if enabled
    if (config->show_debug_info) {
        render_debug_info(world, config);
    }
    
    // Basic HUD for player if available (print occasionally)
    static int player_print_counter = 0;
    if (player_id != INVALID_ENTITY && ++player_print_counter % 60 == 0) {
        struct Transform* transform = entity_get_transform(world, player_id);
        if (transform) {
            printf("🎮 Player at (%.1f,%.1f,%.1f)\n", 
                   transform->position.x, transform->position.y, transform->position.z);
        }
    }
}

void render_debug_info(struct World* world, RenderConfig* config) {
    (void)config; // Unused for now
    
    // Print basic debug info occasionally
    static int debug_counter = 0;
    debug_counter++;
    
    if (debug_counter % 180 == 0) { // Every 3 seconds at 60fps
        printf("🔧 Debug: %d entities, %dx%d viewport\n", 
               world->entity_count, config->screen_width, config->screen_height);
    }
}

void render_clear_screen(RenderConfig* config) {
    (void)config; // Unused - clearing is handled by Sokol pass action
}

bool render_take_screenshot(RenderConfig* config, const char* filename) {
    (void)config; (void)filename;
    printf("📸 Screenshot requested: %s (not implemented yet)\n", filename);
    return false;
}

bool render_take_screenshot_from_position(struct World* world, RenderConfig* config, 
                                         Vector3 position, Vector3 target, const char* filename) {
    (void)world; (void)config; (void)position; (void)target; (void)filename;
    printf("📸 Positioned screenshot requested: %s (not implemented yet)\n", filename);
    return false;
}
