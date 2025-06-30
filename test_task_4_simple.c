#define SOKOL_IMPL
#define SOKOL_DUMMY_BACKEND
#include "sokol_wrapper.h"
#include "sokol_glue.h"
#include "assets.h"

// Simple test for Task 4 without full graphics dependencies
int main() {
    printf("===== Running Sprint 10.5 Task 4 Test (Simplified) =====\n");
    
    // Initialize Sokol GFX with dummy backend
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    
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
    
    // Initialize some test data
    for (int i = 0; i < test_mesh.vertex_count; i++) {
        test_mesh.vertices[i].position = (Vector3){i, i, i};
    }
    for (int i = 0; i < test_mesh.index_count; i++) {
        test_mesh.indices[i] = i;
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
    
    // Cleanup
    sg_shutdown();
    
    printf("=================================================\n");
    return 0;
}
