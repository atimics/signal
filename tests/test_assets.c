// tests/test_assets.c
#include "vendor/unity.h"
#include "assets.h"
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// Test Fixtures: setUp and tearDown
// ============================================================================

// The setUpAssets function is called manually in the assets test suite.
// Its purpose is to create a clean, predictable environment for each test.
// Here, we create a temporary mock filesystem to isolate the test from
// the actual game assets.
void setUpAssets(void) {
    // Using system() is perfectly acceptable for a test utility like this.
    system("mkdir -p tests/temp_assets/props/test_ship");

    // Create the mock index.json, which is the heart of our test.
    FILE* f = fopen("tests/temp_assets/index.json", "w");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Failed to create mock index.json");
    fprintf(f, "{\n  \"assets\": {\n    \"test_ship\": {\n      \"path\": \"props/test_ship/geometry.cobj\"\n    }\n  }\n}");
    fclose(f);

    // Create the dummy mesh file that the index points to.
    f = fopen("tests/temp_assets/props/test_ship/geometry.cobj", "w");
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Failed to create mock geometry.cobj");
    fprintf(f, "# Mock mesh file for testing path resolution.");
    fclose(f);
}

// The tearDownAssets function is called manually in the assets test suite.
// Its purpose is to clean up any resources created in setUpAssets, ensuring
// that tests do not have side effects that could affect other tests.
void tearDownAssets(void) {
    system("rm -rf tests/temp_assets");
}

// ============================================================================
// Test Cases
// ============================================================================

// This test verifies that the asset system can correctly parse the mock
// index.json file and construct a full, correct path to the asset data.
void test_asset_path_resolution_from_index(void) {
    // Arrange: Create a new asset registry and point it to our mock directory.
    AssetRegistry registry = {0};
    assets_init(&registry, "tests/temp_assets");

    // Act: Attempt to resolve the path for the asset defined in our mock index.
    char resolved_path[512];
    bool found = assets_get_mesh_path_from_index(registry.asset_root, "test_ship", resolved_path, sizeof(resolved_path));

    // Assert: Verify that the path was found and is correct.
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_STRING("props/test_ship/geometry.cobj", resolved_path);
    
    // Cleanup
    assets_cleanup(&registry);
}

// ============================================================================
// Test Suite Runner
// ============================================================================

// This function is called by the main test runner to execute all tests in this file.
void suite_assets(void) {
    RUN_TEST(test_asset_path_resolution_from_index);
}
