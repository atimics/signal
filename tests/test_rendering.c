// tests/test_rendering.c
#include "vendor/unity.h"
#include "assets.h"
#include "render_mesh.h"
#include "gpu_resources.h"
#include "sokol_gfx.h" // For SG_INVALID_ID constant in tests

// ============================================================================
// Test Cases
// ============================================================================

// This is a high-level integration test that serves as a regression test
// for the entire mesh-to-renderable pipeline. It verifies that all the
// pieces fixed in Sprint 10.5 are working together correctly.
//
// NOTE: This test requires the real, compiled assets. It should be run
// after `make` has successfully built the assets into the `build/assets` dir.
void test_pipeline_loads_and_prepares_mesh_for_rendering(void) {
    // Arrange: Initialize the asset system and the mesh renderer.
    AssetRegistry registry = {0};
    assets_init(&registry, "build/assets"); 
    MeshRenderer renderer = {0};
    bool renderer_inited = mesh_renderer_init(&renderer);
    TEST_ASSERT_TRUE(renderer_inited);
    // Note: With PIMPL, we can't directly check renderer internals from tests
    // The fact that mesh_renderer_init returned true indicates success

    // Act: Load all assets from the metadata, which should include parsing
    // the mesh file and uploading its data to the (headless) GPU.
    bool loaded = load_assets_from_metadata(&registry);

    // Assert: Check every stage of the pipeline for success.
    TEST_ASSERT_TRUE(loaded);

    // 1. Was the mesh data loaded into the CPU correctly?
    Mesh* ship_mesh = assets_get_mesh(&registry, "Wedge Ship");
    TEST_ASSERT_NOT_NULL(ship_mesh);
    TEST_ASSERT_TRUE(ship_mesh->loaded);

    // 2. Was the mesh data uploaded to the GPU correctly?
    // Use PIMPL-compliant accessor functions
    sg_buffer vbuf = {0};
    sg_buffer ibuf = {0};
    mesh_get_gpu_buffers(ship_mesh, (void*)&vbuf, (void*)&ibuf);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, vbuf.id);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ibuf.id);

    // 3. Can we create a final 'Renderable' component from the mesh?
    struct Renderable renderable = {0};
    bool renderable_created = assets_create_renderable_from_mesh(&registry, "Wedge Ship", &renderable);
    TEST_ASSERT_TRUE(renderable_created);
    TEST_ASSERT_NOT_NULL(renderable.gpu_resources);
    
    // 4. Does the renderable have the correct GPU resource handles?
    gpu_buffer_t renderable_vbuf = gpu_resources_get_vertex_buffer(renderable.gpu_resources);
    // Convert sg_buffer to gpu_buffer_t for comparison
    gpu_buffer_t expected_vbuf = {.id = vbuf.id};
    TEST_ASSERT_EQUAL_UINT32(expected_vbuf.id, renderable_vbuf.id);

    // Cleanup
    gpu_resources_destroy(renderable.gpu_resources);
    mesh_renderer_cleanup(&renderer);
    assets_cleanup(&registry);
}

// ============================================================================
// Test Suite Runner
// ============================================================================

void suite_rendering(void) {
    RUN_TEST(test_pipeline_loads_and_prepares_mesh_for_rendering);
}
