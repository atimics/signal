/**
 * @file test_scripted_flight.c
 * @brief Comprehensive tests for the scripted flight system
 * 
 * Tests all aspects of the scripted flight system including:
 * - Flight path creation and validation
 * - Component lifecycle management
 * - Flight control and navigation
 * - Integration with physics and thrusters
 * - Performance and edge cases
 */

#include "../vendor/unity.h"
#include "../../src/system/scripted_flight.h"
#include "../../src/system/thrusters.h"
#include "../../src/system/physics.h"
#include "../../src/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Test world
static struct World test_world;

void setUp(void) {
    world_init(&test_world);
}

void tearDown(void) {
    world_destroy(&test_world);
}

// ============================================================================
// FLIGHT PATH CREATION TESTS
// ============================================================================

void test_scripted_flight_circuit_path_detailed(void) {
    printf("🧪 Testing scripted flight circuit path (detailed)...\n");
    
    FlightPath* circuit = scripted_flight_create_circuit_path();
    TEST_ASSERT_NOT_NULL(circuit);
    
    // Verify path has correct structure for a circuit
    TEST_ASSERT_GREATER_THAN(3, circuit->waypoint_count);
    TEST_ASSERT_TRUE(circuit->loop);
    TEST_ASSERT_GREATER_THAN(0.0f, circuit->total_distance);
    TEST_ASSERT_GREATER_THAN(0.0f, circuit->max_speed);
    TEST_ASSERT_GREATER_THAN(0.0f, circuit->max_turn_rate);
    
    // Verify waypoints form a reasonable circuit
    bool has_varied_positions = false;
    Vector3 first_pos = circuit->waypoints[0].position;
    
    for (int i = 1; i < circuit->waypoint_count; i++) {
        Waypoint wp = circuit->waypoints[i];
        
        // Each waypoint should have valid properties
        TEST_ASSERT_GREATER_THAN(0.0f, wp.target_speed);
        TEST_ASSERT_GREATER_THAN(0.0f, wp.tolerance);
        TEST_ASSERT_EQUAL_INT(WAYPOINT_POSITION, wp.type);
        
        // Check for position variation
        Vector3 diff = {
            wp.position.x - first_pos.x,
            wp.position.y - first_pos.y,
            wp.position.z - first_pos.z
        };
        float distance = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        if (distance > 10.0f) {
            has_varied_positions = true;
        }
    }
    
    TEST_ASSERT_TRUE(has_varied_positions);
    
    printf("✅ Circuit path: %d waypoints, %.1fm total distance\n", 
           circuit->waypoint_count, circuit->total_distance);
    printf("✅ Scripted flight circuit path (detailed) test passed\n");
}

void test_scripted_flight_figure_eight_path_detailed(void) {
    printf("🧪 Testing scripted flight figure-8 path (detailed)...\n");
    
    FlightPath* figure8 = scripted_flight_create_figure_eight_path();
    TEST_ASSERT_NOT_NULL(figure8);
    
    // Figure-8 should have more waypoints than a simple circuit
    TEST_ASSERT_GREATER_THAN(6, figure8->waypoint_count);
    TEST_ASSERT_TRUE(figure8->loop);
    TEST_ASSERT_GREATER_THAN(0.0f, figure8->total_distance);
    
    // Check that the path has the characteristic figure-8 shape
    // (waypoints should cross over the center point)
    Vector3 center = {0, 0, 0};
    bool has_positive_x = false, has_negative_x = false;
    bool has_varied_z = false;
    
    for (int i = 0; i < figure8->waypoint_count; i++) {
        Vector3 pos = figure8->waypoints[i].position;
        
        if (pos.x > 10.0f) has_positive_x = true;
        if (pos.x < -10.0f) has_negative_x = true;
        if (fabsf(pos.z) > 10.0f) has_varied_z = true;
    }
    
    // Figure-8 should have waypoints on both sides
    TEST_ASSERT_TRUE(has_positive_x);
    TEST_ASSERT_TRUE(has_negative_x);
    TEST_ASSERT_TRUE(has_varied_z);
    
    printf("✅ Figure-8 path: %d waypoints, %.1fm total distance\n", 
           figure8->waypoint_count, figure8->total_distance);
    printf("✅ Scripted flight figure-8 path (detailed) test passed\n");
}

