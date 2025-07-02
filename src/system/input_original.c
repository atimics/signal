// src/system/input.c
// Unified input abstraction implementation

#include "input.h"
#include "gamepad.h"
#include "gamepad_hotplug.h"
#include "../input_processing.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Enhanced input processing
static ProductionInputProcessor input_processor = {0};

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
    
    // Initialize enhanced input processing system
    production_input_processor_init(&input_processor);
    printf("✨ Enhanced input processing system initialized\n");
    
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
    
    // Update hot-plug detection
    gamepad_update_hotplug(0.016f); // Assume 60 FPS for now
    
    // Poll gamepad
    gamepad_poll();
    
    // Clear current input
    memset(&current_input, 0, sizeof(InputState));
    
    // Process keyboard input (digital)
    // Linear movement
    if (keyboard_state[INPUT_ACTION_THRUST_FORWARD]) current_input.thrust += 1.0f;
    if (keyboard_state[INPUT_ACTION_THRUST_BACKWARD]) current_input.thrust -= 1.0f;
    if (keyboard_state[INPUT_ACTION_STRAFE_RIGHT]) current_input.strafe += 1.0f;
    if (keyboard_state[INPUT_ACTION_STRAFE_LEFT]) current_input.strafe -= 1.0f;
    if (keyboard_state[INPUT_ACTION_MANEUVER_UP]) current_input.vertical += 1.0f;
    if (keyboard_state[INPUT_ACTION_MANEUVER_DOWN]) current_input.vertical -= 1.0f;
    
    // Angular movement (6DOF)
    if (keyboard_state[INPUT_ACTION_PITCH_UP]) current_input.pitch += 1.0f;
    if (keyboard_state[INPUT_ACTION_PITCH_DOWN]) current_input.pitch -= 1.0f;
    if (keyboard_state[INPUT_ACTION_YAW_RIGHT]) current_input.yaw += 1.0f;
    if (keyboard_state[INPUT_ACTION_YAW_LEFT]) current_input.yaw -= 1.0f;
    if (keyboard_state[INPUT_ACTION_ROLL_RIGHT]) current_input.roll += 1.0f;
    if (keyboard_state[INPUT_ACTION_ROLL_LEFT]) current_input.roll -= 1.0f;
    
    // Modifiers
    if (keyboard_state[INPUT_ACTION_BOOST]) current_input.boost = 1.0f;
    if (keyboard_state[INPUT_ACTION_BRAKE]) current_input.brake = true;
    
    // Add gamepad input (analog) - Enhanced Processing Pipeline
    GamepadState* gamepad = gamepad_get_state(0);
    if (gamepad && gamepad->connected) {
        // Get raw gamepad input
        InputVector2 raw_left_stick = {gamepad->left_stick_x, gamepad->left_stick_y};
        
        // Process left stick through enhanced pipeline (primary flight control)
        Vector6 left_stick_output = production_input_process(&input_processor, raw_left_stick, 0.016f);
        
        // Apply processed left stick to flight controls
        current_input.pitch -= left_stick_output.pitch; // Invert Y (up stick = nose up)
        current_input.yaw += left_stick_output.yaw;     // Left stick X = turn
        
        // Process right stick for banking/vertical thrust (simpler processing for now)
        const float deadzone = 0.08f;
        if (fabsf(gamepad->right_stick_x) > deadzone) {
            current_input.strafe += gamepad->right_stick_x * 1.2f; // Amplified banking input
        }
        if (fabsf(gamepad->right_stick_y) > deadzone) {
            current_input.vertical -= gamepad->right_stick_y; // Invert Y (up stick = thrust up)
        }
        
        // Triggers for forward/reverse thrust
        if (gamepad->right_trigger > deadzone) {
            current_input.thrust += gamepad->right_trigger; // Right trigger = forward
        }
        if (gamepad->left_trigger > deadzone) {
            current_input.thrust -= gamepad->left_trigger; // Left trigger = reverse
        }
        
        // Track that gamepad was used
        if (fabsf(gamepad->left_stick_x) > deadzone || fabsf(gamepad->left_stick_y) > deadzone ||
            fabsf(gamepad->right_stick_x) > deadzone || fabsf(gamepad->right_stick_y) > deadzone ||
            gamepad->left_trigger > deadzone || gamepad->right_trigger > deadzone ||
            gamepad->buttons[GAMEPAD_BUTTON_A] || gamepad->buttons[GAMEPAD_BUTTON_B]) {
            input_set_last_device_type(INPUT_DEVICE_GAMEPAD);
        }
        
        // Debug: Log enhanced input processing
        static int input_debug_counter = 0;
        if (++input_debug_counter % 10 == 0) {  // Every ~0.16 seconds at 60fps
            const float debug_deadzone = 0.08f;
            if (fabsf(gamepad->left_stick_x) > debug_deadzone || fabsf(gamepad->left_stick_y) > debug_deadzone ||
                fabsf(gamepad->right_stick_x) > debug_deadzone || fabsf(gamepad->right_stick_y) > debug_deadzone ||
                gamepad->left_trigger > debug_deadzone || gamepad->right_trigger > debug_deadzone) {
                printf("🕹️ RAW: LS(%.3f,%.3f) RS(%.3f,%.3f) LT:%.3f RT:%.3f\n",
                       gamepad->left_stick_x, gamepad->left_stick_y,
                       gamepad->right_stick_x, gamepad->right_stick_y,
                       gamepad->left_trigger, gamepad->right_trigger);
                printf("   ENHANCED: thrust=%.3f pitch=%.3f yaw=%.3f roll=%.3f strafe=%.3f\n",
                       current_input.thrust, current_input.pitch, current_input.yaw, 
                       current_input.roll, current_input.strafe);
                printf("   Processor: Cal=%d Neural=%d MRAC=%d\n",
                       input_processor.calibration_state, 
                       input_processor.config.enable_neural_processing,
                       input_processor.config.enable_mrac_safety);
                
                // Extra debug for trigger issue
                if (gamepad->right_trigger > debug_deadzone) {
                    printf("   ⚠️ RT pressed: %.3f -> thrust: %.3f (yaw should be: %.3f)\n",
                           gamepad->right_trigger, current_input.thrust, current_input.yaw);
                }
            }
        }
        
        // Shoulder buttons for roll
        if (gamepad->buttons[GAMEPAD_BUTTON_RB]) {
            current_input.roll += 1.0f; // Right bumper = roll right
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_LB]) {
            current_input.roll -= 1.0f; // Left bumper = roll left
        }
        
        // Face buttons
        if (gamepad->buttons[GAMEPAD_BUTTON_A]) {
            current_input.boost = 1.0f; // A = Boost
        }
        if (gamepad->buttons[GAMEPAD_BUTTON_B]) {
            current_input.brake = true; // B = Brake
        }
    }
    
    // Clamp combined values
    current_input.thrust = fmaxf(-1.0f, fminf(1.0f, current_input.thrust));
    current_input.strafe = fmaxf(-1.0f, fminf(1.0f, current_input.strafe));
    current_input.vertical = fmaxf(-1.0f, fminf(1.0f, current_input.vertical));
    current_input.pitch = fmaxf(-1.0f, fminf(1.0f, current_input.pitch));
    current_input.yaw = fmaxf(-1.0f, fminf(1.0f, current_input.yaw));
    current_input.roll = fmaxf(-1.0f, fminf(1.0f, current_input.roll));
    current_input.boost = fmaxf(0.0f, fminf(1.0f, current_input.boost));
}

