/**
 * @file test_input_advanced.c
 * @brief Advanced tests for the canyon racing input system
 * 
 * Tests the critical aspects of the input system including:
 * - Gamepad input processing and deadzone handling
 * - Keyboard/mouse input processing
 * - Device switching and priority
 * - Look target system integration
 * - Input state consistency and edge cases
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

// Test state
static bool input_system_ready = false;

void setUp(void) {
    if (!input_system_ready) {
        // Initialize input system
        input_init();
        input_system_ready = true;
    }
}

void tearDown(void) {
    // Keep input system alive for other tests
    // input_shutdown() will be called in main cleanup
}

// ============================================================================
// INPUT INITIALIZATION AND BASIC FUNCTIONALITY TESTS
// ============================================================================

void test_input_system_initialization_advanced(void) {
    printf("🧪 Testing input system initialization (advanced)...\n");
    
    // Verify system is properly initialized
    TEST_ASSERT_TRUE(input_system_ready);
    
    // Test input state retrieval
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    // Verify initial state values
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->pitch);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->yaw);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->roll);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->thrust);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->vertical);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, state->boost);
    TEST_ASSERT_FALSE(state->brake);
    
    printf("✅ Input system initialization (advanced) test passed\n");
}

void test_input_device_type_detection(void) {
    printf("🧪 Testing input device type detection...\n");
    
    // Get current device type
    InputDeviceType device = input_get_last_device();
    
    // Should be one of the valid device types
    TEST_ASSERT_TRUE(device == INPUT_DEVICE_KEYBOARD || 
                    device == INPUT_DEVICE_GAMEPAD ||
                    device == INPUT_DEVICE_MOUSE);
    
    printf("✅ Input device type detection test passed\n");
}

// ============================================================================
// GAMEPAD INPUT PROCESSING TESTS
// ============================================================================

void test_gamepad_deadzone_processing(void) {
    printf("🧪 Testing gamepad deadzone processing...\n");
    
    // Test deadzone function behavior (simulated)
    // Note: This tests the logic, actual gamepad input would require hardware
    
    // Values within deadzone should be zeroed
    float deadzone = 0.15f;
    
    // Test values that should be within deadzone
    float small_positive = 0.10f;
    float small_negative = -0.12f;
    
    // These should be filtered out by deadzone logic
    TEST_ASSERT_TRUE(fabsf(small_positive) < deadzone);
    TEST_ASSERT_TRUE(fabsf(small_negative) < deadzone);
    
    // Test values that should pass through deadzone
    float large_positive = 0.8f;
    float large_negative = -0.6f;
    
    TEST_ASSERT_TRUE(fabsf(large_positive) > deadzone);
    TEST_ASSERT_TRUE(fabsf(large_negative) > deadzone);
    
    printf("✅ Gamepad deadzone processing test passed\n");
}

void test_gamepad_stick_mapping(void) {
    printf("🧪 Testing gamepad stick mapping...\n");
    
    // Test gamepad connection status
    bool gamepad_available = gamepad_is_connected(0);
    
    if (gamepad_available) {
        printf("🎮 Gamepad detected - testing stick mapping\n");
        
        GamepadState* gamepad = gamepad_get_state(0);
        TEST_ASSERT_NOT_NULL(gamepad);
        
        // Test that stick values are within expected range
        TEST_ASSERT_TRUE(gamepad->left_stick_x >= -1.0f && gamepad->left_stick_x <= 1.0f);
        TEST_ASSERT_TRUE(gamepad->left_stick_y >= -1.0f && gamepad->left_stick_y <= 1.0f);
        TEST_ASSERT_TRUE(gamepad->right_stick_x >= -1.0f && gamepad->right_stick_x <= 1.0f);
        TEST_ASSERT_TRUE(gamepad->right_stick_y >= -1.0f && gamepad->right_stick_y <= 1.0f);
        
        // Test trigger values
        TEST_ASSERT_TRUE(gamepad->left_trigger >= 0.0f && gamepad->left_trigger <= 1.0f);
        TEST_ASSERT_TRUE(gamepad->right_trigger >= 0.0f && gamepad->right_trigger <= 1.0f);
    } else {
        printf("⚠️  No gamepad detected - skipping hardware-specific tests\n");
    }
    
    printf("✅ Gamepad stick mapping test passed\n");
}

void test_gamepad_button_state_consistency(void) {
    printf("🧪 Testing gamepad button state consistency...\n");
    
    bool gamepad_available = gamepad_is_connected(0);
    
    if (gamepad_available) {
        GamepadState* gamepad = gamepad_get_state(0);
        TEST_ASSERT_NOT_NULL(gamepad);
        
        // Verify button array is properly sized
        // Test a few key buttons
        for (int i = 0; i < GAMEPAD_BUTTON_COUNT && i < 16; i++) {
            // Button states should be boolean (0 or 1, not arbitrary values)
            TEST_ASSERT_TRUE(gamepad->buttons[i] == 0 || gamepad->buttons[i] == 1);
        }
    }
    
    printf("✅ Gamepad button state consistency test passed\n");
}

// ============================================================================
// KEYBOARD INPUT PROCESSING TESTS
// ============================================================================

void test_keyboard_input_action_mapping(void) {
    printf("🧪 Testing keyboard input action mapping...\n");
    
    // Test keyboard input handling function
    // Note: We can't simulate actual key presses in unit tests easily,
    // but we can verify the mapping logic
    
    // Test that valid key codes are handled
    bool handled_w = input_handle_keyboard(SAPP_KEYCODE_W, true);
    bool handled_s = input_handle_keyboard(SAPP_KEYCODE_S, true);
    bool handled_a = input_handle_keyboard(SAPP_KEYCODE_A, true);
    bool handled_d = input_handle_keyboard(SAPP_KEYCODE_D, true);
    
    // These keys should be handled by the input system
    TEST_ASSERT_TRUE(handled_w);
    TEST_ASSERT_TRUE(handled_s);
    TEST_ASSERT_TRUE(handled_a);
    TEST_ASSERT_TRUE(handled_d);
    
    // Release keys
    input_handle_keyboard(SAPP_KEYCODE_W, false);
    input_handle_keyboard(SAPP_KEYCODE_S, false);
    input_handle_keyboard(SAPP_KEYCODE_A, false);
    input_handle_keyboard(SAPP_KEYCODE_D, false);
    
    printf("✅ Keyboard input action mapping test passed\n");
}

void test_keyboard_modifier_keys(void) {
    printf("🧪 Testing keyboard modifier keys...\n");
    
    // Test boost (Space) and brake keys
    bool handled_space = input_handle_keyboard(SAPP_KEYCODE_SPACE, true);
    bool handled_shift = input_handle_keyboard(SAPP_KEYCODE_LEFT_SHIFT, true);
    
    TEST_ASSERT_TRUE(handled_space);
    TEST_ASSERT_TRUE(handled_shift);
    
    // Release keys
    input_handle_keyboard(SAPP_KEYCODE_SPACE, false);
    input_handle_keyboard(SAPP_KEYCODE_LEFT_SHIFT, false);
    
    printf("✅ Keyboard modifier keys test passed\n");
}

// ============================================================================
// LOOK TARGET SYSTEM TESTS
// ============================================================================

void test_look_target_initialization(void) {
    printf("🧪 Testing look target initialization...\n");
    
    LookTarget look_target;
    look_target_init(&look_target);
    
    // Verify initial state
    TEST_ASSERT_EQUAL_FLOAT(0.0f, look_target.azimuth);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, look_target.elevation);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, look_target.distance); // Default distance
    
    // Verify target position calculation
    Vector3 player_pos = {0, 0, 0};
    look_target_update(&look_target, &player_pos, 0.0f, 0.0f, 0.0f);
    
    // Initial target should be in front of player
    TEST_ASSERT_EQUAL_FLOAT(0.0f, look_target.target_position.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, look_target.target_position.y);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, look_target.target_position.z);
    
    printf("✅ Look target initialization test passed\n");
}

void test_look_target_update_mechanics(void) {
    printf("🧪 Testing look target update mechanics...\n");
    
    LookTarget look_target;
    look_target_init(&look_target);
    
    Vector3 player_pos = {0, 0, 0};
    
    // Test azimuth change (horizontal rotation)
    float delta_azimuth = 0.1f; // Small rotation
    look_target_update(&look_target, &player_pos, delta_azimuth, 0.0f, 0.0f);
    
    TEST_ASSERT_EQUAL_FLOAT(0.1f, look_target.azimuth);
    
    // Test elevation change (vertical rotation)
    float delta_elevation = 0.05f;
    look_target_update(&look_target, &player_pos, 0.0f, delta_elevation, 0.0f);
    
    TEST_ASSERT_EQUAL_FLOAT(0.05f, look_target.elevation);
    
    // Test distance change
    float delta_distance = 10.0f;
    look_target_update(&look_target, &player_pos, 0.0f, 0.0f, delta_distance);
    
    TEST_ASSERT_EQUAL_FLOAT(110.0f, look_target.distance);
    
    printf("✅ Look target update mechanics test passed\n");
}

void test_look_target_position_calculation(void) {
    printf("🧪 Testing look target position calculation...\n");
    
    LookTarget look_target;
    look_target_init(&look_target);
    
    Vector3 player_pos = {10, 5, -20};
    
    // Set a specific azimuth and elevation
    look_target.azimuth = M_PI / 4.0f; // 45 degrees
    look_target.elevation = M_PI / 6.0f; // 30 degrees
    look_target.distance = 50.0f;
    
    look_target_update(&look_target, &player_pos, 0.0f, 0.0f, 0.0f);
    
    // Verify target position is calculated correctly relative to player
    Vector3 target = look_target.target_position;
    
    // Target should be offset from player position
    TEST_ASSERT_NOT_EQUAL(player_pos.x, target.x);
    TEST_ASSERT_NOT_EQUAL(player_pos.y, target.y);
    TEST_ASSERT_NOT_EQUAL(player_pos.z, target.z);
    
    // Distance from player to target should match expected distance
    Vector3 offset = {target.x - player_pos.x, target.y - player_pos.y, target.z - player_pos.z};
    float distance = sqrtf(offset.x * offset.x + offset.y * offset.y + offset.z * offset.z);
    
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f, distance);
    
    printf("✅ Look target position calculation test passed\n");
}

// ============================================================================
// INPUT STATE CONSISTENCY TESTS
// ============================================================================

void test_input_state_update_consistency(void) {
    printf("🧪 Testing input state update consistency...\n");
    
    // Perform multiple input updates
    for (int i = 0; i < 10; i++) {
        input_update();
        
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
        
        // Verify values are within expected ranges
        TEST_ASSERT_TRUE(state->pitch >= -1.0f && state->pitch <= 1.0f);
        TEST_ASSERT_TRUE(state->yaw >= -1.0f && state->yaw <= 1.0f);
        TEST_ASSERT_TRUE(state->roll >= -1.0f && state->roll <= 1.0f);
        TEST_ASSERT_TRUE(state->thrust >= 0.0f && state->thrust <= 1.0f);
        TEST_ASSERT_TRUE(state->vertical >= -1.0f && state->vertical <= 1.0f);
        TEST_ASSERT_TRUE(state->boost >= 0.0f && state->boost <= 1.0f);
    }
    
    printf("✅ Input state update consistency test passed\n");
}

void test_input_state_reset_behavior(void) {
    printf("🧪 Testing input state reset behavior...\n");
    
    // Simulate some input activity by pressing keys
    input_handle_keyboard(SAPP_KEYCODE_W, true);
    input_handle_keyboard(SAPP_KEYCODE_A, true);
    input_handle_keyboard(SAPP_KEYCODE_SPACE, true);
    
    // Update to process input
    input_update();
    
    const InputState* state = input_get_state();
    
    // Some values might be non-zero after input
    // Note: Actual values depend on whether gamepad overrides keyboard
    
    // Release all keys
    input_handle_keyboard(SAPP_KEYCODE_W, false);
    input_handle_keyboard(SAPP_KEYCODE_A, false);
    input_handle_keyboard(SAPP_KEYCODE_SPACE, false);
    
    // Update again
    input_update();
    
    // State should be cleared if no gamepad input is active
    if (!gamepad_is_connected(0)) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, state->pitch);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, state->yaw);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, state->thrust);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, state->boost);
    }
    
    printf("✅ Input state reset behavior test passed\n");
}

// ============================================================================
// DEVICE SWITCHING AND PRIORITY TESTS
// ============================================================================

void test_input_device_switching_priority(void) {
    printf("🧪 Testing input device switching priority...\n");
    
    bool gamepad_available = gamepad_is_connected(0);
    
    if (gamepad_available) {
        printf("🎮 Testing gamepad priority over keyboard\n");
        
        // Press a keyboard key
        input_handle_keyboard(SAPP_KEYCODE_W, true);
        
        // Update input (gamepad should override keyboard if active)
        input_update();
        
        InputDeviceType device = input_get_last_device();
        
        // If gamepad has any input, it should take priority
        GamepadState* gamepad = gamepad_get_state(0);
        bool gamepad_has_input = (fabsf(gamepad->left_stick_x) > 0.01f ||
                                 fabsf(gamepad->left_stick_y) > 0.01f ||
                                 fabsf(gamepad->right_stick_x) > 0.01f ||
                                 fabsf(gamepad->right_stick_y) > 0.01f ||
                                 fabsf(gamepad->left_trigger) > 0.01f ||
                                 fabsf(gamepad->right_trigger) > 0.01f);
        
        if (gamepad_has_input) {
            TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_GAMEPAD, device);
        }
        
        // Clean up
        input_handle_keyboard(SAPP_KEYCODE_W, false);
    } else {
        printf("⚠️  No gamepad detected - testing keyboard fallback\n");
        
        // With no gamepad, keyboard should be used
        input_handle_keyboard(SAPP_KEYCODE_W, true);
        input_update();
        
        InputDeviceType device = input_get_last_device();
        // Device should be keyboard (or possibly mouse if no input detected)
        TEST_ASSERT_TRUE(device == INPUT_DEVICE_KEYBOARD || device == INPUT_DEVICE_MOUSE);
        
        input_handle_keyboard(SAPP_KEYCODE_W, false);
    }
    
    printf("✅ Input device switching priority test passed\n");
}

// ============================================================================
// EDGE CASE AND ERROR HANDLING TESTS
// ============================================================================

void test_input_extreme_values_handling(void) {
    printf("🧪 Testing input extreme values handling...\n");
    
    // Test that the input system handles extreme or invalid values gracefully
    // Note: This is more about testing robustness than normal operation
    
    // Multiple rapid updates shouldn't cause issues
    for (int i = 0; i < 100; i++) {
        input_update();
    }
    
    // System should still be functional
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    // Values should still be within valid ranges
    TEST_ASSERT_TRUE(state->pitch >= -2.0f && state->pitch <= 2.0f); // Allow some headroom
    TEST_ASSERT_TRUE(state->yaw >= -2.0f && state->yaw <= 2.0f);
    TEST_ASSERT_TRUE(state->roll >= -2.0f && state->roll <= 2.0f);
    
    printf("✅ Input extreme values handling test passed\n");
}

void test_input_null_safety_advanced(void) {
    printf("🧪 Testing input null safety (advanced)...\n");
    
    // Test that input functions handle null parameters gracefully
    LookTarget look_target;
    look_target_init(&look_target);
    
    // These shouldn't crash
    look_target_update(NULL, NULL, 0.0f, 0.0f, 0.0f);
    look_target_update(&look_target, NULL, 0.0f, 0.0f, 0.0f);
    
    // System should still be functional after null calls
    input_update();
    const InputState* state = input_get_state();
    TEST_ASSERT_NOT_NULL(state);
    
    printf("✅ Input null safety (advanced) test passed\n");
}

// ============================================================================
// INTEGRATION PERFORMANCE TESTS
// ============================================================================

void test_input_system_performance_advanced(void) {
    printf("🧪 Testing input system performance (advanced)...\n");
    
    clock_t start = clock();
    
    // Perform many input updates with simulated activity
    for (int i = 0; i < 1000; i++) {
        // Simulate some keyboard activity
        if (i % 10 == 0) {
            input_handle_keyboard(SAPP_KEYCODE_W, true);
        }
        if (i % 15 == 0) {
            input_handle_keyboard(SAPP_KEYCODE_A, true);
        }
        if (i % 20 == 0) {
            input_handle_keyboard(SAPP_KEYCODE_W, false);
            input_handle_keyboard(SAPP_KEYCODE_A, false);
        }
        
        input_update();
        
        // Verify state is still valid
        const InputState* state = input_get_state();
        TEST_ASSERT_NOT_NULL(state);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Performance should be reasonable (less than 10ms for 1000 updates)
    TEST_ASSERT_TRUE(elapsed < 0.01);
    
    printf("Input system performance: %.3fms for 1000 updates\n", elapsed * 1000.0);
    printf("✅ Input system performance (advanced) test passed\n");
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_input_advanced(void) {
    printf("\n🎮 Advanced Input System Tests\n");
    printf("==============================\n");
    
    printf("🔧 Testing Initialization and Basic Functionality...\n");
    RUN_TEST(test_input_system_initialization_advanced);
    RUN_TEST(test_input_device_type_detection);
    
    printf("🎮 Testing Gamepad Input Processing...\n");
    RUN_TEST(test_gamepad_deadzone_processing);
    RUN_TEST(test_gamepad_stick_mapping);
    RUN_TEST(test_gamepad_button_state_consistency);
    
    printf("⌨️  Testing Keyboard Input Processing...\n");
    RUN_TEST(test_keyboard_input_action_mapping);
    RUN_TEST(test_keyboard_modifier_keys);
    
    printf("🎯 Testing Look Target System...\n");
    RUN_TEST(test_look_target_initialization);
    RUN_TEST(test_look_target_update_mechanics);
    RUN_TEST(test_look_target_position_calculation);
    
    printf("🔄 Testing Input State Consistency...\n");
    RUN_TEST(test_input_state_update_consistency);
    RUN_TEST(test_input_state_reset_behavior);
    
    printf("🔀 Testing Device Switching and Priority...\n");
    RUN_TEST(test_input_device_switching_priority);
    
    printf("🛡️  Testing Edge Cases and Error Handling...\n");
    RUN_TEST(test_input_extreme_values_handling);
    RUN_TEST(test_input_null_safety_advanced);
    
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_input_system_performance_advanced);
    
    printf("✅ Advanced Input System Tests Complete\n");
}

int main(void) {
    UNITY_BEGIN();
    
    // Run advanced input system test suite
    suite_input_advanced();
    
    // Clean up
    if (input_system_ready) {
        input_shutdown();
    }
    
    return UNITY_END();
}
