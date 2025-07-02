/**
 * @file test_physics_critical.c
 * @brief Critical physics tests for Sprint 21 velocity integration bug
 * 
 * Isolated tests to debug the velocity integration issue identified in Sprint 21.
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/system/physics.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Test world
static struct World test_world;

void setUp(void)
{
    // Initialize test world properly using world_init
    if (!world_init(&test_world)) {
        printf("❌ Failed to initialize test world\n");
        return;
    }
    
    // Override max entities for test efficiency
    test_world.max_entities = 100;
}

void tearDown(void)
{
    // Properly destroy the world
    world_destroy(&test_world);
}

/**
 * @brief Critical Test 1: Basic Velocity Integration Isolation
 * 
 * This test isolates the velocity accumulation bug identified in Sprint 21.
 * Expected to FAIL if the integration bug exists, PASS if fixed.
 */
void test_physics_velocity_integration_basic(void)
{
    printf("🔍 Testing basic velocity integration with known values...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    TEST_ASSERT_NOT_NULL(physics);
    TEST_ASSERT_NOT_NULL(transform);
    
    // Set up known test conditions
    physics->mass = 100.0f;  // 100kg mass
    physics->drag_linear = 1.0f;  // NO drag to isolate issue
    physics->kinematic = false;
    physics->has_6dof = true;
    
    // Clear initial state
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics->acceleration = (Vector3){0.0f, 0.0f, 0.0f};
    physics->force_accumulator = (Vector3){0.0f, 0.0f, 0.0f};
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    // Apply a known force: 1000N forward
    physics_add_force(physics, (Vector3){1000.0f, 0.0f, 0.0f});
    
    // Expected: 1000N / 100kg = 10 m/s² acceleration
    // With dt=0.016s (60 FPS): velocity change = 10 * 0.016 = 0.16 m/s
    float delta_time = 0.016f;
    RenderConfig dummy_render_config = {0};
    
    // Debug: Print initial state
    printf("BEFORE: Force:[%.2f,%.2f,%.2f] Vel:[%.2f,%.2f,%.2f] Pos:[%.2f,%.2f,%.2f]\n",
           physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z,
           physics->velocity.x, physics->velocity.y, physics->velocity.z,
           transform->position.x, transform->position.y, transform->position.z);
    
    // Run one physics update
    physics_system_update(&test_world, &dummy_render_config, delta_time);
    
    // Debug: Print final state
    printf("AFTER:  Force:[%.2f,%.2f,%.2f] Vel:[%.2f,%.2f,%.2f] Pos:[%.6f,%.6f,%.6f]\n",
           physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z,
           physics->velocity.x, physics->velocity.y, physics->velocity.z,
           transform->position.x, transform->position.y, transform->position.z);
    
    // Critical test: velocity should have changed from applied force
    TEST_ASSERT_GREATER_THAN_FLOAT(0.1f, physics->velocity.x);  // Should be ~0.16 m/s
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.y);  // No Y force applied
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.z);  // No Z force applied
    
    // Position should also have changed (velocity * dt)
    // Expected: 0.16 m/s * 0.016s = 0.00256m
    printf("Expected position change: %.6f m\n", physics->velocity.x * delta_time);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.001f, transform->position.x);  // Should be ~0.00256m
}

/**
 * @brief Critical Test 2: Force Accumulator Timing
 * 
 * Tests that forces are applied before being cleared, ensuring proper timing
 * of the force accumulation and integration pipeline.
 */
void test_physics_force_accumulator_timing(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    physics->mass = 100.0f;
    physics->drag_linear = 1.0f;  // No drag
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    
    // Add force and verify it's in accumulator
    physics_add_force(physics, (Vector3){1000.0f, 0.0f, 0.0f});
    TEST_ASSERT_EQUAL_FLOAT(1000.0f, physics->force_accumulator.x);
    
    // Run physics update
    RenderConfig dummy_render_config = {0};
    physics_system_update(&test_world, &dummy_render_config, 0.016f);
    
    // Force accumulator should be cleared after update
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.x);
    
    // But velocity should have changed
    TEST_ASSERT_GREATER_THAN_FLOAT(0.1f, physics->velocity.x);
}