bool input_handle_keyboard(int key_code, bool is_pressed) {
    if (!input_initialized) return false;
    
    InputAction action = INPUT_ACTION_COUNT; // Invalid
    
    switch (key_code) {
        // Modern WASD + Mouse flight scheme
        case SAPP_KEYCODE_W:
            action = INPUT_ACTION_PITCH_DOWN;  // W = nose down (dive)
            break;
        case SAPP_KEYCODE_S:
            action = INPUT_ACTION_PITCH_UP;    // S = nose up (climb)
            break;
        case SAPP_KEYCODE_A:
            action = INPUT_ACTION_STRAFE_LEFT;
            break;
        case SAPP_KEYCODE_D:
            action = INPUT_ACTION_STRAFE_RIGHT;
            break;
        case SAPP_KEYCODE_SPACE:
            action = INPUT_ACTION_THRUST_FORWARD;
            break;
        case SAPP_KEYCODE_X:
            action = INPUT_ACTION_THRUST_BACKWARD;
            break;
        case SAPP_KEYCODE_R:
            action = INPUT_ACTION_MANEUVER_UP;
            break;
        case SAPP_KEYCODE_F:
            action = INPUT_ACTION_MANEUVER_DOWN;
            break;
        case SAPP_KEYCODE_LEFT_SHIFT:
        case SAPP_KEYCODE_RIGHT_SHIFT:
            action = INPUT_ACTION_BOOST;
            break;
        case SAPP_KEYCODE_LEFT_ALT:
        case SAPP_KEYCODE_RIGHT_ALT:
            action = INPUT_ACTION_BRAKE;
            break;
        
        // Roll controls (Q/E like many flight games)
        case SAPP_KEYCODE_Q:
            action = INPUT_ACTION_ROLL_LEFT;
            break;
        case SAPP_KEYCODE_E:
            action = INPUT_ACTION_ROLL_RIGHT;
            break;
            
        // Arrow keys as backup for pitch/yaw (for non-mouse users)
        case SAPP_KEYCODE_UP:
            action = INPUT_ACTION_PITCH_UP;
            break;
        case SAPP_KEYCODE_DOWN:
            action = INPUT_ACTION_PITCH_DOWN;
            break;
        case SAPP_KEYCODE_LEFT:
            action = INPUT_ACTION_YAW_LEFT;
            break;
        case SAPP_KEYCODE_RIGHT:
            action = INPUT_ACTION_YAW_RIGHT;
            break;
            
        default:
            return false;
    }
    
    if (action < INPUT_ACTION_COUNT) {
        keyboard_state[action] = is_pressed;
        
        // Track that keyboard was used
        if (is_pressed) {
            input_set_last_device_type(INPUT_DEVICE_KEYBOARD);
        }
        
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
    
    printf("🎮 Input: T:%.2f S:%.2f V:%.2f P:%.2f Y:%.2f R:%.2f B:%.2f Brake:%s Gamepad:%s\n",
           current_input.thrust,
           current_input.strafe,
           current_input.vertical,
           current_input.pitch,
           current_input.yaw,
           current_input.roll,
           current_input.boost,
           current_input.brake ? "ON" : "OFF",
           input_has_gamepad() ? "YES" : "NO");
}

// Get access to the enhanced input processor for debugging
ProductionInputProcessor* input_get_processor(void) {
    return input_initialized ? &input_processor : NULL;
}

// Enable/disable enhanced processing features
void input_set_processing_config(bool enable_neural, bool enable_mrac, bool enable_kalman) {
    if (!input_initialized) return;
    
    input_processor.config.enable_neural_processing = enable_neural;
    input_processor.config.enable_mrac_safety = enable_mrac;
    input_processor.config.enable_kalman_filtering = enable_kalman;
    
    printf("🔧 Input processing config: Neural=%s MRAC=%s Kalman=%s\n",
           enable_neural ? "ON" : "OFF",
           enable_mrac ? "ON" : "OFF", 
           enable_kalman ? "ON" : "OFF");
}
