// tests/sprint_10_5/test_task_1.c
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Forward-declare the function we are testing (to be implemented in src/assets.c)
bool assets_get_mesh_path_from_index(const char* index_path, const char* asset_name, char* out_path, size_t out_size);

void test_asset_path_resolution() {
    printf("Running Test: test_asset_path_resolution\n");

    const char* test_index_path = "tests/sprint_10_5/test_assets/meshes/index.json";
    char result_path[512];

    // --- Test Case 1: Successful lookup ---
    printf("  Case 1: Find 'test_ship'...\n");
    bool found = assets_get_mesh_path_from_index(test_index_path, "test_ship", result_path, sizeof(result_path));
    assert(found == true);
    
    // The expected path is relative to the index file itself.
    const char* expected_path = "props/test_ship/geometry.cobj";
    assert(strcmp(result_path, expected_path) == 0);
    printf("    ... PASSED\n");

    // --- Test Case 2: Asset not found ---
    printf("  Case 2: Fail to find 'non_existent_asset'...\n");
    bool not_found = assets_get_mesh_path_from_index(test_index_path, "non_existent_asset", result_path, sizeof(result_path));
    assert(not_found == false);
    printf("    ... PASSED\n");

    // --- Test Case 3: Buffer too small ---
    printf("  Case 3: Handle buffer too small...\n");
    char small_buffer[10];
    bool buffer_too_small = assets_get_mesh_path_from_index(test_index_path, "test_ship", small_buffer, sizeof(small_buffer));
    assert(buffer_too_small == false);
    printf("    ... PASSED\n");
    
    // --- Test Case 4: Null arguments ---
    printf("  Case 4: Handle NULL arguments...\n");
    assert(assets_get_mesh_path_from_index(NULL, "test_ship", result_path, sizeof(result_path)) == false);
    assert(assets_get_mesh_path_from_index(test_index_path, NULL, result_path, sizeof(result_path)) == false);
    assert(assets_get_mesh_path_from_index(test_index_path, "test_ship", NULL, 0) == false);
    printf("    ... PASSED\n");

    printf("Test Finished: test_asset_path_resolution\n\n");
}

int main() {
    test_asset_path_resolution();
    printf("All Task 1 tests passed!\n");
    return 0;
}
