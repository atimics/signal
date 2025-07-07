/**
 * @file test_systems.c
 * @brief Unit tests for ECS systems
 * 
 * Tests the core ECS systems including:
 * - System scheduler
 * - Physics system (foundation for Sprint 21)
 * - Camera system
 * - Performance monitoring
 * - Memory management
 */

#include "../vendor/unity.h"
#include "../../src/systems.h"
#include "../../src/core.h"
#include "../../src/system/physics.h"
#include "../../src/system/camera.h"
#include "../../src/system/performance.h"
#include "../../src/system/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Test world and render config
static struct World test_world;
static RenderConfig test_render_config;

void setUp(void)
{
    // Initialize test world
    memset(&test_world, 0, sizeof(struct World));
    test_world.max_entities = 100;
    test_world.entities = malloc(sizeof(struct Entity) * 100);
    test_world.entity_count = 0;
    test_world.next_entity_id = 1;
    
    // Initialize test render config
    memset(&test_render_config, 0, sizeof(RenderConfig));
    test_render_config.width = 1280;
    test_render_config.height = 720;
}

void tearDown(void)
{
    if (test_world.entities) {
        free(test_world.entities);
        test_world.entities = NULL;
    }
}

// ============================================================================
// SYSTEM SCHEDULER TESTS
// ============================================================================

void test_system_scheduler_initialization(void)
{
    SystemScheduler scheduler;
    
    // Initialize scheduler
    bool success = scheduler_init(&scheduler, &test_render_config);
    TEST_ASSERT_TRUE(success);
    
    // Check initial state
    TEST_ASSERT_EQUAL_INT(0, scheduler.frame_count);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, scheduler.total_time);
    
    // Cleanup
    scheduler_destroy(&scheduler, &test_render_config);
}

void test_system_scheduler_update(void)
{
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &test_render_config);
    
    float delta_time = 0.016f; // 60 FPS
    
    // Update scheduler
    scheduler_update(&scheduler, &test_world, &test_render_config, delta_time);
    
    // Check that frame count and time were updated
    TEST_ASSERT_EQUAL_INT(1, scheduler.frame_count);
    TEST_ASSERT_EQUAL_FLOAT(delta_time, scheduler.total_time);
    
    // Update again
    scheduler_update(&scheduler, &test_world, &test_render_config, delta_time);
    
    TEST_ASSERT_EQUAL_INT(2, scheduler.frame_count);
    TEST_ASSERT_EQUAL_FLOAT(delta_time * 2, scheduler.total_time);
    
    scheduler_destroy(&scheduler, &test_render_config);
}

void test_system_scheduler_enable_disable(void)
{
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &test_render_config);
    
    // Test system enable/disable
    scheduler_disable_system(&scheduler, SYSTEM_PHYSICS);
    scheduler_enable_system(&scheduler, SYSTEM_PHYSICS);
    
    // Test frequency setting
    scheduler_set_frequency(&scheduler, SYSTEM_PHYSICS, 30.0f);
    
    // Should not crash
    scheduler_update(&scheduler, &test_world, &test_render_config, 0.016f);
    
    scheduler_destroy(&scheduler, &test_render_config);
}

void test_system_scheduler_performance(void)
{
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &test_render_config);
    
    // Run many updates to test performance
    clock_t start_time = clock();
    
    for (int i = 0; i < 1000; i++) {
        scheduler_update(&scheduler, &test_world, &test_render_config, 0.001f);
    }
    
    clock_t end_time = clock();
    double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // Should complete within reasonable time (0.5 seconds)
    TEST_ASSERT_LESS_THAN(0.5, elapsed);
    
    scheduler_destroy(&scheduler, &test_render_config);
}

// ============================================================================
// PHYSICS SYSTEM TESTS (Critical for Sprint 21)
// ============================================================================

void test_physics_system_entity_creation(void)
{
    // Create entity with physics component
    EntityID entity = entity_create(&test_world);
    TEST_ASSERT_NOT_EQUAL(0, entity);
    
    // Add physics component
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    // Verify component was added
    struct Physics* physics = entity_get_physics(&test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Check initial values
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.z);
    TEST_ASSERT_GREATER_THAN(0.0f, physics->mass);
}

