// tests/sprint_10_5/test_task_1_standalone.c
// Standalone test for assets_get_mesh_path_from_index function
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

// Standalone implementation of the path resolution function
bool assets_get_mesh_path_from_index(const char* index_path, const char* asset_name, char* out_path, size_t out_size) {
    if (!index_path || !asset_name || !out_path || out_size == 0) {
        return false;
    }
    
    FILE* file = fopen(index_path, "r");
    if (!file) {
        printf("❌ Failed to open index file: %s\n", index_path);
        return false;
    }
    
    // Read the entire file into memory for simple parsing
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        return false;
    }
    
    char* file_content = malloc(file_size + 1);
    if (!file_content) {
        fclose(file);
        return false;
    }
    
    size_t bytes_read = fread(file_content, 1, file_size, file);
    file_content[bytes_read] = '\0';
    fclose(file);
    
    // Simple string-based JSON parsing
    // Look for the asset name in quotes followed by a colon
    char search_pattern[128];
    snprintf(search_pattern, sizeof(search_pattern), "\"%s\"", asset_name);
    
    char* asset_location = strstr(file_content, search_pattern);
    if (!asset_location) {
        free(file_content);
        return false;
    }
    
    // Find the "path" field after the asset name
    char* path_start = strstr(asset_location, "\"path\"");
    if (!path_start) {
        free(file_content);
        return false;
    }
    
    // Find the colon after "path"
    char* colon = strchr(path_start, ':');
    if (!colon) {
        free(file_content);
        return false;
    }
    
    // Skip whitespace and find the opening quote
    char* quote1 = strchr(colon, '"');
    if (!quote1) {
        free(file_content);
        return false;
    }
    quote1++; // Move past the opening quote
    
    // Find the closing quote
    char* quote2 = strchr(quote1, '"');
    if (!quote2) {
        free(file_content);
        return false;
    }
    
    // Calculate the path length
    size_t path_length = quote2 - quote1;
    
    // Check if the output buffer is large enough
    if (path_length >= out_size) {
        free(file_content);
        return false;
    }
    
    // Copy the path to the output buffer
    strncpy(out_path, quote1, path_length);
    out_path[path_length] = '\0';
    
    free(file_content);
    return true;
}

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
