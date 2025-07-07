/**
 * @file test_mock_hal.c
 * @brief Unit tests for Mock Input HAL
 */

#include "../../src/hal/input_hal.h"
#include "../../tests/test_framework.h"
#include <string.h>

void test_mock_hal_creation(void) {
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_NOT_NULL(hal);
    TEST_ASSERT_NOT_NULL(hal->init);
    TEST_ASSERT_NOT_NULL(hal->shutdown);
    TEST_ASSERT_NOT_NULL(hal->poll_events);
    TEST_ASSERT_NOT_NULL(hal->get_next_event);
    
    // Initialize
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Cleanup
    hal->shutdown(hal);
    free(hal);
}

void test_mock_hal_keyboard_events(void) {
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_NOT_NULL(hal);
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Queue some keyboard events
    mock_input_queue_key_event(hal, 65, true);   // 'A' pressed
    mock_input_queue_key_event(hal, 65, false);  // 'A' released
    mock_input_queue_key_event(hal, 32, true);   // Space pressed
    
    // Poll events (no-op for mock)
    hal->poll_events(hal);
    
    // Read events back
    HardwareInputEvent event;
    
    // Event 1: A pressed
    TEST_ASSERT_TRUE(hal->get_next_event(hal, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_KEYBOARD, event.device);
    TEST_ASSERT_EQUAL_INT(65, event.data.keyboard.key);
    TEST_ASSERT_TRUE(event.data.keyboard.pressed);
    
    // Event 2: A released
    TEST_ASSERT_TRUE(hal->get_next_event(hal, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_KEYBOARD, event.device);
    TEST_ASSERT_EQUAL_INT(65, event.data.keyboard.key);
    TEST_ASSERT_FALSE(event.data.keyboard.pressed);
    
    // Event 3: Space pressed
    TEST_ASSERT_TRUE(hal->get_next_event(hal, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_KEYBOARD, event.device);
    TEST_ASSERT_EQUAL_INT(32, event.data.keyboard.key);
    TEST_ASSERT_TRUE(event.data.keyboard.pressed);
    
    // No more events
    TEST_ASSERT_FALSE(hal->get_next_event(hal, &event));
    
    // Test key state query
    TEST_ASSERT_TRUE(hal->is_key_pressed(hal, 32));   // Space is still pressed
    TEST_ASSERT_FALSE(hal->is_key_pressed(hal, 65));  // A was released
    
    hal->shutdown(hal);
    free(hal);
}

void test_mock_hal_mouse_events(void) {
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_NOT_NULL(hal);
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Initial mouse position
    float x, y;
    hal->get_mouse_position(hal, &x, &y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, y);
    
    // Queue mouse move events
    mock_input_queue_mouse_move(hal, 100.0f, 200.0f);
    mock_input_queue_mouse_move(hal, 150.0f, 250.0f);
    
    // Read first move event
    HardwareInputEvent event;
    TEST_ASSERT_TRUE(hal->get_next_event(hal, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_MOUSE, event.device);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, event.data.mouse.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 200.0f, event.data.mouse.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, event.data.mouse.dx);  // Delta from 0,0
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 200.0f, event.data.mouse.dy);
    
    // Read second move event
    TEST_ASSERT_TRUE(hal->get_next_event(hal, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_MOUSE, event.device);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 150.0f, event.data.mouse.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 250.0f, event.data.mouse.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, event.data.mouse.dx);   // Delta from previous
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, event.data.mouse.dy);
    
    // Check mouse position state
    hal->get_mouse_position(hal, &x, &y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 150.0f, x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 250.0f, y);
    
    hal->shutdown(hal);
    free(hal);
}

void test_mock_hal_queue_overflow(void) {
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_NOT_NULL(hal);
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Queue many events to test overflow handling
    for (int i = 0; i < 300; i++) {  // More than MAX_QUEUED_EVENTS (256)
        mock_input_queue_key_event(hal, i % 128, true);
    }
    
    // Should be able to read exactly MAX_QUEUED_EVENTS
    HardwareInputEvent event;
    int event_count = 0;
    while (hal->get_next_event(hal, &event)) {
        event_count++;
        if (event_count > 256) break;  // Safety check
    }
    
    // Mock HAL should limit to 256 events
    TEST_ASSERT_EQUAL_INT(256, event_count);
    
    hal->shutdown(hal);
    free(hal);
}

void test_mock_hal_mouse_features(void) {
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_NOT_NULL(hal);
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Test mouse capture
    hal->set_mouse_capture(hal, true);
    // Note: Mock doesn't track this state externally, but shouldn't crash
    
    // Test mouse visibility
    hal->set_mouse_visible(hal, false);
    hal->set_mouse_visible(hal, true);
    
    // Test gamepad vibration (no-op in mock)
    hal->vibrate_gamepad(hal, 0, 0.5f, 1.0f);
    
    hal->shutdown(hal);
    free(hal);
}

int main(void) {
    printf("Running Mock HAL tests...\n\n");
    
    RUN_TEST(test_mock_hal_creation);
    RUN_TEST(test_mock_hal_keyboard_events);
    RUN_TEST(test_mock_hal_mouse_events);
    RUN_TEST(test_mock_hal_queue_overflow);
    RUN_TEST(test_mock_hal_mouse_features);
    
    printf("\n✅ All Mock HAL tests passed!\n");
    return 0;
}