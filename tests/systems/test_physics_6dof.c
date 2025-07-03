/**
 * @file test_physics_6dof.c
 * @brief Comprehensive tests for 6DOF physics system
 * 
 * Tests the enhanced physics system with 6 degrees of freedom,
 * force/torque accumulation, and angular dynamics.
 * Critical for Sprint 21 flight mechanics validation.
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

// ============================================================================
// 6DOF PHYSICS CORE TESTS
// ============================================================================

void test_physics_6dof_enabled_flag(void)
{
    printf("DEBUG: Starting basic component test...\n");
    
    // Create physics entity
    EntityID entity = entity_create(&test_world);
    printf("DEBUG: Created entity ID: %d\n", entity);
    
    if (entity == INVALID_ENTITY) {
        printf("DEBUG: Failed to create entity!\n");
        TEST_FAIL_MESSAGE("Failed to create entity");
        return;
    }
    
    printf("DEBUG: Adding transform component...\n");
    bool add_transform = entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    printf("DEBUG: Add transform result: %s\n", add_transform ? "SUCCESS" : "FAILED");
    
    printf("DEBUG: Adding physics component...\n");
    bool add_physics = entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    printf("DEBUG: Add physics result: %s\n", add_physics ? "SUCCESS" : "FAILED");
    
    if (!add_transform || !add_physics) {
        TEST_FAIL_MESSAGE("Failed to add components");
        return;
    }
    
    printf("DEBUG: Getting physics component...\n");
    struct Physics* physics = entity_get_physics(&test_world, entity);
    printf("DEBUG: Physics pointer: %p\n", (void*)physics);
    
    TEST_ASSERT_NOT_NULL(physics);
    
    // 6DOF should be disabled by default
    TEST_ASSERT_FALSE(physics->has_6dof);
    
    // Enable 6DOF
    physics_set_6dof_enabled(physics, true);
    TEST_ASSERT_TRUE(physics->has_6dof);
    
    // Disable 6DOF should clear angular state
    physics->angular_velocity = (Vector3){ 1.0f, 2.0f, 3.0f };
    physics_set_6dof_enabled(physics, false);
    TEST_ASSERT_FALSE(physics->has_6dof);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->angular_velocity.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->angular_velocity.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->angular_velocity.z);
}

void test_physics_force_accumulation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Initial force accumulator should be zero
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.z);
    
    // Add multiple forces
    physics_add_force(physics, (Vector3){ 10.0f, 0.0f, 0.0f });
    physics_add_force(physics, (Vector3){ 0.0f, 5.0f, 0.0f });
    physics_add_force(physics, (Vector3){ -3.0f, 2.0f, 7.0f });
    
    // Forces should accumulate
    TEST_ASSERT_EQUAL_FLOAT(7.0f, physics->force_accumulator.x);   // 10 + 0 - 3
    TEST_ASSERT_EQUAL_FLOAT(7.0f, physics->force_accumulator.y);   // 0 + 5 + 2
    TEST_ASSERT_EQUAL_FLOAT(7.0f, physics->force_accumulator.z);   // 0 + 0 + 7
}

void test_physics_torque_accumulation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, true);
    
    // Initial torque accumulator should be zero
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.z);
    
    // Add multiple torques
    physics_add_torque(physics, (Vector3){ 2.0f, 0.0f, 0.0f });
    physics_add_torque(physics, (Vector3){ 0.0f, -1.5f, 0.0f });
    physics_add_torque(physics, (Vector3){ 1.0f, 0.5f, 3.0f });
    
    // Torques should accumulate
    TEST_ASSERT_EQUAL_FLOAT(3.0f, physics->torque_accumulator.x);   // 2 + 0 + 1
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, physics->torque_accumulator.y);  // 0 - 1.5 + 0.5
    TEST_ASSERT_EQUAL_FLOAT(3.0f, physics->torque_accumulator.z);   // 0 + 0 + 3
}

void test_physics_torque_only_when_6dof_enabled(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Ensure 6DOF is disabled
    physics_set_6dof_enabled(physics, false);
    
    // Try to add torque - should be ignored
    physics_add_torque(physics, (Vector3){ 5.0f, 5.0f, 5.0f });
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.z);
}

// ============================================================================
// ANGULAR DYNAMICS TESTS
// ============================================================================

void test_physics_angular_velocity_integration(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, true);
    
    // Set moment of inertia and apply torques to get angular acceleration
    physics->moment_of_inertia = (Vector3){ 1.0f, 1.0f, 1.0f };
    physics->drag_angular = 0.0f;  // No angular drag
    
    printf("DEBUG: Initial angular velocity: [%.3f,%.3f,%.3f]\n", 
           physics->angular_velocity.x, physics->angular_velocity.y, physics->angular_velocity.z);
    printf("DEBUG: Moment of inertia: [%.3f,%.3f,%.3f]\n", 
           physics->moment_of_inertia.x, physics->moment_of_inertia.y, physics->moment_of_inertia.z);
    
    // Apply torques: τ = I·α, so α = τ/I
    // For α = (1.0, 2.0, 0.5), we need τ = (1.0, 2.0, 0.5) with I = (1.0, 1.0, 1.0)
    physics_add_torque(physics, (Vector3){ 1.0f, 2.0f, 0.5f });
    
    printf("DEBUG: After adding torque: [%.3f,%.3f,%.3f]\n", 
           physics->torque_accumulator.x, physics->torque_accumulator.y, physics->torque_accumulator.z);
    
    float delta_time = 0.1f;
    
    // Update physics system
    RenderConfig dummy_config = {0};
    physics_system_update(&test_world, &dummy_config, delta_time);
    
    printf("DEBUG: Final angular velocity: [%.3f,%.3f,%.3f]\n", 
           physics->angular_velocity.x, physics->angular_velocity.y, physics->angular_velocity.z);
    printf("DEBUG: Final angular acceleration: [%.3f,%.3f,%.3f]\n", 
           physics->angular_acceleration.x, physics->angular_acceleration.y, physics->angular_acceleration.z);
    
    // Angular velocity should integrate acceleration: ω = α·dt
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, physics->angular_velocity.x);  // 1.0 * 0.1
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, physics->angular_velocity.y);  // 2.0 * 0.1
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.05f, physics->angular_velocity.z); // 0.5 * 0.1
}

void test_physics_moment_of_inertia_effects(void)
{
    // Create two entities with different moments of inertia
    EntityID entity1 = entity_create(&test_world);
    EntityID entity2 = entity_create(&test_world);
    
    entity_add_component(&test_world, entity1, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity1, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity2, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity2, COMPONENT_TRANSFORM);
    
    struct Physics* physics1 = entity_get_physics(&test_world, entity1);
    struct Physics* physics2 = entity_get_physics(&test_world, entity2);
    
    physics_set_6dof_enabled(physics1, true);
    physics_set_6dof_enabled(physics2, true);
    
    // Set different moments of inertia and disable drag
    physics1->moment_of_inertia = (Vector3){ 1.0f, 1.0f, 1.0f };
    physics2->moment_of_inertia = (Vector3){ 2.0f, 2.0f, 2.0f };
    physics1->drag_angular = 0.0f;
    physics2->drag_angular = 0.0f;
    
    // Apply same torque
    physics_add_torque(physics1, (Vector3){ 2.0f, 0.0f, 0.0f });
    physics_add_torque(physics2, (Vector3){ 2.0f, 0.0f, 0.0f });
    
    RenderConfig dummy_config = {0};
    physics_system_update(&test_world, &dummy_config, 0.1f);
    
    printf("DEBUG: physics1 angular velocity: [%.3f,%.3f,%.3f]\n", 
           physics1->angular_velocity.x, physics1->angular_velocity.y, physics1->angular_velocity.z);
    printf("DEBUG: physics2 angular velocity: [%.3f,%.3f,%.3f]\n", 
           physics2->angular_velocity.x, physics2->angular_velocity.y, physics2->angular_velocity.z);
    
    // Lower moment of inertia should result in higher angular velocity
    // Angular acceleration = torque / moment_of_inertia
    // physics1: 2.0 / 1.0 = 2.0 rad/s² → ω = 2.0 * 0.1 = 0.2 rad/s
    // physics2: 2.0 / 2.0 = 1.0 rad/s² → ω = 1.0 * 0.1 = 0.1 rad/s
    // physics1 should have higher angular velocity than physics2
    TEST_ASSERT_GREATER_THAN_FLOAT(physics2->angular_velocity.x, physics1->angular_velocity.x);
}

// ============================================================================
// FORCE AT POINT TESTS
// ============================================================================

void test_physics_force_at_point_generates_torque(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, true);
    
    Vector3 force = { 0.0f, 10.0f, 0.0f };        // Upward force
    Vector3 application_point = { 1.0f, 0.0f, 0.0f }; // Right of center
    Vector3 center_of_mass = { 0.0f, 0.0f, 0.0f };   // At origin
    
    physics_add_force_at_point(physics, force, application_point, center_of_mass);
    
    // Should add the force
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, physics->force_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.z);
    
    // Should generate torque around Z axis (right-hand rule)
    // offset = (1, 0, 0), force = (0, 10, 0)
    // torque = offset × force = (0, 0, 10)
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, physics->torque_accumulator.z);
}

void test_physics_force_at_point_no_torque_without_6dof(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, false); // Disable 6DOF
    
    Vector3 force = { 0.0f, 10.0f, 0.0f };
    Vector3 application_point = { 1.0f, 0.0f, 0.0f };
    Vector3 center_of_mass = { 0.0f, 0.0f, 0.0f };
    
    physics_add_force_at_point(physics, force, application_point, center_of_mass);
    
    // Should add the force
    TEST_ASSERT_EQUAL_FLOAT(10.0f, physics->force_accumulator.y);
    
    // Should NOT generate torque when 6DOF is disabled
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.z);
}

// ============================================================================
// ENVIRONMENTAL PHYSICS TESTS
// ============================================================================

void test_physics_environmental_contexts(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Test different environment settings
    physics->environment = PHYSICS_SPACE;
    TEST_ASSERT_EQUAL_INT(PHYSICS_SPACE, physics->environment);
    
    physics->environment = PHYSICS_ATMOSPHERE;
    TEST_ASSERT_EQUAL_INT(PHYSICS_ATMOSPHERE, physics->environment);
    
    physics->environment = PHYSICS_ATMOSPHERE;
    TEST_ASSERT_EQUAL_INT(PHYSICS_ATMOSPHERE, physics->environment);
}

void test_physics_angular_drag_effects(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, true);
    
    // Set initial angular velocity
    physics->angular_velocity = (Vector3){ 2.0f, 0.0f, 0.0f };
    
    // Set angular drag
    physics->drag_angular = 0.1f; // 10% drag per frame (90% retention)
    
    Vector3 initial_velocity = physics->angular_velocity;
    
    RenderConfig dummy_config = {0};
    physics_system_update(&test_world, &dummy_config, 1.0f);
    
    // Angular velocity should be reduced by drag
    TEST_ASSERT_GREATER_THAN_FLOAT(initial_velocity.x, physics->angular_velocity.x); // initial > current (drag reduces)
    TEST_ASSERT_GREATER_THAN_FLOAT(physics->angular_velocity.x, 0.0f); // But not zero
}

// ============================================================================
// INTEGRATION AND EDGE CASE TESTS
// ============================================================================

void test_physics_null_pointer_safety(void)
{
    // Test that physics functions handle NULL pointers gracefully
    physics_add_force(NULL, (Vector3){ 1.0f, 1.0f, 1.0f });
    physics_add_torque(NULL, (Vector3){ 1.0f, 1.0f, 1.0f });
    physics_set_6dof_enabled(NULL, true);
    
    // Should not crash (TEST_ASSERT_TRUE just to have an assertion)
    TEST_ASSERT_TRUE(true);
}

void test_physics_large_force_stability(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Set reasonable physics parameters
    physics->mass = 100.0f;  // Avoid very high accelerations
    physics->drag_linear = 0.0f;  // No drag for this test
    
    // Apply very large forces
    physics_add_force(physics, (Vector3){ 1000000.0f, 0.0f, 0.0f });
    physics_add_force(physics, (Vector3){ -999999.0f, 0.0f, 0.0f });
    
    // Net force should be 1.0
    TEST_ASSERT_EQUAL_FLOAT(1.0f, physics->force_accumulator.x);
    
    RenderConfig dummy_config = {0};
    physics_system_update(&test_world, &dummy_config, 0.016f);
    
    // Should not produce NaN or infinite values
    TEST_ASSERT_FALSE(isnan(physics->velocity.x));
    TEST_ASSERT_FALSE(isinf(physics->velocity.x));
}

void test_physics_multiple_entities_6dof_performance(void)
{
    const int entity_count = 50;
    EntityID entities[entity_count];
    
    // Create many 6DOF entities
    for (int i = 0; i < entity_count; i++) {
        entities[i] = entity_create(&test_world);
        entity_add_component(&test_world, entities[i], COMPONENT_PHYSICS);
    entity_add_component(&test_world, entities[i], COMPONENT_TRANSFORM);
        
        struct Physics* physics = entity_get_physics(&test_world, entities[i]);
        physics_set_6dof_enabled(physics, true);
        
        // Add some forces and torques
        physics_add_force(physics, (Vector3){ (float)i, 0.0f, 0.0f });
        physics_add_torque(physics, (Vector3){ 0.0f, (float)i * 0.1f, 0.0f });
    }
    
    // Update should complete in reasonable time
    clock_t start = clock();
    RenderConfig dummy_config = {0};
    physics_system_update(&test_world, &dummy_config, 0.016f);
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete within 10ms for 50 entities
    TEST_ASSERT_LESS_THAN(elapsed, 0.01);
    
    // Verify all entities were processed
    for (int i = 0; i < entity_count; i++) {
        struct Physics* physics = entity_get_physics(&test_world, entities[i]);
        TEST_ASSERT_NOT_EQUAL_FLOAT(0.0f, physics->velocity.x); // Should have moved
    }
}

// ============================================================================
// CRITICAL BUG ISOLATION TESTS
// ============================================================================

void test_physics_velocity_integration_basic(void)
{
    printf("🔍 Testing basic velocity integration with known values...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set up known test conditions
    physics->mass = 100.0f;  // 100kg mass
    physics->drag_linear = 0.0f;  // NO drag to isolate issue
    physics->kinematic = false;
    physics->has_6dof = true;
    
    // Clear initial state
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics->acceleration = (Vector3){0.0f, 0.0f, 0.0f};
    physics->force_accumulator = (Vector3){0.0f, 0.0f, 0.0f};
    
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    printf("Initial state: velocity=[%.3f,%.3f,%.3f] position=[%.3f,%.3f,%.3f]\n",
           physics->velocity.x, physics->velocity.y, physics->velocity.z,
           transform->position.x, transform->position.y, transform->position.z);
    
    // Apply a known force: 1000N forward
    physics_add_force(physics, (Vector3){1000.0f, 0.0f, 0.0f});
    
    printf("Applied 1000N force. Force accumulator=[%.1f,%.1f,%.1f]\n",
           physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z);
    
    // Expected: 1000N / 100kg = 10 m/s² acceleration
    // With dt=0.016s (60 FPS): velocity change = 10 * 0.016 = 0.16 m/s
    float delta_time = 0.016f;  // 60 FPS
    
    // Create a minimal render config for the physics update
    RenderConfig dummy_render_config = {0};
    
    // Run one physics update
    physics_system_update(&test_world, &dummy_render_config, delta_time);
    
    printf("After 1 update (dt=%.3f): velocity=[%.3f,%.3f,%.3f] position=[%.3f,%.3f,%.3f]\n",
           delta_time,
           physics->velocity.x, physics->velocity.y, physics->velocity.z,
           transform->position.x, transform->position.y, transform->position.z);
    
    // Test that velocity changed
    TEST_ASSERT_GREATER_THAN_FLOAT(0.1f, physics->velocity.x);  // Should be ~0.16 m/s
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.y);  // No Y force applied
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.z);  // No Z force applied
    
    // Test that position changed (velocity * dt)
    TEST_ASSERT_GREATER_THAN_FLOAT(0.001f, transform->position.x);  // Should be ~0.0026m
    
    printf("✅ Basic integration test passed - velocity accumulation working\n");
}

void test_physics_force_accumulator_timing(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    physics->mass = 100.0f;
    physics->drag_linear = 0.0f;  // No drag
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

void test_physics_multiple_force_accumulation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    physics->mass = 100.0f;
    physics->drag_linear = 0.0f;  // No drag
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    
    // Apply multiple forces in the same frame
    physics_add_force(physics, (Vector3){500.0f, 0.0f, 0.0f});
    physics_add_force(physics, (Vector3){300.0f, 200.0f, 0.0f});
    physics_add_force(physics, (Vector3){200.0f, -200.0f, 100.0f});
    
    // Total force should be [1000, 0, 100]
    TEST_ASSERT_EQUAL_FLOAT(1000.0f, physics->force_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, physics->force_accumulator.z);
    
    // Run physics update
    RenderConfig dummy_render_config = {0};
    physics_system_update(&test_world, &dummy_render_config, 0.016f);
    
    // Check resulting velocity (F/m * dt)
    // X: 1000N / 100kg * 0.016s = 0.16 m/s
    // Y: 0N / 100kg * 0.016s = 0.0 m/s  
    // Z: 100N / 100kg * 0.016s = 0.016 m/s
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.16f, physics->velocity.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->velocity.y);
    TEST_ASSERT_FLOAT_WITHIN(0.005f, 0.016f, physics->velocity.z);
}

void test_physics_zero_mass_safety(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Test with zero mass (should be handled safely)
    physics->mass = 0.0f;
    physics->drag_linear = 1.0f;
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    
    physics_add_force(physics, (Vector3){1000.0f, 0.0f, 0.0f});
    
    RenderConfig dummy_render_config = {0};
    physics_system_update(&test_world, &dummy_render_config, 0.016f);
    
    // With zero mass, velocity should remain zero (no infinite acceleration)
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.z);
}

// ============================================================================
// ADDITIONAL CRITICAL TESTS FOR SPRINT 21
// ============================================================================

/**
 * @brief Critical Test: Consecutive Frame Integration
 * 
 * Tests velocity accumulation across multiple frames to ensure
 * integration works consistently over time.
 */