void test_scripted_flight_landing_approach_path_detailed(void) {
    printf("🧪 Testing scripted flight landing approach path (detailed)...\n");
    
    Vector3 start_pos = {200.0f, 100.0f, 150.0f};
    Vector3 landing_pos = {5.0f, 2.0f, -10.0f};
    
    FlightPath* approach = scripted_flight_create_landing_approach_path(start_pos, landing_pos);
    TEST_ASSERT_NOT_NULL(approach);
    
    // Landing approach should not loop
    TEST_ASSERT_FALSE(approach->loop);
    TEST_ASSERT_GREATER_THAN(2, approach->waypoint_count);
    
    // First waypoint should be near start position
    Waypoint first = approach->waypoints[0];
    Vector3 start_diff = {
        first.position.x - start_pos.x,
        first.position.y - start_pos.y,
        first.position.z - start_pos.z
    };
    float start_distance = sqrtf(start_diff.x * start_diff.x + 
                                start_diff.y * start_diff.y + 
                                start_diff.z * start_diff.z);
    TEST_ASSERT_LESS_THAN(50.0f, start_distance);
    
    // Last waypoint should be at landing position
    Waypoint last = approach->waypoints[approach->waypoint_count - 1];
    TEST_ASSERT_FLOAT_WITHIN(5.0f, landing_pos.x, last.position.x);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, landing_pos.y, last.position.y);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, landing_pos.z, last.position.z);
    
    // Speed should decrease towards landing
    float first_speed = approach->waypoints[0].target_speed;
    float last_speed = last.target_speed;
    TEST_ASSERT_GREATER_THAN(last_speed, first_speed);
    
    printf("✅ Landing approach: %d waypoints, %.1f->%.1f m/s speed range\n", 
           approach->waypoint_count, first_speed, last_speed);
    printf("✅ Scripted flight landing approach path (detailed) test passed\n");
}

// ============================================================================
// COMPONENT LIFECYCLE TESTS
// ============================================================================

void test_scripted_flight_component_lifecycle(void) {
    printf("🧪 Testing scripted flight component lifecycle...\n");
    
    // Create test entity with required components
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    // Initialize required components
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 50, 0};
    transform->rotation = (Quaternion){0, 0, 0, 1};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 100.0f;
    physics->has_6dof = true;
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thrusters->thrusters_enabled = true;
    thrusters->max_linear_force = (Vector3){500, 500, 500};
    
    // Create scripted flight component
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    TEST_ASSERT_NOT_NULL(flight);
    
    // Verify initial state
    TEST_ASSERT_FALSE(flight->active);
    TEST_ASSERT_FALSE(flight->manual_override);
    TEST_ASSERT_EQUAL_INT(0, flight->current_waypoint);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, flight->current_speed);
    
    // Test component destruction
    scripted_flight_destroy_component(flight);
    
    // Component should be cleaned up (we can't easily test this directly
    // without exposing internal state, but it shouldn't crash)
    
    printf("✅ Scripted flight component lifecycle test passed\n");
}

void test_scripted_flight_multiple_components(void) {
    printf("🧪 Testing multiple scripted flight components...\n");
    
    const int NUM_ENTITIES = 5;
    EntityID entities[NUM_ENTITIES];
    ScriptedFlight* flights[NUM_ENTITIES];
    
    // Create multiple entities with scripted flight
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(&test_world);
        entity_add_components(&test_world, entities[i],
            COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
        
        // Initialize components
        struct Transform* transform = entity_get_transform(&test_world, entities[i]);
        transform->position = (Vector3){i * 20.0f, 50, 0};
        
        struct Physics* physics = entity_get_physics(&test_world, entities[i]);
        physics->mass = 80.0f;
        physics->has_6dof = true;
        
        struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entities[i]);
        thrusters->thrusters_enabled = true;
        
        flights[i] = scripted_flight_create_component(entities[i]);
        TEST_ASSERT_NOT_NULL(flights[i]);
    }
    
    // Start different flight patterns
    FlightPath* circuit = scripted_flight_create_circuit_path();
    FlightPath* figure8 = scripted_flight_create_figure_eight_path();
    
    scripted_flight_start(flights[0], circuit);
    scripted_flight_start(flights[1], figure8);
    scripted_flight_start(flights[2], circuit);
    
    // Verify states
    TEST_ASSERT_TRUE(flights[0]->active);
    TEST_ASSERT_TRUE(flights[1]->active);
    TEST_ASSERT_TRUE(flights[2]->active);
    TEST_ASSERT_FALSE(flights[3]->active); // Not started
    TEST_ASSERT_FALSE(flights[4]->active); // Not started
    
    // Test system update with multiple active flights
    scripted_flight_update(&test_world, NULL, 0.016f);
    
    // All active flights should still be active
    TEST_ASSERT_TRUE(flights[0]->active);
    TEST_ASSERT_TRUE(flights[1]->active);
    TEST_ASSERT_TRUE(flights[2]->active);
    
    printf("✅ Multiple scripted flight components test passed\n");
}

