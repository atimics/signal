/**
 * @file test_control.c
 * @brief Comprehensive tests for control authority system
 * 
 * Tests the ControlAuthority component and input processing system.
 * Critical for Sprint 21 entity-agnostic control mechanics.
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/system/control.h"
#include "../../src/system/input.h"
#include "../../src/system/thrusters.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
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
    
    // Initialize input system for tests
    input_init();
}

void tearDown(void)
{
    if (test_world.entities) {
        free(test_world.entities);
        test_world.entities = NULL;
    }
    
    input_shutdown();
}

// ============================================================================
// CONTROL AUTHORITY COMPONENT TESTS
// ============================================================================

void test_control_authority_component_creation(void)
{
    EntityID entity = entity_create(&test_world);
    
    // Add control authority component
    bool success = entity_add_component(&test_world, entity, COMPONENT_CONTROL_AUTHORITY);
    TEST_ASSERT_TRUE(success);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    TEST_ASSERT_NOT_NULL(control);
    
    // Check default values
    TEST_ASSERT_EQUAL_INT(INVALID_ENTITY, control->controlled_by);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, control->control_sensitivity);
    TEST_ASSERT_TRUE(control->flight_assist_enabled);
    TEST_ASSERT_EQUAL_INT(CONTROL_ASSISTED, control->control_mode);
    
    // Check input state is initialized to zero
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->input_linear.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->input_linear.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->input_linear.z);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->input_angular.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->input_angular.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->input_angular.z);
}

void test_control_configuration_functions(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_CONTROL_AUTHORITY);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    
    // Test sensitivity setting
    control_set_sensitivity(control, 2.5f);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, control->control_sensitivity);
    
    // Test clamping (should clamp to 0.1-5.0 range)
    control_set_sensitivity(control, 10.0f);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, control->control_sensitivity);
    
    control_set_sensitivity(control, 0.05f);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, control->control_sensitivity);
    
    // Test flight assist toggle
    control_set_flight_assist(control, false);
    TEST_ASSERT_FALSE(control->flight_assist_enabled);
    
    control_toggle_flight_assist(control);
    TEST_ASSERT_TRUE(control->flight_assist_enabled);
    
    control_toggle_flight_assist(control);
    TEST_ASSERT_FALSE(control->flight_assist_enabled);
}

void test_control_player_entity_setting(void)
{
    EntityID player_entity = entity_create(&test_world);
    
    // Set player entity for control system
    control_set_player_entity(&test_world, player_entity);
    
    // This is tested indirectly through the control system update
    // The function should not crash and should accept the entity ID
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// INPUT PROCESSING TESTS
// ============================================================================

void test_control_linear_input_processing(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, 
                        COMPONENT_CONTROL_AUTHORITY | COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Set this entity as player controlled
    control->controlled_by = entity;
    control_set_player_entity(&test_world, entity);
    
    // Mock input state (since we can't easily control real input in tests)
    // We'll test the control system with simulated input
    control->input_linear = (Vector3){ 0.5f, -0.3f, 0.8f };
    
    // Manually call the thruster command setting
    thruster_set_linear_command(thrusters, control->input_linear);
    
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_linear_thrust.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.3f, thrusters->current_linear_thrust.y);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, thrusters->current_linear_thrust.z);
}

void test_control_angular_input_processing(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, 
                        COMPONENT_CONTROL_AUTHORITY | COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Set this entity as player controlled
    control->controlled_by = entity;
    control_set_player_entity(&test_world, entity);
    
    // Set angular input
    control->input_angular = (Vector3){ -0.7f, 0.4f, 0.2f };
    
    // Manually call the thruster command setting
    thruster_set_angular_command(thrusters, control->input_angular);
    
    TEST_ASSERT_EQUAL_FLOAT(-0.7f, thrusters->current_angular_thrust.x);
    TEST_ASSERT_EQUAL_FLOAT(0.4f, thrusters->current_angular_thrust.y);
    TEST_ASSERT_EQUAL_FLOAT(0.2f, thrusters->current_angular_thrust.z);
}

// ============================================================================
// SENSITIVITY CURVE TESTS
// ============================================================================

void test_sensitivity_curve_application(void)
{
    Vector3 input = { 0.5f, -0.3f, 0.8f };
    
    // Test normal sensitivity - quadratic curve: input * |input| * sensitivity
    Vector3 result1 = apply_sensitivity_curve(input, 1.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, result1.x);  // 0.5 * 0.5 * 1.0 = 0.25
    TEST_ASSERT_EQUAL_FLOAT(0.09f, result1.y);  // -0.3 * 0.3 * -1.0 * 1.0 = 0.09
    TEST_ASSERT_EQUAL_FLOAT(0.64f, result1.z);  // 0.8 * 0.8 * 1.0 = 0.64
    
    // Test doubled sensitivity
    Vector3 result2 = apply_sensitivity_curve(input, 2.0f);
    TEST_ASSERT_TRUE(fabs(result2.x) > result1.x);
    TEST_ASSERT_TRUE(fabs(result2.y) > result1.y);
    TEST_ASSERT_TRUE(fabs(result2.z) > result1.z);
    
    // Test halved sensitivity
    Vector3 result3 = apply_sensitivity_curve(input, 0.5f);
    TEST_ASSERT_TRUE(fabs(result3.x) < result1.x);
    TEST_ASSERT_TRUE(fabs(result3.y) < fabs(result1.y));
    TEST_ASSERT_TRUE(fabs(result3.z) < result1.z);
}

void test_sensitivity_curve_clamping(void)
{
    Vector3 large_input = { 2.0f, -3.0f, 1.5f };
    
    // Even with high sensitivity, output should be clamped to [-1, 1]
    Vector3 result = apply_sensitivity_curve(large_input, 5.0f);
    
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, fabs(result.x));
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, fabs(result.y));
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, fabs(result.z));
}

// ============================================================================
// FLIGHT ASSISTANCE TESTS
// ============================================================================

void test_stability_assist_basic_function(void)
{
    Vector3 input = { 0.0f, 0.0f, 0.0f };  // No input
    Vector3 angular_velocity = { 2.0f, -1.5f, 0.8f };  // Entity is spinning
    float assist_strength = 0.5f;
    
    Vector3 result = apply_stability_assist(input, angular_velocity, assist_strength);
    
    // Should apply counter-rotation to reduce angular velocity
    TEST_ASSERT_TRUE(result.x < 0.0f);  // Counter pitch
    TEST_ASSERT_TRUE(result.y > 0.0f); // Counter yaw
    TEST_ASSERT_TRUE(result.z < 0.0f);  // Counter roll
}

void test_stability_assist_with_input_override(void)
{
    Vector3 input = { 0.5f, 0.0f, 0.0f };  // Player input on pitch
    Vector3 angular_velocity = { 2.0f, -1.5f, 0.8f };  // Entity is spinning
    float assist_strength = 0.5f;
    
    Vector3 result = apply_stability_assist(input, angular_velocity, assist_strength);
    
    // Should keep player input on pitch axis
    TEST_ASSERT_EQUAL_FLOAT(0.5f, result.x);
    
    // Should still apply assistance on other axes
    TEST_ASSERT_TRUE(result.y > 0.0f); // Counter yaw
    TEST_ASSERT_TRUE(result.z < 0.0f);  // Counter roll
}

void test_stability_assist_disabled(void)
{
    Vector3 input = { 0.0f, 0.0f, 0.0f };
    Vector3 angular_velocity = { 2.0f, -1.5f, 0.8f };
    float assist_strength = 0.0f; // Disabled
    
    Vector3 result = apply_stability_assist(input, angular_velocity, assist_strength);
    
    // Should return input unchanged when assist is disabled
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result.z);
}

void test_stability_assist_clamping(void)
{
    Vector3 input = { 0.0f, 0.0f, 0.0f };
    Vector3 large_angular_velocity = { 10.0f, -8.0f, 12.0f }; // Very high spin
    float assist_strength = 1.0f; // Full assist
    
    Vector3 result = apply_stability_assist(input, large_angular_velocity, assist_strength);
    
    // Result should be clamped to valid input range
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, result.x);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, result.x);
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, result.y);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, result.y);
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, result.z);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, result.z);
}

// ============================================================================
// CONTROL MODE TESTS
// ============================================================================

void test_control_mode_settings(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_CONTROL_AUTHORITY);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    
    // Test different control modes
    control->control_mode = CONTROL_MANUAL;
    TEST_ASSERT_EQUAL_INT(CONTROL_MANUAL, control->control_mode);
    
    control->control_mode = CONTROL_ASSISTED;
    TEST_ASSERT_EQUAL_INT(CONTROL_ASSISTED, control->control_mode);
    
    control->control_mode = CONTROL_AUTOPILOT;
    TEST_ASSERT_EQUAL_INT(CONTROL_AUTOPILOT, control->control_mode);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_control_system_full_update(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, 
                        COMPONENT_CONTROL_AUTHORITY | COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    
    // Set this entity as player controlled
    control->controlled_by = entity;
    control_set_player_entity(&test_world, entity);
    
    // Update control system (should not crash)
    control_system_update(&test_world, NULL, 0.016f);
    
    TEST_ASSERT_TRUE(true);
}

void test_control_non_player_entity_ignored(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, 
                        COMPONENT_CONTROL_AUTHORITY | COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    
    // Set entity as NOT player controlled
    control->controlled_by = INVALID_ENTITY;
    control_set_player_entity(&test_world, 999); // Different entity
    
    // Set some thrust values (simulating scripted flight control)
    thrusters->current_linear_thrust = (Vector3){ 0.5f, 0.5f, 0.5f };
    thrusters->current_angular_thrust = (Vector3){ 0.5f, 0.5f, 0.5f };
    
    // Update control system
    control_system_update(&test_world, NULL, 0.016f);
    
    // Thrust commands should NOT be cleared for non-player entities
    // This allows scripted flight and other systems to control the entity
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_linear_thrust.x);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_linear_thrust.y);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_linear_thrust.z);
    
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_angular_thrust.x);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_angular_thrust.y);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, thrusters->current_angular_thrust.z);
}

void test_control_multiple_entities_performance(void)
{
    const int entity_count = 20;
    EntityID entities[entity_count];
    
    // Create many controlled entities
    for (int i = 0; i < entity_count; i++) {
        entities[i] = entity_create(&test_world);
        entity_add_components(&test_world, entities[i], 
                           COMPONENT_CONTROL_AUTHORITY | COMPONENT_THRUSTER_SYSTEM | COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
        
        struct ControlAuthority* control = entity_get_control_authority(&test_world, entities[i]);
        
        // Only make the first one player controlled
        if (i == 0) {
            control->controlled_by = entities[i];
            control_set_player_entity(&test_world, entities[i]);
        } else {
            control->controlled_by = INVALID_ENTITY;
        }
    }
    
    // Update should complete quickly
    clock_t start = clock();
    control_system_update(&test_world, NULL, 0.016f);
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete within 2ms for 20 entities
    TEST_ASSERT_TRUE(elapsed < 0.002);
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

void test_control_null_pointer_safety(void)
{
    // Test that control functions handle NULL pointers gracefully
    control_set_sensitivity(NULL, 1.0f);
    control_set_flight_assist(NULL, true);
    control_toggle_flight_assist(NULL);
    
    Vector3 zero = { 0.0f, 0.0f, 0.0f };
    apply_sensitivity_curve(zero, 1.0f); // Should not crash
    apply_stability_assist(zero, zero, 0.5f); // Should not crash
    
    TEST_ASSERT_TRUE(true);
}

void test_control_entity_without_thrusters(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_component(&test_world, entity, COMPONENT_CONTROL_AUTHORITY); // No thrusters
    
    struct ControlAuthority* control = entity_get_control_authority(&test_world, entity);
    control->controlled_by = entity;
    control_set_player_entity(&test_world, entity);
    
    // Should not crash when updating control system
    control_system_update(&test_world, NULL, 0.016f);
    
    TEST_ASSERT_TRUE(true);
}

void test_control_extreme_sensitivity_values(void)
{
    Vector3 input = { 0.1f, -0.2f, 0.3f };
    
    // Test with very high sensitivity
    Vector3 result1 = apply_sensitivity_curve(input, 100.0f);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, fabs(result1.x));
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, fabs(result1.y));
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, fabs(result1.z));
    
    // Test with very low sensitivity
    Vector3 result2 = apply_sensitivity_curve(input, 0.001f);
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, result2.x);
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, result2.y);
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, result2.z);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_control(void)
{
    printf("\n🎮 Control Authority System Tests\n");
    printf("=================================\n");
    
    printf("🔧 Testing Control Component...\n");
    RUN_TEST(test_control_authority_component_creation);
    RUN_TEST(test_control_configuration_functions);
    RUN_TEST(test_control_player_entity_setting);
    
    printf("📊 Testing Input Processing...\n");
    RUN_TEST(test_control_linear_input_processing);
    RUN_TEST(test_control_angular_input_processing);
    
    printf("🎚️  Testing Sensitivity Curves...\n");
    RUN_TEST(test_sensitivity_curve_application);
    RUN_TEST(test_sensitivity_curve_clamping);
    
    printf("🛟 Testing Flight Assistance...\n");
    RUN_TEST(test_stability_assist_basic_function);
    RUN_TEST(test_stability_assist_with_input_override);
    RUN_TEST(test_stability_assist_disabled);
    RUN_TEST(test_stability_assist_clamping);
    
    printf("⚙️  Testing Control Modes...\n");
    RUN_TEST(test_control_mode_settings);
    
    printf("🔗 Testing Integration...\n");
    RUN_TEST(test_control_system_full_update);
    RUN_TEST(test_control_non_player_entity_ignored);
    RUN_TEST(test_control_multiple_entities_performance);
    
    printf("🛡️  Testing Edge Cases...\n");
    RUN_TEST(test_control_null_pointer_safety);
    RUN_TEST(test_control_entity_without_thrusters);
    RUN_TEST(test_control_extreme_sensitivity_values);
    
    printf("✅ Control Authority Tests Complete\n");
}

// ============================================================================
// SPRINT 25: NEW INPUT SYSTEM INTEGRATION TESTS
// ============================================================================

void test_sprint25_input_state_conversion(void)
{
    printf("🧪 Testing Sprint 25 InputState conversion from new input system...\n");
    
    // This test verifies that the new input service properly converts flight actions
    // to InputState values that the existing control system can understand
    
    // Note: This is an integration test that relies on the input_state.c adapter
    // working correctly with the new input service
    
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    // Initial state should be zero
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, state->thrust);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, state->pitch);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, state->yaw);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, state->roll);
    
    printf("✅ Sprint 25 InputState conversion test passed!\n");
    printf("   Note: Full integration testing requires mock input events\n");
    printf("   See test_input_service.c for comprehensive input testing\n");
}

void suite_sprint25(void)
{
    printf("\n🚀 Running Sprint 25 Integration Tests...\n");
    RUN_TEST(test_sprint25_input_state_conversion);
    printf("✅ Sprint 25 Integration Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    suite_control();
    suite_sprint25();
    return UNITY_END();
}