void test_physics_consecutive_frame_integration(void)
{
    printf("🔍 Testing consecutive frame integration...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    physics->mass = 100.0f;
    physics->drag_linear = 0.0f;  // No drag
    physics->kinematic = false;
    physics->has_6dof = true;
    
    // Clear initial state
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    float delta_time = 0.016f;
    RenderConfig dummy_render_config = {0};
    
    // Apply constant force for 5 frames
    for (int frame = 0; frame < 5; frame++) {
        physics_add_force(physics, (Vector3){100.0f, 0.0f, 0.0f});
        physics_system_update(&test_world, &dummy_render_config, delta_time);
        
        printf("Frame %d: vel=[%.3f,%.3f,%.3f] pos=[%.6f,%.6f,%.6f]\n", 
               frame + 1,
               physics->velocity.x, physics->velocity.y, physics->velocity.z,
               transform->position.x, transform->position.y, transform->position.z);
    }
    
    // Expected velocity after 5 frames: 5 * (100N/100kg * 0.016s) = 5 * 0.016 = 0.08 m/s
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.08f, physics->velocity.x);
    
    // Position should show accumulated movement
    TEST_ASSERT_GREATER_THAN_FLOAT(0.001f, transform->position.x);
    
    printf("✅ Consecutive frame integration working correctly\n");
}

