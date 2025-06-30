// TASK 2.1: Restored and modernized mesh renderer with Sokol GFX integration
#include "render_mesh.h"
#include "render_camera.h"
#include "render_lighting.h"
#include "render.h"
#include "assets.h"
#include "graphics_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

// Global mesh renderer instance (will be used when fully implemented)
// static MeshRenderer g_mesh_renderer = {0};

bool mesh_renderer_init(MeshRenderer* renderer) {
    if (!renderer) return false;
    
    printf("🔧 Initializing mesh renderer...\n");
    
    // Initialize renderer state
    memset(renderer, 0, sizeof(MeshRenderer));
    
    // Create a basic shader for mesh rendering
    // For now, we'll create a simple shader that can render basic meshes
    sg_shader_desc shader_desc = {
        .attrs = {
            [0].name = "position",
            [1].name = "normal", 
            [2].name = "texcoord"
        },
        .vs.uniform_blocks[0] = {
            .size = 64,  // 4x4 matrix (16 floats * 4 bytes)
            .uniforms = {
                [0] = { .name = "mvp", .type = SG_UNIFORMTYPE_MAT4 }
            }
        }
    };
    
    // Set platform-specific shader source
    #ifdef SOKOL_METAL
    shader_desc.vs.source = 
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "struct vs_in {\n"
        "  float3 position [[attribute(0)]];\n"
        "  float3 normal [[attribute(1)]];\n"
        "  float2 texcoord [[attribute(2)]];\n"
        "};\n"
        "struct vs_out {\n"
        "  float4 position [[position]];\n"
        "  float3 color;\n"
        "};\n"
        "vertex vs_out _main(vs_in in [[stage_in]], constant float4x4& mvp [[buffer(0)]]) {\n"
        "  vs_out out;\n"
        "  out.position = mvp * float4(in.position, 1.0);\n"
        "  out.color = in.normal * 0.5 + 0.5;\n"  // Normal as color
        "  return out;\n"
        "}\n";
    
    shader_desc.fs.source = 
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "struct fs_out {\n"
        "  float4 color [[color(0)]];\n"
        "};\n"
        "fragment fs_out _main(float3 color [[stage_in]]) {\n"
        "  fs_out out;\n"
        "  out.color = float4(color, 1.0);\n"
        "  return out;\n"
        "}\n";
    #else
    // OpenGL/GLSL version
    shader_desc.vs.source = 
        "#version 330\n"
        "in vec3 position;\n"
        "in vec3 normal;\n"
        "in vec2 texcoord;\n"
        "uniform mat4 mvp;\n"
        "out vec3 color;\n"
        "void main() {\n"
        "  gl_Position = mvp * vec4(position, 1.0);\n"
        "  color = normal * 0.5 + 0.5;\n"
        "}\n";
    
    shader_desc.fs.source = 
        "#version 330\n"
        "in vec3 color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  fragColor = vec4(color, 1.0);\n"
        "}\n";
    #endif
    
    renderer->shader = sg_make_shader(&shader_desc);
    
    // Create pipeline state object
    sg_pipeline_desc pipeline_desc = {
        .shader = renderer->shader,
        .layout = {
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3 }, // position
                [1] = { .format = SG_VERTEXFORMAT_FLOAT3 }, // normal
                [2] = { .format = SG_VERTEXFORMAT_FLOAT2 }  // texcoord
            }
        },
        .index_type = SG_INDEXTYPE_UINT32,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .cull_mode = SG_CULLMODE_BACK,
        .label = "mesh_renderer_pipeline"
    };
    
    renderer->pipeline = sg_make_pipeline(&pipeline_desc);
    
    // Verify that resources were created successfully
    if (renderer->shader.id == SG_INVALID_ID || renderer->pipeline.id == SG_INVALID_ID) {
        printf("❌ Failed to create mesh renderer resources\n");
        return false;
    }
    
    printf("✅ Mesh renderer initialized with pipeline ID %d\n", renderer->pipeline.id);
    return true;
}

void mesh_renderer_render_entity(MeshRenderer* renderer, struct Entity* entity, 
                                 struct Transform* transform, struct Renderable* renderable,
                                 float* view_projection_matrix) {
    (void)renderer;  // Suppress unused parameter warning
    if (!entity || !transform || !renderable || !view_projection_matrix) return;
    
    // This function will be called by render_3d.c for each entity
    // For now, we'll implement a simple version that logs the call
    // The actual rendering will continue to happen in render_3d.c until we complete the refactor
    
    static int call_count = 0;
    call_count++;
    
    if (call_count < 5) {  // Log first few calls
        printf("🎨 mesh_renderer_render_entity called for entity %d\n", entity->id);
    }
}

void mesh_renderer_cleanup(MeshRenderer* renderer) {
    if (!renderer) return;
    
    printf("🔧 Cleaning up mesh renderer...\n");
    
    // Destroy Sokol resources
    if (renderer->pipeline.id != SG_INVALID_ID) {
        sg_destroy_pipeline(renderer->pipeline);
        renderer->pipeline.id = SG_INVALID_ID;
    }
    
    if (renderer->shader.id != SG_INVALID_ID) {
        sg_destroy_shader(renderer->shader);
        renderer->shader.id = SG_INVALID_ID;
    }
    
    printf("✅ Mesh renderer cleaned up\n");
}