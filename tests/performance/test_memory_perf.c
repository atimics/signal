// tests/performance/test_memory_perf.c
// Performance tests for memory management system - Sprint 19 TDD
// Test-Driven Development: Performance benchmarks and regression testing

#include "../vendor/unity.h"
#include "../../src/system/memory.h"
#include "../../src/core.h"

// Use mocks for graphics dependencies in testing
#ifdef MOCK_GRAPHICS
#include "../mocks/mock_graphics.h"
#else
#include "../../src/assets.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Performance test configuration
#define PERF_TEST_ITERATIONS 1000
#define PERF_TEST_POOL_SIZE (10 * 1024 * 1024) // 10MB
#define PERF_TEST_ALLOCATION_SIZE 4096 // 4KB allocations
#define PERF_MAX_ENTITIES 500

// Performance timing utilities
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

// Test data structures
static struct World* perf_world;
static uint32_t test_pool_id;

void setUp(void) {
    // Initialize memory system with larger capacity for performance testing
    memory_system_init(128); // 128MB for performance testing
    
    // Create test pool
    test_pool_id = memory_create_pool("PerfTestPool", PERF_TEST_POOL_SIZE);
    
    // Initialize test world
    perf_world = malloc(sizeof(struct World));
    world_init(perf_world);
}

void tearDown(void) {
    if (perf_world) {
        world_destroy(perf_world);
        free(perf_world);
        perf_world = NULL;
    }
    memory_system_shutdown();
}

// ============================================================================
// MEMORY ALLOCATION PERFORMANCE TESTS
// ============================================================================

void test_memory_pool_allocation_speed(void) {
    printf("🚀 Testing memory pool allocation speed...\n");
    
    double start_time = get_time_ms();
    void* allocations[PERF_TEST_ITERATIONS];
    
    // Perform many small allocations
    for (int i = 0; i < PERF_TEST_ITERATIONS; i++) {
        allocations[i] = memory_pool_alloc(test_pool_id, PERF_TEST_ALLOCATION_SIZE);
        TEST_ASSERT_NOT_NULL(allocations[i]);
    }
    
    double allocation_time = get_time_ms() - start_time;
    
    // Free all allocations
    start_time = get_time_ms();
    for (int i = 0; i < PERF_TEST_ITERATIONS; i++) {
        memory_pool_free(test_pool_id, allocations[i]);
    }
    double deallocation_time = get_time_ms() - start_time;
    
    // Performance requirements (may fail initially - RED phase)
    double alloc_per_ms = PERF_TEST_ITERATIONS / allocation_time;
    double free_per_ms = PERF_TEST_ITERATIONS / deallocation_time;
    
    printf("📊 Allocation rate: %.2f allocs/ms (%.2f ms total)\n", alloc_per_ms, allocation_time);
    printf("📊 Deallocation rate: %.2f frees/ms (%.2f ms total)\n", free_per_ms, deallocation_time);
    
    // Performance targets: Should be able to do >100 allocations per millisecond
    TEST_ASSERT_GREATER_THAN(100.0, alloc_per_ms);
    TEST_ASSERT_GREATER_THAN(100.0, free_per_ms);
    
    printf("✅ Memory pool allocation speed test passed\n");
}

