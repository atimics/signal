// tests/sprint_10_5/test_task_5.c
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "core.h"
#include "gpu_resources.h"
#include "render_mesh.h"
#include "assets.h"

// Forward-declare the sokol_gfx main functions we need for the test harness
void sg_setup(const sg_desc* desc);
void sg_shutdown(void);

void test_modular_renderer_lifecycle() {
    printf("Running Test: test_modular_renderer_lifecycle\n");

    // --- Test Case 1: Init and Shutdown ---
    printf("  Case 1: Initialize and shut down the renderer...\n");
    MeshRenderer renderer = {0};
    
    bool init_success = mesh_renderer_init(&renderer);
    assert(init_success == true);
    // After init, the renderer should have valid resource handles.
    assert(renderer.pipeline.id != SG_INVALID_ID);
    assert(renderer.shader.id != SG_INVALID_ID);

    mesh_renderer_cleanup(&renderer);
    // After cleanup, the handles should be invalidated.
    assert(renderer.pipeline.id == SG_INVALID_ID);
    assert(renderer.shader.id == SG_INVALID_ID);
    printf("    ... PASSED\n");

    // --- Test Case 2: Draw call with valid data ---
    printf("  Case 2: Perform a draw call...\n");
    mesh_renderer_init(&renderer);

    // Create dummy data for the draw call
    struct Transform transform = { .position = {0,0,0}, .rotation = {0,0,0,1}, .scale = {1,1,1} };
    
    // Create a dummy Renderable with mock GPU buffer handles
    struct GpuResources* gpu_res = gpu_resources_create();
    gpu_resources_set_vertex_buffer(gpu_res, (sg_buffer){.id=1});
    gpu_resources_set_index_buffer(gpu_res, (sg_buffer){.id=2});

    struct Renderable renderable = { .gpu_resources = gpu_res, .index_count = 3 };
    
    float view_proj[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}; // Identity matrix

    // The test is that this function can be called without crashing.
    // A more advanced test would involve capturing the draw call with a mocked backend.
    mesh_renderer_render_entity(&renderer, NULL, &transform, &renderable, view_proj);
    printf("    ... PASSED (if no crash)\n");

    gpu_resources_destroy(gpu_res);
    mesh_renderer_cleanup(&renderer);

    printf("Test Finished: test_modular_renderer_lifecycle\n\n");
}

void run_task_5_tests() {
    // We need to wrap the test in sg_setup and sg_shutdown
    // to have a valid graphics context for creating resources.
    sg_desc desc = {.logger.func = slog_func};
    sg_setup(&desc);
    
    test_modular_renderer_lifecycle();
    
    sg_shutdown();
    printf("All Task 5 tests passed!\n");
}
