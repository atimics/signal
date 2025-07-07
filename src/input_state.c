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
    InputService* service = game_input_get_service();
    if (!service) {
        memset(&g_input_state, 0, sizeof(InputState));
        return;
    }

    // Query continuous-state actions every frame
    g_input_state.thrust = service->get_action_value(service, INPUT_ACTION_THRUST_FORWARD) - 
                           service->get_action_value(service, INPUT_ACTION_THRUST_BACK);
    g_input_state.pitch = service->get_action_value(service, INPUT_ACTION_PITCH_UP) - 
                          service->get_action_value(service, INPUT_ACTION_PITCH_DOWN);
    g_input_state.yaw = service->get_action_value(service, INPUT_ACTION_YAW_RIGHT) - 
                        service->get_action_value(service, INPUT_ACTION_YAW_LEFT);
    g_input_state.roll = service->get_action_value(service, INPUT_ACTION_ROLL_RIGHT) - 
                         service->get_action_value(service, INPUT_ACTION_ROLL_LEFT);
    g_input_state.vertical = service->get_action_value(service, INPUT_ACTION_MOVE_UP) - 
                             service->get_action_value(service, INPUT_ACTION_MOVE_DOWN);
    g_input_state.boost = service->get_action_value(service, INPUT_ACTION_BOOST);
    g_input_state.brake_intensity = service->get_action_value(service, INPUT_ACTION_BRAKE);

    // Handle one-shot (just pressed) events from the queue
    g_input_state.menu = false;
    g_input_state.confirm = false;
    g_input_state.cancel = false;
    g_input_state.camera_next = false;
    g_input_state.camera_prev = false;
    g_input_state.brake = (g_input_state.brake_intensity > 0.1f);

    InputEvent event;
    while (service->get_next_event(service, &event)) {
        if (event.just_pressed) {
            switch (event.action) {
                case INPUT_ACTION_UI_MENU: g_input_state.menu = true; break;
                case INPUT_ACTION_UI_CONFIRM: g_input_state.confirm = true; break;
                case INPUT_ACTION_UI_CANCEL: g_input_state.cancel = true; break;
                case INPUT_ACTION_PRIMARY: g_input_state.camera_next = true; break;
                case INPUT_ACTION_SECONDARY: g_input_state.camera_prev = true; break;
                case INPUT_ACTION_BRAKE: g_input_state.brake = true; break;
                default: break;
            }
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