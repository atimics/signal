// tests/performance/test_memory_isolated.c
// Isolated memory performance tests - Sprint 19 TDD REFACTOR Phase
// Tests memory system without graphics/asset dependencies

#include "../vendor/unity.h"
#include "../../src/system/memory.h"
#include "../../src/core.h"
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
    memset(perf_world, 0, sizeof(struct World));
    perf_world->max_entities = PERF_MAX_ENTITIES;
    perf_world->entities = malloc(sizeof(struct Entity) * PERF_MAX_ENTITIES);
    memset(perf_world->entities, 0, sizeof(struct Entity) * PERF_MAX_ENTITIES);
}

void tearDown(void) {
    if (perf_world) {
        if (perf_world->entities) {
            free(perf_world->entities);
        }
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
    const int allocations_per_size = 100;
    
    void** allocations = malloc(sizeof(void*) * num_sizes * allocations_per_size);
    int allocation_count = 0;
    
    // Allocate in mixed sizes
    for (int round = 0; round < allocations_per_size; round++) {
        for (int i = 0; i < num_sizes; i++) {
            allocations[allocation_count] = memory_pool_alloc(test_pool_id, sizes[i]);
            TEST_ASSERT_NOT_NULL(allocations[allocation_count]);
            allocation_count++;
        }
    }
    
    // Free every other allocation to create fragmentation
    int freed_count = 0;
    for (int i = 0; i < allocation_count; i += 2) {
        memory_pool_free(test_pool_id, allocations[i]);
        allocations[i] = NULL;
        freed_count++;
    }
    
    // Try to allocate new blocks in the fragmented space
    int successful_reallocs = 0;
    for (int i = 0; i < freed_count; i++) {
        void* ptr = memory_pool_alloc(test_pool_id, 512); // Medium size
        if (ptr != NULL) {
            successful_reallocs++;
            memory_pool_free(test_pool_id, ptr);
        }
    }
    
    // Should be able to reuse at least 50% of fragmented space
    double reuse_rate = (double)successful_reallocs / freed_count;
    printf("📊 Fragmentation reuse rate: %.2f%% (%d/%d)\n", 
           reuse_rate * 100, successful_reallocs, freed_count);
    
    TEST_ASSERT_GREATER_THAN(0.5, reuse_rate);
    
    // Clean up remaining allocations
    for (int i = 1; i < allocation_count; i += 2) {
        if (allocations[i] != NULL) {
            memory_pool_free(test_pool_id, allocations[i]);
        }
    }
    
    free(allocations);
    printf("✅ Memory fragmentation resistance test passed\n");
}

void test_memory_tracking_accuracy(void) {
    printf("🚀 Testing memory tracking accuracy...\n");
    
    // Get initial memory stats
    struct MemoryStats initial_stats = memory_get_stats();
    size_t initial_allocated = initial_stats.bytes_allocated;
    
    // Allocate known amount of memory
    const size_t test_allocation_size = 1024 * 1024; // 1MB
    void* test_ptr = memory_pool_alloc(test_pool_id, test_allocation_size);
    TEST_ASSERT_NOT_NULL(test_ptr);
    
    // Check that memory tracking reflects the allocation
    struct MemoryStats after_alloc_stats = memory_get_stats();
    size_t allocated_difference = after_alloc_stats.bytes_allocated - initial_allocated;
    
    printf("📊 Allocated difference: %zu bytes (expected: %zu)\n", 
           allocated_difference, test_allocation_size);
    
    // Should track at least the requested size (may be slightly more due to alignment)
    TEST_ASSERT_GREATER_OR_EQUAL(test_allocation_size, allocated_difference);
    
    // Free the allocation
    memory_pool_free(test_pool_id, test_ptr);
    
    // Check that memory tracking reflects the deallocation
    struct MemoryStats after_free_stats = memory_get_stats();
    size_t final_allocated = after_free_stats.bytes_allocated;
    
    printf("📊 Final allocated: %zu bytes (initial: %zu)\n", 
           final_allocated, initial_allocated);
    
    // Should be back to initial level (or very close)
    size_t difference = (final_allocated > initial_allocated) ? 
                       (final_allocated - initial_allocated) : 
                       (initial_allocated - final_allocated);
    TEST_ASSERT_LESS_THAN(1024, difference); // Allow small difference for bookkeeping
    
    printf("✅ Memory tracking accuracy test passed\n");
}

void test_concurrent_pool_usage(void) {
    printf("🚀 Testing concurrent pool usage patterns...\n");
    
    // Create multiple pools
    uint32_t pool_ids[3];
    pool_ids[0] = memory_create_pool("Pool1", 1024 * 1024);
    pool_ids[1] = memory_create_pool("Pool2", 2 * 1024 * 1024);
    pool_ids[2] = memory_create_pool("Pool3", 512 * 1024);
    
    // Allocate from each pool simultaneously
    void* ptrs[3][100];
    for (int i = 0; i < 100; i++) {
        ptrs[0][i] = memory_pool_alloc(pool_ids[0], 1024);
        ptrs[1][i] = memory_pool_alloc(pool_ids[1], 2048);
        ptrs[2][i] = memory_pool_alloc(pool_ids[2], 512);
        
        TEST_ASSERT_NOT_NULL(ptrs[0][i]);
        TEST_ASSERT_NOT_NULL(ptrs[1][i]);
        TEST_ASSERT_NOT_NULL(ptrs[2][i]);
    }
    
    // Free in different order to test pool independence
    for (int i = 99; i >= 0; i--) {
        memory_pool_free(pool_ids[0], ptrs[0][i]);
    }
    for (int i = 0; i < 100; i += 2) {
        memory_pool_free(pool_ids[1], ptrs[1][i]);
    }
    for (int i = 1; i < 100; i += 2) {
        memory_pool_free(pool_ids[1], ptrs[1][i]);
    }
    for (int i = 50; i < 100; i++) {
        memory_pool_free(pool_ids[2], ptrs[2][i]);
    }
    for (int i = 0; i < 50; i++) {
        memory_pool_free(pool_ids[2], ptrs[2][i]);
    }
    
    // Clean up pools
    memory_destroy_pool(pool_ids[0]);
    memory_destroy_pool(pool_ids[1]);
    memory_destroy_pool(pool_ids[2]);
    
    printf("✅ Concurrent pool usage test passed\n");
}

// ============================================================================
// ECS PERFORMANCE TESTS
// ============================================================================

void test_entity_creation_performance(void) {
    printf("🚀 Testing entity creation performance...\n");
    
    double start_time = get_time_ms();
    
    // Create many entities
    EntityID entities[PERF_MAX_ENTITIES];
    for (int i = 0; i < PERF_MAX_ENTITIES; i++) {
        entities[i] = entity_create(perf_world);
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entities[i]);
    }
    
    double creation_time = get_time_ms() - start_time;
    double entities_per_ms = PERF_MAX_ENTITIES / creation_time;
    
    printf("📊 Entity creation rate: %.2f entities/ms (%.2f ms total)\n", 
           entities_per_ms, creation_time);
    
    // Performance target: Should create >1000 entities per millisecond
    TEST_ASSERT_GREATER_THAN(1000.0, entities_per_ms);
    
    // Clean up entities
    start_time = get_time_ms();
    for (int i = 0; i < PERF_MAX_ENTITIES; i++) {
        bool destroyed = entity_destroy(perf_world, entities[i]);
        TEST_ASSERT_TRUE(destroyed);
    }
    double destruction_time = get_time_ms() - start_time;
    double destroy_per_ms = PERF_MAX_ENTITIES / destruction_time;
    
    printf("📊 Entity destruction rate: %.2f entities/ms (%.2f ms total)\n", 
           destroy_per_ms, destruction_time);
    
    TEST_ASSERT_GREATER_THAN(1000.0, destroy_per_ms);
    
    printf("✅ Entity creation performance test passed\n");
}

