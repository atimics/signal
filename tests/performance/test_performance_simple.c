/**
 * @file test_performance_simple.c
 * @brief Simple performance tests for core systems
 * 
 * Basic performance tests that work with our current test infrastructure.
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/ui_components.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Test setup and teardown functions (required by Unity framework)
void setUp(void)
{
    // Setup before each test
}

void tearDown(void)
{
    // Cleanup after each test
}

// Performance test configuration
#define PERFORMANCE_ENTITY_COUNT 100
#define PERFORMANCE_ITERATIONS 1000

// Simple timing helper
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

void test_entity_creation_performance(void)
{
    printf("🚀 Testing entity creation performance...\n");
    
    struct World world;
    world_init(&world);
    
    double start_time = get_time_ms();
    
    // Create many entities
    for (int i = 0; i < PERFORMANCE_ENTITY_COUNT; i++) {
        EntityID entity = entity_create(&world);
        entity_add_component(&world, entity, COMPONENT_TRANSFORM);
        
        struct Transform* transform = entity_get_transform(&world, entity);
        if (transform) {
            transform->position = (Vector3){(float)i, 0.0f, 0.0f};
        }
    }
    
    double end_time = get_time_ms();
    double duration_ms = end_time - start_time;
    
    printf("   Created %d entities in %.2f ms (%.2f entities/ms)\n", 
           PERFORMANCE_ENTITY_COUNT, duration_ms, PERFORMANCE_ENTITY_COUNT / duration_ms);
    
    // Performance should be reasonable (less than 100ms for 100 entities)
    TEST_ASSERT_LESS_THAN(100.0, duration_ms);
    
    world_destroy(&world);
}

void test_ui_widget_performance(void)
{
    printf("🚀 Testing UI widget update performance...\n");
    
    PerformanceWidget widget;
    performance_widget_init(&widget);
    
    double start_time = get_time_ms();
    
    // Simulate many widget updates
    for (int i = 0; i < PERFORMANCE_ITERATIONS; i++) {
        performance_widget_update(&widget, 0.016f);
    }
    
    double end_time = get_time_ms();
    double duration_ms = end_time - start_time;
    
    printf("   Updated widget %d times in %.2f ms (%.2f updates/ms)\n", 
           PERFORMANCE_ITERATIONS, duration_ms, PERFORMANCE_ITERATIONS / duration_ms);
    
    // Performance should be reasonable (less than 50ms for 1000 updates)
    TEST_ASSERT_LESS_THAN(50.0, duration_ms);
}

void test_vector_math_performance(void)
{
    printf("🚀 Testing vector math performance...\n");
    
    Vector3 a = {1.0f, 2.0f, 3.0f};
    Vector3 b = {4.0f, 5.0f, 6.0f};
    Vector3 result;
    
    double start_time = get_time_ms();
    
    // Perform many vector operations
    for (int i = 0; i < PERFORMANCE_ITERATIONS * 10; i++) {
        result = vector3_add(a, b);
        result = vector3_multiply(result, 1.1f);
        result = vector3_normalize(result);
    }
    
    double end_time = get_time_ms();
    double duration_ms = end_time - start_time;
    
    printf("   Performed %d vector operations in %.2f ms (%.2f ops/ms)\n", 
           PERFORMANCE_ITERATIONS * 10 * 3, duration_ms, (PERFORMANCE_ITERATIONS * 10 * 3) / duration_ms);
    
    // Performance should be reasonable (less than 10ms for 30000 operations)
    TEST_ASSERT_LESS_THAN(10.0, duration_ms);
    
    // Ensure result is valid (not optimized away)
    TEST_ASSERT_GREATER_THAN(0.0f, vector3_length(result));
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_performance(void)
{
    printf("\n🚀 Performance Tests\n");
    printf("====================\n");
    
    RUN_TEST(test_entity_creation_performance);
    RUN_TEST(test_ui_widget_performance);
    RUN_TEST(test_vector_math_performance);
    
    printf("✅ Performance Tests Complete\n");
}

int main(void)
{
    UNITY_BEGIN();
    suite_performance();
    return UNITY_END();
}
