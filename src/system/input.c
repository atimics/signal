// Canyon Racing Input System - Simplified and Direct
// This replaces the overly complex neural network input processing

#include "input.h"
#include "gamepad.h"
#include "../component/look_target.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Input configuration
#define GAMEPAD_DEADZONE 0.15f          // 15% deadzone (reduced for better responsiveness)
#define LOOK_SENSITIVITY 2.5f           // Camera rotation speed (increased)
#define PITCH_YAW_SENSITIVITY 1.2f      // Ship rotation speed (increased for snappier controls)
#define MOUSE_SENSITIVITY 0.005f         // Mouse look sensitivity
#define AUTO_LEVEL_STRENGTH 2.0f         // How fast ship auto-levels

// Canyon racing input state
typedef struct CanyonRacingInput {
    // Current processed input
    InputState current_state;
    
    // Look target for camera
    LookTarget look_target;
    
    // Mouse state
    float mouse_delta_x;
    float mouse_delta_y;
    bool mouse_captured;
    
    // Auto-level state
    bool auto_leveling;
    float auto_level_timer;
    
    // Device state
    InputDeviceType last_device;
    bool initialized;
} CanyonRacingInput;

// Global state
static CanyonRacingInput canyon_input = {0};
static bool keyboard_state[INPUT_ACTION_COUNT] = {0};

// Simple deadzone function
static float apply_deadzone(float value, float deadzone) {
    if (fabsf(value) < deadzone) {
        return 0.0f;
    }
    
    // Scale to remove deadzone
    float sign = value > 0 ? 1.0f : -1.0f;
    return sign * (fabsf(value) - deadzone) / (1.0f - deadzone);
}

bool input_init(void) {
    // Initialize gamepad system
    if (!gamepad_init()) {
        printf("⚠️  Canyon Racing Input: Gamepad initialization failed - keyboard/mouse only\n");
    } else {
        printf("🎮 Canyon Racing Input: Gamepad system ready\n");
    }
    
    // Clear input state
    memset(&canyon_input, 0, sizeof(CanyonRacingInput));
    memset(keyboard_state, 0, sizeof(keyboard_state));
    
    // Initialize look target
    look_target_init(&canyon_input.look_target);
    
    canyon_input.initialized = true;
    printf("✅ Canyon Racing Input system initialized\n");
    return true;
}

void input_shutdown(void) {
    if (!canyon_input.initialized) return;
    
    gamepad_shutdown();
    canyon_input.initialized = false;
    printf("🎮 Canyon Racing Input system shutdown\n");
}

