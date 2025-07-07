/**
 * @file test_sprint_21_velocity_bug.c
 * @brief Regression test for Sprint 21 velocity integration bug
 * 
 * This test ensures that the velocity accumulation bug identified in Sprint 21
 * does not reoccur. The bug involved velocity values accumulating incorrectly
 * over multiple physics update cycles.
 * 
 * Bug Description:
 * - Physics system was not properly resetting force accumulators
 * - Velocity would accumulate across frames
 * - Objects would accelerate uncontrollably
 * 
 * Expected Behavior:
 * - Forces applied once should only affect one physics cycle
 * - Velocity should remain constant in absence of forces
 * - Force accumulator should reset after each physics update
 */

#include "../support/test_utilities.h"
#include "../../src/core.h"
#include "../../src/system/physics.h"

void setUp(void) {
    test_world_setup();
}

void tearDown(void) {
    test_world_teardown();
}

// ============================================================================
// REGRESSION TEST: Sprint 21 Velocity Bug
// ============================================================================

void test_sprint_21_velocity_accumulation_bug(void) {
    TEST_LOG_PROGRESS("Testing Sprint 21 velocity accumulation regression");
    
    // Create entity with physics
    EntityID entity = entity_create(test_world);
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Set initial conditions
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics->mass = 1.0f;
    physics->drag = 0.0f;  // No drag for clean test
    
    // Apply a single impulse force
    Vector3 test_force = {10.0f, 0.0f, 0.0f};
    physics_apply_force(physics, test_force);
    
    // Run physics update once
    float delta_time = 1.0f / 60.0f;  // 60 FPS
    physics_system_update(test_world, delta_time);
    
    // Calculate expected velocity after one frame
    float expected_velocity_x = test_force.x / physics->mass * delta_time;
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, expected_velocity_x, physics->velocity.x);
    
    // Store velocity after first frame
    float first_frame_velocity = physics->velocity.x;
    
    // Run physics update again WITHOUT applying new forces
    physics_system_update(test_world, delta_time);
    
    // Velocity should remain the same (no new forces applied)
    // This is the key test - if the bug exists, velocity will continue to increase
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, first_frame_velocity, physics->velocity.x);
    
    // Run several more updates to ensure velocity remains stable
    for (int i = 0; i < 10; i++) {
        physics_system_update(test_world, delta_time);
        TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, first_frame_velocity, physics->velocity.x);
    }
    
    TEST_LOG_SUCCESS("Sprint 21 velocity accumulation bug regression test passed");
}

void test_sprint_21_force_accumulator_reset(void) {
    TEST_LOG_PROGRESS("Testing force accumulator reset behavior");
    
    EntityID entity = entity_create(test_world);
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Physics* physics = entity_get_physics(test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Initialize physics
    physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics->mass = 1.0f;
    physics->drag = 0.0f;
    
    float delta_time = 1.0f / 60.0f;
    
    // Apply force and update
    Vector3 force1 = {5.0f, 0.0f, 0.0f};
    physics_apply_force(physics, force1);
    physics_system_update(test_world, delta_time);
    
    float velocity_after_force1 = physics->velocity.x;
    
    // Apply different force and update
    Vector3 force2 = {3.0f, 0.0f, 0.0f};
    physics_apply_force(physics, force2);
    physics_system_update(test_world, delta_time);
    
    // Velocity change should be based only on force2, not force1 + force2
    float expected_velocity_change = force2.x / physics->mass * delta_time;
    float expected_velocity = velocity_after_force1 + expected_velocity_change;
    
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, expected_velocity, physics->velocity.x);
    
    TEST_LOG_SUCCESS("Force accumulator reset correctly");
}

void test_sprint_21_multiple_entities_isolation(void) {
    TEST_LOG_PROGRESS("Testing force isolation between multiple entities");
    
    // Create two entities
    EntityID entity1 = entity_create(test_world);
    EntityID entity2 = entity_create(test_world);
    
    entity_add_component(test_world, entity1, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    entity_add_component(test_world, entity2, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Physics* physics1 = entity_get_physics(test_world, entity1);
    struct Physics* physics2 = entity_get_physics(test_world, entity2);
    
    TEST_ASSERT_NOT_NULL(physics1);
    TEST_ASSERT_NOT_NULL(physics2);
    
    // Initialize both entities
    physics1->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics1->mass = 1.0f;
    physics1->drag = 0.0f;
    
    physics2->velocity = (Vector3){0.0f, 0.0f, 0.0f};
    physics2->mass = 1.0f;
    physics2->drag = 0.0f;
    
    // Apply force only to entity1
    Vector3 force = {10.0f, 0.0f, 0.0f};
    physics_apply_force(physics1, force);
    
    // Update physics
    float delta_time = 1.0f / 60.0f;
    physics_system_update(test_world, delta_time);
    
    // Entity1 should have velocity, entity2 should not
    float expected_velocity = force.x / physics1->mass * delta_time;
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, expected_velocity, physics1->velocity.x);
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, physics2->velocity.x);
    
    // Run more updates to ensure no cross-contamination
    for (int i = 0; i < 5; i++) {
        physics_system_update(test_world, delta_time);
        TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, physics2->velocity.x);
    }
    
    TEST_LOG_SUCCESS("Multiple entity force isolation works correctly");
}

// ============================================================================
// PERFORMANCE REGRESSION TEST
// ============================================================================

void test_sprint_21_physics_performance_regression(void) {
    TEST_LOG_PROGRESS("Testing physics performance regression");
    
    const int NUM_ENTITIES = 50;
    EntityID entities[NUM_ENTITIES];
    
    // Create many entities with physics
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        entity_add_component(test_world, entities[i], COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
        
        struct Physics* physics = entity_get_physics(test_world, entities[i]);
        physics->mass = 1.0f;
        physics->drag = 0.1f;
        
        // Apply some random forces
        Vector3 force = test_random_vector3(-10.0f, 10.0f);
        physics_apply_force(physics, force);
    }
    
    // Time physics update
    float delta_time = 1.0f / 60.0f;
    double duration = test_run_system_timed(physics_system_update, test_world, delta_time);
    
    // Physics update should complete well within frame budget
    TEST_ASSERT_PERFORMANCE(duration, TEST_PERF_SYSTEM_UPDATE_MAX_MS);
    
    TEST_LOG_SUCCESS("Physics performance within acceptable bounds");
}

// ============================================================================
// TEST SUITE
// ============================================================================

void suite_sprint_21_regression(void) {
    RUN_TEST(test_sprint_21_velocity_accumulation_bug);
    RUN_TEST(test_sprint_21_force_accumulator_reset);
    RUN_TEST(test_sprint_21_multiple_entities_isolation);
    RUN_TEST(test_sprint_21_physics_performance_regression);
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_sprint_21_regression();
    return UNITY_END();
}
#endif