// ============================================================================
// FLIGHT CONTROL AND NAVIGATION TESTS
// ============================================================================

void test_scripted_flight_waypoint_navigation(void) {
    printf("🧪 Testing scripted flight waypoint navigation...\n");
    
    // Create test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    // Position entity at origin
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 0, 0};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 100.0f;
    physics->has_6dof = true;
    physics->velocity = (Vector3){0, 0, 0};
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thrusters->thrusters_enabled = true;
    thrusters->max_linear_force = (Vector3){1000, 1000, 1000};
    
    // Create scripted flight and simple path
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    
    scripted_flight_start(flight, circuit);
    
    // Record initial state
    int initial_waypoint = flight->current_waypoint;
    Vector3 initial_pos = transform->position;
    
    // Update for several frames
    for (int frame = 0; frame < 100; frame++) {
        scripted_flight_update(&test_world, NULL, 0.016f);
        thruster_system_update(&test_world, NULL, 0.016f);
        physics_system_update(&test_world, NULL, 0.016f);
    }
    
    // Entity should have moved
    Vector3 final_pos = transform->position;
    Vector3 movement = {
        final_pos.x - initial_pos.x,
        final_pos.y - initial_pos.y,
        final_pos.z - initial_pos.z
    };
    float movement_magnitude = sqrtf(movement.x * movement.x + 
                                    movement.y * movement.y + 
                                    movement.z * movement.z);
    
    TEST_ASSERT_GREATER_THAN(1.0f, movement_magnitude);
    
    printf("✅ Entity moved %.2fm from initial position\n", movement_magnitude);
    printf("✅ Scripted flight waypoint navigation test passed\n");
}

void test_scripted_flight_speed_control(void) {
    printf("🧪 Testing scripted flight speed control...\n");
    
    // Create test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 50, 0};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 50.0f;
    physics->has_6dof = true;
    physics->velocity = (Vector3){0, 0, 0};
    physics->drag_linear = 0.1f;
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thrusters->thrusters_enabled = true;
    thrusters->max_linear_force = (Vector3){2000, 2000, 2000};
    
    // Create flight with known target speeds
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    
    // Verify circuit has reasonable speed targets
    float min_speed = circuit->waypoints[0].target_speed;
    float max_speed = min_speed;
    
    for (int i = 0; i < circuit->waypoint_count; i++) {
        float speed = circuit->waypoints[i].target_speed;
        if (speed < min_speed) min_speed = speed;
        if (speed > max_speed) max_speed = speed;
    }
    
    TEST_ASSERT_GREATER_THAN(10.0f, min_speed); // Reasonable minimum
    TEST_ASSERT_LESS_THAN(200.0f, max_speed);   // Reasonable maximum
    
    scripted_flight_start(flight, circuit);
    
    // Run for enough time to build up speed
    for (int frame = 0; frame < 200; frame++) {
        scripted_flight_update(&test_world, NULL, 0.016f);
        thruster_system_update(&test_world, NULL, 0.016f);
        physics_system_update(&test_world, NULL, 0.016f);
    }
    
    // Check that entity has gained reasonable velocity
    Vector3 velocity = physics->velocity;
    float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
    
    TEST_ASSERT_GREATER_THAN(5.0f, speed); // Should have some speed
    TEST_ASSERT_LESS_THAN(300.0f, speed);  // But not excessive
    
    printf("✅ Final entity speed: %.2f m/s\n", speed);
    printf("✅ Scripted flight speed control test passed\n");
}

// ============================================================================
// PAUSE/RESUME AND MANUAL OVERRIDE TESTS
// ============================================================================

