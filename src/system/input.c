// Canyon Racing Input System
// Integrates advanced input processing for smooth gamepad handling

#include "input.h"
#include "gamepad.h"
#include "../component/look_target.h"
#include "../input_processing.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Input configuration 
#define LOOK_SENSITIVITY 2.0f           // Camera rotation speed
#define MOUSE_SENSITIVITY 0.003f        // Mouse look sensitivity 
#define AUTO_LEVEL_STRENGTH 2.0f        // How fast ship auto-levels

// Input State with Advanced Processing
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
    
    // Sprint 22 Advanced Input Processing
    ProductionInputProcessor processor;
    bool processor_enabled;
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
        printf("⚠️  AAA Input: Gamepad initialization failed - keyboard/mouse only\n");
    } else {
        printf("🎮 AAA Input: Gamepad system ready\n");
    }
    
    // Clear input state
    memset(&canyon_input, 0, sizeof(CanyonRacingInput));
    memset(keyboard_state, 0, sizeof(keyboard_state));
    
    // Initialize look target
    look_target_init(&canyon_input.look_target);
    
    // Initialize Sprint 22 advanced input processor
    production_input_processor_init(&canyon_input.processor);
    canyon_input.processor_enabled = false;  // Temporarily disable for debugging
    
    canyon_input.initialized = true;
    printf("✅ AAA Input system initialized with statistical processing\n");
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
    
    // Process gamepad input with AAA Statistical Processing
    if (using_gamepad && canyon_input.processor_enabled) {
        // Get raw left stick input - pass unfiltered values to let advanced processor handle filtering
        InputVector2 raw_stick = {gamepad->left_stick_x, gamepad->left_stick_y};
        
        // Only process if we have meaningful input (avoid processing pure noise)
        float stick_magnitude = sqrtf(raw_stick.x * raw_stick.x + raw_stick.y * raw_stick.y);
        
        if (stick_magnitude > 0.05f) { // Only process if stick is moved beyond minimal threshold
            // Process through Sprint 22 advanced pipeline
            Vector6 processed_output = production_input_process(&canyon_input.processor, raw_stick, 0.016f);
            
            // Extract processed input values - much smoother than raw gamepad
            canyon_input.current_state.yaw = processed_output.yaw;     // Processed yaw from left stick X
            canyon_input.current_state.pitch = -processed_output.pitch; // Invert pitch for flight (negative Y = nose down)
            
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
            
            // Debug output for processed values
            static int stick_debug_counter = 0;
            if (++stick_debug_counter % 60 == 0) { // Show processed vs raw
                printf("🎮 AAA STICK: Raw(%.3f,%.3f) → Processed Pitch:%.3f Yaw:%.3f\n", 
                       raw_stick.x, raw_stick.y, canyon_input.current_state.pitch, canyon_input.current_state.yaw);
            }
        } else {
            // Stick at rest - let advanced processor calibrate but don't use output for control
            production_input_process(&canyon_input.processor, raw_stick, 0.016f);
        }
    }
    else if (using_gamepad) {
        // Fallback to simple processing if advanced processor is disabled
        float left_x = apply_deadzone(gamepad->left_stick_x, 0.08f);
        float left_y = apply_deadzone(gamepad->left_stick_y, 0.08f);
        
        canyon_input.current_state.yaw = left_x * 0.5f;     // Increased sensitivity
        canyon_input.current_state.pitch = -left_y * 0.5f;  // Inverted for flight
        
        canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        
    }
    
    // Common gamepad processing (for both advanced and fallback modes)
    if (using_gamepad) {
        // Right stick: Targeting reticle control (where you want to fly)
        float right_x = apply_deadzone(gamepad->right_stick_x, 0.15f);
        float right_y = apply_deadzone(gamepad->right_stick_y, 0.15f);
        
        if (right_x != 0.0f || right_y != 0.0f) {
            float delta_azimuth = right_x * LOOK_SENSITIVITY * 0.016f;
            float delta_elevation = right_y * LOOK_SENSITIVITY * 0.016f; // Remove inversion - let user control naturally
            
            look_target_update(&canyon_input.look_target, &player_position,
                             delta_azimuth, delta_elevation, 0.0f);
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        }
        
        // Right trigger for forward thrust (smooth, reliable processing)
        float right_trigger = apply_deadzone(gamepad->right_trigger, 0.08f); // Lower deadzone for responsiveness
        
        // Debug thrust input more frequently
        static int thrust_debug_counter = 0;
        if (++thrust_debug_counter % 30 == 0) { // Show every half second
            printf("🚀 THRUST DEBUG: Raw RT=%.3f, Processed RT=%.3f, Final thrust=%.3f\n", 
                   gamepad->right_trigger, right_trigger, 
                   right_trigger > 0.0f ? right_trigger * 0.8f : 0.0f);
        }
        
        if (right_trigger > 0.0f) {
            // Direct thrust mapping with gentle scaling
            canyon_input.current_state.thrust = right_trigger * 0.8f; // Slightly reduced maximum thrust for control
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
        } else {
            // Ensure thrust is cleared when trigger not pressed
            canyon_input.current_state.thrust = 0.0f;
        }
        
        // Left trigger for braking with enhanced zero-g stopping
        float left_trigger = apply_deadzone(gamepad->left_trigger, 0.05f);
        if (left_trigger > 0.0f) {
            // Proportional braking based on trigger pressure
            canyon_input.current_state.brake = true;
            canyon_input.current_state.brake_intensity = left_trigger; // Store brake intensity
            canyon_input.last_device = INPUT_DEVICE_GAMEPAD;
            
        } else {
            // Ensure brake is cleared when not pressed
            canyon_input.current_state.brake = false;
            canyon_input.current_state.brake_intensity = 0.0f;
        }
        
        // Bumpers for roll (gentle zero-g roll control)
        float roll_input = 0.0f;
        if (gamepad->buttons[GAMEPAD_BUTTON_RB]) {
            roll_input += 0.5f; // Gentle roll
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_LB]) {
            roll_input -= 0.5f; // Gentle roll
        }
        
        if (roll_input != 0.0f) {
            canyon_input.current_state.roll = roll_input;
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

// Production input processor access
ProductionInputProcessor* input_get_processor(void) { 
    return canyon_input.initialized ? &canyon_input.processor : NULL; 
}

void input_set_processing_config(bool enable_neural, bool enable_mrac, bool enable_kalman) {
    if (!canyon_input.initialized) return;
    
    canyon_input.processor.config.enable_neural_processing = enable_neural;
    canyon_input.processor.config.enable_mrac_safety = enable_mrac;
    canyon_input.processor.config.enable_kalman_filtering = enable_kalman;
    
    printf("🎮 Input Processing Config: Neural=%s MRAC=%s Kalman=%s\n",
           enable_neural ? "ON" : "OFF",
           enable_mrac ? "ON" : "OFF", 
           enable_kalman ? "ON" : "OFF");
}