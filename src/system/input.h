// src/system/input.h
// Unified input abstraction for keyboard and gamepad

#pragma once

#include <stdbool.h>
#include "../input_processing.h"  // For ProductionInputProcessor type
#include "../component/look_target.h"  // For LookTarget struct

// Input actions (logical inputs independent of device)
typedef enum {
    INPUT_ACTION_THRUST_FORWARD,
    INPUT_ACTION_THRUST_BACKWARD,
    INPUT_ACTION_STRAFE_LEFT,
    INPUT_ACTION_STRAFE_RIGHT,
    INPUT_ACTION_MANEUVER_UP,
    INPUT_ACTION_MANEUVER_DOWN,
    INPUT_ACTION_PITCH_UP,
    INPUT_ACTION_PITCH_DOWN,
    INPUT_ACTION_YAW_LEFT,
    INPUT_ACTION_YAW_RIGHT,
    INPUT_ACTION_ROLL_LEFT,
    INPUT_ACTION_ROLL_RIGHT,
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_COUNT
} InputAction;

// Input device type
typedef enum {
    INPUT_DEVICE_KEYBOARD,
    INPUT_DEVICE_GAMEPAD
} InputDeviceType;

// Input state for a single frame
typedef struct {
    // Linear movement
    float thrust;      // -1.0 to 1.0 (negative = backward)
    float strafe;      // -1.0 to 1.0 (negative = left)
    float vertical;    // -1.0 to 1.0 (negative = down)
    
    // Angular movement (6DOF)
    float pitch;       // -1.0 to 1.0 (negative = nose down)
    float yaw;         // -1.0 to 1.0 (negative = turn left)
    float roll;        // -1.0 to 1.0 (negative = roll left)
    
    // Modifiers
    float boost;       // 0.0 to 1.0 (analog boost intensity)
    bool brake;        // digital brake on/off
    
    // Canyon racing extensions
    bool look_based_thrust;    // True if thrust should go towards look target
    float auto_level;          // Auto-leveling strength (0.0 = off)
    LookTarget look_target;    // Camera look target
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

// Enhanced input processing access (Sprint 22)
ProductionInputProcessor* input_get_processor(void);
void input_set_processing_config(bool enable_neural, bool enable_mrac, bool enable_kalman);

// Canyon racing extensions
void input_handle_mouse_motion(float delta_x, float delta_y);
void input_handle_mouse_button(int button, bool is_pressed);
void input_handle_mouse_wheel(float delta);
void input_update_player_position(const Vector3* position);
InputDeviceType input_get_last_device_type(void);
void input_set_last_device_type(InputDeviceType type);