void input_update(void) {
    if (!canyon_input.initialized) return;
    
    // Poll gamepad
    gamepad_poll();
    
    // Clear current input
    memset(&canyon_input.current_state, 0, sizeof(InputState));
    
    // Get player entity position (will be set by control system)
    Vector3 player_position = {0, 0, 0}; // TODO: Get from player entity
    
    // Process keyboard input
    // WASD for pitch/yaw
    if (keyboard_state[INPUT_ACTION_PITCH_DOWN]) canyon_input.current_state.pitch -= 1.0f; // W = dive
    if (keyboard_state[INPUT_ACTION_PITCH_UP]) canyon_input.current_state.pitch += 1.0f;   // S = climb
    if (keyboard_state[INPUT_ACTION_YAW_LEFT]) canyon_input.current_state.yaw -= 1.0f;     // A = left
    if (keyboard_state[INPUT_ACTION_YAW_RIGHT]) canyon_input.current_state.yaw += 1.0f;    // D = right
    
    // Q/E for roll
    if (keyboard_state[INPUT_ACTION_ROLL_LEFT]) canyon_input.current_state.roll -= 1.0f;
    if (keyboard_state[INPUT_ACTION_ROLL_RIGHT]) canyon_input.current_state.roll += 1.0f;
    
    // Space for forward thrust (simplified - no look-based complexity)
    if (keyboard_state[INPUT_ACTION_THRUST_FORWARD]) {
        canyon_input.current_state.thrust = 1.0f;
    }
    
    // Modifiers
    if (keyboard_state[INPUT_ACTION_BOOST]) canyon_input.current_state.boost = 1.0f;
    if (keyboard_state[INPUT_ACTION_BRAKE]) {
        canyon_input.current_state.brake = true;
        canyon_input.auto_leveling = true;
    }
    
    // Process gamepad input
    GamepadState* gamepad = gamepad_get_state(0);
    if (gamepad && gamepad->connected) {
        // CANYON RACING CONTROLS:
        // Left stick: Banking and fine targeting adjustments (ship orientation)
        float left_x = apply_deadzone(gamepad->left_stick_x, GAMEPAD_DEADZONE);
        float left_y = apply_deadzone(gamepad->left_stick_y, GAMEPAD_DEADZONE);
        
        if (left_x != 0.0f || left_y != 0.0f) {
            // Left stick X = banking (roll + coordinated yaw for racing-style turns)
            float bank_input = left_x * PITCH_YAW_SENSITIVITY;
            canyon_input.current_state.roll += bank_input * 1.5f;     // Primary roll
            canyon_input.current_state.yaw += bank_input * 0.3f;      // Coordinated turn
            
            // Left stick Y = fine pitch adjustments
            canyon_input.current_state.pitch -= left_y * PITCH_YAW_SENSITIVITY * 0.5f; // Reduced sensitivity for fine control
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Right stick: Targeting reticle control (where you want to fly)
        float right_x = apply_deadzone(gamepad->right_stick_x, GAMEPAD_DEADZONE);
        float right_y = apply_deadzone(gamepad->right_stick_y, GAMEPAD_DEADZONE);
        
        if (right_x != 0.0f || right_y != 0.0f) {
            float delta_azimuth = right_x * LOOK_SENSITIVITY * 0.016f;
            float delta_elevation = -right_y * LOOK_SENSITIVITY * 0.016f; // Invert Y
            
            look_target_update(&canyon_input.look_target, &player_position,
                             delta_azimuth, delta_elevation, 0.0f);
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Right trigger for forward thrust (simplified)
        float right_trigger = apply_deadzone(gamepad->right_trigger, GAMEPAD_DEADZONE);
        if (right_trigger > 0.0f) {
            canyon_input.current_state.thrust = right_trigger;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Left trigger for brake/reverse thrust
        float left_trigger = apply_deadzone(gamepad->left_trigger, GAMEPAD_DEADZONE);
        if (left_trigger > 0.0f) {
            canyon_input.current_state.brake = true;
            // Note: Auto-leveling removed to prevent control conflicts
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Bumpers for roll
        if (gamepad->buttons[GAMEPAD_BUTTON_RB]) {
            canyon_input.current_state.roll += 1.0f;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_LB]) {
            canyon_input.current_state.roll -= 1.0f;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Face buttons
        if (gamepad->buttons[GAMEPAD_BUTTON_A]) {
            canyon_input.current_state.boost = 1.0f;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_B]) {
            canyon_input.current_state.brake = true;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
    }
    
    // Process mouse input for look target
    if (canyon_input.mouse_captured && 
        (fabsf(canyon_input.mouse_delta_x) > 0.001f || fabsf(canyon_input.mouse_delta_y) > 0.001f)) {
        
        float delta_azimuth = canyon_input.mouse_delta_x * MOUSE_SENSITIVITY;
        float delta_elevation = canyon_input.mouse_delta_y * MOUSE_SENSITIVITY;
        
        look_target_update(&canyon_input.look_target, &player_position,
                         delta_azimuth, delta_elevation, 0.0f);
        
        canyon_input.last_device = INPUT_DEVICE_KEYBOARD; // Mouse counts as keyboard
        
        // Clear mouse deltas
        canyon_input.mouse_delta_x = 0.0f;
        canyon_input.mouse_delta_y = 0.0f;
    }
    
    // Auto-leveling removed to prevent control system conflicts
    // User has full manual control
    
    // Store look target in input state
    canyon_input.current_state.look_target = canyon_input.look_target;
    
    // Clamp values
    canyon_input.current_state.thrust = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.thrust));
    canyon_input.current_state.pitch = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.pitch));
    canyon_input.current_state.yaw = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.yaw));
    canyon_input.current_state.roll = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.roll));
    canyon_input.current_state.boost = fmaxf(0.0f, fminf(1.0f, canyon_input.current_state.boost));
    
    // Debug output
    static int debug_counter = 0;
    if (++debug_counter % 60 == 0) { // Every second
        if (canyon_input.current_state.thrust > 0.0f || 
            fabsf(canyon_input.current_state.pitch) > 0.1f ||
            fabsf(canyon_input.current_state.yaw) > 0.1f) {
            
            printf("🏎️ Canyon Racing Input: T:%.2f P:%.2f Y:%.2f R:%.2f ",
                   canyon_input.current_state.thrust,
                   canyon_input.current_state.pitch,
                   canyon_input.current_state.yaw,
                   canyon_input.current_state.roll);
            
            // Simplified - no complex auto-assist systems
            
            printf("Look: Az:%.2f El:%.2f Dist:%.1f\n",
                   canyon_input.look_target.azimuth,
                   canyon_input.look_target.elevation,
                   canyon_input.look_target.distance);
        }
    }
}

