/**
 * @file input_hal_mock.c
 * @brief Mock implementation of Input HAL for testing
 */

#include "input_hal.h"
#include <stdlib.h>
#include <string.h>

#define MAX_QUEUED_EVENTS 256

typedef struct MockInputHAL {
    InputHAL base;
    
    // Event queue
    HardwareInputEvent events[MAX_QUEUED_EVENTS];
    int event_count;
    int event_read_pos;
    
    // State tracking
    bool keys[512];
    float mouse_x, mouse_y;
    bool mouse_captured;
} MockInputHAL;

// Implementation functions
static bool mock_init(InputHAL* self, void* platform_data) {
    (void)platform_data;
    return true;
}

static void mock_shutdown(InputHAL* self) {
    // Nothing to clean up in mock
    (void)self;
}

static void mock_poll_events(InputHAL* self) {
    // In mock, events are queued externally via test functions
    (void)self;
}

static bool mock_get_next_event(InputHAL* self, HardwareInputEvent* event) {
    MockInputHAL* mock = (MockInputHAL*)self->platform_data;
    
    if (mock->event_read_pos < mock->event_count) {
        *event = mock->events[mock->event_read_pos++];
        return true;
    }
    
    // Reset for next frame
    if (mock->event_read_pos >= mock->event_count) {
        mock->event_count = 0;
        mock->event_read_pos = 0;
    }
    
    return false;
}

static bool mock_is_key_pressed(InputHAL* self, uint32_t key) {
    MockInputHAL* mock = (MockInputHAL*)self->platform_data;
    if (key < 512) {
        return mock->keys[key];
    }
    return false;
}

static void mock_get_mouse_position(InputHAL* self, float* x, float* y) {
    MockInputHAL* mock = (MockInputHAL*)self->platform_data;
    if (x) *x = mock->mouse_x;
    if (y) *y = mock->mouse_y;
}

static void mock_set_mouse_capture(InputHAL* self, bool captured) {
    MockInputHAL* mock = (MockInputHAL*)self->platform_data;
    mock->mouse_captured = captured;
}

static void mock_set_mouse_visible(InputHAL* self, bool visible) {
    (void)self;
    (void)visible;  // No-op in mock
}

static void mock_vibrate_gamepad(InputHAL* self, uint8_t gamepad_id, float left, float right) {
    (void)self;
    (void)gamepad_id;
    (void)left;
    (void)right;
    // No-op in mock
}

// Factory function
InputHAL* input_hal_create_mock(void) {
    MockInputHAL* mock = calloc(1, sizeof(MockInputHAL));
    if (!mock) return NULL;
    
    mock->base.init = mock_init;
    mock->base.shutdown = mock_shutdown;
    mock->base.poll_events = mock_poll_events;
    mock->base.get_next_event = mock_get_next_event;
    mock->base.is_key_pressed = mock_is_key_pressed;
    mock->base.get_mouse_position = mock_get_mouse_position;
    mock->base.set_mouse_capture = mock_set_mouse_capture;
    mock->base.set_mouse_visible = mock_set_mouse_visible;
    mock->base.vibrate_gamepad = mock_vibrate_gamepad;
    mock->base.platform_data = mock;
    
    return &mock->base;
}

// Test helper functions
void mock_input_queue_key_event(InputHAL* hal, uint32_t key, bool pressed) {
    if (!hal || !hal->platform_data) return;
    
    MockInputHAL* mock = (MockInputHAL*)hal->platform_data;
    if (mock->event_count < MAX_QUEUED_EVENTS) {
        HardwareInputEvent* event = &mock->events[mock->event_count++];
        event->timestamp = mock->event_count;  // Simple incrementing timestamp
        event->device = INPUT_DEVICE_KEYBOARD;
        event->data.keyboard.key = key;
        event->data.keyboard.pressed = pressed;
        event->data.keyboard.modifiers = 0;
        
        // Update state
        if (key < 512) {
            mock->keys[key] = pressed;
        }
    }
}

void mock_input_queue_mouse_move(InputHAL* hal, float x, float y) {
    if (!hal || !hal->platform_data) return;
    
    MockInputHAL* mock = (MockInputHAL*)hal->platform_data;
    if (mock->event_count < MAX_QUEUED_EVENTS) {
        HardwareInputEvent* event = &mock->events[mock->event_count++];
        event->timestamp = mock->event_count;
        event->device = INPUT_DEVICE_MOUSE;
        event->data.mouse.x = x;
        event->data.mouse.y = y;
        event->data.mouse.dx = x - mock->mouse_x;
        event->data.mouse.dy = y - mock->mouse_y;
        event->data.mouse.buttons = 0;  // No change in buttons
        
        // Update state
        mock->mouse_x = x;
        mock->mouse_y = y;
    }
}