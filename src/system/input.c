// src/system/input.c
// Unified input abstraction implementation

#include "input.h"
#include "gamepad.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Input state
static InputState current_input = {0};
static bool input_initialized = false;

// Keyboard state tracking
static bool keyboard_state[INPUT_ACTION_COUNT] = {0};

bool input_init(void) {
    if (input_initialized) return true;
    
    // Initialize gamepad system
    if (!gamepad_init()) {
        printf("⚠️  Input: Gamepad initialization failed - keyboard only\n");
    } else {
        printf("🎮 Input: Gamepad system ready\n");
    }
    
    // Clear input state
    memset(&current_input, 0, sizeof(InputState));
    memset(keyboard_state, 0, sizeof(keyboard_state));
    
    input_initialized = true;
    printf("✅ Input system initialized\n");
    return true;
}

void input_shutdown(void) {
    if (!input_initialized) return;
    
    gamepad_shutdown();
    input_initialized = false;
    printf("🎮 Input system shutdown\n");
}

void input_update(void) {
    if (!input_initialized) return;
    
    // Poll gamepad
    gamepad_poll();
    
    // Clear current input
    memset(&current_input, 0, sizeof(InputState));
    
    // Process keyboard input (digital)
    if (keyboard_state[INPUT_ACTION_THRUST_FORWARD]) current_input.thrust += 1.0f;
    if (keyboard_state[INPUT_ACTION_THRUST_BACKWARD]) current_input.thrust -= 1.0f;
    if (keyboard_state[INPUT_ACTION_STRAFE_RIGHT]) current_input.strafe += 1.0f;
    if (keyboard_state[INPUT_ACTION_STRAFE_LEFT]) current_input.strafe -= 1.0f;
    if (keyboard_state[INPUT_ACTION_MANEUVER_UP]) current_input.vertical += 1.0f;
    if (keyboard_state[INPUT_ACTION_MANEUVER_DOWN]) current_input.vertical -= 1.0f;
    if (keyboard_state[INPUT_ACTION_BOOST]) current_input.boost = 1.0f;
    if (keyboard_state[INPUT_ACTION_BRAKE]) current_input.brake = true;
    
    // Add gamepad input (analog)
    GamepadState* gamepad = gamepad_get_state(0);
    if (gamepad && gamepad->connected) {
        const float deadzone = 0.15f;
        
        // Left stick for thrust/strafe
        if (fabsf(gamepad->left_stick_y) > deadzone) {
            current_input.thrust -= gamepad->left_stick_y; // Invert Y
        }
        if (fabsf(gamepad->left_stick_x) > deadzone) {
            current_input.strafe += gamepad->left_stick_x;
        }
        
        // Right stick Y for vertical
        if (fabsf(gamepad->right_stick_y) > deadzone) {
            current_input.vertical -= gamepad->right_stick_y; // Invert Y
        }
        
        // Triggers
        current_input.boost = fmaxf(current_input.boost, gamepad->right_trigger);
        if (gamepad->left_trigger > 0.5f) {
            current_input.brake = true;
        }
    }
    
    // Clamp combined values
    current_input.thrust = fmaxf(-1.0f, fminf(1.0f, current_input.thrust));
    current_input.strafe = fmaxf(-1.0f, fminf(1.0f, current_input.strafe));
    current_input.vertical = fmaxf(-1.0f, fminf(1.0f, current_input.vertical));
    current_input.boost = fmaxf(0.0f, fminf(1.0f, current_input.boost));
}

bool input_handle_keyboard(int key_code, bool is_pressed) {
    if (!input_initialized) return false;
    
    InputAction action = INPUT_ACTION_COUNT; // Invalid
    
    switch (key_code) {
        case SAPP_KEYCODE_W:
            action = INPUT_ACTION_THRUST_FORWARD;
            break;
        case SAPP_KEYCODE_S:
            action = INPUT_ACTION_THRUST_BACKWARD;
            break;
        case SAPP_KEYCODE_A:
            action = INPUT_ACTION_STRAFE_LEFT;
            break;
        case SAPP_KEYCODE_D:
            action = INPUT_ACTION_STRAFE_RIGHT;
            break;
        case SAPP_KEYCODE_Q:
            action = INPUT_ACTION_MANEUVER_UP;
            break;
        case SAPP_KEYCODE_E:
            action = INPUT_ACTION_MANEUVER_DOWN;
            break;
        case SAPP_KEYCODE_LEFT_SHIFT:
        case SAPP_KEYCODE_RIGHT_SHIFT:
            action = INPUT_ACTION_BOOST;
            break;
        case SAPP_KEYCODE_LEFT_CONTROL:
        case SAPP_KEYCODE_RIGHT_CONTROL:
            action = INPUT_ACTION_BRAKE;
            break;
        default:
            return false;
    }
    
    if (action < INPUT_ACTION_COUNT) {
        keyboard_state[action] = is_pressed;
        return true;
    }
    
    return false;
}

const InputState* input_get_state(void) {
    if (!input_initialized) return NULL;
    return &current_input;
}

bool input_has_gamepad(void) {
    if (!input_initialized) return false;
    
    GamepadState* gamepad = gamepad_get_state(0);
    return (gamepad && gamepad->connected);
}

void input_print_debug(void) {
    if (!input_initialized) return;
    
    printf("🎮 Input: T:%.2f S:%.2f V:%.2f B:%.2f Brake:%s Gamepad:%s\n",
           current_input.thrust,
           current_input.strafe,
           current_input.vertical,
           current_input.boost,
           current_input.brake ? "ON" : "OFF",
           input_has_gamepad() ? "YES" : "NO");
}