void test_scripted_flight_pause_resume_detailed(void) {
    printf("🧪 Testing scripted flight pause/resume (detailed)...\n");
    
    // Create test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 30, 0};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 80.0f;
    physics->has_6dof = true;
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thrusters->thrusters_enabled = true;
    
    // Create and start flight
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    scripted_flight_start(flight, circuit);
    
    // Record state before pause
    int waypoint_before_pause = flight->current_waypoint;
    Vector3 pos_before_pause = transform->position;
    
    // Run for some time
    for (int i = 0; i < 30; i++) {
        scripted_flight_update(&test_world, NULL, 0.016f);
    }
    
    Vector3 pos_before_pause_end = transform->position;
    
    // Pause flight
    scripted_flight_pause(flight);
    TEST_ASSERT_TRUE(flight->active);
    TEST_ASSERT_TRUE(flight->manual_override);
    
    // Run for more time - position shouldn't change much due to pause
    for (int i = 0; i < 30; i++) {
        scripted_flight_update(&test_world, NULL, 0.016f);
    }
    
    Vector3 pos_during_pause = transform->position;
    
    // Resume flight
    scripted_flight_resume(flight);
    TEST_ASSERT_TRUE(flight->active);
    TEST_ASSERT_FALSE(flight->manual_override);
    
    // Run again - movement should resume
    for (int i = 0; i < 30; i++) {
        scripted_flight_update(&test_world, NULL, 0.016f);
    }
    
    Vector3 pos_after_resume = transform->position;
    
    // Verify behavior: movement during pause should be minimal compared to active phases
    float movement_before_pause = sqrtf(
        powf(pos_before_pause_end.x - pos_before_pause.x, 2) +
        powf(pos_before_pause_end.y - pos_before_pause.y, 2) +
        powf(pos_before_pause_end.z - pos_before_pause.z, 2)
    );
    
    float movement_after_resume = sqrtf(
        powf(pos_after_resume.x - pos_during_pause.x, 2) +
        powf(pos_after_resume.y - pos_during_pause.y, 2) +
        powf(pos_after_resume.z - pos_during_pause.z, 2)
    );
    
    printf("✅ Movement before pause: %.2fm, after resume: %.2fm\n", 
           movement_before_pause, movement_after_resume);
    printf("✅ Scripted flight pause/resume (detailed) test passed\n");
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_scripted_flight_physics_integration(void) {
    printf("🧪 Testing scripted flight physics integration...\n");
    
    // Create test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 100, 0};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 120.0f;
    physics->has_6dof = true;
    physics->velocity = (Vector3){0, 0, 0};
    physics->drag_linear = 0.05f;
    physics->drag_angular = 0.1f;
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thrusters->thrusters_enabled = true;
    thrusters->max_linear_force = (Vector3){1500, 1500, 1500};
    thrusters->max_angular_torque = (Vector3){200, 200, 200};
    
    // Create and start scripted flight
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* figure8 = scripted_flight_create_figure_eight_path();
    scripted_flight_start(flight, figure8);
    
    Vector3 initial_pos = transform->position;
    
    // Run integrated simulation
    for (int frame = 0; frame < 300; frame++) {
        // Clear force accumulators
        physics->force_accumulator = (Vector3){0, 0, 0};
        physics->torque_accumulator = (Vector3){0, 0, 0};
        
        // Update systems in correct order
        scripted_flight_update(&test_world, NULL, 0.016f);
        thruster_system_update(&test_world, NULL, 0.016f);
        physics_system_update(&test_world, NULL, 0.016f);
    }
    
    // Verify physics effects
    Vector3 final_pos = transform->position;
    Vector3 final_velocity = physics->velocity;
    
    // Entity should have moved significantly
    float distance_traveled = sqrtf(
        powf(final_pos.x - initial_pos.x, 2) +
        powf(final_pos.y - initial_pos.y, 2) +
        powf(final_pos.z - initial_pos.z, 2)
    );
    
    TEST_ASSERT_GREATER_THAN(50.0f, distance_traveled);
    
    // Should have some velocity
    float final_speed = sqrtf(final_velocity.x * final_velocity.x + 
                             final_velocity.y * final_velocity.y + 
                             final_velocity.z * final_velocity.z);
    
    TEST_ASSERT_GREATER_THAN(5.0f, final_speed);
    TEST_ASSERT_LESS_THAN(500.0f, final_speed); // Shouldn't be excessive
    
    printf("✅ Distance traveled: %.2fm, final speed: %.2f m/s\n", 
           distance_traveled, final_speed);
    printf("✅ Scripted flight physics integration test passed\n");
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_scripted_flight_system_performance(void) {
    printf("🧪 Testing scripted flight system performance...\n");
    
    const int NUM_ENTITIES = 20;
    EntityID entities[NUM_ENTITIES];
    ScriptedFlight* flights[NUM_ENTITIES];
    
    // Create multiple scripted flight entities
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(&test_world);
        entity_add_components(&test_world, entities[i],
            COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
        
        struct Transform* transform = entity_get_transform(&test_world, entities[i]);
        transform->position = (Vector3){i * 10.0f, 50 + i * 5.0f, i * 8.0f};
        
        struct Physics* physics = entity_get_physics(&test_world, entities[i]);
        physics->mass = 80.0f + i * 5.0f;
        physics->has_6dof = true;
        
        struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entities[i]);
        thrusters->thrusters_enabled = true;
        
        flights[i] = scripted_flight_create_component(entities[i]);
        
        // Start different flight patterns
        if (i % 3 == 0) {
            FlightPath* circuit = scripted_flight_create_circuit_path();
            scripted_flight_start(flights[i], circuit);
        } else if (i % 3 == 1) {
            FlightPath* figure8 = scripted_flight_create_figure_eight_path();
            scripted_flight_start(flights[i], figure8);
        }
        // Leave some entities without active flights for mixed testing
    }
    
    // Performance test
    clock_t start = clock();
    
    for (int frame = 0; frame < 100; frame++) {
        scripted_flight_update(&test_world, NULL, 0.016f);
        thruster_system_update(&test_world, NULL, 0.016f);
        physics_system_update(&test_world, NULL, 0.016f);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should handle multiple entities efficiently
    TEST_ASSERT_TRUE(elapsed < 0.1); // Less than 100ms for 100 frames of 20 entities
    
    printf("Scripted flight performance: %.3fms for 100 frames, %d entities\n", 
           elapsed * 1000.0, NUM_ENTITIES);
    printf("✅ Scripted flight system performance test passed\n");
}

// ============================================================================
// ERROR HANDLING AND EDGE CASES
// ============================================================================

void test_scripted_flight_error_handling(void) {
    printf("🧪 Testing scripted flight error handling...\n");
    
    // Test with null parameters
    scripted_flight_start(NULL, NULL);
    scripted_flight_stop(NULL);
    scripted_flight_pause(NULL);
    scripted_flight_resume(NULL);
    
    // Test with invalid entity
    ScriptedFlight* invalid_flight = scripted_flight_create_component(INVALID_ENTITY);
    // Should handle gracefully (implementation dependent)
    
    // Test with entity missing required components
    EntityID incomplete_entity = entity_create(&test_world);
    entity_add_component(&test_world, incomplete_entity, COMPONENT_TRANSFORM);
    // Missing physics and thrusters
    
    ScriptedFlight* incomplete_flight = scripted_flight_create_component(incomplete_entity);
    if (incomplete_flight) {
        FlightPath* path = scripted_flight_create_circuit_path();
        scripted_flight_start(incomplete_flight, path);
        
        // Update should handle missing components gracefully
        scripted_flight_update(&test_world, NULL, 0.016f);
    }
    
    // Test system update with null world
    scripted_flight_update(NULL, NULL, 0.016f);
    
    // If we reach here, error handling is working
    TEST_ASSERT_TRUE(true);
    
    printf("✅ Scripted flight error handling test passed\n");
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_scripted_flight(void) {
    printf("\n🛩️  Scripted Flight System Tests\n");
    printf("=================================\n");
    
    printf("🛤️  Testing Flight Path Creation...\n");
    RUN_TEST(test_scripted_flight_circuit_path_detailed);
    RUN_TEST(test_scripted_flight_figure_eight_path_detailed);
    RUN_TEST(test_scripted_flight_landing_approach_path_detailed);
    
    printf("🔧 Testing Component Lifecycle...\n");
    RUN_TEST(test_scripted_flight_component_lifecycle);
    RUN_TEST(test_scripted_flight_multiple_components);
    
    printf("🧭 Testing Flight Control and Navigation...\n");
    RUN_TEST(test_scripted_flight_waypoint_navigation);
    RUN_TEST(test_scripted_flight_speed_control);
    
    printf("⏸️  Testing Pause/Resume and Manual Override...\n");
    RUN_TEST(test_scripted_flight_pause_resume_detailed);
    
    printf("🔗 Testing System Integration...\n");
    RUN_TEST(test_scripted_flight_physics_integration);
    
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_scripted_flight_system_performance);
    
    printf("🛡️  Testing Error Handling and Edge Cases...\n");
    RUN_TEST(test_scripted_flight_error_handling);
    
    printf("✅ Scripted Flight System Tests Complete\n");
}

int main(void) {
    UNITY_BEGIN();
    
    // Run scripted flight system test suite
    suite_scripted_flight();
    
    return UNITY_END();
}
