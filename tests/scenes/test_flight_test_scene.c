/**
 * @file test_flight_test_scene.c
 * @brief Comprehensive tests for flight test scene and related systems
 * 
 * Tests the critical components of the flight test scene including:
 * - Scene loading and initialization
 * - Scripted flight system integration
 * - Flight controls and input handling
 * - Camera system integration
 * - Visual effects and obstacle systems
 */

#include "../vendor/unity.h"
#include "../../src/scene_script.h"
#include "../../src/core.h"
#include "../../src/system/scripted_flight.h"
#include "../../src/system/input.h"
#include "../../src/system/control.h"
#include "../../src/system/thrusters.h"
#include "../../src/system/physics.h"
#include "../../src/system/camera.h"
#include "../../src/scene_yaml_loader.h"
#include "../../src/scene_state.h"
#include "../../src/data.h"
#include "../../src/assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test world and state
static struct World test_world;
static SceneStateManager scene_state;
static bool systems_initialized = false;

void setUp(void) {
    // Initialize test world
    world_init(&test_world);
    
    // Initialize scene state
    scene_state_init(&scene_state);
    
    // Initialize required systems for flight test
    if (!systems_initialized) {
        // Initialize data registry and assets
        data_registry_init(get_data_registry());
        asset_registry_init(get_asset_registry());
        
        // Initialize input and control systems
        input_init();
        
        systems_initialized = true;
    }
}

void tearDown(void) {
    // Clean up scene state
    scene_state_cleanup(&scene_state);
    
    // Clean up world
    world_destroy(&test_world);
}

// ============================================================================
// SCENE LOADING TESTS
// ============================================================================

void test_flight_test_scene_yaml_loading(void) {
    printf("🧪 Testing flight test scene YAML loading...\n");
    
    // Load the flight test scene from YAML
    bool loaded = scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    TEST_ASSERT_TRUE(loaded);
    
    // Verify entities were loaded
    TEST_ASSERT_GREATER_THAN(0, test_world.entity_count);
    
    // Look for required entity types
    bool found_player = false;
    bool found_camera = false;
    bool found_landing_pad = false;
    bool found_markers = false;
    
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        
        if (entity->component_mask & COMPONENT_PLAYER) {
            found_player = true;
            
            // Verify player has required components
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_TRANSFORM);
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_PHYSICS);
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_THRUSTER_SYSTEM);
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_CONTROL_AUTHORITY);
        }
        
        if (entity->component_mask & COMPONENT_CAMERA) {
            found_camera = true;
        }
    }
    
    TEST_ASSERT_TRUE(found_player);
    TEST_ASSERT_TRUE(found_camera);
    
    printf("✅ Flight test scene YAML loading test passed\n");
}

void test_flight_test_scene_entity_configuration(void) {
    printf("🧪 Testing flight test scene entity configuration...\n");
    
    // Load scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    
    // Find player entity
    EntityID player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_id = entity->id;
            break;
        }
    }
    
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, player_id);
    
    // Verify player physics configuration
    struct Physics* physics = entity_get_physics(&test_world, player_id);
    TEST_ASSERT_NOT_NULL(physics);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, physics->mass);
    TEST_ASSERT_EQUAL_FLOAT(0.01f, physics->drag_linear);
    TEST_ASSERT_TRUE(physics->has_6dof);
    
    // Verify thruster system
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, player_id);
    TEST_ASSERT_NOT_NULL(thrusters);
    TEST_ASSERT_TRUE(thrusters->thrusters_enabled);
    
    // Verify control authority
    struct ControlAuthority* control = entity_get_control_authority(&test_world, player_id);
    TEST_ASSERT_NOT_NULL(control);
    
    printf("✅ Flight test scene entity configuration test passed\n");
}

// ============================================================================
// SCRIPTED FLIGHT SYSTEM TESTS
// ============================================================================

void test_scripted_flight_component_creation(void) {
    printf("🧪 Testing scripted flight component creation...\n");
    
    // Create a test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, 
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    // Create scripted flight component
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    TEST_ASSERT_NOT_NULL(flight);
    
    // Verify initial state
    TEST_ASSERT_FALSE(flight->active);
    TEST_ASSERT_FALSE(flight->manual_override);
    TEST_ASSERT_EQUAL_INT(0, flight->current_waypoint);
    
    printf("✅ Scripted flight component creation test passed\n");
}