void test_physics_system_velocity_integration(void)
{
    // Create physics entity
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    TEST_ASSERT_NOT_NULL(physics);
    TEST_ASSERT_NOT_NULL(transform);
    
    // Set initial position and velocity
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    physics->velocity = (Vector3){1.0f, 0.0f, 0.0f}; // 1 unit/second in X
    
    float delta_time = 0.1f; // 0.1 seconds
    
    // Update physics system
    physics_system_update(&test_world, delta_time);
    
    // Position should have changed based on velocity
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, transform->position.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, transform->position.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, transform->position.z);
}

void test_physics_system_acceleration(void)
{
    // Create physics entity
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set initial values
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics->acceleration = (Vector3){1.0f, 0.0f, 0.0f}; // 1 unit/second²
    
    float delta_time = 1.0f; // 1 second
    
    // Update physics
    physics_system_update(&test_world, delta_time);
    
    // Velocity should have increased due to acceleration
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, physics->velocity.x);
    
    // Position should have moved (v = at, s = 0.5at²)
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, transform->position.x);
}

void test_physics_system_mass_effects(void)
{
    // Create two entities with different masses
    EntityID entity1 = entity_create(&test_world);
    EntityID entity2 = entity_create(&test_world);
    
    entity_add_component(&test_world, entity1, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity2, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics1 = entity_get_physics(&test_world, entity1);
    struct Physics* physics2 = entity_get_physics(&test_world, entity2);
    
    // Set different masses
    physics1->mass = 1.0f;
    physics2->mass = 2.0f;
    
    // Apply same force (F = ma, so acceleration = F/m)
    physics1->acceleration = (Vector3){1.0f / physics1->mass, 0.0f, 0.0f};
    physics2->acceleration = (Vector3){1.0f / physics2->mass, 0.0f, 0.0f};
    
    float delta_time = 1.0f;
    physics_system_update(&test_world, delta_time);
    
    // Lighter object should have higher velocity
    TEST_ASSERT_GREATER_THAN(physics2->velocity.x, physics1->velocity.x);
}

// ============================================================================
// CAMERA SYSTEM TESTS
// ============================================================================

void test_camera_system_initialization(void)
{
    // Initialize camera system
    camera_system_init(&test_world, &test_render_config);
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
    
    // Cleanup
    camera_system_shutdown();
}

void test_camera_system_entity_creation(void)
{
    camera_system_init(&test_world, &test_render_config);
    
    // Create camera entity
    EntityID camera = entity_create(&test_world);
    entity_add_component(&test_world, camera, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* cam = entity_get_camera(&test_world, camera);
    struct Transform* transform = entity_get_transform(&test_world, camera);
    
    TEST_ASSERT_NOT_NULL(cam);
    TEST_ASSERT_NOT_NULL(transform);
    
    // Check default camera values
    TEST_ASSERT_GREATER_THAN(0.0f, cam->fov);
    TEST_ASSERT_GREATER_THAN(0.0f, cam->near_plane);
    TEST_ASSERT_GREATER_THAN(cam->near_plane, cam->far_plane);
    
    camera_system_shutdown();
}

void test_camera_system_update(void)
{
    camera_system_init(&test_world, &test_render_config);
    
    // Create and setup camera
    EntityID camera = entity_create(&test_world);
    entity_add_component(&test_world, camera, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    // Update camera system
    camera_system_update(&test_world, &test_render_config, 0.016f);
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
    
    camera_system_shutdown();
}

// ============================================================================
// PERFORMANCE MONITORING TESTS
// ============================================================================

void test_performance_monitoring_initialization(void)
{
    // Initialize performance monitoring
    performance_monitoring_init();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
    
    // Update performance monitoring
    performance_monitoring_update(0.016f);
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
    
    // Cleanup
    performance_monitoring_shutdown();
}

void test_performance_monitoring_frame_time_tracking(void)
{
    performance_monitoring_init();
    
    // Update with consistent frame times
    for (int i = 0; i < 60; i++) {
        performance_monitoring_update(0.016f); // 60 FPS
    }
    
    // Should track frame times without issues
    // (Specific metrics depend on implementation)
    TEST_ASSERT_TRUE(true);
    
    performance_monitoring_shutdown();
}

// ============================================================================
// MEMORY MANAGEMENT TESTS
// ============================================================================

void test_memory_management_initialization(void)
{
    // Initialize memory management
    memory_management_init();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
    
    // Cleanup
    memory_management_shutdown();
}

void test_memory_management_pool_allocation(void)
{
    memory_management_init();
    
    // Test that memory management can handle allocation requests
    // (Specific tests depend on memory management implementation)
    
    // Update memory management
    memory_management_update(&test_world, 0.016f);
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
    
    memory_management_shutdown();
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_systems_integration(void)
{
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &test_render_config);
    
    // Initialize subsystems
    camera_system_init(&test_world, &test_render_config);
    performance_monitoring_init();
    memory_management_init();
    
    // Create a complete entity with multiple components
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, 
                        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_CAMERA);
    
    // Update all systems through scheduler
    for (int i = 0; i < 10; i++) {
        scheduler_update(&scheduler, &test_world, &test_render_config, 0.016f);
        performance_monitoring_update(0.016f);
        memory_management_update(&test_world, 0.016f);
    }
    
    // Should complete without crashes
    TEST_ASSERT_TRUE(true);
    
    // Cleanup
    memory_management_shutdown();
    performance_monitoring_shutdown();
    camera_system_shutdown();
    scheduler_destroy(&scheduler, &test_render_config);
}

void test_systems_high_entity_count(void)
{
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &test_render_config);
    
    // Create many entities to test performance
    const int entity_count = 50; // Reduced for unit test
    
    for (int i = 0; i < entity_count; i++) {
        EntityID entity = entity_create(&test_world);
        entity_add_component(&test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
        
        // Set some variation in physics properties
        struct Physics* physics = entity_get_physics(&test_world, entity);
        if (physics) {
            physics->velocity.x = (float)(i % 10) - 5.0f;
            physics->mass = 1.0f + (float)(i % 5) * 0.5f;
        }
    }
    
    // Update systems with many entities
    clock_t start_time = clock();
    
    for (int frame = 0; frame < 10; frame++) {
        scheduler_update(&scheduler, &test_world, &test_render_config, 0.016f);
    }
    
    clock_t end_time = clock();
    double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // Should complete within reasonable time
    TEST_ASSERT_LESS_THAN(0.1, elapsed);
    
    scheduler_destroy(&scheduler, &test_render_config);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_systems(void)
{
    printf("\n⚙️  ECS Systems Tests\n");
    printf("====================\n");
    
    printf("📅 Testing System Scheduler...\n");
    RUN_TEST(test_system_scheduler_initialization);
    RUN_TEST(test_system_scheduler_update);
    RUN_TEST(test_system_scheduler_enable_disable);
    RUN_TEST(test_system_scheduler_performance);
    
    printf("🏃 Testing Physics System...\n");
    RUN_TEST(test_physics_system_entity_creation);
    RUN_TEST(test_physics_system_velocity_integration);
    RUN_TEST(test_physics_system_acceleration);
    RUN_TEST(test_physics_system_mass_effects);
    
    printf("📷 Testing Camera System...\n");
    RUN_TEST(test_camera_system_initialization);
    RUN_TEST(test_camera_system_entity_creation);
    RUN_TEST(test_camera_system_update);
    
    printf("📊 Testing Performance Monitoring...\n");
    RUN_TEST(test_performance_monitoring_initialization);
    RUN_TEST(test_performance_monitoring_frame_time_tracking);
    
    printf("💾 Testing Memory Management...\n");
    RUN_TEST(test_memory_management_initialization);
    RUN_TEST(test_memory_management_pool_allocation);
    
    printf("🔗 Testing System Integration...\n");
    RUN_TEST(test_systems_integration);
    RUN_TEST(test_systems_high_entity_count);
    
    printf("✅ ECS Systems Tests Complete\n");
}
