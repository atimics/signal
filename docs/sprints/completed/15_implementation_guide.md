# Sprint 15: Implementation Guide - Advanced C Testing

**ID**: `guide_sprint_15`
**Status**: **ACTIVE**
**Author**: Gemini (Lead Scientist and Researcher)
**Related Plan**: `sprints/active/15_automated_testing_integration.md`

## 1. Overview

This document provides a comprehensive, practical guide for implementing the advanced integration and regression tests outlined in Sprint 15. With the Unity framework now integrated, this guide focuses on best practices for its application within the CGame engine, covering test fixtures, mocking, and the creation of a headless rendering test environment.

---

## 2. Phase 2: Asset Pipeline Integration Test

*   **Objective**: To create a robust integration test for the asset loading system that validates the fixes from Sprint 10.5.

### 2.1. Test Fixtures: `setUp` and `tearDown`

For tests that require a specific environment (like a mock file system), Unity's test fixtures are essential. They ensure that every test runs in a clean, predictable state.

**Implementation (`tests/test_assets.c`)**:

```c
#include "vendor/unity.h"
#include "assets.h"
#include <stdio.h>
#include <stdlib.h>

// This function is called before each test in this file.
void setUp(void) {
    // 1. Create a temporary directory for our mock assets.
    //    Using system() is acceptable for a test utility.
    system("mkdir -p tests/temp_assets/props/test_ship");

    // 2. Create the mock index.json file.
    FILE* f = fopen("tests/temp_assets/index.json", "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "{\n  \"assets\": {\n    \"test_ship\": {\n      \"path\": \"props/test_ship/geometry.cobj\"\n    }\n  }\n}");
    fclose(f);

    // 3. Create the dummy mesh file.
    f = fopen("tests/temp_assets/props/test_ship/geometry.cobj", "w");
    TEST_ASSERT_NOT_NULL(f);
    fprintf(f, "# Mock mesh file");
    fclose(f);
}

// This function is called after each test in this file.
void tearDown(void) {
    // Clean up the temporary directory to ensure no side effects.
    system("rm -rf tests/temp_assets");
}
```

### 2.2. The Integration Test

With the fixtures in place, the test itself becomes very clean. It initializes the asset system, points it to our temporary directory, and verifies that the path resolution works as expected.

**Implementation (`tests/test_assets.c`)**:

```c
// Test that the asset system can correctly parse our mock index.json
void test_asset_path_resolution_from_index(void) {
    AssetRegistry registry = {0};
    // Point the registry to our temporary asset directory.
    assets_init(&registry, "tests/temp_assets");

    char resolved_path[512];
    bool found = assets_get_mesh_path_from_index(registry.asset_root, "test_ship", resolved_path, sizeof(resolved_path));

    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_STRING("props/test_ship/geometry.cobj", resolved_path);
}
```

---

## 3. Phase 3: Mesh Rendering Regression Test

*   **Objective**: To create a high-level regression test that ensures the entire pipeline, from asset loading to render setup, is functioning correctly.

### 3.1. Headless Sokol GFX Context

To test rendering logic without opening a window, we must initialize Sokol GFX with the `SOKOL_DUMMY_BACKEND`. This is the key to running rendering tests on a CI server or in a command-line environment.

The test runner (`test_main.c`) needs to be modified to set up and tear down the Sokol context around the rendering test suite.

**Implementation (`tests/test_main.c`)**:

```c
// In test_main.c

// Forward-declare the rendering suite
void suite_rendering(void);

// ... existing main function ...
    RUN_TEST_SUITE(suite_core_math);
    RUN_TEST_SUITE(suite_assets); // Add the new suite

    // --- Rendering Test Suite ---
    // These tests require a valid (but headless) GFX context.
    sg_desc desc = {.logger.func = slog_func};
    sg_setup(&desc);

    RUN_TEST_SUITE(suite_rendering);

    sg_shutdown();
    // --- End Rendering Suite ---

    return UNITY_END();
// ...
```

### 3.2. The Regression Test

This test simulates the engine's startup sequence in a controlled environment. It loads a real asset, uploads it to the (headless) GPU, initializes the modular renderer, and verifies that all resulting resource handles are valid.

**Implementation (`tests/test_rendering.c`)**:

```c
#include "vendor/unity.h"
#include "assets.h"
#include "render_mesh.h"

// This test requires the real assets to be available.
// Ensure the asset compiler has been run.
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

    // 3. Verify GPU Upload (this was the goal of Task 3/4)
    // The load_assets_from_metadata function should have already uploaded it.
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_vertex_buffer.id);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_index_buffer.id);

    // 4. Init Renderer (goal of Task 5)
    MeshRenderer renderer = {0};
    bool renderer_inited = mesh_renderer_init(&renderer);
    TEST_ASSERT_TRUE(renderer_inited);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, renderer.pipeline.id);

    // 5. Create a Renderable from the mesh
    Renderable renderable = {0};
    bool renderable_created = assets_create_renderable_from_mesh(&registry, "wedge_ship", &renderable);
    TEST_ASSERT_TRUE(renderable_created);
    TEST_ASSERT_NOT_NULL(renderable.gpu_resources);
    
    // Final check: can we get a valid buffer from the renderable?
    sg_buffer vbuf = gpu_resources_get_vertex_buffer(renderable.gpu_resources);
    TEST_ASSERT_EQUAL_UINT32(ship_mesh->sg_vertex_buffer.id, vbuf.id);

    // Cleanup
    mesh_renderer_cleanup(&renderer);
    assets_cleanup(&registry);
}
```

## 4. Conclusion

By following this guide, the development team can systematically build out the test suite, creating a robust safety net that will prevent regressions and dramatically increase development velocity. The completion of these tests will mark a significant milestone in the maturity of the CGame engine's codebase.

```