void test_memory_fragmentation_resistance(void) {
    printf("🚀 Testing memory fragmentation resistance...\n");
    
    // Allocate various sizes to test fragmentation
    const size_t sizes[] = {64, 128, 256, 512, 1024, 2048, 4096};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const int allocs_per_size = 50;
    
    void* allocations[num_sizes * allocs_per_size];
    int alloc_count = 0;
    
    double start_time = get_time_ms();
    
    // Allocate mixed sizes
    for (int size_idx = 0; size_idx < num_sizes; size_idx++) {
        for (int i = 0; i < allocs_per_size; i++) {
            allocations[alloc_count] = memory_pool_alloc(test_pool_id, sizes[size_idx]);
            TEST_ASSERT_NOT_NULL(allocations[alloc_count]);
            alloc_count++;
        }
    }
    
    // Free every other allocation to create fragmentation
    for (int i = 1; i < alloc_count; i += 2) {
        memory_pool_free(test_pool_id, allocations[i]);
        allocations[i] = NULL;
    }
    
    // Try to allocate in the gaps
    int successful_gap_allocs = 0;
    for (int i = 1; i < alloc_count; i += 2) {
        allocations[i] = memory_pool_alloc(test_pool_id, 128); // Small allocation
        if (allocations[i] != NULL) {
            successful_gap_allocs++;
        }
    }
    
    double fragmentation_time = get_time_ms() - start_time;
    
    printf("📊 Fragmentation test time: %.2f ms\n", fragmentation_time);
    printf("📊 Successful gap allocations: %d/%d\n", successful_gap_allocs, alloc_count / 2);
    
    // Should be able to reuse most fragmented space
    double reuse_rate = (double)successful_gap_allocs / (alloc_count / 2);
    TEST_ASSERT_GREATER_THAN(0.8, reuse_rate); // 80% reuse rate
    
    // Clean up
    for (int i = 0; i < alloc_count; i++) {
        if (allocations[i] != NULL) {
            memory_pool_free(test_pool_id, allocations[i]);
        }
    }
    
    printf("✅ Memory fragmentation resistance test passed\n");
}

// ============================================================================
// ASSET LOADING PERFORMANCE TESTS
// ============================================================================

void test_asset_memory_tracking_performance(void) {
    printf("🚀 Testing asset memory tracking performance...\n");
    
    // Simulate asset loading with memory tracking
    const int num_assets = 10;  // Reduced to fit in 10MB pool
    const size_t asset_size = 512 * 1024; // 512KB per asset (5MB total)
    
    double start_time = get_time_ms();
    
    // Track memory allocations like asset loading would
    for (int i = 0; i < num_assets; i++) {
        void* asset_data = memory_pool_alloc(test_pool_id, asset_size);
        TEST_ASSERT_NOT_NULL(asset_data);
        
        // Create unique asset name for each asset
        char asset_name[32];
        snprintf(asset_name, sizeof(asset_name), "TestAsset_%d", i);
        
        // Simulate tracking this asset
        memory_track_asset_allocation(asset_data, asset_size, asset_name);
        
        // Simulate some processing
        memset(asset_data, i % 256, asset_size);
    }
    
    double tracking_time = get_time_ms() - start_time;
    
    // Get memory statistics
    size_t total_mb, peak_mb;
    uint32_t asset_count;
    memory_get_stats(&total_mb, &peak_mb, &asset_count);
    
    printf("📊 Asset tracking time: %.2f ms for %d assets\n", tracking_time, num_assets);
    printf("📊 Tracked memory: %zu MB, %u assets\n", total_mb, asset_count);
    
    // Performance requirements
    double assets_per_ms = num_assets / tracking_time;
    TEST_ASSERT_GREATER_THAN(1.0, assets_per_ms); // At least 1 asset per ms
    TEST_ASSERT_EQUAL_UINT32(num_assets, asset_count);
    
    printf("✅ Asset memory tracking performance test passed\n");
}

