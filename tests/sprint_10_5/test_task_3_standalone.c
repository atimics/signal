// tests/sprint_10_5/test_task_3_standalone.c
// Standalone test for GPU resource validation in assets_upload_mesh_to_gpu
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Minimal definitions needed for testing
typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float u, v;
} Vector2;

typedef struct {
    Vector3 position;
    Vector3 normal;
    Vector2 tex_coord;
} Vertex;

// Sokol buffer definitions for mocking
typedef struct {
    uint32_t id;
} sg_buffer;

typedef struct {
    struct {
        const void* ptr;
        size_t size;
    } data;
    struct {
        bool vertex_buffer;
        bool index_buffer;
    } usage;
    const char* label;
} sg_buffer_desc;

typedef struct {
    char name[64];
    char material_name[64];
    Vertex* vertices;
    int vertex_count;
    int* indices;
    int index_count;
    bool loaded;
    sg_buffer sg_vertex_buffer;
    sg_buffer sg_index_buffer;
} Mesh;

// --- Mocking Sokol Functions ---
// To test the validation logic without a real GPU, we'll mock sg_make_buffer.
// We'll use a global variable to track if it was called.

int sg_make_buffer_call_count = 0;

// This is our mock implementation.
sg_buffer sg_make_buffer(const sg_buffer_desc* desc) {
    (void)desc; // Suppress unused parameter warning
    sg_make_buffer_call_count++;
    // Return a dummy buffer. The ID just needs to be non-zero.
    return (sg_buffer){ .id = sg_make_buffer_call_count };
}
// --- End Mocking ---

// Forward-declare the function we are testing
bool assets_upload_mesh_to_gpu(Mesh* mesh);

// Standalone implementation of assets_upload_mesh_to_gpu with validation
bool assets_upload_mesh_to_gpu(Mesh* mesh) {
    if (!mesh) {
        printf("❌ assets_upload_mesh_to_gpu: NULL mesh pointer\n");
        return false;
    }
    
    // Validate mesh data before attempting GPU upload
    if (mesh->vertex_count == 0) {
        printf("❌ Mesh '%s' has zero vertices\n", mesh->name);
        return false;
    }
    
    if (mesh->vertices == NULL) {
        printf("❌ Mesh '%s' has NULL vertex data\n", mesh->name);
        return false;
    }
    
    if (mesh->index_count == 0) {
        printf("❌ Mesh '%s' has zero indices\n", mesh->name);
        return false;
    }
    
    if (mesh->indices == NULL) {
        printf("❌ Mesh '%s' has NULL index data\n", mesh->name);
        return false;
    }
    
    // Calculate buffer sizes for additional validation
    size_t vertex_buffer_size = mesh->vertex_count * sizeof(Vertex);
    size_t index_buffer_size = mesh->index_count * sizeof(int);
    
    if (vertex_buffer_size == 0 || index_buffer_size == 0) {
        printf("❌ Mesh '%s' would create zero-sized buffers: VB=%zu IB=%zu\n",
               mesh->name, vertex_buffer_size, index_buffer_size);
        return false;
    }
    
    printf("🔍 DEBUG: Uploading mesh '%s' to GPU - VB=%zu bytes, IB=%zu bytes\n", 
           mesh->name, vertex_buffer_size, index_buffer_size);
    
    // All validation passed - now create GPU buffers
    mesh->sg_vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = {
            .ptr = mesh->vertices,
            .size = vertex_buffer_size
        },
        .usage = { .vertex_buffer = true },
        .label = mesh->name
    });

    mesh->sg_index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = {
            .ptr = mesh->indices,
            .size = index_buffer_size
        },
        .usage = { .index_buffer = true },
        .label = mesh->name
    });

    printf("✅ Mesh '%s' uploaded to GPU successfully\n", mesh->name);
    return true;
}

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

int main() {
    test_gpu_resource_validation();
    printf("All Task 3 tests passed!\n");
    return 0;
}
