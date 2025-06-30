// tests/sprint_10_5/test_task_3.c
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "assets.h"
#include "sokol_gfx.h"

// --- Mocking Sokol Functions ---
// To test the validation logic without a real GPU, we'll mock sg_make_buffer.
// We'll use a global variable to track if it was called.

#ifdef MOCK_SOKOL
int sg_make_buffer_call_count = 0;
int sg_make_image_call_count = 0;

// This is our mock implementation.
sg_buffer sg_make_buffer(const sg_buffer_desc* desc) {
    sg_make_buffer_call_count++;
    // Return a dummy buffer. The ID just needs to be non-zero.
    return (sg_buffer){ .id = sg_make_buffer_call_count };
}

sg_image sg_make_image(const sg_image_desc* desc) {
    sg_make_image_call_count++;
    return (sg_image){ .id = sg_make_image_call_count };
}

sg_resource_state sg_query_buffer_state(sg_buffer buf) {
    return SG_RESOURCESTATE_VALID;
}

#endif // MOCK_SOKOL
// --- End Mocking ---


// Forward-declare the function we are testing
bool assets_upload_mesh_to_gpu(Mesh* mesh);


void test_gpu_resource_validation() {
    printf("Running Test: test_gpu_resource_validation\n");

    // --- Test Case 1: Valid Mesh ---
    printf("  Case 1: Upload valid mesh...\n");
    sg_make_buffer_call_count = 0;
    Vertex valid_vertices[] = { { {1,1,1}, {0,1,0}, {0,0} } };
    int valid_indices[] = { 0 };
    Mesh valid_mesh = {
        .name = "valid",
        .vertex_count = 1, .vertices = valid_vertices,
        .index_count = 1, .indices = valid_indices
    };
    bool success = assets_upload_mesh_to_gpu(&valid_mesh);
    assert(success == true);
    // Should be called twice: once for vertex buffer, once for index buffer.
    assert(sg_make_buffer_call_count == 2);
    printf("    ... PASSED\n");

    // --- Test Case 2: Zero vertices ---
    printf("  Case 2: Reject zero vertices...\n");
    sg_make_buffer_call_count = 0;
    Mesh zero_v_mesh = { .name = "zero_v", .vertex_count = 0, .vertices = NULL, .index_count = 1, .indices = valid_indices };
    success = assets_upload_mesh_to_gpu(&zero_v_mesh);
    assert(success == false);
    assert(sg_make_buffer_call_count == 0);
    printf("    ... PASSED\n");

    // --- Test Case 3: Null vertices ---
    printf("  Case 3: Reject null vertices...\n");
    sg_make_buffer_call_count = 0;
    Mesh null_v_mesh = { .name = "null_v", .vertex_count = 1, .vertices = NULL, .index_count = 1, .indices = valid_indices };
    success = assets_upload_mesh_to_gpu(&null_v_mesh);
    assert(success == false);
    assert(sg_make_buffer_call_count == 0);
    printf("    ... PASSED\n");

    // --- Test Case 4: Zero indices ---
    printf("  Case 4: Reject zero indices...\n");
    sg_make_buffer_call_count = 0;
    Mesh zero_i_mesh = { .name = "zero_i", .vertex_count = 1, .vertices = valid_vertices, .index_count = 0, .indices = NULL };
    success = assets_upload_mesh_to_gpu(&zero_i_mesh);
    assert(success == false);
    assert(sg_make_buffer_call_count == 0);
    printf("    ... PASSED\n");

    // --- Test Case 5: Null indices ---
    printf("  Case 5: Reject null indices...\n");
    sg_make_buffer_call_count = 0;
    Mesh null_i_mesh = { .name = "null_i", .vertex_count = 1, .vertices = valid_vertices, .index_count = 1, .indices = NULL };
    success = assets_upload_mesh_to_gpu(&null_i_mesh);
    assert(success == false);
    assert(sg_make_buffer_call_count == 0);
    printf("    ... PASSED\n");

    printf("Test Finished: test_gpu_resource_validation\n\n");
}

void run_task_3_tests() {
    test_gpu_resource_validation();
    printf("All Task 3 tests passed!\n");
}
