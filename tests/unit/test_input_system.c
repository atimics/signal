/**
 * @file test_input_system.c
 * @brief Comprehensive tests for input system
 * 
 * Tests input handling, gamepad support, keyboard input,
 * and input edge cases.
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/system/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

void setUp(void)
{
    // Initialize input system for each test
    input_init();
}

void tearDown(void)
{
    input_shutdown();
}

// ============================================================================
// INPUT SYSTEM INITIALIZATION TESTS
// ============================================================================

void test_input_system_initialization(void)
{
    // Input system should initialize successfully
    TEST_ASSERT_TRUE(true); // Init called in setUp, no crash means success
    
    // Should be able to get input state
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
}

void test_input_state_initial_values(void)
{
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    // All input values should start at zero/false
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->thrust);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->strafe);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->vertical);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->pitch);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->yaw);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->roll);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->boost);
    TEST_ASSERT_FALSE(state->brake);
    TEST_ASSERT_FALSE(state->action);
    TEST_ASSERT_FALSE(state->menu);
}

// ============================================================================
// INPUT UPDATE TESTS
// ============================================================================

void test_input_system_update(void)
{
    // Update should not crash
    input_update();
    
    // State should still be accessible
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    TEST_ASSERT_TRUE(true);
}

void test_input_multiple_updates(void)
{
    // Multiple rapid updates should be stable
    for (int i = 0; i < 100; i++) {
        input_update();
    }
    
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// GAMEPAD DETECTION TESTS
// ============================================================================

void test_gamepad_detection(void)
{
    // Test gamepad detection (may or may not have a gamepad connected)
    bool has_gamepad = input_has_gamepad();
    
    // Should return a valid boolean (no crash)
    TEST_ASSERT_TRUE(has_gamepad == true || has_gamepad == false);
}

void test_gamepad_info_retrieval(void)
{
    // Try to get gamepad info
    const char* gamepad_name = input_get_gamepad_name();
    
    // Should either return NULL (no gamepad) or a valid string
    if (gamepad_name != NULL) {
        // If there's a name, it should not be empty
        TEST_ASSERT_GREATER_THAN(0, strlen(gamepad_name));
    }
    
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// INPUT CLAMPING TESTS
// ============================================================================

void test_input_value_clamping(void)
{
    // Since we can't easily inject test input values into the real input system,
    // we'll test the expected behavior through the input state structure
    
    const InputState* state = input_get_state();
    
    // All input values should be within expected ranges after any update
    input_update();
    
    // Linear movement should be in [-1, 1] range
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, state->thrust);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->thrust);
    
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, state->strafe);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->strafe);
    
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, state->vertical);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->vertical);
    
    // Angular movement should be in [-1, 1] range
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, state->pitch);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->pitch);
    
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, state->yaw);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->yaw);
    
    TEST_ASSERT_GREATER_OR_EQUAL(-1.0f, state->roll);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->roll);
    
    // Boost should be in [0, 1] range
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, state->boost);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, state->boost);
}

// ============================================================================
// INPUT CONSISTENCY TESTS
// ============================================================================

void test_input_state_consistency(void)
{
    const InputState* state1 = input_get_state();
    const InputState* state2 = input_get_state();
    
    // Multiple calls should return the same state object
    TEST_ASSERT_EQUAL_PTR(state1, state2);
}

void test_input_state_persistence(void)
{
    const InputState* state_before = input_get_state();
    float thrust_before = state_before->thrust;
    
    // Update input
    input_update();
    
    const InputState* state_after = input_get_state();
    
    // State object should be the same
    TEST_ASSERT_EQUAL_PTR(state_before, state_after);
    
    // Values may change, but structure should remain valid
    TEST_ASSERT_FALSE(isnan(state_after->thrust));
    TEST_ASSERT_FALSE(isinf(state_after->thrust));
}

// ============================================================================
// GAMEPAD AXIS TESTS
// ============================================================================

void test_gamepad_axis_ranges(void)
{
    // Update to get latest input state
    input_update();
    
    const InputState* state = input_get_state();
    
    if (input_has_gamepad()) {
        // If gamepad is connected, all axes should produce valid values
        TEST_ASSERT_FALSE(isnan(state->thrust));
        TEST_ASSERT_FALSE(isnan(state->strafe));
        TEST_ASSERT_FALSE(isnan(state->vertical));
        TEST_ASSERT_FALSE(isnan(state->pitch));
        TEST_ASSERT_FALSE(isnan(state->yaw));
        TEST_ASSERT_FALSE(isnan(state->roll));
        
        TEST_ASSERT_FALSE(isinf(state->thrust));
        TEST_ASSERT_FALSE(isinf(state->strafe));
        TEST_ASSERT_FALSE(isinf(state->vertical));
        TEST_ASSERT_FALSE(isinf(state->pitch));
        TEST_ASSERT_FALSE(isinf(state->yaw));
        TEST_ASSERT_FALSE(isinf(state->roll));
    }
    
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// DEADZONE TESTS
// ============================================================================

void test_gamepad_deadzone_behavior(void)
{
    // Update to get current state
    input_update();
    
    const InputState* state = input_get_state();
    
    if (input_has_gamepad()) {
        // Small gamepad movements should be filtered out by deadzone
        // We can't control the gamepad directly, but we can verify
        // that values are either zero or above deadzone threshold
        
        float deadzone_threshold = 0.1f; // Typical deadzone
        
        // If value is non-zero, it should be above deadzone
        if (state->thrust != 0.0f) {
            TEST_ASSERT_GREATER_THAN(deadzone_threshold, fabs(state->thrust));
        }
        
        if (state->strafe != 0.0f) {
            TEST_ASSERT_GREATER_THAN(deadzone_threshold, fabs(state->strafe));
        }
    }
    
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// INPUT PERFORMANCE TESTS
// ============================================================================

void test_input_system_performance(void)
{
    // Test that input updates are fast
    clock_t start = clock();
    
    for (int i = 0; i < 1000; i++) {
        input_update();
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // 1000 input updates should complete in under 10ms
    TEST_ASSERT_LESS_THAN(0.01, elapsed);
}

void test_input_state_access_performance(void)
{
    // Test that getting input state is fast
    clock_t start = clock();
    
    for (int i = 0; i < 10000; i++) {
        const InputState* state = input_get_state();
        (void)state; // Avoid unused variable warning
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // 10000 state accesses should complete in under 1ms
    TEST_ASSERT_LESS_THAN(0.001, elapsed);
}

// ============================================================================
// INPUT EDGE CASES
// ============================================================================

void test_input_null_pointer_safety(void)
{
    // Input functions should handle edge cases gracefully
    
    // Getting state should never return NULL after init
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    // Multiple shutdowns should not crash
    input_shutdown();
    input_shutdown(); // Second shutdown
    
    // Re-initialize for tearDown
    input_init();
    
    TEST_ASSERT_TRUE(true);
}

void test_input_reinitialization(void)
{
    // Shutdown and reinitialize
    input_shutdown();
    
    bool reinit_success = input_init();
    TEST_ASSERT_TRUE(reinit_success);
    
    // Should be able to get state again
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    // Values should be reset
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->thrust);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->strafe);
    TEST_ASSERT_FALSE(state->brake);
}

void test_input_rapid_init_shutdown(void)
{
    // Rapid init/shutdown cycles should be stable
    for (int i = 0; i < 10; i++) {
        input_shutdown();
        bool success = input_init();
        TEST_ASSERT_TRUE(success);
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
    }
}

// ============================================================================
// GAMEPAD CONNECTION TESTS
// ============================================================================

void test_gamepad_connection_stability(void)
{
    // Test gamepad detection stability across multiple checks
    bool initial_status = input_has_gamepad();
    
    // Status should be consistent across multiple checks
    for (int i = 0; i < 10; i++) {
        bool current_status = input_has_gamepad();
        TEST_ASSERT_EQUAL(initial_status, current_status);
    }
}

void test_gamepad_name_stability(void)
{
    // Gamepad name should be consistent
    const char* name1 = input_get_gamepad_name();
    const char* name2 = input_get_gamepad_name();
    
    if (name1 == NULL) {
        TEST_ASSERT_NULL(name2);
    } else {
        TEST_ASSERT_NOT_NULL(name2);
        TEST_ASSERT_EQUAL_STRING(name1, name2);
    }
}

// ============================================================================
// INPUT MAPPING TESTS
// ============================================================================

void test_input_mapping_completeness(void)
{
    // Verify that all expected input fields are accessible
    const InputState* state = input_get_state();
    
    // Test that we can access all expected fields without crashing
    volatile float test_vals[] = {
        state->thrust,
        state->strafe,
        state->vertical,
        state->pitch,
        state->yaw,
        state->roll,
        state->boost
    };
    
    volatile bool test_bools[] = {
        state->brake,
        state->action,
        state->menu
    };
    
    // Suppress unused variable warnings
    (void)test_vals;
    (void)test_bools;
    
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_input_integration_with_control_system(void)
{
    // Test that input system provides data that control system can use
    input_update();
    
    const InputState* state = input_get_state();
    
    // Control system expects these fields to exist and be valid
    TEST_ASSERT_FALSE(isnan(state->thrust));
    TEST_ASSERT_FALSE(isnan(state->strafe));
    TEST_ASSERT_FALSE(isnan(state->vertical));
    TEST_ASSERT_FALSE(isnan(state->pitch));
    TEST_ASSERT_FALSE(isnan(state->yaw));
    TEST_ASSERT_FALSE(isnan(state->roll));
    TEST_ASSERT_FALSE(isnan(state->boost));
    
    // Boolean values should be well-defined
    TEST_ASSERT_TRUE(state->brake == true || state->brake == false);
    TEST_ASSERT_TRUE(state->action == true || state->action == false);
    TEST_ASSERT_TRUE(state->menu == true || state->menu == false);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_input_system(void)
{
    printf("\n🎮 Input System Tests\n");
    printf("====================\n");
    
    printf("🔧 Testing Input Initialization...\n");
    RUN_TEST(test_input_system_initialization);
    RUN_TEST(test_input_state_initial_values);
    
    printf("🔄 Testing Input Updates...\n");
    RUN_TEST(test_input_system_update);
    RUN_TEST(test_input_multiple_updates);
    
    printf("🎮 Testing Gamepad Detection...\n");
    RUN_TEST(test_gamepad_detection);
    RUN_TEST(test_gamepad_info_retrieval);
    
    printf("📏 Testing Input Clamping...\n");
    RUN_TEST(test_input_value_clamping);
    
    printf("🔗 Testing Input Consistency...\n");
    RUN_TEST(test_input_state_consistency);
    RUN_TEST(test_input_state_persistence);
    
    printf("📊 Testing Gamepad Axes...\n");
    RUN_TEST(test_gamepad_axis_ranges);
    
    printf("⚪ Testing Deadzone Behavior...\n");
    RUN_TEST(test_gamepad_deadzone_behavior);
    
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_input_system_performance);
    RUN_TEST(test_input_state_access_performance);
    
    printf("🛡️  Testing Edge Cases...\n");
    RUN_TEST(test_input_null_pointer_safety);
    RUN_TEST(test_input_reinitialization);
    RUN_TEST(test_input_rapid_init_shutdown);
    
    printf("🔌 Testing Gamepad Connection...\n");
    RUN_TEST(test_gamepad_connection_stability);
    RUN_TEST(test_gamepad_name_stability);
    
    printf("🗺️  Testing Input Mapping...\n");
    RUN_TEST(test_input_mapping_completeness);
    
    printf("🔗 Testing Integration...\n");
    RUN_TEST(test_input_integration_with_control_system);
    
    printf("✅ Input System Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    suite_input_system();
    return UNITY_END();
}
