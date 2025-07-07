/**
 * @file test_input_service.c  
 * @brief Unit tests for Input Service with action mapping
 */

#include "../../src/services/input_service.h"
#include "../../src/hal/input_hal.h"
#include "../../tests/test_framework.h"

// Sokol keycodes for testing (corrected values - Sprint 25)
#define SAPP_KEYCODE_UP     265
#define SAPP_KEYCODE_DOWN   264
#define SAPP_KEYCODE_ENTER  36
#define SAPP_KEYCODE_ESCAPE 27
#define SAPP_KEYCODE_W      87
#define SAPP_KEYCODE_S      83
#define SAPP_KEYCODE_A      65
#define SAPP_KEYCODE_D      68
#define SAPP_KEYCODE_Q      81
#define SAPP_KEYCODE_E      69

void test_input_service_creation(void) {
    InputService* service = input_service_create();
    TEST_ASSERT_NOT_NULL(service);
    TEST_ASSERT_NOT_NULL(service->init);
    TEST_ASSERT_NOT_NULL(service->shutdown);
    TEST_ASSERT_NOT_NULL(service->process_frame);
    TEST_ASSERT_NOT_NULL(service->internal);
    
    input_service_destroy(service);
}

void test_input_service_menu_navigation(void) {
    // Create mock HAL
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_NOT_NULL(hal);
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Create service
    InputService* service = input_service_create();
    TEST_ASSERT_NOT_NULL(service);
    
    InputServiceConfig config = {
        .hal = hal,
        .bindings_path = NULL,
        .enable_input_logging = false
    };
    TEST_ASSERT_TRUE(service->init(service, &config));
    
    // Service should start in menu context
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_MENU, service->get_active_context(service));
    
    // Queue test events: Down, Down, Enter
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, true);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, true);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_ENTER, true);
    
    // Process frame
    service->process_frame(service, 0.016f);
    
    // Get events
    InputEvent event;
    
    // First down press
    TEST_ASSERT_TRUE(service->get_next_event(service, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_ACTION_UI_DOWN, event.action);
    TEST_ASSERT_TRUE(event.just_pressed);
    TEST_ASSERT_FALSE(event.just_released);
    
    // First down release
    TEST_ASSERT_TRUE(service->get_next_event(service, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_ACTION_UI_DOWN, event.action);
    TEST_ASSERT_FALSE(event.just_pressed);
    TEST_ASSERT_TRUE(event.just_released);
    
    // Second down press
    TEST_ASSERT_TRUE(service->get_next_event(service, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_ACTION_UI_DOWN, event.action);
    TEST_ASSERT_TRUE(event.just_pressed);
    
    // Enter press
    TEST_ASSERT_TRUE(service->get_next_event(service, &event));
    TEST_ASSERT_EQUAL_INT(INPUT_ACTION_UI_CONFIRM, event.action);
    TEST_ASSERT_TRUE(event.just_pressed);
    
    // No more events
    TEST_ASSERT_FALSE(service->get_next_event(service, &event));
    
    // Cleanup
    service->shutdown(service);
    input_service_destroy(service);
    hal->shutdown(hal);
    free(hal);
}

void test_input_service_context_switching(void) {
    // Create mock HAL
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Create and initialize service
    InputService* service = input_service_create();
    InputServiceConfig config = {
        .hal = hal,
        .bindings_path = NULL,
        .enable_input_logging = false
    };
    TEST_ASSERT_TRUE(service->init(service, &config));
    
    // Test context stack
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_MENU, service->get_active_context(service));
    
    // Push gameplay context
    service->push_context(service, INPUT_CONTEXT_GAMEPLAY);
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_GAMEPLAY, service->get_active_context(service));
    
    // Push dialog context
    service->push_context(service, INPUT_CONTEXT_DIALOG);
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_DIALOG, service->get_active_context(service));
    
    // Pop back to gameplay
    service->pop_context(service);
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_GAMEPLAY, service->get_active_context(service));
    
    // Pop back to menu
    service->pop_context(service);
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_MENU, service->get_active_context(service));
    
    // Shouldn't pop beyond root
    service->pop_context(service);
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_MENU, service->get_active_context(service));
    
    // Cleanup
    service->shutdown(service);
    input_service_destroy(service);
    hal->shutdown(hal);
    free(hal);
}

