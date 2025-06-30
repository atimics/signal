// TASK 2.1: Restored and modernized mesh renderer with Sokol GFX integration
#include "render_mesh.h"
#include "render_camera.h"
#include "render_lighting.h"
#include "render.h"
#include "assets.h"
#include "graphics_api.h"
#include "sokol_gfx.h" // ONLY in the .c file
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

// Define the implementation struct
struct MeshRendererGpuResources {
    sg_pipeline pipeline;
    sg_shader shader;
    sg_sampler sampler;
};

// Global mesh renderer instance (will be used when fully implemented)
// static MeshRenderer g_mesh_renderer = {0};

bool mesh_renderer_init(MeshRenderer* renderer) {
    if (!renderer) return false;
    
    printf("🔧 Initializing mesh renderer...\n");
    
    // Initialize renderer state
    memset(renderer, 0, sizeof(MeshRenderer));
    
    // Allocate memory for our opaque struct
    renderer->gpu_resources = calloc(1, sizeof(struct MeshRendererGpuResources));
    if (!renderer->gpu_resources) {
        printf("❌ Failed to allocate GPU resources for mesh renderer\n");
        return false;
    }
    
    // Create a basic shader for mesh rendering
    // For now, we'll create a simple shader that can render basic meshes
    const char* vs_source = 
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
    
    const char* fs_source = 
        "#version 330\n"
        "in vec3 color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  fragColor = vec4(color, 1.0);\n"
        "}\n";
    
    #ifdef SOKOL_METAL
    vs_source = 
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
        "vertex vs_out vs_main(vs_in in [[stage_in]], constant float4x4& mvp [[buffer(0)]]) {\n"
        "  vs_out out;\n"
        "  out.position = mvp * float4(in.position, 1.0);\n"
        "  out.color = in.normal * 0.5 + 0.5;\n"
        "  return out;\n"
        "}\n";
    
    fs_source = 
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "fragment float4 fs_main(float3 color [[stage_in]]) {\n"
        "  return float4(color, 1.0);\n"
        "}\n";
    #endif
    
    sg_shader_desc shader_desc = {
        .vertex_func = {
            .source = vs_source,
            #ifdef SOKOL_METAL
            .entry = "vs_main"
            #else
            .entry = "main"
            #endif
        },
        .fragment_func = {
            .source = fs_source,
            #ifdef SOKOL_METAL
            .entry = "fs_main"
            #else
            .entry = "main"
            #endif
        },
        .uniform_blocks = {
            [0] = {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = 64,  // 4x4 matrix (16 floats * 4 bytes)
                .layout = SG_UNIFORMLAYOUT_NATIVE
            }
        },
        .label = "mesh_renderer_shader"
    };
    
    renderer->gpu_resources->shader = sg_make_shader(&shader_desc);
    
    // Create pipeline state object
    sg_pipeline_desc pipeline_desc = {
        .shader = renderer->gpu_resources->shader,
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
    
    renderer->gpu_resources->pipeline = sg_make_pipeline(&pipeline_desc);
    
    // Verify that resources were created successfully
    if (renderer->gpu_resources->shader.id == SG_INVALID_ID || renderer->gpu_resources->pipeline.id == SG_INVALID_ID) {
        printf("❌ Failed to create mesh renderer resources\n");
        return false;
    }
    
    printf("✅ Mesh renderer initialized with pipeline ID %d\n", renderer->gpu_resources->pipeline.id);
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
    
    if (renderer->gpu_resources) {
        // Destroy Sokol resources
        if (renderer->gpu_resources->pipeline.id != SG_INVALID_ID) {
            sg_destroy_pipeline(renderer->gpu_resources->pipeline);
        }
        
        if (renderer->gpu_resources->shader.id != SG_INVALID_ID) {
            sg_destroy_shader(renderer->gpu_resources->shader);
        }
        
        // Free the PIMPL struct
        free(renderer->gpu_resources);
        renderer->gpu_resources = NULL;
    }
    
    printf("✅ Mesh renderer cleaned up\n");
}