void test_component_access_performance(void) {
    printf("🚀 Testing component access performance...\n");
    
    // Create entities with transform components
    const int num_entities = 100;
    EntityID entities[num_entities];
    
    for (int i = 0; i < num_entities; i++) {
        entities[i] = entity_create(perf_world);
        entity_add_component(perf_world, entities[i], COMPONENT_TRANSFORM);
    }
    
    // Time component access
    double start_time = get_time_ms();
    const int access_iterations = 10000;
    
    for (int iter = 0; iter < access_iterations; iter++) {
        for (int i = 0; i < num_entities; i++) {
            struct Transform* transform = entity_get_transform(perf_world, entities[i]);
            TEST_ASSERT_NOT_NULL(transform);
            // Simulate some work with the component
            (void)transform;
        }
    }
    
    double access_time = get_time_ms() - start_time;
    double accesses_per_ms = (access_iterations * num_entities) / access_time;
    
    printf("📊 Component access rate: %.2f accesses/ms (%.2f ms total)\n", 
           accesses_per_ms, access_time);
    
    // Performance target: Should access >10000 components per millisecond
    TEST_ASSERT_GREATER_THAN(10000.0, accesses_per_ms);
    
    // Clean up
    for (int i = 0; i < num_entities; i++) {
        entity_destroy(perf_world, entities[i]);
    }
    
    printf("✅ Component access performance test passed\n");
}

// ============================================================================
// MAIN PERFORMANCE TEST RUNNER
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    
    printf("🚀 CGame Engine Performance Tests - Sprint 19 REFACTOR Phase\n");
    printf("============================================================\n");
    
    // Memory performance tests
    RUN_TEST(test_memory_pool_allocation_speed);
    RUN_TEST(test_memory_fragmentation_resistance);
    RUN_TEST(test_memory_tracking_accuracy);
    RUN_TEST(test_concurrent_pool_usage);
    
    // ECS performance tests
    RUN_TEST(test_entity_creation_performance);
    RUN_TEST(test_component_access_performance);
    
    printf("============================================================\n");
    printf("🎯 Performance Test Suite Complete\n");
    
    return UNITY_END();
}
