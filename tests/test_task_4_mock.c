#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assets.h"

// Mock Sokol functions for testing
typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;

#define SG_INVALID_ID 0
#define SG_RESOURCESTATE_VALID 1

sg_buffer sg_make_buffer(void* desc) {
    (void)desc;
    return (sg_buffer){.id = 12345};  // Mock valid ID
}

int sg_query_buffer_state(sg_buffer buf) {
    (void)buf;
    return SG_RESOURCESTATE_VALID;  // Always return valid for test
}

// Simple test for Task 4
int main() {
    printf("===== Running Sprint 10.5 Task 4 Test (Mock) =====\n");
    
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