void test_distance_based_unloading_performance(void) {
    printf("🚀 Testing distance-based memory unloading performance...\n");
    
    // Create entities with positions for distance testing
    const int num_entities = PERF_MAX_ENTITIES;
    EntityID entities[PERF_MAX_ENTITIES];
    
    // Create entities with transform and renderable components
    for (int i = 0; i < num_entities; i++) {
        entities[i] = entity_create(perf_world);
        entity_add_components(perf_world, entities[i], COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
        
        struct Transform* transform = entity_get_transform(perf_world, entities[i]);
        TEST_ASSERT_NOT_NULL(transform);
        
        // Spread entities in a grid
        transform->position.x = (float)(i % 20) * 10.0f;
        transform->position.y = 0.0f;
        transform->position.z = (float)(i / 20) * 10.0f;
    }
    
    // Set player position at origin
    Vector3 player_pos = {0.0f, 0.0f, 0.0f};
    float unload_distance = 100.0f;
    
    double start_time = get_time_ms();
    
    // Perform distance-based unloading check
    int unload_candidates = 0;
    for (int i = 0; i < num_entities; i++) {
        struct Transform* transform = entity_get_transform(perf_world, entities[i]);
        if (transform) {
            float dx = transform->position.x - player_pos.x;
            float dy = transform->position.y - player_pos.y;
            float dz = transform->position.z - player_pos.z;
            float distance = sqrtf(dx*dx + dy*dy + dz*dz);
            
            if (distance > unload_distance) {
                unload_candidates++;
                // Would unload asset here
            }
        }
    }
    
    double unload_check_time = get_time_ms() - start_time;
    
    printf("📊 Distance check time: %.2f ms for %d entities\n", unload_check_time, num_entities);
    printf("📊 Unload candidates: %d entities\n", unload_candidates);
    
    // Performance requirements: Should process all entities quickly
    double entities_per_ms = num_entities / unload_check_time;
    TEST_ASSERT_GREATER_THAN(100.0, entities_per_ms); // >100 entities per ms
    
    printf("✅ Distance-based unloading performance test passed\n");
}

// ============================================================================
// ECS PERFORMANCE INTEGRATION TESTS
// ============================================================================

void test_ecs_memory_integration_performance(void) {
    printf("🚀 Testing ECS-Memory integration performance...\n");
    
    const int num_entities = PERF_MAX_ENTITIES;
    const int iterations = 10;
    
    double total_creation_time = 0.0;
    double total_access_time = 0.0;
    double total_destruction_time = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        EntityID entities[PERF_MAX_ENTITIES];
        
        // Measure entity creation with components
        double start_time = get_time_ms();
        for (int i = 0; i < num_entities; i++) {
            entities[i] = entity_create(perf_world);
            entity_add_components(perf_world, entities[i], 
                COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_RENDERABLE);
        }
        total_creation_time += get_time_ms() - start_time;
        
        // Measure component access performance
        start_time = get_time_ms();
        for (int i = 0; i < num_entities; i++) {
            struct Transform* transform = entity_get_transform(perf_world, entities[i]);
            struct Physics* physics = entity_get_physics(perf_world, entities[i]);
            
            if (transform && physics) {
                // Simulate system processing
                physics->velocity.x += transform->position.x * 0.01f;
                transform->position.x += physics->velocity.x * 0.016f; // 60 FPS
            }
        }
        total_access_time += get_time_ms() - start_time;
        
        // Measure entity destruction
        start_time = get_time_ms();
        for (int i = 0; i < num_entities; i++) {
            entity_destroy(perf_world, entities[i]);
        }
        total_destruction_time += get_time_ms() - start_time;
    }
    
    // Calculate averages
    double avg_creation = total_creation_time / iterations;
    double avg_access = total_access_time / iterations;
    double avg_destruction = total_destruction_time / iterations;
    
    printf("📊 Average creation time: %.2f ms (%d entities)\n", avg_creation, num_entities);
    printf("📊 Average access time: %.2f ms (%d entities)\n", avg_access, num_entities);
    printf("📊 Average destruction time: %.2f ms (%d entities)\n", avg_destruction, num_entities);
    
    // Performance requirements for 60 FPS game loop
    double entities_per_frame_budget = 16.67; // 16.67ms per frame at 60 FPS
    
    // Creation can be slower (not done every frame)
    TEST_ASSERT_LESS_THAN(100.0, avg_creation); // <100ms for batch creation
    
    // Access must be very fast (done every frame)
    TEST_ASSERT_LESS_THAN(entities_per_frame_budget, avg_access);
    
    // Destruction can be moderate
    TEST_ASSERT_LESS_THAN(50.0, avg_destruction);
    
    printf("✅ ECS-Memory integration performance test passed\n");
}