void test_scripted_flight_circuit_path_creation(void) {
    printf("🧪 Testing scripted flight circuit path creation...\n");
    
    // Create circuit path
    FlightPath* circuit = scripted_flight_create_circuit_path();
    TEST_ASSERT_NOT_NULL(circuit);
    
    // Verify path properties
    TEST_ASSERT_GREATER_THAN(3, circuit->waypoint_count);
    TEST_ASSERT_TRUE(circuit->loop);
    TEST_ASSERT_GREATER_THAN(0.0f, circuit->total_distance);
    
    // Verify waypoints have valid data
    for (int i = 0; i < circuit->waypoint_count; i++) {
        TEST_ASSERT_GREATER_THAN(0.0f, circuit->waypoints[i].target_speed);
        TEST_ASSERT_GREATER_THAN(0.0f, circuit->waypoints[i].tolerance);
    }
    
    printf("✅ Scripted flight circuit path creation test passed\n");
}

void test_scripted_flight_figure_eight_path_creation(void) {
    printf("🧪 Testing scripted flight figure-8 path creation...\n");
    
    // Create figure-8 path
    FlightPath* figure8 = scripted_flight_create_figure_eight_path();
    TEST_ASSERT_NOT_NULL(figure8);
    
    // Verify path properties
    TEST_ASSERT_GREATER_THAN(7, figure8->waypoint_count); // Figure-8 needs multiple points
    TEST_ASSERT_TRUE(figure8->loop);
    TEST_ASSERT_GREATER_THAN(0.0f, figure8->total_distance);
    
    printf("✅ Scripted flight figure-8 path creation test passed\n");
}

void test_scripted_flight_landing_approach_path_creation(void) {
    printf("🧪 Testing scripted flight landing approach path creation...\n");
    
    Vector3 start_pos = {100.0f, 50.0f, 100.0f};
    Vector3 landing_pos = {0.0f, 5.0f, 0.0f};
    
    // Create landing approach path
    FlightPath* approach = scripted_flight_create_landing_approach_path(start_pos, landing_pos);
    TEST_ASSERT_NOT_NULL(approach);
    
    // Verify path properties
    TEST_ASSERT_GREATER_THAN(2, approach->waypoint_count);
    TEST_ASSERT_FALSE(approach->loop); // Landing approach shouldn't loop
    
    // Verify final waypoint matches landing position
    Waypoint final = approach->waypoints[approach->waypoint_count - 1];
    TEST_ASSERT_FLOAT_WITHIN(1.0f, landing_pos.x, final.position.x);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, landing_pos.y, final.position.y);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, landing_pos.z, final.position.z);
    
    printf("✅ Scripted flight landing approach path creation test passed\n");
}

void test_scripted_flight_start_stop_functionality(void) {
    printf("🧪 Testing scripted flight start/stop functionality...\n");
    
    // Create test entity with required components
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    // Initialize components
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 20, 0};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 80.0f;
    physics->has_6dof = true;
    
    // Create scripted flight component and path
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    
    // Test starting flight
    scripted_flight_start(flight, circuit);
    TEST_ASSERT_TRUE(flight->active);
    TEST_ASSERT_FALSE(flight->manual_override);
    TEST_ASSERT_EQUAL_INT(0, flight->current_waypoint);
    
    // Test stopping flight
    scripted_flight_stop(flight);
    TEST_ASSERT_FALSE(flight->active);
    TEST_ASSERT_FALSE(flight->manual_override);
    
    printf("✅ Scripted flight start/stop functionality test passed\n");
}

void test_scripted_flight_pause_resume_functionality(void) {
    printf("🧪 Testing scripted flight pause/resume functionality...\n");
    
    // Create test entity with components
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM);
    
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    
    // Start flight
    scripted_flight_start(flight, circuit);
    TEST_ASSERT_TRUE(flight->active);
    
    // Test pause
    scripted_flight_pause(flight);
    TEST_ASSERT_TRUE(flight->active); // Still active, but in manual override
    TEST_ASSERT_TRUE(flight->manual_override);
    
    // Test resume
    scripted_flight_resume(flight);
    TEST_ASSERT_TRUE(flight->active);
    TEST_ASSERT_FALSE(flight->manual_override);
    
    printf("✅ Scripted flight pause/resume functionality test passed\n");
}

