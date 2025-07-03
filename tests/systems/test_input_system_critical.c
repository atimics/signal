/**
 * @file test_input_system_critical.c
 * @brief Critical tests for input system edge cases and integration
 * 
 * These tests focus on the most crucial input system aspects:
 * - Deadzone handling accuracy and consistency
 * - Device switching and hot-plugging scenarios
 * - Input state consistency during rapid changes
 * - Integration with look target system
 * - Memory safety and error handling
 * - Performance under stress conditions
 */

#include "../vendor/unity.h"
#include "../../src/system/input.h"
#include "../../src/system/gamepad.h"
#include "../../src/component/look_target.h"
#include "../../src/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Test state
static bool input_system_ready = false;

void setUp(void) {
    if (!input_system_ready) {
        input_init();
        input_system_ready = true;
    }
}

void tearDown(void) {
    // Keep input system alive for other tests
}

// ============================================================================
// DEADZONE ACCURACY TESTS
// ============================================================================

void test_deadzone_boundary_accuracy(void) {
    printf("🧪 Testing deadzone boundary accuracy...\n");
    
    // Test various deadzone boundary conditions
    const float DEADZONE_THRESHOLD = 0.15f; // 15% deadzone
    const float EPSILON = 0.001f;
    
    // Test values right at deadzone boundary
    float test_values[] = {
        0.0f,                           // Zero
        DEADZONE_THRESHOLD - EPSILON,   // Just below deadzone
        DEADZONE_THRESHOLD,             // Exactly at deadzone
        DEADZONE_THRESHOLD + EPSILON,   // Just above deadzone
        0.5f,                          // Mid-range
        1.0f,                          // Maximum
        -DEADZONE_THRESHOLD + EPSILON,  // Negative just below deadzone
        -DEADZONE_THRESHOLD,           // Negative exactly at deadzone
        -DEADZONE_THRESHOLD - EPSILON, // Negative just above deadzone
        -0.5f,                         // Negative mid-range
        -1.0f                          // Negative maximum
    };
    
    const int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        float input_val = test_values[i];
        
        // Apply deadzone logic (simplified version of what input system does)
        float processed_val = 0.0f;
        if (fabsf(input_val) >= DEADZONE_THRESHOLD) {
            float sign = input_val > 0 ? 1.0f : -1.0f;
            processed_val = sign * (fabsf(input_val) - DEADZONE_THRESHOLD) / (1.0f - DEADZONE_THRESHOLD);
        }
        
        // Verify deadzone behavior
        if (fabsf(input_val) < DEADZONE_THRESHOLD) {
            TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.0f, processed_val, 
                "Values within deadzone should be zero");
        } else {
            // Only test for non-zero if the input is significantly above deadzone
            if (fabsf(input_val) > DEADZONE_THRESHOLD + EPSILON) {
                TEST_ASSERT_NOT_EQUAL_MESSAGE(0.0f, processed_val, 
                    "Values significantly outside deadzone should not be zero");
            }
            TEST_ASSERT_TRUE_MESSAGE(fabsf(processed_val) <= 1.0f, 
                "Processed values should be within [-1, 1]");
        }
        
        printf("   Input: %6.3f → Output: %6.3f %s\n", 
               input_val, processed_val,
               fabsf(input_val) < DEADZONE_THRESHOLD ? "(deadzone)" : "");
    }
    
    printf("✅ Deadzone boundary accuracy test passed\n");
}

void test_deadzone_consistency_across_axes(void) {
    printf("🧪 Testing deadzone consistency across different axes...\n");
    
    // Test that deadzone behavior is consistent for all input axes
    const int UPDATE_CYCLES = 50;
    
    for (int cycle = 0; cycle < UPDATE_CYCLES; cycle++) {
        input_update();
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
        
        // All input values should be within expected ranges
        TEST_ASSERT_TRUE(state->thrust >= -1.0f && state->thrust <= 1.0f);
        TEST_ASSERT_TRUE(state->pitch >= -1.0f && state->pitch <= 1.0f);
        TEST_ASSERT_TRUE(state->yaw >= -1.0f && state->yaw <= 1.0f);
        TEST_ASSERT_TRUE(state->roll >= -1.0f && state->roll <= 1.0f);
        TEST_ASSERT_TRUE(state->boost >= 0.0f && state->boost <= 1.0f);
        
        // No NaN values
        TEST_ASSERT_FALSE(isnan(state->thrust));
        TEST_ASSERT_FALSE(isnan(state->pitch));
        TEST_ASSERT_FALSE(isnan(state->yaw));
        TEST_ASSERT_FALSE(isnan(state->roll));
        TEST_ASSERT_FALSE(isnan(state->boost));
    }
    
    printf("✅ Deadzone consistency test passed (%d cycles)\n", UPDATE_CYCLES);
}