/**
 * @brief Critical Test: High Frequency Updates
 * 
 * Tests physics at different timestep frequencies to ensure
 * numerical stability and consistency.
 */
void test_physics_high_frequency_updates(void)
{
    printf("🔍 Testing high frequency physics updates...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    physics->mass = 100.0f;
    physics->drag_linear = 0.0f;  // No drag
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    RenderConfig dummy_render_config = {0};
    
    // Apply force and run many small timesteps (simulate 120 FPS vs 60 FPS)
    float total_time = 0.1f;  // 100ms total
    float small_dt = 0.008333f;  // 120 FPS timestep
    int steps = (int)(total_time / small_dt);
    
    for (int i = 0; i < steps; i++) {
        physics_add_force(physics, (Vector3){1000.0f, 0.0f, 0.0f});
        physics_system_update(&test_world, &dummy_render_config, small_dt);
    }
    
    float final_velocity = physics->velocity.x;
    float final_position = transform->position.x;
    
    // Reset and test with larger timesteps (60 FPS)
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    float large_dt = 0.016667f;  // 60 FPS timestep
    int large_steps = (int)(total_time / large_dt);
    
    for (int i = 0; i < large_steps; i++) {
        physics_add_force(physics, (Vector3){1000.0f, 0.0f, 0.0f});
        physics_system_update(&test_world, &dummy_render_config, large_dt);
    }
    
    // Results should be similar (within 20% tolerance for numerical integration differences)
    float velocity_tolerance = 0.2f * fabs(final_velocity);
    float position_tolerance = 0.2f * fabs(final_position);
    
    TEST_ASSERT_FLOAT_WITHIN(velocity_tolerance, final_velocity, physics->velocity.x);
    TEST_ASSERT_FLOAT_WITHIN(position_tolerance, final_position, transform->position.x);
    
    printf("✅ High frequency update stability verified\n");
}

/**
 * @brief Critical Test: Component State Persistence
 * 
 * Tests that physics component state persists correctly
 * across multiple updates without corruption.
 */
void test_physics_component_state_persistence(void)
{
    printf("🔍 Testing component state persistence...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Set specific state values
    physics->mass = 42.5f;
    physics->drag_linear = 0.95f;
    physics->drag_angular = 0.88f;
    physics->has_6dof = true;
    physics->kinematic = false;
    physics->environment = PHYSICS_SPACE;
    physics->moment_of_inertia = (Vector3){2.5f, 1.8f, 3.2f};
    
    RenderConfig dummy_render_config = {0};
    
    // Run multiple physics updates
    for (int i = 0; i < 10; i++) {
        physics_add_force(physics, (Vector3){10.0f, 5.0f, -2.0f});
        physics_system_update(&test_world, &dummy_render_config, 0.016f);
        
        // Verify state values haven't been corrupted
        TEST_ASSERT_EQUAL_FLOAT(42.5f, physics->mass);
        TEST_ASSERT_EQUAL_FLOAT(0.95f, physics->drag_linear);
        TEST_ASSERT_EQUAL_FLOAT(0.88f, physics->drag_angular);
        TEST_ASSERT_TRUE(physics->has_6dof);
        TEST_ASSERT_FALSE(physics->kinematic);
        TEST_ASSERT_EQUAL_INT(PHYSICS_SPACE, physics->environment);
        TEST_ASSERT_EQUAL_FLOAT(2.5f, physics->moment_of_inertia.x);
        TEST_ASSERT_EQUAL_FLOAT(1.8f, physics->moment_of_inertia.y);
        TEST_ASSERT_EQUAL_FLOAT(3.2f, physics->moment_of_inertia.z);
    }
    
    printf("✅ Component state persistence verified\n");
}

/**
 * @brief Critical Test: Entity Component Pointer Stability
 * 
 * Tests that entity component pointers remain valid and
 * don't get corrupted during physics operations.
 */
void test_physics_entity_pointer_stability(void)
{
    printf("🔍 Testing entity component pointer stability...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    // Get initial pointers
    struct Physics* initial_physics = entity_get_physics(&test_world, entity);
    struct Transform* initial_transform = entity_get_transform(&test_world, entity);
    
    TEST_ASSERT_NOT_NULL(initial_physics);
    TEST_ASSERT_NOT_NULL(initial_transform);
    
    // Store original values
    uintptr_t initial_physics_addr = (uintptr_t)initial_physics;
    uintptr_t initial_transform_addr = (uintptr_t)initial_transform;
    
    RenderConfig dummy_render_config = {0};
    
    // Run physics updates and verify pointers don't change
    for (int i = 0; i < 50; i++) {
        physics_add_force(initial_physics, (Vector3){100.0f, 0.0f, 0.0f});
        physics_system_update(&test_world, &dummy_render_config, 0.016f);
        
        // Re-get pointers and verify they're the same
        struct Physics* current_physics = entity_get_physics(&test_world, entity);
        struct Transform* current_transform = entity_get_transform(&test_world, entity);
        
        TEST_ASSERT_EQUAL_PTR(initial_physics, current_physics);
        TEST_ASSERT_EQUAL_PTR(initial_transform, current_transform);
        
        // Verify addresses haven't changed
        TEST_ASSERT_EQUAL_UINT64(initial_physics_addr, (uintptr_t)current_physics);
        TEST_ASSERT_EQUAL_UINT64(initial_transform_addr, (uintptr_t)current_transform);
    }
    
    printf("✅ Entity component pointer stability verified\n");
}

/**
 * @brief Critical Test: Drag Effect Precision
 * 
 * Tests that drag calculations work precisely and
 * don't introduce numerical errors that could cause
 * velocity to become zero prematurely.
 */
void test_physics_drag_precision(void)
{
    printf("🔍 Testing drag effect precision...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 100.0f;
    physics->drag_linear = 0.01f;  // 1% drag per frame (99% retention)
    
    // Set initial velocity
    physics->velocity = (Vector3){10.0f, 0.0f, 0.0f};
    
    RenderConfig dummy_render_config = {0};
    
    // Run many frames to see drag effect
    float initial_velocity = physics->velocity.x;
    
    for (int frame = 0; frame < 100; frame++) {
        physics_system_update(&test_world, &dummy_render_config, 0.016f);
        
        // Velocity should decrease but never become exactly zero due to drag
        TEST_ASSERT_GREATER_THAN_FLOAT(physics->velocity.x, 0.0f);
        
        // Should follow exponential decay: v(t) = v0 * (1-drag)^t
        if (frame == 50) {
            float expected_velocity = initial_velocity * pow(0.99f, 51.0f);  // (1-0.01)^51
            TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_velocity, physics->velocity.x);
        }
    }
    
    printf("Final velocity after 100 frames: %.6f m/s\n", physics->velocity.x);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.001f, physics->velocity.x);  // Should still be moving
    
    printf("✅ Drag precision verified\n");
}

/**
 * @brief Critical Test: Zero Velocity Edge Case
 * 
 * Tests the specific case where velocity might be getting
 * incorrectly zeroed out in certain conditions.
 */
void test_physics_zero_velocity_edge_case(void)
{
    printf("🔍 Testing zero velocity edge case...\n");
    
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    physics->mass = 80.0f;  // Same as in Sprint 21 report
    physics->drag_linear = 0.9999f;  // Very high drag (Sprint 21 condition)
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    RenderConfig dummy_render_config = {0};
    
    // Apply exactly the force from Sprint 21 report
    physics_add_force(physics, (Vector3){35000.0f, -240.0f, 0.0f});
    
    printf("Sprint 21 reproduction test:\n");
    printf("Mass: %.1f kg, Drag: %.4f\n", physics->mass, physics->drag_linear);
    printf("Applied force: [%.0f, %.0f, %.0f] N\n", 
           physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z);
    
    // Expected acceleration: F/m = 35000N/80kg = 437.5 m/s²
    float expected_accel = 35000.0f / 80.0f;
    printf("Expected acceleration: %.2f m/s²\n", expected_accel);
    
    physics_system_update(&test_world, &dummy_render_config, 0.016f);
    
    printf("Actual velocity: [%.6f, %.6f, %.6f] m/s\n", 
           physics->velocity.x, physics->velocity.y, physics->velocity.z);
    
    // With dt=0.016s: expected velocity = 437.5 * 0.016 = 7.0 m/s (before drag)
    // After drag: 7.0 * 0.9999 = 6.9993 m/s
    float expected_velocity = expected_accel * 0.016f * physics->drag_linear;
    printf("Expected velocity (after drag): %.4f m/s\n", expected_velocity);
    
    // Critical test: velocity should NOT be zero!
    TEST_ASSERT_GREATER_THAN_FLOAT(6.0f, physics->velocity.x);
    TEST_ASSERT_NOT_EQUAL_FLOAT(0.0f, physics->velocity.x);
    
    printf("✅ Sprint 21 conditions reproduced - velocity integration working\n");
}

/**
 * @brief Critical Test: System Update Order Independence
 * 
 * Tests that physics results are consistent regardless
 * of when physics_system_update is called.
 */
void test_physics_update_order_independence(void)
{
    printf("🔍 Testing physics update order independence...\n");
    
    // Create two identical entities
    EntityID entity1 = entity_create(&test_world);
    EntityID entity2 = entity_create(&test_world);
    
    entity_add_component(&test_world, entity1, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity1, COMPONENT_PHYSICS);
    entity_add_component(&test_world, entity2, COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity2, COMPONENT_PHYSICS);
    
    struct Physics* physics1 = entity_get_physics(&test_world, entity1);
    struct Physics* physics2 = entity_get_physics(&test_world, entity2);
    
    // Set identical initial conditions
    physics1->mass = 100.0f;
    physics2->mass = 100.0f;
    physics1->drag_linear = 0.95f;
    physics2->drag_linear = 0.95f;
    physics1->velocity = (Vector3){1.0f, 2.0f, 3.0f};
    physics2->velocity = (Vector3){1.0f, 2.0f, 3.0f};
    
    RenderConfig dummy_render_config = {0};
    
    // Apply identical forces
    physics_add_force(physics1, (Vector3){500.0f, -100.0f, 200.0f});
    physics_add_force(physics2, (Vector3){500.0f, -100.0f, 200.0f});
    
    // Update physics system (both entities updated together)
    physics_system_update(&test_world, &dummy_render_config, 0.016f);
    
    // Results should be identical
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, physics1->velocity.x, physics2->velocity.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, physics1->velocity.y, physics2->velocity.y);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, physics1->velocity.z, physics2->velocity.z);
    
    printf("✅ Physics update order independence verified\n");
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_physics_6dof(void)
{
    printf("\n🚀 6DOF Physics System Tests\n");
    printf("=============================\n");
    
    printf("🔧 Testing 6DOF Core Features...\n");
    RUN_TEST(test_physics_6dof_enabled_flag);
    RUN_TEST(test_physics_force_accumulation);
    RUN_TEST(test_physics_torque_accumulation);
    RUN_TEST(test_physics_torque_only_when_6dof_enabled);
    
    printf("🌀 Testing Angular Dynamics...\n");
    RUN_TEST(test_physics_angular_velocity_integration);
    RUN_TEST(test_physics_moment_of_inertia_effects);
    
    printf("📍 Testing Force at Point...\n");
    RUN_TEST(test_physics_force_at_point_generates_torque);
    RUN_TEST(test_physics_force_at_point_no_torque_without_6dof);
    
    printf("🌍 Testing Environmental Physics...\n");
    RUN_TEST(test_physics_environmental_contexts);
    RUN_TEST(test_physics_angular_drag_effects);
    
    printf("🛡️  Testing Edge Cases...\n");
    RUN_TEST(test_physics_null_pointer_safety);
    RUN_TEST(test_physics_large_force_stability);
    RUN_TEST(test_physics_multiple_entities_6dof_performance);
    
    printf("🔧 Testing Critical Integration Bug...\n");
    RUN_TEST(test_physics_velocity_integration_basic);
    RUN_TEST(test_physics_force_accumulator_timing);
    RUN_TEST(test_physics_multiple_force_accumulation);
    RUN_TEST(test_physics_zero_mass_safety);
    
    printf("🧪 Testing Advanced Critical Scenarios...\n");
    RUN_TEST(test_physics_consecutive_frame_integration);
    RUN_TEST(test_physics_high_frequency_updates);
    RUN_TEST(test_physics_component_state_persistence);
    RUN_TEST(test_physics_entity_pointer_stability);
    RUN_TEST(test_physics_drag_precision);
    RUN_TEST(test_physics_zero_velocity_edge_case);
    RUN_TEST(test_physics_update_order_independence);
    
    printf("✅ 6DOF Physics Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    suite_physics_6dof();
    return UNITY_END();
}