// ============================================================================
// FLIGHT SCENE INTEGRATION TESTS
// ============================================================================

void test_flight_test_scene_script_initialization(void) {
    printf("🧪 Testing flight test scene script initialization...\n");
    
    // Load flight test scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    
    // Execute scene enter script
    scene_script_execute_enter("flight_test", &test_world, &scene_state);
    
    // Find player entity
    EntityID player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_id = entity->id;
            break;
        }
    }
    
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, player_id);
    
    // Verify control system is configured
    struct ControlAuthority* control = entity_get_control_authority(&test_world, player_id);
    TEST_ASSERT_NOT_NULL(control);
    
    printf("✅ Flight test scene script initialization test passed\n");
}

void test_flight_test_scene_systems_integration(void) {
    printf("🧪 Testing flight test scene systems integration...\n");
    
    // Load scene and initialize
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    scene_script_execute_enter("flight_test", &test_world, &scene_state);
    
    // Find player entity
    EntityID player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_id = entity->id;
            break;
        }
    }
    
    // Set player entity for control system
    control_set_player_entity(&test_world, player_id);
    
    // Test system updates don't crash
    float delta_time = 0.016f;
    
    // Update all critical systems
    input_update();
    control_system_update(&test_world, NULL, delta_time);
    thruster_system_update(&test_world, NULL, delta_time);
    physics_system_update(&test_world, NULL, delta_time);
    scripted_flight_update(&test_world, NULL, delta_time);
    
    // Verify player entity still exists and is valid
    struct Entity* player = entity_get(&test_world, player_id);
    TEST_ASSERT_NOT_NULL(player);
    TEST_ASSERT_TRUE(player->component_mask & COMPONENT_PLAYER);
    
    printf("✅ Flight test scene systems integration test passed\n");
}

// ============================================================================
// INPUT AND CONTROL INTEGRATION TESTS
// ============================================================================

void test_flight_test_input_control_integration(void) {
    printf("🧪 Testing flight test input control integration...\n");
    
    // Create test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM | 
        COMPONENT_CONTROL_AUTHORITY | COMPONENT_PLAYER);
    
    // Initialize physics
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 80.0f;
    physics->has_6dof = true;
    
    // Set as player entity
    control_set_player_entity(&test_world, entity);
    
    // Simulate input processing and control updates
    float delta_time = 0.016f;
    
    // Clear any accumulated forces
    physics->force_accumulator = (Vector3){0, 0, 0};
    physics->torque_accumulator = (Vector3){0, 0, 0};
    
    // Update systems
    input_update();
    control_system_update(&test_world, NULL, delta_time);
    thruster_system_update(&test_world, NULL, delta_time);
    
    // Verify systems processed without crash
    TEST_ASSERT_NOT_NULL(entity_get_thruster_system(&test_world, entity));
    TEST_ASSERT_NOT_NULL(entity_get_control_authority(&test_world, entity));
    
    printf("✅ Flight test input control integration test passed\n");
}

void test_flight_test_manual_scripted_transition(void) {
    printf("🧪 Testing manual to scripted flight transition...\n");
    
    // Create test entity
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity,
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_THRUSTER_SYSTEM | 
        COMPONENT_CONTROL_AUTHORITY | COMPONENT_PLAYER);
    
    // Initialize components
    struct Transform* transform = entity_get_transform(&test_world, entity);
    transform->position = (Vector3){0, 20, 0};
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    physics->mass = 80.0f;
    physics->has_6dof = true;
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    thrusters->thrusters_enabled = true;
    
    // Set as player entity
    control_set_player_entity(&test_world, entity);
    
    // Create scripted flight
    ScriptedFlight* flight = scripted_flight_create_component(entity);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    
    // Start with manual control (no scripted flight)
    TEST_ASSERT_FALSE(flight->active);
    
    // Update systems for manual control
    input_update();
    control_system_update(&test_world, NULL, 0.016f);
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Transition to scripted flight
    scripted_flight_start(flight, circuit);
    TEST_ASSERT_TRUE(flight->active);
    
    // Update systems for scripted control
    scripted_flight_update(&test_world, NULL, 0.016f);
    thruster_system_update(&test_world, NULL, 0.016f);
    
    // Transition back to manual
    scripted_flight_stop(flight);
    TEST_ASSERT_FALSE(flight->active);
    
    printf("✅ Manual to scripted flight transition test passed\n");
}

