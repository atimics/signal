#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Minimal types for testing
typedef struct { float x, y, z; } Vector3;
typedef struct { float u, v; } Vector2;
typedef struct {
    Vector3 position;
    Vector3 normal;
    Vector2 tex_coord;
} Vertex;

typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;
typedef int sg_resource_state;

#define SG_INVALID_ID 0
#define SG_RESOURCESTATE_VALID 1

typedef struct {
    char name[64];
    Vertex* vertices;
    int* indices;
    uint32_t vertex_count;
    uint32_t index_count;
    sg_buffer sg_vertex_buffer;
    sg_buffer sg_index_buffer;
    bool loaded;
    char material_name[64];
} Mesh;

// Mock Sokol functions
sg_buffer sg_make_buffer(void* desc) {
    (void)desc;
    return (sg_buffer){.id = 12345};  // Mock valid ID
}

sg_resource_state sg_query_buffer_state(sg_buffer buf) {
    (void)buf;
    return SG_RESOURCESTATE_VALID;  // Always return valid for test
}

// The actual function we're testing (copied from assets.c)
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
    
    // All validation passed - now create GPU buffers (mocked)
    mesh->sg_vertex_buffer = sg_make_buffer(NULL);
    mesh->sg_index_buffer = sg_make_buffer(NULL);

    // Post-creation validation: Ensure GPU buffers were created successfully
    if (sg_query_buffer_state(mesh->sg_vertex_buffer) != SG_RESOURCESTATE_VALID ||
        sg_query_buffer_state(mesh->sg_index_buffer) != SG_RESOURCESTATE_VALID) {
        printf("❌ Sokol failed to create buffers for mesh '%s'\n", mesh->name);
        return false;
    }

    // Free CPU-side memory after successful upload (Task 4 requirement)
    // This is a critical optimization to reduce memory footprint
    free(mesh->vertices);
    mesh->vertices = NULL;
    free(mesh->indices);
    mesh->indices = NULL;

    printf("✅ Mesh '%s' uploaded to GPU successfully\n", mesh->name);
    return true;
}

// Test Task 4
int main() {
    printf("===== Running Sprint 10.5 Task 4 Test (Isolated) =====\n");
    
    // Create a test mesh
    Mesh test_mesh = {0};
    strcpy(test_mesh.name, "test_mesh");
    test_mesh.vertex_count = 4;
    test_mesh.index_count = 6;
    
    // Allocate CPU-side data
    test_mesh.vertices = malloc(test_mesh.vertex_count * sizeof(Vertex));
    test_mesh.indices = malloc(test_mesh.index_count * sizeof(int));
    
    if (!test_mesh.vertices || !test_mesh.indices) {
        printf("❌ Failed to allocate test data\n");
        return 1;
    }
    
    printf("🔍 Before upload: vertices=%p, indices=%p\n", 
           (void*)test_mesh.vertices, (void*)test_mesh.indices);
    
    // Test the upload function
    bool result = assets_upload_mesh_to_gpu(&test_mesh);
    
    printf("🔍 After upload: vertices=%p, indices=%p\n", 
           (void*)test_mesh.vertices, (void*)test_mesh.indices);
    
    if (result) {
        printf("✅ Upload succeeded\n");
        if (test_mesh.vertices == NULL && test_mesh.indices == NULL) {
            printf("✅ CPU memory correctly freed\n");
            printf("All Task 4 tests passed!\n");
        } else {
            printf("❌ CPU memory not freed: vertices=%p, indices=%p\n", 
                   (void*)test_mesh.vertices, (void*)test_mesh.indices);
            return 1;
        }
    } else {
        printf("❌ Upload failed\n");
        return 1;
    }
    
    printf("=================================================\n");
    return 0;
}
