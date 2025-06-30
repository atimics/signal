// tests/test_assets.c
#include "vendor/unity.h"
#include "assets.h"
#include <stdio.h>
#include <stdlib.h>

// This function is called before each test in this file.
void setUp(void) {
    // 1. Create a temporary directory for our mock assets.
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

// Test that the asset system can correctly parse our mock index.json
void test_asset_path_resolution_from_index(void) {
    AssetRegistry registry = {0};
    // Point the registry to our temporary asset directory.
    assets_init(&registry, "tests/temp_assets");

    char resolved_path[512];
    // This function needs to be implemented in assets.c based on Sprint 10.5 work
    bool found = assets_get_mesh_path_from_index(registry.asset_root, "test_ship", resolved_path, sizeof(resolved_path));

    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_STRING("props/test_ship/geometry.cobj", resolved_path);
    
    assets_cleanup(&registry);
}

void suite_assets(void) {
    RUN_TEST(test_asset_path_resolution_from_index);
}