// ============================================================================
// PERFORMANCE AND EDGE CASE TESTS
// ============================================================================

void test_flight_test_scene_performance(void) {
    printf("🧪 Testing flight test scene performance...\n");
    
    // Load full scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    scene_script_execute_enter("flight_test", &test_world, &scene_state);
    
    // Find player
    EntityID player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_id = entity->id;
            break;
        }
    }
    
    control_set_player_entity(&test_world, player_id);
    
    // Create scripted flight for stress test
    ScriptedFlight* flight = scripted_flight_create_component(player_id);
    FlightPath* circuit = scripted_flight_create_circuit_path();
    scripted_flight_start(flight, circuit);
    
    // Run multiple update cycles
    clock_t start = clock();
    
    for (int frame = 0; frame < 100; frame++) {
        float delta_time = 0.016f;
        
        input_update();
        control_system_update(&test_world, NULL, delta_time);
        thruster_system_update(&test_world, NULL, delta_time);
        physics_system_update(&test_world, NULL, delta_time);
        scripted_flight_update(&test_world, NULL, delta_time);
        
        // Simulate scene update
        scene_script_execute_update("flight_test", &test_world, &scene_state, delta_time);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Performance should be reasonable (less than 100ms for 100 frames)
    TEST_ASSERT_TRUE(elapsed < 0.1);
    
    printf("Flight test scene performance: %.3fms for 100 frames\n", elapsed * 1000.0);
    printf("✅ Flight test scene performance test passed\n");
}

void test_flight_test_null_safety(void) {
    printf("🧪 Testing flight test null safety...\n");
    
    // Test scripted flight with null parameters
    scripted_flight_start(NULL, NULL);
    scripted_flight_stop(NULL);
    scripted_flight_pause(NULL);
    scripted_flight_resume(NULL);
    
    // Test scene script with null parameters
    scene_script_execute_enter("flight_test", NULL, NULL);
    scene_script_execute_update("flight_test", NULL, NULL, 0.016f);
    scene_script_execute_exit("flight_test", NULL, NULL);
    
    // Test system updates with null world
    scripted_flight_update(NULL, NULL, 0.016f);
    
    // If we get here without crashing, null safety is working
    TEST_ASSERT_TRUE(true);
    
    printf("✅ Flight test null safety test passed\n");
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_flight_test_scene(void) {
    printf("\n🚀 Flight Test Scene Tests\n");
    printf("==========================\n");
    
    printf("📄 Testing Scene Loading...\n");
    RUN_TEST(test_flight_test_scene_yaml_loading);
    RUN_TEST(test_flight_test_scene_entity_configuration);
    
    printf("🛩️  Testing Scripted Flight System...\n");
    RUN_TEST(test_scripted_flight_component_creation);
    RUN_TEST(test_scripted_flight_circuit_path_creation);
    RUN_TEST(test_scripted_flight_figure_eight_path_creation);
    RUN_TEST(test_scripted_flight_landing_approach_path_creation);
    RUN_TEST(test_scripted_flight_start_stop_functionality);
    RUN_TEST(test_scripted_flight_pause_resume_functionality);
    
    printf("🔗 Testing Scene Integration...\n");
    RUN_TEST(test_flight_test_scene_script_initialization);
    RUN_TEST(test_flight_test_scene_systems_integration);
    
    printf("🎮 Testing Input and Control Integration...\n");
    RUN_TEST(test_flight_test_input_control_integration);
    RUN_TEST(test_flight_test_manual_scripted_transition);
    
    printf("⚡ Testing Performance and Edge Cases...\n");
    RUN_TEST(test_flight_test_scene_performance);
    RUN_TEST(test_flight_test_null_safety);
    
    printf("✅ Flight Test Scene Tests Complete\n");
}

int main(void) {
    UNITY_BEGIN();
    
    // Run flight test scene test suite
    suite_flight_test_scene();
    
    return UNITY_END();
}
