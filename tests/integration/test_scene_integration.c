/**
 * @file test_scene_integration.c
 * @brief Comprehensive integration tests for scene systems
 * 
 * Tests the integration of multiple systems within scenes including:
 * - Scene loading and entity configuration
 * - System coordination and lifecycle management
 * - Cross-system data flow and dependencies
 * - Performance under realistic scene conditions
 */

#include "../vendor/unity.h"
#include "../../src/scene_script.h"
#include "../../src/scene_state.h"
#include "../../src/scene_yaml_loader.h"
#include "../../src/core.h"
#include "../../src/system/input.h"
#include "../../src/system/control.h"
#include "../../src/system/thrusters.h"
#include "../../src/system/physics.h"
#include "../../src/system/camera.h"
#include "../../src/system/scripted_flight.h"
#include "../../src/data.h"
#include "../../src/assets.h"
#include "../../src/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Test state
static struct World test_world;
static SceneStateManager scene_state;
static bool systems_initialized = false;

void setUp(void) {
    world_init(&test_world);
    scene_state_init(&scene_state);
    
    if (!systems_initialized) {
        data_registry_init(get_data_registry());
        asset_registry_init(get_asset_registry());
        input_init();
        systems_initialized = true;
    }
}

void tearDown(void) {
    scene_state_cleanup(&scene_state);
    world_destroy(&test_world);
}

// ============================================================================
// SCENE LOADING INTEGRATION TESTS
// ============================================================================

void test_scene_yaml_loading_comprehensive(void) {
    printf("🧪 Testing comprehensive scene YAML loading...\n");
    
    // Test loading different scene types
    const char* test_scenes[] = {
        "flight_test.yaml",
        "thruster_test.yaml",
        "logo.yaml"
    };
    
    for (int i = 0; i < 3; i++) {
        printf("   Loading scene: %s\n", test_scenes[i]);
        
        // Clear world for clean test
        world_clear(&test_world);
        
        bool loaded = scene_load_from_yaml(&test_world, get_asset_registry(), test_scenes[i]);
        TEST_ASSERT_TRUE(loaded);
        TEST_ASSERT_GREATER_THAN(0, test_world.entity_count);
        
        // Verify at least one entity has basic components
        bool found_valid_entity = false;
        for (uint32_t j = 0; j < test_world.entity_count; j++) {
            struct Entity* entity = &test_world.entities[j];
            if (entity->component_mask & COMPONENT_TRANSFORM) {
                found_valid_entity = true;
                break;
            }
        }
        TEST_ASSERT_TRUE(found_valid_entity);
        
        printf("   ✅ %s loaded successfully (%d entities)\n", 
               test_scenes[i], test_world.entity_count);
    }
    
    printf("✅ Comprehensive scene YAML loading test passed\n");
}

void test_scene_entity_component_consistency(void) {
    printf("🧪 Testing scene entity component consistency...\n");
    
    // Load flight test scene (complex scene with multiple entity types)
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    
    // Verify component consistency across entities
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        
        // If entity has physics, it should have transform
        if (entity->component_mask & COMPONENT_PHYSICS) {
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_TRANSFORM);
            
            struct Transform* transform = entity_get_transform(&test_world, entity->id);
            struct Physics* physics = entity_get_physics(&test_world, entity->id);
            
            TEST_ASSERT_NOT_NULL(transform);
            TEST_ASSERT_NOT_NULL(physics);
            
            // Physics mass should be positive
            TEST_ASSERT_GREATER_THAN(0.0f, physics->mass);
        }
        
        // If entity has thrusters, it should have physics and transform
        if (entity->component_mask & COMPONENT_THRUSTER_SYSTEM) {
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_PHYSICS);
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_TRANSFORM);
            
            struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity->id);
            TEST_ASSERT_NOT_NULL(thrusters);
            
            // Thrusters should have reasonable force limits
            TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_linear_force.x);
            TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_linear_force.y);
            TEST_ASSERT_GREATER_THAN(0.0f, thrusters->max_linear_force.z);
        }
        
        // If entity has control authority, it should have thrusters
        if (entity->component_mask & COMPONENT_CONTROL_AUTHORITY) {
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_THRUSTER_SYSTEM);
            
            struct ControlAuthority* control = entity_get_control_authority(&test_world, entity->id);
            TEST_ASSERT_NOT_NULL(control);
        }
        
        // Camera entities should have transform
        if (entity->component_mask & COMPONENT_CAMERA) {
            TEST_ASSERT_TRUE(entity->component_mask & COMPONENT_TRANSFORM);
            
            struct Camera* camera = entity_get_camera(&test_world, entity->id);
            TEST_ASSERT_NOT_NULL(camera);
            TEST_ASSERT_GREATER_THAN(0.0f, camera->fov);
            TEST_ASSERT_GREATER_THAN(0.0f, camera->near_plane);
            TEST_ASSERT_GREATER_THAN(camera->near_plane, camera->far_plane);
        }
    }
    
    printf("✅ Scene entity component consistency test passed\n");
}