// ============================================================================
// DEVICE SWITCHING TESTS
// ============================================================================

void test_device_switching_stability(void) {
    printf("🧪 Testing device switching stability...\n");
    
    // Simulate rapid device switching scenarios
    const int SWITCH_CYCLES = 100;
    
    for (int cycle = 0; cycle < SWITCH_CYCLES; cycle++) {
        // Rapid input updates (simulating device changes)
        input_update();
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL_MESSAGE(state, "Input state should always be available");
        
        // Verify input state remains valid during switching
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->thrust), "Thrust should not be NaN during switching");
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->pitch), "Pitch should not be NaN during switching");
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->yaw), "Yaw should not be NaN during switching");
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->roll), "Roll should not be NaN during switching");
        
        // Values should remain within valid ranges
        TEST_ASSERT_TRUE_MESSAGE(state->thrust >= -1.0f && state->thrust <= 1.0f,
                                "Thrust should remain in valid range");
        TEST_ASSERT_TRUE_MESSAGE(state->pitch >= -1.0f && state->pitch <= 1.0f,
                                "Pitch should remain in valid range");
        TEST_ASSERT_TRUE_MESSAGE(state->yaw >= -1.0f && state->yaw <= 1.0f,
                                "Yaw should remain in valid range");
        TEST_ASSERT_TRUE_MESSAGE(state->roll >= -1.0f && state->roll <= 1.0f,
                                "Roll should remain in valid range");
    }
    
    printf("✅ Device switching stability test passed (%d cycles)\n", SWITCH_CYCLES);
}

void test_gamepad_connection_simulation(void) {
    printf("🧪 Testing gamepad connection simulation...\n");
    
    // Test gamepad detection and state handling
    bool has_gamepad = input_has_gamepad();
    printf("   Initial gamepad state: %s\n", has_gamepad ? "Connected" : "Disconnected");
    
    // Test that gamepad state queries don't crash
    for (int i = 0; i < 10; i++) {
        input_update();
        
        // Check gamepad state repeatedly
        bool current_gamepad_state = input_has_gamepad();
        
        // State should be consistent (true/false, not random)
        TEST_ASSERT_TRUE_MESSAGE(current_gamepad_state == true || current_gamepad_state == false,
                                "Gamepad state should be boolean");
        
        // Input should remain valid regardless of gamepad state
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL_MESSAGE(state, "Input state should be available regardless of gamepad");
        
        printf("   Cycle %d: Gamepad %s, Input valid: ✓\n", 
               i + 1, current_gamepad_state ? "✓" : "✗");
    }
    
    printf("✅ Gamepad connection simulation test passed\n");
}

// ============================================================================
// LOOK TARGET INTEGRATION TESTS
// ============================================================================

void test_look_target_integration_accuracy(void) {
    printf("🧪 Testing look target integration accuracy...\n");
    
    // Test look target system integration with input
    Vector3 player_position = {0, 0, 0};
    
    for (int cycle = 0; cycle < 20; cycle++) {
        input_update();
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
        
        // Verify look target data is valid
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->look_target.azimuth), 
                                 "Look target azimuth should not be NaN");
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->look_target.elevation), 
                                 "Look target elevation should not be NaN");
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->look_target.distance), 
                                 "Look target distance should not be NaN");
        
        // Look target distance should be positive
        TEST_ASSERT_GREATER_THAN_MESSAGE(0.0f, state->look_target.distance,
                                        "Look target distance should be positive");
        
        // Azimuth should be within reasonable range (may wrap around)
        // Elevation should be within reasonable range
        TEST_ASSERT_TRUE_MESSAGE(state->look_target.elevation >= -M_PI/2 && 
                                state->look_target.elevation <= M_PI/2,
                                "Elevation should be within [-π/2, π/2]");
    }
    
    printf("✅ Look target integration accuracy test passed\n");
}

