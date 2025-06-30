// tests/test_rendering.c
#include "vendor/unity.h"
#include "assets.h"
#include "render_mesh.h"
#include "gpu_resources.h"

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
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, renderer.pipeline.id);

    // Act: Load all assets from the metadata, which should include parsing
    // the mesh file and uploading its data to the (headless) GPU.
    bool loaded = load_assets_from_metadata(&registry);

    // Assert: Check every stage of the pipeline for success.
    TEST_ASSERT_TRUE(loaded);

    // 1. Was the mesh data loaded into the CPU correctly?
    Mesh* ship_mesh = assets_get_mesh(&registry, "wedge_ship");
    TEST_ASSERT_NOT_NULL(ship_mesh);
    TEST_ASSERT_TRUE(ship_mesh->loaded);

    // 2. Was the mesh data uploaded to the GPU correctly?
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_vertex_buffer.id);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_index_buffer.id);

    // 3. Can we create a final 'Renderable' component from the mesh?
    struct Renderable renderable = {0};
    bool renderable_created = assets_create_renderable_from_mesh(&registry, "wedge_ship", &renderable);
    TEST_ASSERT_TRUE(renderable_created);
    TEST_ASSERT_NOT_NULL(renderable.gpu_resources);
    
    // 4. Does the renderable have the correct GPU resource handles?
    sg_buffer vbuf = gpu_resources_get_vertex_buffer(renderable.gpu_resources);
    TEST_ASSERT_EQUAL_UINT32(ship_mesh->sg_vertex_buffer.id, vbuf.id);

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
