/**
 * @file input_state.c
 * @brief Implementation of input state adapter for new input system
 */

#include "input_state.h"
#include "game_input.h"
#include "services/input_service.h"
#include <string.h>

// Static input state updated each frame
static InputState g_input_state = {0};

// Update the input state from the new input service
static void update_input_state(void) {
    // Clear previous state
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
                
            // Action controls
            case INPUT_ACTION_BOOST:
                g_input_state.boost = event.value;
                break;
            case INPUT_ACTION_BRAKE:
                g_input_state.brake = event.just_pressed || (event.value > 0.1f);
                g_input_state.brake_intensity = event.value;
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
}

const InputState* input_get_state(void) {
    // Update state from service
    update_input_state();
    return &g_input_state;
}