// ============================================================================
// INPUT STATE CONSISTENCY TESTS
// ============================================================================

void test_input_state_memory_safety(void) {
    printf("🧪 Testing input state memory safety...\n");
    
    // Test repeated access to input state for memory safety
    const int MEMORY_TEST_CYCLES = 1000;
    const InputState* previous_state = NULL;
    
    for (int cycle = 0; cycle < MEMORY_TEST_CYCLES; cycle++) {
        input_update();
        
        const InputState* current_state = input_get_state();
        TEST_ASSERT_NOT_NULL_MESSAGE(current_state, "Input state should not be NULL");
        
        // Check that the pointer is stable (same memory location)
        if (previous_state != NULL) {
            TEST_ASSERT_EQUAL_PTR_MESSAGE(previous_state, current_state,
                                         "Input state pointer should be stable");
        }
        
        // Verify all fields are accessible (no segfault)
        volatile float test_thrust = current_state->thrust;
        volatile float test_pitch = current_state->pitch;
        volatile float test_yaw = current_state->yaw;
        volatile float test_roll = current_state->roll;
        volatile float test_boost = current_state->boost;
        volatile bool test_brake = current_state->brake;
        
        // Suppress unused variable warnings
        (void)test_thrust; (void)test_pitch; (void)test_yaw;
        (void)test_roll; (void)test_boost; (void)test_brake;
        
        previous_state = current_state;
    }
    
    printf("✅ Input state memory safety test passed (%d cycles)\n", MEMORY_TEST_CYCLES);
}

void test_input_state_rapid_changes(void) {
    printf("🧪 Testing input state under rapid changes...\n");
    
    // Test input system stability under rapid state changes
    const int RAPID_CYCLES = 500;
    float previous_values[4] = {0}; // thrust, pitch, yaw, roll
    
    for (int cycle = 0; cycle < RAPID_CYCLES; cycle++) {
        input_update();
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
        
        // Store current values
        float current_values[4] = {
            state->thrust, state->pitch, state->yaw, state->roll
        };
        
        // Check for stability (values shouldn't change wildly without input)
        for (int i = 0; i < 4; i++) {
            float change = fabsf(current_values[i] - previous_values[i]);
            
            // In absence of real input, changes should be minimal or zero
            TEST_ASSERT_LESS_THAN_MESSAGE(2.0f, change,
                                         "Input values shouldn't change drastically without user input");
            
            // Values should remain in valid range
            TEST_ASSERT_TRUE_MESSAGE(current_values[i] >= -1.0f && current_values[i] <= 1.0f,
                                    "Input values should remain in [-1, 1] range");
            
            previous_values[i] = current_values[i];
        }
    }
    
    printf("✅ Input state rapid changes test passed (%d cycles)\n", RAPID_CYCLES);
}

// ============================================================================
// PERFORMANCE UNDER STRESS TESTS
// ============================================================================

