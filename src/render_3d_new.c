#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "render.h"
#include "render_camera.h"
#include "render_lighting.h"
#include "render_mesh.h"
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

// Vertex shader for basic 3D rendering (Metal/OpenGL compatible)
static const char* vs_source = 
    "#version 330\n"
    "uniform mat4 mvp;\n"
    "layout(location=0) in vec3 position;\n"
    "layout(location=1) in vec3 normal;\n"
    "layout(location=2) in vec2 texcoord;\n"
    "out vec3 frag_normal;\n"
    "out vec2 frag_texcoord;\n"
    "void main() {\n"
    "    gl_Position = mvp * vec4(position, 1.0);\n"
    "    frag_normal = normal;\n"
    "    frag_texcoord = texcoord;\n"
    "}\n";

// Fragment shader for basic 3D rendering
static const char* fs_source = 
    "#version 330\n"
    "uniform sampler2D diffuse_texture;\n"
    "uniform vec3 light_dir;\n"
    "in vec3 frag_normal;\n"
    "in vec2 frag_texcoord;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "    vec3 normal = normalize(frag_normal);\n"
    "    float light = max(0.0, dot(normal, -light_dir));\n"
    "    vec4 color = texture(diffuse_texture, frag_texcoord);\n"
    "    frag_color = vec4(color.rgb * (0.3 + 0.7 * light), color.a);\n"
    "}\n";

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
    bool initialized;
} render_state = {0};

// ============================================================================
// MATRIX MATH HELPERS
// ============================================================================

static void mat4_identity(float* m) {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
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
    
    // Create shader - simplified for modern Sokol API
    render_state.shader = sg_make_shader(&(sg_shader_desc){
        .vs.source = vs_source,
        .fs.source = fs_source,
        .label = "basic_3d_shader"
    });
    
    if (render_state.shader.id == SG_INVALID_ID) {
        printf("❌ Failed to create shader\n");
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
    
    // Create pipeline
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
            .write_enabled = true
        },
        .cull_mode = SG_CULLMODE_BACK,
        .label = "basic_3d_pipeline"
    });
    
    if (render_state.pipeline.id == SG_INVALID_ID) {
        printf("❌ Failed to create pipeline\n");
        return false;
    }
    
    render_state.initialized = true;
    printf("✅ Sokol rendering pipeline initialized\n");
    return true;
}

// ============================================================================
// LEGACY SDL COMPATIBILITY LAYER
// ============================================================================

bool render_init(int width, int height) {
    printf("🚀 Initializing Sokol-based renderer (%dx%d)\n", width, height);
    
    // Initialize Sokol graphics
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    
    if (!sg_isvalid()) {
        printf("❌ Failed to initialize Sokol graphics\n");
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
        sg_destroy_pipeline(render_state.pipeline);
        sg_destroy_shader(render_state.shader);
        sg_destroy_sampler(render_state.sampler);
        render_state.initialized = false;
    }
    sg_shutdown();
    printf("🔄 Render system shut down\n");
}

void render_clear(float r, float g, float b, float a) {
    sg_begin_pass(&(sg_pass){
        .action = {
            .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = {r, g, b, a} },
            .depth = { .load_action = SG_LOADACTION_CLEAR, .clear_value = 1.0f }
        },
        .swapchain = sglue_swapchain()
    });
}

void render_present() {
    sg_end_pass();
    sg_commit();
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
    
    // Set up matrices
    float model[16], view[16], proj[16], mvp[16], temp[16];
    
    // Model matrix (identity for now - TODO: use transform data)
    mat4_identity(model);
    
    // View matrix from camera
    Vector3 eye = {config->camera_position.x, config->camera_position.y, config->camera_position.z};
    Vector3 target = {0, 0, 0};  // Look at origin for now
    Vector3 up = {0, 1, 0};
    mat4_lookat(view, eye, target, up);
    
    // Projection matrix
    mat4_perspective(proj, M_PI/4.0f, 16.0f/9.0f, 0.1f, 100.0f);
    
    // Combine matrices: MVP = P * V * M
    mat4_multiply(temp, view, model);
    mat4_multiply(mvp, proj, temp);
    
    // Apply pipeline
    sg_apply_pipeline(render_state.pipeline);
    
    // Apply uniforms
    vs_uniforms_t vs_uniforms = {0};
    memcpy(vs_uniforms.mvp, mvp, sizeof(mvp));
    sg_apply_uniforms(0, &SG_RANGE(vs_uniforms));
    
    fs_uniforms_t fs_uniforms = {
        .light_dir = {0.0f, -1.0f, -0.5f}
    };
    sg_apply_uniforms(1, &SG_RANGE(fs_uniforms));
    
    // TODO: Apply mesh vertex/index buffers and texture bindings
    // TODO: sg_draw() call when we have actual mesh data
    
    printf("🎨 Rendered entity %d (placeholder - no mesh data yet)\n", entity_id);
}

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS  
// ============================================================================

void render_set_camera(Vector3 position, Vector3 target) {
    // Legacy function - camera is now handled through render_entity_3d config
    printf("📷 Camera set: pos(%.1f,%.1f,%.1f) target(%.1f,%.1f,%.1f)\n", 
           position.x, position.y, position.z, target.x, target.y, target.z);
}

void render_set_lighting(Vector3 direction, Color color) {
    // Legacy function - lighting is now handled in shaders
    printf("💡 Lighting set: dir(%.2f,%.2f,%.2f) color(%d,%d,%d)\n", 
           direction.x, direction.y, direction.z, color.r, color.g, color.b);
}

void render_add_mesh(Mesh* mesh) {
    // Legacy function - meshes are now handled through ECS
    if (mesh && mesh->name) {
        printf("🔺 Mesh added: %s (%d vertices, %d indices)\n", 
               mesh->name, mesh->vertex_count, mesh->index_count);
    }
}

void render_apply_material(Material* material) {
    // Legacy function - materials are now handled through uniform bindings
    if (material && material->name) {
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
