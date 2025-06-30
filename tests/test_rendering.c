// tests/test_rendering.c
#include "vendor/unity.h"
#include "assets.h"
#include "render_mesh.h"
#include "gpu_resources.h" // Required for the Renderable struct

// This test requires the real assets to be available.
// Ensure the asset compiler has been run via `make` before `make test`.
void test_full_mesh_to_renderable_pipeline(void) {
    // 1. Init Asset System
    AssetRegistry registry = {0};
    // Use the real asset root for this integration test.
    assets_init(&registry, "build/assets"); 

    // 2. Load the wedge_ship from the real index.json
    bool loaded = load_assets_from_metadata(&registry);
    TEST_ASSERT_TRUE(loaded);

    Mesh* ship_mesh = assets_get_mesh(&registry, "wedge_ship");
    TEST_ASSERT_NOT_NULL(ship_mesh);
    TEST_ASSERT_TRUE(ship_mesh->loaded);

    // 3. Verify GPU Upload
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_vertex_buffer.id);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_index_buffer.id);

    // 4. Init Renderer
    MeshRenderer renderer = {0};
    bool renderer_inited = mesh_renderer_init(&renderer);
    TEST_ASSERT_TRUE(renderer_inited);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, renderer.pipeline.id);

    // 5. Create a Renderable from the mesh
    Renderable renderable = {0};
    bool renderable_created = assets_create_renderable_from_mesh(&registry, "wedge_ship", &renderable);
    TEST_ASSERT_TRUE(renderable_created);
    TEST_ASSERT_NOT_NULL(renderable.gpu_resources);
    
    // 6. Final check: can we get a valid buffer from the renderable?
    sg_buffer vbuf = gpu_resources_get_vertex_buffer(renderable.gpu_resources);
    TEST_ASSERT_EQUAL_UINT32(ship_mesh->sg_vertex_buffer.id, vbuf.id);

    // Cleanup
    gpu_resources_destroy(renderable.gpu_resources);
    mesh_renderer_cleanup(&renderer);
    assets_cleanup(&registry);
}

void suite_rendering(void) {
    RUN_TEST(test_full_mesh_to_renderable_pipeline);
}