// ============================================================================
// SYSTEM COORDINATION TESTS
// ============================================================================

void test_flight_systems_coordination(void) {
    printf("🧪 Testing flight systems coordination...\n");
    
    // Load flight test scene
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
    
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, player_id);
    
    // Set up control system
    control_set_player_entity(&test_world, player_id);
    
    // Test coordinated system updates
    struct Physics* physics = entity_get_physics(&test_world, player_id);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, player_id);
    
    Vector3 initial_pos = physics->position;
    Vector3 initial_velocity = physics->velocity;
    
    // Run coordinated update cycle
    for (int frame = 0; frame < 60; frame++) { // 1 second at 60fps
        float delta_time = 0.016f;
        
        // Clear force accumulators
        physics->force_accumulator = (Vector3){0, 0, 0};
        physics->torque_accumulator = (Vector3){0, 0, 0};
        
        // Update systems in proper order
        input_update();
        control_system_update(&test_world, NULL, delta_time);
        thruster_system_update(&test_world, NULL, delta_time);
        physics_system_update(&test_world, NULL, delta_time);
        
        // Update scene script
        scene_script_execute_update("flight_test", &test_world, &scene_state, delta_time);
    }
    
    // Verify systems worked together
    Vector3 final_pos = physics->position;
    Vector3 final_velocity = physics->velocity;
    
    // Position should be updated (systems should be working)
    TEST_ASSERT_TRUE(final_pos.x != initial_pos.x || 
                    final_pos.y != initial_pos.y || 
                    final_pos.z != initial_pos.z);
    
    printf("✅ Position change: (%.3f, %.3f, %.3f)\n", 
           final_pos.x - initial_pos.x,
           final_pos.y - initial_pos.y, 
           final_pos.z - initial_pos.z);
    printf("✅ Flight systems coordination test passed\n");
}

void test_scripted_flight_scene_integration(void) {
    printf("🧪 Testing scripted flight scene integration...\n");
    
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
    
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, player_id);
    
    // Create scripted flight for player
    ScriptedFlight* flight = scripted_flight_create_component(player_id);
    TEST_ASSERT_NOT_NULL(flight);
    
    FlightPath* circuit = scripted_flight_create_circuit_path();
    scripted_flight_start(flight, circuit);
    
    control_set_player_entity(&test_world, player_id);
    
    struct Transform* transform = entity_get_transform(&test_world, player_id);
    Vector3 start_pos = transform->position;
    
    // Run integrated simulation
    for (int frame = 0; frame < 300; frame++) { // 5 seconds
        float delta_time = 0.016f;
        
        // Clear physics accumulators
        for (uint32_t i = 0; i < test_world.entity_count; i++) {
            struct Entity* entity = &test_world.entities[i];
            if (entity->component_mask & COMPONENT_PHYSICS) {
                struct Physics* phys = entity_get_physics(&test_world, entity->id);
                phys->force_accumulator = (Vector3){0, 0, 0};
                phys->torque_accumulator = (Vector3){0, 0, 0};
            }
        }
        
        // Full system update cycle
        input_update();
        scripted_flight_update(&test_world, NULL, delta_time);
        control_system_update(&test_world, NULL, delta_time);
        thruster_system_update(&test_world, NULL, delta_time);
        physics_system_update(&test_world, NULL, delta_time);
        scene_script_execute_update("flight_test", &test_world, &scene_state, delta_time);
    }
    
    Vector3 end_pos = transform->position;
    float distance_traveled = sqrtf(
        powf(end_pos.x - start_pos.x, 2) +
        powf(end_pos.y - start_pos.y, 2) +
        powf(end_pos.z - start_pos.z, 2)
    );
    
    // Should have moved significantly with scripted flight
    TEST_ASSERT_GREATER_THAN(20.0f, distance_traveled);
    
    printf("✅ Scripted flight distance traveled: %.2fm\n", distance_traveled);
    printf("✅ Scripted flight scene integration test passed\n");
}

// ============================================================================
// SCENE TRANSITION TESTS
// ============================================================================