bool input_handle_keyboard(int key_code, bool is_pressed) {
    if (!canyon_input.initialized) return false;
    
    InputAction action = INPUT_ACTION_COUNT; // Invalid
    
    switch (key_code) {
        // WASD for pitch/yaw
        case SAPP_KEYCODE_W:
            action = INPUT_ACTION_PITCH_DOWN;  // W = nose down (dive)
            break;
        case SAPP_KEYCODE_S:
            action = INPUT_ACTION_PITCH_UP;    // S = nose up (climb)
            break;
        case SAPP_KEYCODE_A:
            action = INPUT_ACTION_YAW_LEFT;    // A = yaw left
            break;
        case SAPP_KEYCODE_D:
            action = INPUT_ACTION_YAW_RIGHT;   // D = yaw right
            break;
            
        // Space for thrust towards look target
        case SAPP_KEYCODE_SPACE:
            action = INPUT_ACTION_THRUST_FORWARD;
            break;
            
        // Q/E for roll
        case SAPP_KEYCODE_Q:
            action = INPUT_ACTION_ROLL_LEFT;
            break;
        case SAPP_KEYCODE_E:
            action = INPUT_ACTION_ROLL_RIGHT;
            break;
            
        // Modifiers
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
        
        if (is_pressed) {
            canyon_input.last_device = INPUT_DEVICE_KEYBOARD;
        }
        
        return true;
    }
    
    return false;
}

void input_handle_mouse_motion(float delta_x, float delta_y) {
    if (!canyon_input.initialized) return;
    
    canyon_input.mouse_delta_x += delta_x;
    canyon_input.mouse_delta_y += delta_y;
}

void input_handle_mouse_button(int button, bool is_pressed) {
    if (!canyon_input.initialized) return;
    
    // Right mouse button to capture/release mouse
    if (button == 1) { // Right button
        canyon_input.mouse_captured = is_pressed;
        
        if (is_pressed) {
            printf("🖱️ Mouse captured for look control\n");
        } else {
            printf("🖱️ Mouse released\n");
        }
    }
}

void input_handle_mouse_wheel(float delta) {
    if (!canyon_input.initialized) return;
    
    // Zoom camera in/out
    float distance_delta = -delta * 5.0f; // Invert and scale
    look_target_update(&canyon_input.look_target, NULL, 0.0f, 0.0f, distance_delta);
}

const InputState* input_get_state(void) {
    if (!canyon_input.initialized) return NULL;
    return &canyon_input.current_state;
}

void input_update_player_position(const Vector3* position) {
    if (!canyon_input.initialized || !position) return;
    
    // Update look target to follow player
    look_target_update_world_position(&canyon_input.look_target, position);
}

bool input_has_gamepad(void) {
    if (!canyon_input.initialized) return false;
    
    GamepadState* gamepad = gamepad_get_state(0);
    return (gamepad && gamepad->connected);
}

InputDeviceType input_get_last_device_type(void) {
    return canyon_input.last_device;
}

void input_set_last_device_type(InputDeviceType type) {
    canyon_input.last_device = type;
}

void input_print_debug(void) {
    if (!canyon_input.initialized) return;
    
    printf("🏎️ Canyon Racing - T:%.2f P:%.2f Y:%.2f R:%.2f B:%.2f Brake:%s Look:%s Auto:%s Device:%s\n",
           canyon_input.current_state.thrust,
           canyon_input.current_state.pitch,
           canyon_input.current_state.yaw,
           canyon_input.current_state.roll,
           canyon_input.current_state.boost,
           canyon_input.current_state.brake ? "ON" : "OFF",
           canyon_input.current_state.look_based_thrust ? "YES" : "NO",
           canyon_input.auto_leveling ? "YES" : "NO",
           canyon_input.last_device == INPUT_DEVICE_GAMEPAD ? "GAMEPAD" : "KB/MOUSE");
}

// Stub functions for compatibility
ProductionInputProcessor* input_get_processor(void) { return NULL; }
void input_set_processing_config(bool enable_neural, bool enable_mrac, bool enable_kalman) {
    (void)enable_neural; (void)enable_mrac; (void)enable_kalman;
}