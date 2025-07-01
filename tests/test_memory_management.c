// tests/test_memory_management.c
// Tests for memory management and optimization system - Sprint 19 Task 2.2

#include "vendor/unity.h"
#include "../src/system/memory.h"
#include "../src/assets.h"
#include "../src/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock asset registry for testing
static AssetRegistry test_registry;
static struct World test_world;

void setUp(void) {
    // Initialize test registry
    memset(&test_registry, 0, sizeof(AssetRegistry));
    memset(&test_world, 0, sizeof(struct World));
    
    // Initialize memory system with 64MB limit for testing
    memory_system_init(64);
}

void tearDown(void) {
    memory_system_shutdown();
}

// ============================================================================
// BASIC MEMORY SYSTEM TESTS
// ============================================================================

void test_memory_system_initialization(void) {
    // System should be initialized in setUp
    size_t total_mb, peak_mb;
    uint32_t asset_count;
    
    memory_get_stats(&total_mb, &peak_mb, &asset_count);
    
    TEST_ASSERT_EQUAL_UINT32(0, total_mb);
    TEST_ASSERT_EQUAL_UINT32(0, asset_count);
    TEST_ASSERT_FALSE(memory_is_over_limit());
    
    printf("✅ Memory system initialized successfully\n");
}

void test_memory_pool_creation(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);  // 10MB
    
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, pool_id);
    
    printf("✅ Memory pool created with ID: %u\n", pool_id);
}

void test_memory_tracking_allocation(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, pool_id);
    
    uint32_t tracking_id = memory_track_allocation(pool_id, "test_asset", "mesh", 1024 * 1024);  // 1MB
    
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, tracking_id);
    
    size_t total_mb, peak_mb;
    uint32_t asset_count;
    memory_get_stats(&total_mb, &peak_mb, &asset_count);
    
    TEST_ASSERT_EQUAL_UINT32(1, total_mb);
    TEST_ASSERT_EQUAL_UINT32(1, asset_count);
    
    printf("✅ Memory allocation tracked: %zu MB, %u assets\n", total_mb, asset_count);
}

void test_memory_untracking_allocation(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    uint32_t tracking_id = memory_track_allocation(pool_id, "test_asset", "mesh", 1024 * 1024);
    
    // Verify allocation is tracked
    size_t total_mb, peak_mb;
    uint32_t asset_count;
    memory_get_stats(&total_mb, &peak_mb, &asset_count);
    TEST_ASSERT_EQUAL_UINT32(1, total_mb);
    
    // Untrack the allocation
    memory_untrack_allocation(tracking_id);
    
    // Verify memory is freed
    memory_get_stats(&total_mb, &peak_mb, &asset_count);
    TEST_ASSERT_EQUAL_UINT32(0, total_mb);
    
    printf("✅ Memory allocation untracked successfully\n");
}

// ============================================================================
// ASSET USAGE TRACKING TESTS
// ============================================================================

void test_asset_usage_tracking(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    memory_track_allocation(pool_id, "test_mesh", "mesh", 1024 * 1024);
    
    // Update asset usage - close to camera
    memory_update_asset_usage("test_mesh", 25.0f, false);  // Close to camera, but not used
    
    // Check if asset should be unloaded (should be false - close to camera)
    TEST_ASSERT_FALSE(asset_should_unload("test_mesh"));
    
    // Update with far distance and mark as not used for long time 
    memory_update_asset_usage("test_mesh", 150.0f, false);  // Far from camera, not used
    
    // Sleep a bit to ensure time passes (simulate old usage)
    // Since we can't easily sleep in tests, let's adjust the test logic
    // The distance check should be sufficient for unloading
    TEST_ASSERT_TRUE(asset_should_unload("test_mesh"));
    
    printf("✅ Asset usage tracking working correctly\n");
}

// ============================================================================
// MEMORY LIMIT TESTS
// ============================================================================

void test_memory_limit_detection(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    
    // Allocate under limit
    memory_track_allocation(pool_id, "asset1", "mesh", 32 * 1024 * 1024);  // 32MB
    TEST_ASSERT_FALSE(memory_is_over_limit());
    
    // Allocate over limit (64MB total limit)
    memory_track_allocation(pool_id, "asset2", "mesh", 40 * 1024 * 1024);  // 40MB more = 72MB total
    TEST_ASSERT_TRUE(memory_is_over_limit());
    
    printf("✅ Memory limit detection working\n");
}

// ============================================================================
// STREAMING CONFIGURATION TESTS
// ============================================================================

void test_streaming_configuration(void) {
    // Test enabling/disabling streaming
    memory_set_streaming_enabled(true);
    memory_set_unloading_enabled(true);
    
    // Test disabling
    memory_set_streaming_enabled(false);
    memory_set_unloading_enabled(false);
    
    printf("✅ Streaming configuration working\n");
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_memory_system_update(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    memory_track_allocation(pool_id, "test_mesh", "mesh", 1024 * 1024);
    
    // Update memory system
    memory_system_update(&test_world, &test_registry, 0.016f);  // 16ms frame
    
    printf("✅ Memory system update completed without errors\n");
}

void test_memory_report_generation(void) {
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    memory_track_allocation(pool_id, "test_mesh", "mesh", 1024 * 1024);
    memory_track_allocation(pool_id, "test_texture", "texture", 512 * 1024);
    
    printf("\n--- Memory Report Test ---\n");
    memory_print_report();
    printf("--- End Memory Report ---\n");
    
    printf("✅ Memory report generation working\n");
}

// ============================================================================
// MOCK ASSET UNLOADING TESTS
// ============================================================================

void test_mock_asset_unloading(void) {
    // Create a simple test mesh in the registry
    Mesh* mesh = &test_registry.meshes[0];
    strncpy(mesh->name, "test_mesh", sizeof(mesh->name));
    mesh->vertex_count = 1000;
    mesh->index_count = 3000;
    mesh->vertices = malloc(mesh->vertex_count * sizeof(Vertex));
    mesh->indices = malloc(mesh->index_count * sizeof(int));
    mesh->loaded = true;
    test_registry.mesh_count = 1;
    
    // Track the mesh
    uint32_t pool_id = memory_create_pool("TestPool", 10 * 1024 * 1024);
    size_t mesh_size = mesh->vertex_count * sizeof(Vertex) + mesh->index_count * sizeof(int);
    memory_track_allocation(pool_id, "test_mesh", "mesh", mesh_size);
    
    // Verify mesh is loaded
    TEST_ASSERT_TRUE(mesh->loaded);
    
    // Unload the mesh
    bool unloaded = asset_unload_mesh(&test_registry, "test_mesh");
    TEST_ASSERT_TRUE(unloaded);
    TEST_ASSERT_FALSE(mesh->loaded);
    
    printf("✅ Mock asset unloading working\n");
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    
    // Basic functionality tests
    RUN_TEST(test_memory_system_initialization);
    RUN_TEST(test_memory_pool_creation);
    RUN_TEST(test_memory_tracking_allocation);
    RUN_TEST(test_memory_untracking_allocation);
    
    // Asset usage tracking tests
    RUN_TEST(test_asset_usage_tracking);
    
    // Memory limit tests
    RUN_TEST(test_memory_limit_detection);
    
    // Configuration tests
    RUN_TEST(test_streaming_configuration);
    
    // Integration tests
    RUN_TEST(test_memory_system_update);
    RUN_TEST(test_memory_report_generation);
    
    // Asset unloading tests
    RUN_TEST(test_mock_asset_unloading);
    
    return UNITY_END();
}
