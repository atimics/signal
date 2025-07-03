// Canyon Racing Input System - Simplified and Direct
// This replaces the overly complex neural network input processing

#include "input.h"
#include "gamepad.h"
#include "../component/look_target.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Zero-G Input configuration - STABILITY FOCUSED
#define GAMEPAD_DEADZONE 0.15f          // 15% deadzone for stability
#define LOOK_SENSITIVITY 2.0f           // Camera rotation speed (reduced)
#define PITCH_YAW_SENSITIVITY 0.4f      // Ship rotation speed (much lower for stability)
#define MOUSE_SENSITIVITY 0.003f        // Mouse look sensitivity 
#define AUTO_LEVEL_STRENGTH 2.0f        // How fast ship auto-levels
#define THRUST_SENSITIVITY 0.6f         // Thrust responsiveness (reduced)
#define INPUT_SMOOTHING 0.85f           // Input smoothing factor (0.0 = no smoothing, 1.0 = full smoothing)

// Zero-G input state with smoothing
typedef struct CanyonRacingInput {
    // Current processed input
    InputState current_state;
    
    // Previous input for smoothing
    InputState previous_state;
    
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
    
    // Store previous input for smoothing
    canyon_input.previous_state = canyon_input.current_state;
    
    // Clear current input - IMPORTANT: Reset all values to prevent accumulation
    memset(&canyon_input.current_state, 0, sizeof(InputState));
    canyon_input.current_state.pitch = 0.0f;
    canyon_input.current_state.yaw = 0.0f;
    canyon_input.current_state.roll = 0.0f;
    canyon_input.current_state.thrust = 0.0f;
    
    // Get player entity position (will be set by control system)
    Vector3 player_position = {0, 0, 0}; // TODO: Get from player entity
    
    // Check if gamepad is connected first
    GamepadState* gamepad = gamepad_get_state(0);
    bool using_gamepad = (gamepad && gamepad->connected);
    
    // Process keyboard input ONLY if no gamepad is active
    if (!using_gamepad) {
        // WASD for pitch/yaw
        if (keyboard_state[INPUT_ACTION_PITCH_DOWN]) canyon_input.current_state.pitch = -1.0f; // W = dive
        if (keyboard_state[INPUT_ACTION_PITCH_UP]) canyon_input.current_state.pitch = 1.0f;   // S = climb
        if (keyboard_state[INPUT_ACTION_YAW_LEFT]) canyon_input.current_state.yaw = -1.0f;     // A = left
        if (keyboard_state[INPUT_ACTION_YAW_RIGHT]) canyon_input.current_state.yaw = 1.0f;    // D = right
        
        // Q/E for roll
        if (keyboard_state[INPUT_ACTION_ROLL_LEFT]) canyon_input.current_state.roll = -1.0f;
        if (keyboard_state[INPUT_ACTION_ROLL_RIGHT]) canyon_input.current_state.roll = 1.0f;
        
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
    }
    
    // Process gamepad input for ZERO-G FLIGHT
    if (using_gamepad) {
        // ZERO-G CONTROLS OPTIMIZED FOR XBOX CONTROLLER:
        // Left stick: Primary flight control (pitch/yaw) with response curve
        float left_x = apply_deadzone(gamepad->left_stick_x, GAMEPAD_DEADZONE);
        float left_y = apply_deadzone(gamepad->left_stick_y, GAMEPAD_DEADZONE);
        
        if (left_x != 0.0f || left_y != 0.0f) {
            // Linear response for more predictable control in zero-g
            float raw_yaw = left_x * PITCH_YAW_SENSITIVITY;
            float raw_pitch = -left_y * PITCH_YAW_SENSITIVITY; // Inverted for flight
            
            // Apply input smoothing to reduce oscillations
            canyon_input.current_state.yaw = canyon_input.previous_state.yaw * INPUT_SMOOTHING + 
                                            raw_yaw * (1.0f - INPUT_SMOOTHING);
            canyon_input.current_state.pitch = canyon_input.previous_state.pitch * INPUT_SMOOTHING + 
                                              raw_pitch * (1.0f - INPUT_SMOOTHING);
            
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
            
            // Debug output
            static int stick_debug_counter = 0;
            if (++stick_debug_counter % 60 == 0) { // Reduced debug frequency
                printf("🎮 ZERO-G STICK: Raw(%.3f,%.3f) → Smoothed Pitch:%.3f Yaw:%.3f\n", 
                       left_x, left_y, canyon_input.current_state.pitch, canyon_input.current_state.yaw);
            }
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
        
        // Right trigger for forward thrust with smoothing
        float right_trigger = apply_deadzone(gamepad->right_trigger, GAMEPAD_DEADZONE * 0.6f); // Slightly higher deadzone
        if (right_trigger > 0.0f) {
            // Apply smoothing to thrust for stability
            float raw_thrust = right_trigger * THRUST_SENSITIVITY;
            canyon_input.current_state.thrust = canyon_input.previous_state.thrust * INPUT_SMOOTHING + 
                                               raw_thrust * (1.0f - INPUT_SMOOTHING);
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Left trigger for braking with enhanced zero-g stopping
        float left_trigger = apply_deadzone(gamepad->left_trigger, GAMEPAD_DEADZONE * 0.5f);
        if (left_trigger > 0.0f) {
            // Proportional braking based on trigger pressure
            canyon_input.current_state.brake = true;
            canyon_input.current_state.brake_intensity = left_trigger; // Store brake intensity
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Bumpers for roll (gentle zero-g roll control)
        float roll_input = 0.0f;
        if (gamepad->buttons[GAMEPAD_BUTTON_RB]) {
            roll_input += 0.4f; // Much more gentle
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_LB]) {
            roll_input -= 0.4f; // Much more gentle
        }
        
        if (roll_input != 0.0f) {
            // Apply smoothing to roll for stability
            canyon_input.current_state.roll = canyon_input.previous_state.roll * INPUT_SMOOTHING + 
                                             roll_input * (1.0f - INPUT_SMOOTHING);
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Face buttons for zero-g maneuvering
        if (gamepad->buttons[GAMEPAD_BUTTON_A]) {
            // A button = boost (for emergency maneuvers)
            canyon_input.current_state.boost = 1.0f;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_B]) {
            // B button = emergency stop (full brake)
            canyon_input.current_state.brake = true;
            canyon_input.current_state.brake_intensity = 1.0f;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_X]) {
            // X button = strafe left (for translations)
            canyon_input.current_state.strafe_left = 0.5f;
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_Y]) {
            // Y button = strafe right (for translations)
            canyon_input.current_state.strafe_right = 0.5f;
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
    
    // Clamp values for zero-g precision
    canyon_input.current_state.thrust = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.thrust));
    canyon_input.current_state.pitch = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.pitch));
    canyon_input.current_state.yaw = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.yaw));
    canyon_input.current_state.roll = fmaxf(-1.0f, fminf(1.0f, canyon_input.current_state.roll));
    canyon_input.current_state.boost = fmaxf(0.0f, fminf(1.0f, canyon_input.current_state.boost));
    
    // Clamp additional zero-g controls
    canyon_input.current_state.brake_intensity = fmaxf(0.0f, fminf(1.0f, canyon_input.current_state.brake_intensity));
    canyon_input.current_state.strafe_left = fmaxf(0.0f, fminf(1.0f, canyon_input.current_state.strafe_left));
    canyon_input.current_state.strafe_right = fmaxf(0.0f, fminf(1.0f, canyon_input.current_state.strafe_right));
    
    // Debug output
    static int debug_counter = 0;
    if (++debug_counter % 60 == 0) { // Every second
        if (canyon_input.current_state.thrust > 0.0f || 
            fabsf(canyon_input.current_state.pitch) > 0.05f ||
            fabsf(canyon_input.current_state.yaw) > 0.05f ||
            canyon_input.current_state.brake) {
            
            printf("🚀 Zero-G Input: T:%.2f P:%.2f Y:%.2f R:%.2f ",
                   canyon_input.current_state.thrust,
                   canyon_input.current_state.pitch,
                   canyon_input.current_state.yaw,
                   canyon_input.current_state.roll);
            
            if (canyon_input.current_state.brake) {
                printf("Brake:%.2f ", canyon_input.current_state.brake_intensity);
            }
            if (canyon_input.current_state.strafe_left > 0.0f || canyon_input.current_state.strafe_right > 0.0f) {
                printf("Strafe L:%.2f R:%.2f ", canyon_input.current_state.strafe_left, canyon_input.current_state.strafe_right);
            }
            
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