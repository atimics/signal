// tests/sprint_10_5/test_task_4.c
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "assets.h"

// --- Mocking Sokol Functions ---
#ifdef MOCK_SOKOL
int sg_make_buffer_call_count_task4 = 0;
bool force_sokol_failure = false;

sg_buffer sg_make_buffer(const sg_buffer_desc* desc) {
    sg_make_buffer_call_count_task4++;
    if (force_sokol_failure) {
        return (sg_buffer){ .id = SG_INVALID_ID };
    }
    return (sg_buffer){ .id = sg_make_buffer_call_count_task4 };
}

sg_resource_state sg_query_buffer_state(sg_buffer buf) {
    if (force_sokol_failure) {
        return SG_RESOURCESTATE_FAILED;
    }
    return (buf.id != SG_INVALID_ID) ? SG_RESOURCESTATE_VALID : SG_RESOURCESTATE_INVALID;
}

sg_image sg_make_image(const sg_image_desc* desc) {
    return (sg_image){ .id = 1 };
}
#endif

// Forward-declare the function we are testing
bool assets_upload_mesh_to_gpu(Mesh* mesh);

void test_cpu_memory_deallocation() {
    printf("Running Test: test_cpu_memory_deallocation\n");

    // --- Test Case 1: Successful upload frees memory ---
    printf("  Case 1: Successful upload frees CPU memory...\n");
    sg_make_buffer_call_count_task4 = 0;
    force_sokol_failure = false;
    
    // We must use malloc so we can check if it's freed.
    Vertex* vertices_case1 = malloc(sizeof(Vertex));
    int* indices_case1 = malloc(sizeof(int));
    Mesh valid_mesh = {
        .name = "valid_mem",
        .vertex_count = 1, .vertices = vertices_case1,
        .index_count = 1, .indices = indices_case1
    };

    bool success = assets_upload_mesh_to_gpu(&valid_mesh);
    assert(success == true);
    assert(sg_make_buffer_call_count_task4 == 2);
    // The core of the test: pointers should be NULL after successful upload.
    assert(valid_mesh.vertices == NULL);
    assert(valid_mesh.indices == NULL);
    printf("    ... PASSED\n");

    // --- Test Case 2: Failed upload does NOT free memory ---
    printf("  Case 2: Failed upload does not free CPU memory...\n");
    sg_make_buffer_call_count_task4 = 0;
    force_sokol_failure = true; // Force sg_make_buffer to fail

    Vertex* vertices_case2 = malloc(sizeof(Vertex));
    int* indices_case2 = malloc(sizeof(int));
    Mesh failing_mesh = {
        .name = "failing_mem",
        .vertex_count = 1, .vertices = vertices_case2,
        .index_count = 1, .indices = indices_case2
    };

    success = assets_upload_mesh_to_gpu(&failing_mesh);
    assert(success == false);
    // Pointers should NOT be NULL, as the data is still needed.
    assert(failing_mesh.vertices != NULL);
    assert(failing_mesh.indices != NULL);
    printf("    ... PASSED\n");

    // Cleanup the memory that wasn't freed by the function
    free(failing_mesh.vertices);
    free(failing_mesh.indices);

    printf("Test Finished: test_cpu_memory_deallocation\n\n");
}

void run_task_4_tests() {
    test_cpu_memory_deallocation();
    printf("All Task 4 tests passed!\n");
}