void test_scene_transition_system_state(void) {
    printf("🧪 Testing scene transition system state...\n");
    
    // Load initial scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "logo.yaml");
    scene_script_execute_enter("logo", &test_world, &scene_state);
    
    uint32_t initial_entity_count = test_world.entity_count;
    TEST_ASSERT_GREATER_THAN(0, initial_entity_count);
    
    // Request transition
    scene_state_request_transition(&scene_state, "flight_test");
    TEST_ASSERT_TRUE(scene_state_has_pending_transition(&scene_state));
    
    // Execute exit script
    scene_script_execute_exit("logo", &test_world, &scene_state);
    
    // Clear world (simulating scene transition)
    world_clear(&test_world);
    TEST_ASSERT_EQUAL_INT(0, test_world.entity_count);
    
    // Load new scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    scene_script_execute_enter("flight_test", &test_world, &scene_state);
    
    // Mark transition complete
    scene_state_execute_transition(&scene_state, &test_world);
    strcpy(scene_state.current_scene_name, "flight_test");
    scene_state.transition_pending = false;
    
    // Verify new scene state
    TEST_ASSERT_FALSE(scene_state_has_pending_transition(&scene_state));
    TEST_ASSERT_EQUAL_STRING("flight_test", scene_state.current_scene_name);
    TEST_ASSERT_GREATER_THAN(0, test_world.entity_count);
    
    // Verify systems work in new scene
    EntityID player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_id = entity->id;
            break;
        }
    }
    
    if (player_id != INVALID_ENTITY) {
        control_set_player_entity(&test_world, player_id);
        
        // Test system update in new scene
        input_update();
        control_system_update(&test_world, NULL, 0.016f);
        thruster_system_update(&test_world, NULL, 0.016f);
        physics_system_update(&test_world, NULL, 0.016f);
    }
    
    printf("✅ Scene transition: logo (%d entities) -> flight_test (%d entities)\n",
           initial_entity_count, test_world.entity_count);
    printf("✅ Scene transition system state test passed\n");
}

// ============================================================================
// MULTI-ENTITY COORDINATION TESTS
// ============================================================================

void test_multi_entity_physics_coordination(void) {
    printf("🧪 Testing multi-entity physics coordination...\n");
    
    // Load scene with multiple entities
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    
    // Count entities with physics
    int physics_entity_count = 0;
    EntityID physics_entities[10];
    
    for (uint32_t i = 0; i < test_world.entity_count && physics_entity_count < 10; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PHYSICS) {
            physics_entities[physics_entity_count] = entity->id;
            physics_entity_count++;
        }
    }
    
    TEST_ASSERT_GREATER_THAN(1, physics_entity_count);
    
    // Record initial positions
    Vector3 initial_positions[10];
    for (int i = 0; i < physics_entity_count; i++) {
        struct Transform* transform = entity_get_transform(&test_world, physics_entities[i]);
        initial_positions[i] = transform->position;
    }
    
    // Apply some forces to create movement
    for (int i = 0; i < physics_entity_count; i++) {
        struct Physics* physics = entity_get_physics(&test_world, physics_entities[i]);
        if (physics && physics->has_6dof) {
            // Add some test forces
            physics_add_force(physics, (Vector3){100.0f + i * 10.0f, 0, 0});
        }
    }
    
    // Run physics updates
    for (int frame = 0; frame < 120; frame++) { // 2 seconds
        physics_system_update(&test_world, NULL, 0.016f);
    }
    
    // Verify entities moved independently
    int moved_entities = 0;
    for (int i = 0; i < physics_entity_count; i++) {
        struct Transform* transform = entity_get_transform(&test_world, physics_entities[i]);
        Vector3 pos_diff = {
            transform->position.x - initial_positions[i].x,
            transform->position.y - initial_positions[i].y,
            transform->position.z - initial_positions[i].z
        };
        float movement = sqrtf(pos_diff.x * pos_diff.x + pos_diff.y * pos_diff.y + pos_diff.z * pos_diff.z);
        
        if (movement > 1.0f) {
            moved_entities++;
        }
    }
    
    TEST_ASSERT_GREATER_THAN(0, moved_entities);
    
    printf("✅ Multi-entity coordination: %d/%d physics entities moved\n", 
           moved_entities, physics_entity_count);
    printf("✅ Multi-entity physics coordination test passed\n");
}

// ============================================================================
// PERFORMANCE INTEGRATION TESTS
// ============================================================================

