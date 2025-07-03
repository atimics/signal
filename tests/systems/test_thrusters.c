/**
 * @file test_thrusters.c
 * @brief Comprehensive tests for thruster system
 * 
 * Tests the ThrusterSystem component and thruster force application.
 * Critical for Sprint 21 entity-agnostic flight mechanics.
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/system/thrusters.h"
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
    // Initialize test world
    memset(&test_world, 0, sizeof(struct World));
    test_world.max_entities = 100;
    test_world.entities = malloc(sizeof(struct Entity) * 100);
    test_world.entity_count = 0;
    test_world.next_entity_id = 1;
}

void tearDown(void)
{
    if (test_world.entities) {
        free(test_world.entities);
        test_world.entities = NULL;
    }
}

// ============================================================================
// THRUSTER COMPONENT TESTS
// ============================================================================

void test_thruster_component_creation(void)
{
    EntityID entity = entity_create(&test_world);
    
    // Add thruster component
    bool success = entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM);
    TEST_ASSERT_TRUE(success);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    TEST_ASSERT_NOT_NULL(thrusters);
    
    // Check default values
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_linear_force.x);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_linear_force.y);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_linear_force.z);
    
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_angular_torque.x);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_angular_torque.y);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_angular_torque.z);
    
    TEST_ASSERT_TRUE(thrusters->thrusters_enabled);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->atmosphere_efficiency);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->vacuum_efficiency);
}

void test_thruster_command_setting(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Set linear thrust commands
    Vector3 linear_command = { 0.5f, -0.3f, 1.0f };
    thruster_set_linear_command(thrusters, linear_command);
    
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_linear_thrust.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.3f, thrusters->current_linear_thrust.y);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, thrusters->current_linear_thrust.z);
    
    // Set angular thrust commands
    Vector3 angular_command = { -0.7f, 0.2f, 0.8f };
    thruster_set_angular_command(thrusters, angular_command);
    
    TEST_ASSERT_EQUAL_FLOAT(-0.7f, thrusters->current_angular_thrust.x);
    TEST_ASSERT_EQUAL_FLOAT(0.2f, thrusters->current_angular_thrust.y);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, thrusters->current_angular_thrust.z);
}

void test_thruster_command_clamping(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Test commands beyond [-1, 1] range get clamped
    thruster_set_linear_command(thrusters, (Vector3){ 2.0f, -1.5f, 0.5f });
    
    TEST_ASSERT_EQUAL_FLOAT(1.0f, thrusters->current_linear_thrust.x);   // Clamped to 1.0
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, thrusters->current_linear_thrust.y);  // Clamped to -1.0
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_linear_thrust.z);   // Within range
    
    thruster_set_angular_command(thrusters, (Vector3){ -3.0f, 0.0f, 1.2f });
    
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, thrusters->current_angular_thrust.x); // Clamped to -1.0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, thrusters->current_angular_thrust.y);  // Within range
    TEST_ASSERT_EQUAL_FLOAT(1.0f, thrusters->current_angular_thrust.z);  // Clamped to 1.0
}

// ============================================================================
// THRUSTER FORCE GENERATION TESTS
// ============================================================================

void test_thruster_linear_force_generation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Set thruster capabilities
    thrusters->max_linear_force = (Vector3){ 100.0f, 80.0f, 120.0f };
    
    // Set thrust command
    thruster_set_linear_command(thrusters, (Vector3){ 0.5f, -0.25f, 1.0f });
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Check that correct forces were applied
    // Expected force = max_force * thrust_command * efficiency
    float expected_x = 100.0f * 0.5f * thrusters->vacuum_efficiency;
    float expected_y = 80.0f * -0.25f * thrusters->vacuum_efficiency;
    float expected_z = 120.0f * 1.0f * thrusters->vacuum_efficiency;
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_x, physics->force_accumulator.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_y, physics->force_accumulator.y);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_z, physics->force_accumulator.z);
}

void test_thruster_angular_force_generation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Enable 6DOF for torque
    physics_set_6dof_enabled(physics, true);
    
    // Set thruster capabilities
    thrusters->max_angular_torque = (Vector3){ 50.0f, 40.0f, 60.0f };
    
    // Set angular thrust command
    thruster_set_angular_command(thrusters, (Vector3){ 0.8f, -0.6f, 0.2f });
    
    // Clear any existing torques
    physics->torque_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Check that correct torques were applied
    float expected_x = 50.0f * 0.8f * thrusters->vacuum_efficiency;
    float expected_y = 40.0f * -0.6f * thrusters->vacuum_efficiency;
    float expected_z = 60.0f * 0.2f * thrusters->vacuum_efficiency;
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_x, physics->torque_accumulator.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_y, physics->torque_accumulator.y);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_z, physics->torque_accumulator.z);
}

void test_thruster_disabled_no_force(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Disable thrusters
    thrusters->thrusters_enabled = false;
    
    // Set thrust commands
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 1.0f, 1.0f });
    thruster_set_angular_command(thrusters, (Vector3){ 1.0f, 1.0f, 1.0f });
    
    // Clear forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    physics->torque_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // No forces should be applied when thrusters are disabled
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.z);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.z);
}

// ============================================================================
// ENVIRONMENTAL EFFICIENCY TESTS
// ============================================================================

void test_thruster_atmosphere_efficiency(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Set different efficiency for atmosphere
    thrusters->atmosphere_efficiency = 0.7f;
    thrusters->vacuum_efficiency = 1.0f;
    
    // Set environment to atmosphere
    physics->environment = PHYSICS_ATMOSPHERE;
    
    thrusters->max_linear_force = (Vector3){ 100.0f, 100.0f, 100.0f };
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 0.0f, 0.0f });
    
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Force should be reduced by atmosphere efficiency
    float expected_force = 100.0f * 1.0f * 0.7f; // max * command * atmosphere_efficiency
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_force, physics->force_accumulator.x);
}

void test_thruster_vacuum_efficiency(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Set environment to space
    physics->environment = PHYSICS_SPACE;
    
    thrusters->max_linear_force = (Vector3){ 100.0f, 100.0f, 100.0f };
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 0.0f, 0.0f });
    
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Force should use vacuum efficiency (typically 1.0)
    float expected_force = 100.0f * 1.0f * thrusters->vacuum_efficiency;
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_force, physics->force_accumulator.x);
}

// ============================================================================
// THRUSTER RESPONSE TIME TESTS
// ============================================================================

void test_thruster_response_time_gradual_change(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Set slow response time
    thrusters->thrust_response_time = 1.0f; // 1 second to reach target
    
    // Start with zero thrust
    thrusters->current_linear_thrust = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Command full thrust
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 0.0f, 0.0f });
    
    // Update with small time step
    float delta_time = 0.1f; // 100ms
    thruster_system_update(&test_world, NULL, delta_time);
    
    // Thrust should not have reached full value immediately
    TEST_ASSERT_LESS_THAN(1.0f, thrusters->current_linear_thrust.x);
    TEST_ASSERT_GREATER_THAN(0.0f, thrusters->current_linear_thrust.x);
}

void test_thruster_instant_response_when_fast(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Set very fast response time
    thrusters->thrust_response_time = 0.01f; // 10ms to reach target
    
    // Start with zero thrust
    thrusters->current_linear_thrust = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Command thrust
    thruster_set_linear_command(thrusters, (Vector3){ 0.5f, 0.0f, 0.0f });
    
    // Update with larger time step
    float delta_time = 0.016f; // 16ms (typical frame time)
    thruster_system_update(&test_world, NULL, delta_time);
    
    // Thrust should have reached target value quickly
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, thrusters->current_linear_thrust.x);
}

// ============================================================================
// INTEGRATION AND EDGE CASE TESTS
// ============================================================================

void test_thruster_null_pointer_safety(void)
{
    // Test that thruster functions handle NULL pointers gracefully
    thruster_set_linear_command(NULL, (Vector3){ 1.0f, 1.0f, 1.0f });
    thruster_set_angular_command(NULL, (Vector3){ 1.0f, 1.0f, 1.0f });
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

void test_thruster_entity_without_physics(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM); // No physics component
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 1.0f, 1.0f });
    
    // Should not crash when updating thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// THRUST DIRECTION TRANSFORMATION TESTS
// ============================================================================

void test_thruster_direction_identity_rotation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set identity rotation (no rotation)
    transform->rotation = (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f };
    
    // Enable thrusters and set forward thrust
    thrusters->thrusters_enabled = true;
    thruster_set_linear_command(thrusters, (Vector3){ 0.0f, 0.0f, 1.0f }); // Forward thrust
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // With identity rotation, force should still be in Z direction
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->force_accumulator.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->force_accumulator.y);
    TEST_ASSERT_TRUE(physics->force_accumulator.z > 0.0f);
}

void test_thruster_direction_90_degree_yaw(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set 90-degree rotation around Y axis (yaw right)
    // sin(45°) = 0.707, cos(45°) = 0.707 for quaternion
    transform->rotation = (Quaternion){ 0.0f, 0.707f, 0.0f, 0.707f };
    
    // Enable thrusters and set forward thrust
    thrusters->thrusters_enabled = true;
    thruster_set_linear_command(thrusters, (Vector3){ 0.0f, 0.0f, 1.0f }); // Forward thrust
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // After 90-degree yaw, forward thrust should point along positive X
    TEST_ASSERT_TRUE(physics->force_accumulator.x > 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->force_accumulator.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->force_accumulator.z);
}

void test_thruster_direction_90_degree_pitch(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set 90-degree rotation around X axis (pitch up)
    transform->rotation = (Quaternion){ 0.707f, 0.0f, 0.0f, 0.707f };
    
    // Enable thrusters and set forward thrust
    thrusters->thrusters_enabled = true;
    thruster_set_linear_command(thrusters, (Vector3){ 0.0f, 0.0f, 1.0f }); // Forward thrust
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // After 90-degree pitch up, forward thrust should point along negative Y
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->force_accumulator.x);
    TEST_ASSERT_TRUE(physics->force_accumulator.y < -0.01f); // Pointing down
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, physics->force_accumulator.z);
}

void test_thruster_direction_combined_rotation(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set arbitrary rotation (normalized quaternion)
    transform->rotation = (Quaternion){ 0.2f, 0.3f, 0.1f, 0.924f }; // Arbitrary but normalized
    
    // Enable thrusters and set thrust in multiple directions
    thrusters->thrusters_enabled = true;
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 0.5f, 2.0f });
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Force should be non-zero and transformed
    float force_magnitude = sqrtf(
        physics->force_accumulator.x * physics->force_accumulator.x +
        physics->force_accumulator.y * physics->force_accumulator.y +
        physics->force_accumulator.z * physics->force_accumulator.z
    );
    TEST_ASSERT_TRUE(force_magnitude > 0.0f);
}

void test_thruster_direction_zero_thrust(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set arbitrary rotation
    transform->rotation = (Quaternion){ 0.2f, 0.3f, 0.1f, 0.924f };
    
    // Enable thrusters but set zero thrust
    thrusters->thrusters_enabled = true;
    thruster_set_linear_command(thrusters, (Vector3){ 0.0f, 0.0f, 0.0f });
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update thruster system
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Zero thrust should remain zero after transformation
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, physics->force_accumulator.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, physics->force_accumulator.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, physics->force_accumulator.z);
}

void test_thruster_multiple_entities_performance(void)
{
    const int entity_count = 30;
    EntityID entities[entity_count];
    
    // Create many entities with thrusters
    for (int i = 0; i < entity_count; i++) {
        entities[i] = entity_create(&test_world);
        entity_add_component(&test_world, entities[i], 
                           COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
        
        struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entities[i]);
        
        // Set different thrust commands for variety
        float thrust = (float)i / entity_count;
        thruster_set_linear_command(thrusters, (Vector3){ thrust, 0.0f, 0.0f });
        thruster_set_angular_command(thrusters, (Vector3){ 0.0f, thrust, 0.0f });
    }
    
    // Update should complete in reasonable time
    clock_t start = clock();
    thruster_system_update(&test_world, NULL, 0.016f);
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete within 5ms for 30 entities
    TEST_ASSERT_LESS_THAN(0.005, elapsed);
}

void test_thruster_zero_max_force_safety(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Set zero max forces (disabled thrusters)
    thrusters->max_linear_force = (Vector3){ 0.0f, 0.0f, 0.0f };
    thrusters->max_angular_torque = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    thruster_set_linear_command(thrusters, (Vector3){ 1.0f, 1.0f, 1.0f });
    thruster_set_angular_command(thrusters, (Vector3){ 1.0f, 1.0f, 1.0f });
    
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    physics->torque_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // No forces should be generated
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->force_accumulator.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->torque_accumulator.x);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_thrusters(void)
{
    printf("\n🚀 Thruster System Tests\n");
    printf("========================\n");
    
    printf("🔧 Testing Thruster Component...\n");
    RUN_TEST(test_thruster_component_creation);
    RUN_TEST(test_thruster_command_setting);
    RUN_TEST(test_thruster_command_clamping);
    
    printf("⚡ Testing Force Generation...\n");
    RUN_TEST(test_thruster_linear_force_generation);
    RUN_TEST(test_thruster_angular_force_generation);
    RUN_TEST(test_thruster_disabled_no_force);
    
    printf("🌍 Testing Environmental Efficiency...\n");
    RUN_TEST(test_thruster_atmosphere_efficiency);
    RUN_TEST(test_thruster_vacuum_efficiency);
    
    printf("⏱️  Testing Response Time...\n");
    RUN_TEST(test_thruster_response_time_gradual_change);
    RUN_TEST(test_thruster_instant_response_when_fast);
    
    printf("🛡️  Testing Edge Cases...\n");
    RUN_TEST(test_thruster_null_pointer_safety);
    RUN_TEST(test_thruster_entity_without_physics);
    RUN_TEST(test_thruster_multiple_entities_performance);
    RUN_TEST(test_thruster_zero_max_force_safety);
    
    printf("🧭 Testing Thrust Direction Transformation...\n");
    RUN_TEST(test_thruster_direction_identity_rotation);
    RUN_TEST(test_thruster_direction_90_degree_yaw);
    RUN_TEST(test_thruster_direction_90_degree_pitch);
    RUN_TEST(test_thruster_direction_combined_rotation);
    RUN_TEST(test_thruster_direction_zero_thrust);
    
    printf("✅ Thruster System Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    suite_thrusters();
    return UNITY_END();
}