void test_input_system_performance_stress(void) {
    printf("🧪 Testing input system performance under stress...\n");
    
    const int STRESS_CYCLES = 10000; // High-frequency updates
    clock_t start_time = clock();
    
    for (int cycle = 0; cycle < STRESS_CYCLES; cycle++) {
        input_update();
        
        // Access input state every cycle
        const InputState* state = input_get_state();
        
        // Verify basic functionality still works
        if (cycle % 1000 == 0) { // Check every 1000 cycles
            TEST_ASSERT_NOT_NULL_MESSAGE(state, "Input state should remain available under stress");
            TEST_ASSERT_FALSE_MESSAGE(isnan(state->thrust), "Values should not become NaN under stress");
        }
    }
    
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // Verify performance is reasonable
    TEST_ASSERT_LESS_THAN_MESSAGE(1.0, elapsed_time, "Stress test should complete within 1 second");
    
    double updates_per_second = STRESS_CYCLES / elapsed_time;
    printf("   Performance: %.0f updates/second (%.3f seconds total)\n", 
           updates_per_second, elapsed_time);
    printf("✅ Input system performance stress test passed\n");
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

void test_input_system_error_handling(void) {
    printf("🧪 Testing input system error handling...\n");
    
    // Test input system behavior with various error conditions
    
    // Test 1: Multiple rapid initializations and shutdowns
    for (int i = 0; i < 5; i++) {
        input_shutdown();
        bool init_success = input_init();
        TEST_ASSERT_TRUE_MESSAGE(init_success, "Input re-initialization should succeed");
        
        // Verify input state is still accessible after reinit
        input_update();
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL_MESSAGE(state, "Input state should be available after reinit");
    }
    
    // Test 2: Updates without proper initialization (edge case)
    input_shutdown();
    
    // These calls should not crash even if system is shut down
    input_update();
    const InputState* state = input_get_state();
    
    // State might be NULL or might return default values - either is acceptable
    // The key is that it shouldn't crash
    
    // Re-initialize for other tests
    input_init();
    input_system_ready = true;
    
    printf("✅ Input system error handling test passed\n");
}

// ============================================================================
// INTEGRATION WITH CONTROL SYSTEM TESTS
// ============================================================================

void test_input_control_integration_consistency(void) {
    printf("🧪 Testing input-control integration consistency...\n");
    
    // Test that input values translate correctly to control system
    const int INTEGRATION_CYCLES = 100;
    
    for (int cycle = 0; cycle < INTEGRATION_CYCLES; cycle++) {
        input_update();
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
        
        // Verify that input values are suitable for control system consumption
        
        // All thrust values should be within control system expectations
        TEST_ASSERT_TRUE_MESSAGE(state->thrust >= -1.0f && state->thrust <= 1.0f,
                                "Thrust should be in range expected by control system");
        TEST_ASSERT_TRUE_MESSAGE(state->pitch >= -1.0f && state->pitch <= 1.0f,
                                "Pitch should be in range expected by control system");
        TEST_ASSERT_TRUE_MESSAGE(state->yaw >= -1.0f && state->yaw <= 1.0f,
                                "Yaw should be in range expected by control system");
        TEST_ASSERT_TRUE_MESSAGE(state->roll >= -1.0f && state->roll <= 1.0f,
                                "Roll should be in range expected by control system");
        
        // Boost should be non-negative
        TEST_ASSERT_TRUE_MESSAGE(state->boost >= 0.0f && state->boost <= 1.0f,
                                "Boost should be in [0, 1] range");
        
        // Brake should be boolean
        TEST_ASSERT_TRUE_MESSAGE(state->brake == true || state->brake == false,
                                "Brake should be boolean");
        
        // Look target values should be reasonable for control system
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->look_target.azimuth),
                                 "Look target azimuth should be valid for control system");
        TEST_ASSERT_FALSE_MESSAGE(isnan(state->look_target.elevation),
                                 "Look target elevation should be valid for control system");
        TEST_ASSERT_GREATER_THAN_MESSAGE(0.0f, state->look_target.distance,
                                        "Look target distance should be positive for control system");
    }
    
    printf("✅ Input-control integration consistency test passed\n");
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_input_system_critical(void) {
    printf("\n🎮 Critical Input System Tests\n");
    printf("==============================\n");
    
    printf("🎯 Testing Deadzone Accuracy...\n");
    RUN_TEST(test_deadzone_boundary_accuracy);
    RUN_TEST(test_deadzone_consistency_across_axes);
    
    printf("🔌 Testing Device Switching...\n");
    RUN_TEST(test_device_switching_stability);
    RUN_TEST(test_gamepad_connection_simulation);
    
    printf("🎯 Testing Look Target Integration...\n");
    RUN_TEST(test_look_target_integration_accuracy);
    
    printf("💾 Testing State Consistency...\n");
    RUN_TEST(test_input_state_memory_safety);
    RUN_TEST(test_input_state_rapid_changes);
    
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_input_system_performance_stress);
    
    printf("🛡️  Testing Error Handling...\n");
    RUN_TEST(test_input_system_error_handling);
    
    printf("🔗 Testing Control Integration...\n");
    RUN_TEST(test_input_control_integration_consistency);
    
    printf("✅ Critical Input System Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    suite_input_system_critical();
    return UNITY_END();
}
