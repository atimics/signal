/**
 * @file input_state.c
 * @brief Implementation of input state adapter for new input system
 */

#include "input_state.h"
#include "game_input.h"
#include "services/input_service.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

// Static input state updated each frame
static InputState g_input_state = {0};

// Debug logging state
static bool debug_input_enabled = false;
static uint32_t debug_frame_counter = 0;

// Update the input state from the new input service
static void update_input_state(void) {
    // Clear previous state - just_pressed flags are one-frame only
    memset(&g_input_state, 0, sizeof(InputState));
    
    // Get input service
    InputService* service = game_input_get_service();
    if (!service || !service->get_next_event) {
        return;
    }
    
    // Process all events for this frame
    InputEvent event;
    while (service->get_next_event(service, &event)) {
        switch (event.action) {
            // Movement controls
            case INPUT_ACTION_MOVE_FORWARD:
                g_input_state.thrust = event.value;
                break;
            case INPUT_ACTION_MOVE_UP:
                g_input_state.vertical = event.value;
                break;
            case INPUT_ACTION_CAMERA_PITCH:
                g_input_state.pitch = event.value;
                break;
            case INPUT_ACTION_CAMERA_YAW:
                g_input_state.yaw = event.value;
                break;
            case INPUT_ACTION_MOVE_LEFT:
                g_input_state.strafe_left = event.value;
                g_input_state.roll = -event.value; // Banking turns
                break;
            case INPUT_ACTION_MOVE_RIGHT:
                g_input_state.strafe_right = event.value;
                g_input_state.roll = event.value; // Banking turns
                break;
                
            // Flight controls (Sprint 25) - additive for multi-device support
            case INPUT_ACTION_THRUST_FORWARD:
                g_input_state.thrust += event.value;
                break;
            case INPUT_ACTION_THRUST_BACK:
                g_input_state.thrust -= event.value;
                break;
            case INPUT_ACTION_PITCH_UP:
                g_input_state.pitch += event.value;
                break;
            case INPUT_ACTION_PITCH_DOWN:
                g_input_state.pitch -= event.value;
                break;
            case INPUT_ACTION_YAW_LEFT:
                g_input_state.yaw -= event.value;
                break;
            case INPUT_ACTION_YAW_RIGHT:
                g_input_state.yaw += event.value;
                break;
            case INPUT_ACTION_ROLL_LEFT:
                g_input_state.roll -= event.value;
                break;
            case INPUT_ACTION_ROLL_RIGHT:
                g_input_state.roll += event.value;
                break;
                
            // Action controls - boost/brake with just_pressed detection
            case INPUT_ACTION_BOOST:
                g_input_state.boost = event.value;
                if (event.just_pressed) {
                    g_input_state.boost_pressed = true;
                }
                break;
            case INPUT_ACTION_BRAKE:
                g_input_state.brake = event.just_pressed || (event.value > 0.1f);
                g_input_state.brake_intensity = event.value;
                if (event.just_pressed) {
                    g_input_state.brake_pressed = true;
                }
                break;
                
            // UI controls
            case INPUT_ACTION_UI_MENU:
                if (event.just_pressed) {
                    g_input_state.menu = true;
                }
                break;
            case INPUT_ACTION_UI_CONFIRM:
                if (event.just_pressed) {
                    g_input_state.confirm = true;
                }
                break;
            case INPUT_ACTION_UI_CANCEL:
                if (event.just_pressed) {
                    g_input_state.cancel = true;
                }
                break;
                
            // Camera controls - using primary/secondary for now
            case INPUT_ACTION_PRIMARY:
                if (event.just_pressed) {
                    g_input_state.camera_next = true;
                }
                break;
            case INPUT_ACTION_SECONDARY:
                if (event.just_pressed) {
                    g_input_state.camera_prev = true;
                }
                break;
                
            default:
                break;
        }
    }
    
    // Clamp all analog values to prevent overflow from multiple inputs
    g_input_state.thrust = fmaxf(-1.0f, fminf(1.0f, g_input_state.thrust));
    g_input_state.pitch = fmaxf(-1.0f, fminf(1.0f, g_input_state.pitch));
    g_input_state.yaw = fmaxf(-1.0f, fminf(1.0f, g_input_state.yaw));
    g_input_state.roll = fmaxf(-1.0f, fminf(1.0f, g_input_state.roll));
    g_input_state.boost = fmaxf(0.0f, fminf(1.0f, g_input_state.boost));
    g_input_state.brake_intensity = fmaxf(0.0f, fminf(1.0f, g_input_state.brake_intensity));
    
    // Debug logging (Sprint 25)
    if (debug_input_enabled && ++debug_frame_counter % 60 == 0) {
        printf("📍 Input Debug [Frame %d]:\n", debug_frame_counter);
        printf("   Thrust: %.3f | Vertical: %.3f | Pitch: %.3f | Yaw: %.3f | Roll: %.3f\n",
               g_input_state.thrust, g_input_state.vertical, g_input_state.pitch, 
               g_input_state.yaw, g_input_state.roll);
        printf("   Strafe L/R: %.3f/%.3f | Boost: %.3f%s | Brake: %s (%.3f)\n",
               g_input_state.strafe_left, g_input_state.strafe_right,
               g_input_state.boost, g_input_state.boost_pressed ? " [PRESSED]" : "",
               g_input_state.brake ? "ON" : "OFF", g_input_state.brake_intensity);
        if (g_input_state.brake_pressed) printf("   🛑 BRAKE JUST PRESSED\n");
    }
}

const InputState* input_get_state(void) {
    // Update state from service
    update_input_state();
    return &g_input_state;
}

// Debug functions (Sprint 25)
void input_set_debug_enabled(bool enabled) {
    debug_input_enabled = enabled;
    if (enabled) {
        printf("🎮 Input debug logging ENABLED\n");
    } else {
        printf("🎮 Input debug logging DISABLED\n");
    }
}

bool input_is_debug_enabled(void) {
    return debug_input_enabled;
}