void test_input_service_state_queries(void) {
    // Create mock HAL
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Create and initialize service
    InputService* service = input_service_create();
    InputServiceConfig config = {
        .hal = hal,
        .bindings_path = NULL,
        .enable_input_logging = false
    };
    TEST_ASSERT_TRUE(service->init(service, &config));
    
    // Initially nothing pressed
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FALSE(service->is_action_just_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FALSE(service->is_action_just_released(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, service->get_action_value(service, INPUT_ACTION_UI_UP));
    
    // Queue UP key press
    mock_input_queue_key_event(hal, SAPP_KEYCODE_UP, true);
    service->process_frame(service, 0.016f);
    
    // Check state after press
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_TRUE(service->is_action_just_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FALSE(service->is_action_just_released(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, service->get_action_value(service, INPUT_ACTION_UI_UP));
    
    // Process another frame (just_pressed should clear)
    service->process_frame(service, 0.016f);
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FALSE(service->is_action_just_pressed(service, INPUT_ACTION_UI_UP));
    
    // Queue UP key release
    mock_input_queue_key_event(hal, SAPP_KEYCODE_UP, false);
    service->process_frame(service, 0.016f);
    
    // Check state after release
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FALSE(service->is_action_just_pressed(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_TRUE(service->is_action_just_released(service, INPUT_ACTION_UI_UP));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, service->get_action_value(service, INPUT_ACTION_UI_UP));
    
    // Cleanup
    service->shutdown(service);
    input_service_destroy(service);
    hal->shutdown(hal);
    free(hal);
}

void test_input_service_custom_bindings(void) {
    // Create mock HAL and service
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    InputService* service = input_service_create();
    InputServiceConfig config = {
        .hal = hal,
        .bindings_path = NULL,
        .enable_input_logging = false
    };
    TEST_ASSERT_TRUE(service->init(service, &config));
    
    // Clear default bindings for UI_LEFT
    service->clear_action_bindings(service, INPUT_ACTION_UI_LEFT, INPUT_CONTEXT_MENU);
    
    // Add custom binding: Q key for UI_LEFT
    InputBinding binding = {
        .device = INPUT_DEVICE_KEYBOARD,
        .binding.keyboard.key = 81,  // Q
        .binding.keyboard.modifiers = 0,
        .scale = 1.0f,
        .invert = false
    };
    service->bind_action(service, INPUT_ACTION_UI_LEFT, INPUT_CONTEXT_MENU, &binding);
    
    // Test the custom binding
    mock_input_queue_key_event(hal, 81, true);  // Q pressed
    service->process_frame(service, 0.016f);
    
    TEST_ASSERT_TRUE(service->is_action_just_pressed(service, INPUT_ACTION_UI_LEFT));
    
    // Cleanup
    service->shutdown(service);
    input_service_destroy(service);
    hal->shutdown(hal);
    free(hal);
}

// Sprint 25: Test flight control key mappings and context switching
void test_sprint25_flight_controls(void) {
    printf("🧪 Testing Sprint 25 flight controls...\n");
    
    // Create mock HAL
    InputHAL* hal = input_hal_create_mock();
    TEST_ASSERT_TRUE(hal->init(hal, NULL));
    
    // Create and initialize service
    InputService* service = input_service_create();
    InputServiceConfig config = {
        .hal = hal,
        .bindings_path = NULL,
        .enable_input_logging = false
    };
    TEST_ASSERT_TRUE(service->init(service, &config));
    
    // Switch to gameplay context for flight controls
    service->push_context(service, INPUT_CONTEXT_GAMEPLAY);
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_GAMEPLAY, service->get_active_context(service));
    
    // Test thrust controls: W (forward) and S (backward)
    mock_input_queue_key_event(hal, SAPP_KEYCODE_W, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_THRUST_FORWARD));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, service->get_action_value(service, INPUT_ACTION_THRUST_FORWARD));
    
    mock_input_queue_key_event(hal, SAPP_KEYCODE_W, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_S, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_THRUST_FORWARD));
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_THRUST_BACK));
    
    // Test yaw controls: A (left) and D (right)
    mock_input_queue_key_event(hal, SAPP_KEYCODE_S, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_A, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_YAW_LEFT));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, service->get_action_value(service, INPUT_ACTION_YAW_LEFT));
    
    mock_input_queue_key_event(hal, SAPP_KEYCODE_A, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_D, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_YAW_LEFT));
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_YAW_RIGHT));
    
    // Test roll controls: Q (left) and E (right)
    mock_input_queue_key_event(hal, SAPP_KEYCODE_D, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_Q, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_ROLL_LEFT));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, service->get_action_value(service, INPUT_ACTION_ROLL_LEFT));
    
    mock_input_queue_key_event(hal, SAPP_KEYCODE_Q, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_E, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_ROLL_LEFT));
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_ROLL_RIGHT));
    
    // Test pitch controls: Up Arrow (up) and Down Arrow (down)
    mock_input_queue_key_event(hal, SAPP_KEYCODE_E, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_UP, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_PITCH_UP));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, service->get_action_value(service, INPUT_ACTION_PITCH_UP));
    
    mock_input_queue_key_event(hal, SAPP_KEYCODE_UP, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, true);
    service->process_frame(service, 0.016f);
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_PITCH_UP));
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_PITCH_DOWN));
    
    // Test that flight controls are NOT active in menu context
    service->pop_context(service);  // Back to menu context
    TEST_ASSERT_EQUAL_INT(INPUT_CONTEXT_MENU, service->get_active_context(service));
    
    mock_input_queue_key_event(hal, SAPP_KEYCODE_DOWN, false);
    mock_input_queue_key_event(hal, SAPP_KEYCODE_W, true);
    service->process_frame(service, 0.016f);
    // W should trigger UI_UP in menu context, not THRUST_FORWARD
    TEST_ASSERT_FALSE(service->is_action_pressed(service, INPUT_ACTION_THRUST_FORWARD));
    TEST_ASSERT_TRUE(service->is_action_pressed(service, INPUT_ACTION_UI_UP));
    
    printf("✅ Sprint 25 flight controls test passed!\n");
    
    // Cleanup
    service->shutdown(service);
    input_service_destroy(service);
    hal->shutdown(hal);
    free(hal);
}

int main(void) {
    printf("Running Input Service tests...\n\n");
    
    RUN_TEST(test_input_service_creation);
    RUN_TEST(test_input_service_menu_navigation);
    RUN_TEST(test_input_service_context_switching);
    RUN_TEST(test_input_service_state_queries);
    RUN_TEST(test_input_service_custom_bindings);
    RUN_TEST(test_sprint25_flight_controls);
    
    printf("\n✅ All Input Service tests passed!\n");
    return 0;
}