void test_memory_pool_scaling_performance(void) {
    printf("🚀 Testing memory pool scaling performance...\n");
    
    // Test pool performance with different sizes
    const size_t pool_sizes[] = {
        1 * 1024 * 1024,    // 1MB
        5 * 1024 * 1024,    // 5MB
        10 * 1024 * 1024,   // 10MB
        25 * 1024 * 1024,   // 25MB
    };
    const int num_pool_sizes = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
    const int allocations_per_test = 500;
    
    for (int pool_idx = 0; pool_idx < num_pool_sizes; pool_idx++) {
        uint32_t pool_id = memory_create_pool("ScaleTestPool", pool_sizes[pool_idx]);
        TEST_ASSERT_NOT_EQUAL(UINT32_MAX, pool_id);
        
        void* allocations[allocations_per_test];
        
        // Test allocation performance for this pool size
        double start_time = get_time_ms();
        for (int i = 0; i < allocations_per_test; i++) {
            allocations[i] = memory_pool_alloc(pool_id, 1024); // 1KB allocations
            TEST_ASSERT_NOT_NULL(allocations[i]);
        }
        double alloc_time = get_time_ms() - start_time;
        
        // Clean up
        for (int i = 0; i < allocations_per_test; i++) {
            memory_pool_free(pool_id, allocations[i]);
        }
        
        memory_destroy_pool(pool_id);
        
        double allocs_per_ms = allocations_per_test / alloc_time;
        printf("📊 Pool size %zu MB: %.2f allocs/ms\n", 
               pool_sizes[pool_idx] / (1024*1024), allocs_per_ms);
        
        // Performance should not degrade significantly with larger pools
        TEST_ASSERT_GREATER_THAN(50.0, allocs_per_ms);
    }
    
    printf("✅ Memory pool scaling performance test passed\n");
}

// ============================================================================
// MEMORY PRESSURE AND LIMITS TESTS
// ============================================================================

void test_memory_pressure_handling(void) {
    printf("🚀 Testing memory pressure handling...\n");
    
    // Fill memory to near capacity
    const size_t pressure_pool_size = 50 * 1024 * 1024; // 50MB
    uint32_t pressure_pool = memory_create_pool("PressurePool", pressure_pool_size);
    TEST_ASSERT_NOT_EQUAL(UINT32_MAX, pressure_pool);
    
    // Allocate until we hit pressure
    void* allocations[1000];
    int successful_allocs = 0;
    const size_t large_alloc_size = 1024 * 1024; // 1MB allocations
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < 1000; i++) {
        allocations[i] = memory_pool_alloc(pressure_pool, large_alloc_size);
        if (allocations[i] != NULL) {
            successful_allocs++;
        } else {
            break; // Hit pool limit
        }
    }
    
    double pressure_time = get_time_ms() - start_time;
    
    printf("📊 Allocated %d MB before pressure (%.2f ms)\n", 
           successful_allocs, pressure_time);
    
    // Should handle pressure gracefully
    TEST_ASSERT_GREATER_THAN(30, successful_allocs); // Should get at least 30MB
    TEST_ASSERT_LESS_THAN(60, successful_allocs); // But not more than capacity
    
    // Test memory pressure detection
    bool over_limit = memory_is_over_limit();
    printf("📊 Memory over limit: %s\n", over_limit ? "YES" : "NO");
    
    // Clean up
    for (int i = 0; i < successful_allocs; i++) {
        if (allocations[i] != NULL) {
            memory_pool_free(pressure_pool, allocations[i]);
        }
    }
    
    memory_destroy_pool(pressure_pool);
    
    printf("✅ Memory pressure handling test passed\n");
}

// ============================================================================
// TEST RUNNER SETUP
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    
    printf("🚀 Starting Memory Performance Tests (TDD Red Phase)\n");
    printf("======================================================\n");
    
    // Core Memory Performance Tests
    RUN_TEST(test_memory_pool_allocation_speed);
    RUN_TEST(test_memory_fragmentation_resistance);
    
    // Asset Loading Performance Tests
    RUN_TEST(test_asset_memory_tracking_performance);
    RUN_TEST(test_distance_based_unloading_performance);
    
    // ECS Integration Performance Tests
    RUN_TEST(test_ecs_memory_integration_performance);
    RUN_TEST(test_memory_pool_scaling_performance);
    
    // Memory Pressure Tests
    RUN_TEST(test_memory_pressure_handling);
    
    printf("======================================================\n");
    printf("🎯 Performance Tests Complete - Ready for optimization\n");
    
    return UNITY_END();
}