/**
 * @brief Critical Test 3: Sprint 21 Exact Reproduction
 * 
 * Tests the exact conditions reported in Sprint 21 to verify
 * if the bug can be reproduced in the test environment.
 */
void test_physics_sprint21_exact_reproduction(void)
{
    printf("🔍 Reproducing exact Sprint 21 conditions...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    TEST_ASSERT_NOT_NULL(physics);
    TEST_ASSERT_NOT_NULL(transform);
    
    // Set exact Sprint 21 conditions
    physics->mass = 80.0f;  // Ship mass from Sprint 21
    physics->drag_linear = 0.9999f;  // High drag from Sprint 21
    physics->kinematic = false;
    physics->has_6dof = true;
    
    // Clear initial state
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics->acceleration = (Vector3){0.0f, 0.0f, 0.0f};
    physics->force_accumulator = (Vector3){0.0f, 0.0f, 0.0f};
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    // Apply exact forces from Sprint 21 debug output
    physics_add_force(physics, (Vector3){35000.0f, -240.0f, 0.0f});
    
    printf("🚀 Sprint 21 Reproduction Test\n");
    printf("Mass: %.1f kg\n", physics->mass);
    printf("Drag: %.4f\n", physics->drag_linear);
    printf("Applied Force: [%.0f, %.0f, %.0f] N\n", 
           physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z);
    
    // Expected acceleration: F/m = 35000N/80kg = 437.5 m/s²
    float expected_accel_x = 35000.0f / 80.0f;
    float expected_accel_y = -240.0f / 80.0f;
    printf("Expected Acceleration: [%.2f, %.2f, 0.00] m/s²\n", expected_accel_x, expected_accel_y);
    
    float delta_time = 0.016f;
    RenderConfig dummy_render_config = {0};
    
    // Run one physics update
    physics_system_update(&test_world, &dummy_render_config, delta_time);
    
    printf("⚡ LINEAR: Force:[%.0f,%.0f,%.0f] -> Accel:[%.2f,%.2f,%.2f] -> Vel:[%.2f,%.2f,%.2f]\n",
           physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z,
           physics->acceleration.x, physics->acceleration.y, physics->acceleration.z,
           physics->velocity.x, physics->velocity.y, physics->velocity.z);
    
    // Expected velocity after one frame with drag:
    // v = (F/m * dt) * drag = (437.5 * 0.016) * 0.9999 = 7.0 * 0.9999 = 6.9993 m/s
    float expected_vel_x = expected_accel_x * delta_time * physics->drag_linear;
    float expected_vel_y = expected_accel_y * delta_time * physics->drag_linear;
    
    printf("Expected Velocity: [%.4f, %.4f, 0.0000] m/s\n", expected_vel_x, expected_vel_y);
    
    // CRITICAL TEST: In Sprint 21, velocity was staying at [0.00, 0.00, 0.00]
    // Here it should be working correctly
    TEST_ASSERT_GREATER_THAN_FLOAT(6.0f, physics->velocity.x);  // Should be ~7.0 m/s
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_vel_y, physics->velocity.y);   // Should match expected Y velocity  
    TEST_ASSERT_NOT_EQUAL_FLOAT(0.0f, physics->velocity.x);    // Should NOT be zero
    TEST_ASSERT_NOT_EQUAL_FLOAT(0.0f, physics->velocity.y);    // Should NOT be zero
    
    printf("✅ Sprint 21 conditions reproduced - physics working correctly in test environment\n");
}

// ============================================================================
// TEST SUITE RUNNER
// ============================================================================

void suite_physics_critical(void)
{
    printf("\n🚀 Critical Physics Integration Tests\n");
    printf("=====================================\n");
    
    printf("🔧 Testing Critical Integration Bug...\n");
    RUN_TEST(test_physics_velocity_integration_basic);
    RUN_TEST(test_physics_force_accumulator_timing);
    RUN_TEST(test_physics_sprint21_exact_reproduction);
    RUN_TEST(test_physics_sprint21_exact_reproduction);
    
    printf("✅ Critical Physics Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    suite_physics_critical();
    return UNITY_END();
}
