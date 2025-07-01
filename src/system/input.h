// src/system/input.h
// Unified input abstraction for keyboard and gamepad

#pragma once

#include <stdbool.h>

// Input actions (logical inputs independent of device)
typedef enum {
    INPUT_ACTION_THRUST_FORWARD,
    INPUT_ACTION_THRUST_BACKWARD,
    INPUT_ACTION_STRAFE_LEFT,
    INPUT_ACTION_STRAFE_RIGHT,
    INPUT_ACTION_MANEUVER_UP,
    INPUT_ACTION_MANEUVER_DOWN,
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_COUNT
} InputAction;

// Input state for a single frame
typedef struct {
    float thrust;      // -1.0 to 1.0 (negative = backward)
    float strafe;      // -1.0 to 1.0 (negative = left)
    float vertical;    // -1.0 to 1.0 (negative = down)
    float boost;       // 0.0 to 1.0 (analog boost intensity)
    bool brake;        // digital brake on/off
} InputState;

// Initialize input system
bool input_init(void);

// Shutdown input system
void input_shutdown(void);

// Update input state (call once per frame)
void input_update(void);

// Handle keyboard events (call from event handler)
bool input_handle_keyboard(int key_code, bool is_pressed);

// Get current input state
const InputState* input_get_state(void);

// Check if input device is connected
bool input_has_gamepad(void);

// Debug info
void input_print_debug(void);
