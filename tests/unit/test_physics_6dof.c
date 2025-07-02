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
// 6DOF PHYSICS CORE TESTS
// ============================================================================

void test_physics_6dof_enabled_flag(void)
{
    // Create physics entity
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, true);
    
    // Set angular acceleration
    physics->angular_acceleration = (Vector3){ 1.0f, 2.0f, 0.5f };
    
    float delta_time = 0.1f;
    
    // Update physics system
    physics_system_update(&test_world, NULL, delta_time);
    
    // Angular velocity should integrate acceleration
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, physics->angular_velocity.x);  // 1.0 * 0.1
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, physics->angular_velocity.y);  // 2.0 * 0.1
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.05f, physics->angular_velocity.z); // 0.5 * 0.1
}

void test_physics_moment_of_inertia_effects(void)
{
    // Create two entities with different moments of inertia
    EntityID entity1 = entity_create(&test_world);
    EntityID entity2 = entity_create(&test_world);
    
    entity_add_component(&test_world, entity1, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    entity_add_component(&test_world, entity2, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics1 = entity_get_physics(&test_world, entity1);
    struct Physics* physics2 = entity_get_physics(&test_world, entity2);
    
    physics_set_6dof_enabled(physics1, true);
    physics_set_6dof_enabled(physics2, true);
    
    // Set different moments of inertia
    physics1->moment_of_inertia = (Vector3){ 1.0f, 1.0f, 1.0f };
    physics2->moment_of_inertia = (Vector3){ 2.0f, 2.0f, 2.0f };
    
    // Apply same torque
    physics_add_torque(physics1, (Vector3){ 2.0f, 0.0f, 0.0f });
    physics_add_torque(physics2, (Vector3){ 2.0f, 0.0f, 0.0f });
    
    physics_system_update(&test_world, NULL, 0.1f);
    
    // Lower moment of inertia should result in higher angular acceleration
    // Angular acceleration = torque / moment_of_inertia
    TEST_ASSERT_GREATER_THAN(physics2->angular_velocity.x, physics1->angular_velocity.x);
}

// ============================================================================
// FORCE AT POINT TESTS
// ============================================================================

void test_physics_force_at_point_generates_torque(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics_set_6dof_enabled(physics, true);
    
    // Set initial angular velocity
    physics->angular_velocity = (Vector3){ 2.0f, 0.0f, 0.0f };
    
    // Set angular drag
    physics->drag_angular = 0.9f; // 10% drag per frame
    
    Vector3 initial_velocity = physics->angular_velocity;
    
    physics_system_update(&test_world, NULL, 1.0f);
    
    // Angular velocity should be reduced by drag
    TEST_ASSERT_LESS_THAN(initial_velocity.x, physics->angular_velocity.x);
    TEST_ASSERT_GREATER_THAN(0.0f, physics->angular_velocity.x); // But not zero
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
    entity_add_component(&test_world, entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    
    // Apply very large forces
    physics_add_force(physics, (Vector3){ 1000000.0f, 0.0f, 0.0f });
    physics_add_force(physics, (Vector3){ -999999.0f, 0.0f, 0.0f });
    
    // Net force should be 1.0
    TEST_ASSERT_EQUAL_FLOAT(1.0f, physics->force_accumulator.x);
    
    physics_system_update(&test_world, NULL, 0.016f);
    
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
        entity_add_component(&test_world, entities[i], COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
        
        struct Physics* physics = entity_get_physics(&test_world, entities[i]);
        physics_set_6dof_enabled(physics, true);
        
        // Add some forces and torques
        physics_add_force(physics, (Vector3){ (float)i, 0.0f, 0.0f });
        physics_add_torque(physics, (Vector3){ 0.0f, (float)i * 0.1f, 0.0f });
    }
    
    // Update should complete in reasonable time
    clock_t start = clock();
    physics_system_update(&test_world, NULL, 0.016f);
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete within 10ms for 50 entities
    TEST_ASSERT_LESS_THAN(0.01, elapsed);
    
    // Verify all entities were processed
    for (int i = 0; i < entity_count; i++) {
        struct Physics* physics = entity_get_physics(&test_world, entities[i]);
        TEST_ASSERT_NOT_EQUAL_FLOAT(0.0f, physics->velocity.x); // Should have moved
    }
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