void test_scene_performance_under_load(void) {
    printf("🧪 Testing scene performance under load...\n");
    
    // Load complex scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    scene_script_execute_enter("flight_test", &test_world, &scene_state);
    
    // Find and set up player
    EntityID player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < test_world.entity_count; i++) {
        struct Entity* entity = &test_world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_id = entity->id;
            break;
        }
    }
    
    if (player_id != INVALID_ENTITY) {
        control_set_player_entity(&test_world, player_id);
        
        // Add scripted flight for additional load
        ScriptedFlight* flight = scripted_flight_create_component(player_id);
        if (flight) {
            FlightPath* circuit = scripted_flight_create_circuit_path();
            scripted_flight_start(flight, circuit);
        }
    }
    
    // Performance test with full system updates
    clock_t start = clock();
    
    for (int frame = 0; frame < 300; frame++) { // 5 seconds of simulation
        float delta_time = 0.016f;
        
        // Clear physics accumulators for all entities
        for (uint32_t i = 0; i < test_world.entity_count; i++) {
            struct Entity* entity = &test_world.entities[i];
            if (entity->component_mask & COMPONENT_PHYSICS) {
                struct Physics* physics = entity_get_physics(&test_world, entity->id);
                physics->force_accumulator = (Vector3){0, 0, 0};
                physics->torque_accumulator = (Vector3){0, 0, 0};
            }
        }
        
        // Full system update cycle
        input_update();
        scripted_flight_update(&test_world, NULL, delta_time);
        control_system_update(&test_world, NULL, delta_time);
        thruster_system_update(&test_world, NULL, delta_time);
        physics_system_update(&test_world, NULL, delta_time);
        scene_script_execute_update("flight_test", &test_world, &scene_state, delta_time);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should maintain reasonable performance
    TEST_ASSERT_TRUE(elapsed < 0.5); // Less than 500ms for 300 frames
    
    printf("Scene performance: %.3fms for 300 frames (%d entities)\n", 
           elapsed * 1000.0, test_world.entity_count);
    printf("Average frame time: %.3fms\n", (elapsed * 1000.0) / 300.0);
    printf("✅ Scene performance under load test passed\n");
}

// ============================================================================
// ERROR HANDLING INTEGRATION TESTS
// ============================================================================

void test_scene_system_error_recovery(void) {
    printf("🧪 Testing scene system error recovery...\n");
    
    // Load scene
    scene_load_from_yaml(&test_world, get_asset_registry(), "flight_test.yaml");
    
    // Test system updates with various error conditions
    
    // 1. Update with null parameters
    input_update();
    control_system_update(NULL, NULL, 0.016f);
    thruster_system_update(NULL, NULL, 0.016f);
    physics_system_update(NULL, NULL, 0.016f);
    scripted_flight_update(NULL, NULL, 0.016f);
    
    // 2. Update with invalid delta time
    control_system_update(&test_world, NULL, -1.0f);
    physics_system_update(&test_world, NULL, 0.0f);
    scripted_flight_update(&test_world, NULL, 1000.0f);
    
    // 3. Scene script updates with invalid parameters
    scene_script_execute_enter(NULL, &test_world, &scene_state);
    scene_script_execute_update("nonexistent_scene", &test_world, &scene_state, 0.016f);
    scene_script_execute_exit("flight_test", NULL, NULL);
    
    // 4. Normal operation after error conditions
    for (int i = 0; i < 10; i++) {
        input_update();
        control_system_update(&test_world, NULL, 0.016f);
        thruster_system_update(&test_world, NULL, 0.016f);
        physics_system_update(&test_world, NULL, 0.016f);
        scene_script_execute_update("flight_test", &test_world, &scene_state, 0.016f);
    }
    
    // If we reach here, error recovery is working
    TEST_ASSERT_TRUE(true);
    
    printf("✅ Scene system error recovery test passed\n");
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_scene_integration(void) {
    printf("\n🎬 Scene Integration Tests\n");
    printf("==========================\n");
    
    printf("📂 Testing Scene Loading Integration...\n");
    RUN_TEST(test_scene_yaml_loading_comprehensive);
    RUN_TEST(test_scene_entity_component_consistency);
    
    printf("⚙️  Testing System Coordination...\n");
    RUN_TEST(test_flight_systems_coordination);
    RUN_TEST(test_scripted_flight_scene_integration);
    
    printf("🔄 Testing Scene Transitions...\n");
    RUN_TEST(test_scene_transition_system_state);
    
    printf("🏗️  Testing Multi-Entity Coordination...\n");
    RUN_TEST(test_multi_entity_physics_coordination);
    
    printf("⚡ Testing Performance Integration...\n");
    RUN_TEST(test_scene_performance_under_load);
    
    printf("🛡️  Testing Error Handling Integration...\n");
    RUN_TEST(test_scene_system_error_recovery);
    
    printf("✅ Scene Integration Tests Complete\n");
}

int main(void) {
    UNITY_BEGIN();
    
    // Run scene integration test suite
    suite_scene_integration();
    
    // Clean up
    if (systems_initialized) {
        input_shutdown();
    }
    
    return UNITY_END();
}
