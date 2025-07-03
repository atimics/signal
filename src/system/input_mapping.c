// src/system/input_mapping.c
// Implementation of configurable input mapping system

#include "input_mapping.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Global input mapping state
static InputMapping g_input_mapping = {0};

// Initialize input mapping system
void input_mapping_init(void) {
    memset(&g_input_mapping, 0, sizeof(InputMapping));
    
    // Create default context
    g_input_mapping.contexts[0].active = true;
    strcpy(g_input_mapping.contexts[0].name, "default");
    g_input_mapping.context_count = 1;
    g_input_mapping.active_context = 0;
    
    printf("✅ Input mapping system initialized\n");
}

void input_mapping_shutdown(void) {
    // Nothing to clean up for now
    printf("✅ Input mapping system shut down\n");
}

// Context management
uint32_t input_mapping_create_context(const char* name) {
    if (g_input_mapping.context_count >= MAX_INPUT_CONTEXTS) {
        printf("⚠️ Maximum input contexts reached\n");
        return 0;
    }
    
    uint32_t id = g_input_mapping.context_count++;
    InputContext* ctx = &g_input_mapping.contexts[id];
    strncpy(ctx->name, name, sizeof(ctx->name) - 1);
    ctx->active = false;
    ctx->action_count = 0;
    
    return id;
}

void input_mapping_activate_context(uint32_t context_id) {
    if (context_id < g_input_mapping.context_count) {
        g_input_mapping.contexts[context_id].active = true;
    }
}

void input_mapping_deactivate_context(uint32_t context_id) {
    if (context_id < g_input_mapping.context_count) {
        g_input_mapping.contexts[context_id].active = false;
    }
}

// Action registration
InputAction input_mapping_register_action(const char* name, bool is_axis) {
    // Check if action already exists
    for (uint32_t i = 0; i < MAX_INPUT_ACTIONS; i++) {
        if (g_input_mapping.actions[i].name[0] != '\0' && 
            strcmp(g_input_mapping.actions[i].name, name) == 0) {
            return i;
        }
    }
    
    // Find free slot
    for (uint32_t i = 0; i < MAX_INPUT_ACTIONS; i++) {
        if (g_input_mapping.actions[i].name[0] == '\0') {
            InputActionDef* action = &g_input_mapping.actions[i];
            strncpy(action->name, name, sizeof(action->name) - 1);
            action->is_axis = is_axis;
            action->binding_count = 0;
            action->axis_value = 0.0f;
            action->is_pressed = false;
            action->was_pressed = false;
            action->hold_time = 0.0f;
            return i;
        }
    }
    
    printf("⚠️ Maximum input actions reached\n");
    return INPUT_ACTION_NONE;
}

// Binding functions
void input_mapping_bind_key(InputAction action, sapp_keycode key, bool shift, bool ctrl, bool alt) {
    if (action >= MAX_INPUT_ACTIONS) return;
    
    InputActionDef* def = &g_input_mapping.actions[action];
    if (def->binding_count >= MAX_BINDINGS_PER_ACTION) {
        printf("⚠️ Maximum bindings for action '%s' reached\n", def->name);
        return;
    }
    
    InputBinding* binding = &def->bindings[def->binding_count++];
    binding->source_type = INPUT_SOURCE_KEYBOARD;
    binding->source.key_code = key;
    binding->require_shift = shift;
    binding->require_ctrl = ctrl;
    binding->require_alt = alt;
}

void input_mapping_bind_gamepad_button(InputAction action, GamepadButton button) {
    if (action >= MAX_INPUT_ACTIONS) return;
    
    InputActionDef* def = &g_input_mapping.actions[action];
    if (def->binding_count >= MAX_BINDINGS_PER_ACTION) {
        printf("⚠️ Maximum bindings for action '%s' reached\n", def->name);
        return;
    }
    
    InputBinding* binding = &def->bindings[def->binding_count++];
    binding->source_type = INPUT_SOURCE_GAMEPAD_BUTTON;
    binding->source.gamepad_button = button;
}

void input_mapping_bind_gamepad_axis(InputAction action, GamepadAxis axis, bool positive, float threshold) {
    if (action >= MAX_INPUT_ACTIONS) return;
    
    InputActionDef* def = &g_input_mapping.actions[action];
    if (def->binding_count >= MAX_BINDINGS_PER_ACTION) {
        printf("⚠️ Maximum bindings for action '%s' reached\n", def->name);
        return;
    }
    
    InputBinding* binding = &def->bindings[def->binding_count++];
    binding->source_type = INPUT_SOURCE_GAMEPAD_AXIS;
    binding->source.gamepad_axis = axis;
    binding->axis_positive = positive;
    binding->axis_threshold = threshold > 0 ? threshold : 0.5f;
}

void input_mapping_bind_mouse_button(InputAction action, uint8_t button) {
    if (action >= MAX_INPUT_ACTIONS) return;
    
    InputActionDef* def = &g_input_mapping.actions[action];
    if (def->binding_count >= MAX_BINDINGS_PER_ACTION) {
        printf("⚠️ Maximum bindings for action '%s' reached\n", def->name);
        return;
    }
    
    InputBinding* binding = &def->bindings[def->binding_count++];
    binding->source_type = INPUT_SOURCE_MOUSE_BUTTON;
    binding->source.mouse_button = button;
}

void input_mapping_clear_bindings(InputAction action) {
    if (action >= MAX_INPUT_ACTIONS) return;
    g_input_mapping.actions[action].binding_count = 0;
}

// Update gamepad state
static void update_gamepad_state(void) {
    // Check if gamepad is connected (gamepad 0)
    g_input_mapping.gamepad_connected = gamepad_is_connected(0);
    
    if (g_input_mapping.gamepad_connected) {
        // Get gamepad state
        GamepadState* state = gamepad_get_state(0);
        if (state) {
            // Update button states
            for (int i = 0; i < GAMEPAD_BUTTON_COUNT; i++) {
                g_input_mapping.gamepad_buttons[i] = state->buttons[i];
            }
            
            // Update axis states
            g_input_mapping.gamepad_axes[GAMEPAD_AXIS_LEFT_X] = state->left_stick_x;
            g_input_mapping.gamepad_axes[GAMEPAD_AXIS_LEFT_Y] = state->left_stick_y;
            g_input_mapping.gamepad_axes[GAMEPAD_AXIS_RIGHT_X] = state->right_stick_x;
            g_input_mapping.gamepad_axes[GAMEPAD_AXIS_RIGHT_Y] = state->right_stick_y;
            g_input_mapping.gamepad_axes[GAMEPAD_AXIS_LEFT_TRIGGER] = state->left_trigger;
            g_input_mapping.gamepad_axes[GAMEPAD_AXIS_RIGHT_TRIGGER] = state->right_trigger;
        }
    }
}

// Check if a binding is activated
static bool is_binding_activated(const InputBinding* binding) {
    switch (binding->source_type) {
        case INPUT_SOURCE_KEYBOARD: {
            if (!g_input_mapping.keyboard_state[binding->source.key_code]) {
                return false;
            }
            
            // Check modifiers
            bool shift = g_input_mapping.keyboard_state[SAPP_KEYCODE_LEFT_SHIFT] || 
                         g_input_mapping.keyboard_state[SAPP_KEYCODE_RIGHT_SHIFT];
            bool ctrl = g_input_mapping.keyboard_state[SAPP_KEYCODE_LEFT_CONTROL] || 
                        g_input_mapping.keyboard_state[SAPP_KEYCODE_RIGHT_CONTROL];
            bool alt = g_input_mapping.keyboard_state[SAPP_KEYCODE_LEFT_ALT] || 
                       g_input_mapping.keyboard_state[SAPP_KEYCODE_RIGHT_ALT];
            
            if (binding->require_shift && !shift) return false;
            if (binding->require_ctrl && !ctrl) return false;
            if (binding->require_alt && !alt) return false;
            
            return true;
        }
        
        case INPUT_SOURCE_GAMEPAD_BUTTON:
            return g_input_mapping.gamepad_connected && 
                   g_input_mapping.gamepad_buttons[binding->source.gamepad_button];
        
        case INPUT_SOURCE_GAMEPAD_AXIS: {
            if (!g_input_mapping.gamepad_connected) return false;
            
            float value = g_input_mapping.gamepad_axes[binding->source.gamepad_axis];
            if (binding->axis_positive) {
                return value > binding->axis_threshold;
            } else {
                return value < -binding->axis_threshold;
            }
        }
        
        case INPUT_SOURCE_MOUSE_BUTTON:
            return g_input_mapping.mouse_buttons[binding->source.mouse_button];
        
        default:
            return false;
    }
}

// Get axis value from binding
static float get_binding_axis_value(const InputBinding* binding) {
    switch (binding->source_type) {
        case INPUT_SOURCE_KEYBOARD:
            return is_binding_activated(binding) ? 1.0f : 0.0f;
        
        case INPUT_SOURCE_GAMEPAD_BUTTON:
            return is_binding_activated(binding) ? 1.0f : 0.0f;
        
        case INPUT_SOURCE_GAMEPAD_AXIS: {
            if (!g_input_mapping.gamepad_connected) return 0.0f;
            
            float value = g_input_mapping.gamepad_axes[binding->source.gamepad_axis];
            if (binding->axis_positive) {
                return fmaxf(0.0f, value);
            } else {
                return fabsf(fminf(0.0f, value));
            }
        }
        
        default:
            return 0.0f;
    }
}

// Update - call once per frame
void input_mapping_update(float delta_time) {
    // Update gamepad state
    update_gamepad_state();
    
    // Update all actions
    for (uint32_t i = 0; i < MAX_INPUT_ACTIONS; i++) {
        InputActionDef* action = &g_input_mapping.actions[i];
        if (action->name[0] == '\0') continue;
        
        // Save previous state
        action->was_pressed = action->is_pressed;
        
        // Check all bindings
        bool any_pressed = false;
        float max_axis_value = 0.0f;
        
        for (uint32_t j = 0; j < action->binding_count; j++) {
            InputBinding* binding = &action->bindings[j];
            
            if (action->is_axis) {
                float value = get_binding_axis_value(binding);
                if (fabsf(value) > fabsf(max_axis_value)) {
                    max_axis_value = value;
                }
            } else {
                if (is_binding_activated(binding)) {
                    any_pressed = true;
                    break;
                }
            }
        }
        
        // Update action state
        if (action->is_axis) {
            action->axis_value = max_axis_value;
            action->is_pressed = fabsf(max_axis_value) > 0.1f;
        } else {
            action->is_pressed = any_pressed;
            action->axis_value = any_pressed ? 1.0f : 0.0f;
        }
        
        // Update hold time
        if (action->is_pressed) {
            action->hold_time += delta_time;
        } else {
            action->hold_time = 0.0f;
        }
    }
    
    // Reset mouse delta after processing
    g_input_mapping.mouse_delta_x = 0.0f;
    g_input_mapping.mouse_delta_y = 0.0f;
}

// Event handling
bool input_mapping_handle_event(const sapp_event* event) {
    switch (event->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (event->key_code < SAPP_MAX_KEYCODES) {
                g_input_mapping.keyboard_state[event->key_code] = true;
            }
            break;
            
        case SAPP_EVENTTYPE_KEY_UP:
            if (event->key_code < SAPP_MAX_KEYCODES) {
                g_input_mapping.keyboard_state[event->key_code] = false;
            }
            break;
            
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            if (event->mouse_button < 3) {
                g_input_mapping.mouse_buttons[event->mouse_button] = true;
            }
            break;
            
        case SAPP_EVENTTYPE_MOUSE_UP:
            if (event->mouse_button < 3) {
                g_input_mapping.mouse_buttons[event->mouse_button] = false;
            }
            break;
            
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            g_input_mapping.mouse_delta_x += event->mouse_dx;
            g_input_mapping.mouse_delta_y += event->mouse_dy;
            break;
            
        default:
            break;
    }
    
    return false; // Input mapping doesn't consume events
}

// Query actions
bool input_mapping_is_pressed(InputAction action) {
    if (action >= MAX_INPUT_ACTIONS) return false;
    return g_input_mapping.actions[action].is_pressed;
}

bool input_mapping_just_pressed(InputAction action) {
    if (action >= MAX_INPUT_ACTIONS) return false;
    InputActionDef* def = &g_input_mapping.actions[action];
    return def->is_pressed && !def->was_pressed;
}

bool input_mapping_just_released(InputAction action) {
    if (action >= MAX_INPUT_ACTIONS) return false;
    InputActionDef* def = &g_input_mapping.actions[action];
    return !def->is_pressed && def->was_pressed;
}

float input_mapping_get_axis(InputAction action) {
    if (action >= MAX_INPUT_ACTIONS) return 0.0f;
    return g_input_mapping.actions[action].axis_value;
}

float input_mapping_get_hold_time(InputAction action) {
    if (action >= MAX_INPUT_ACTIONS) return 0.0f;
    return g_input_mapping.actions[action].hold_time;
}

// Default flight controls
void input_mapping_load_default_flight_controls(void) {
    // Movement
    input_mapping_bind_key(INPUT_ACTION_MOVE_FORWARD, SAPP_KEYCODE_W, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_MOVE_FORWARD, SAPP_KEYCODE_SPACE, false, false, false);
    input_mapping_bind_gamepad_axis(INPUT_ACTION_MOVE_FORWARD, GAMEPAD_AXIS_RIGHT_TRIGGER, true, 0.1f);
    
    input_mapping_bind_key(INPUT_ACTION_MOVE_BACKWARD, SAPP_KEYCODE_S, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_MOVE_BACKWARD, SAPP_KEYCODE_X, false, false, false);
    input_mapping_bind_gamepad_axis(INPUT_ACTION_MOVE_BACKWARD, GAMEPAD_AXIS_LEFT_TRIGGER, true, 0.1f);
    
    input_mapping_bind_key(INPUT_ACTION_MOVE_LEFT, SAPP_KEYCODE_A, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_MOVE_RIGHT, SAPP_KEYCODE_D, false, false, false);
    
    input_mapping_bind_key(INPUT_ACTION_MOVE_UP, SAPP_KEYCODE_R, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_MOVE_DOWN, SAPP_KEYCODE_F, false, false, false);
    
    // Rotation (analog)
    InputAction pitch = input_mapping_register_action("pitch", true);
    input_mapping_bind_key(pitch, SAPP_KEYCODE_UP, false, false, false);
    input_mapping_bind_key(pitch, SAPP_KEYCODE_DOWN, false, false, false);
    input_mapping_bind_gamepad_axis(pitch, GAMEPAD_AXIS_LEFT_Y, false, 0.1f);
    
    InputAction yaw = input_mapping_register_action("yaw", true);
    input_mapping_bind_key(yaw, SAPP_KEYCODE_LEFT, false, false, false);
    input_mapping_bind_key(yaw, SAPP_KEYCODE_RIGHT, false, false, false);
    input_mapping_bind_gamepad_axis(yaw, GAMEPAD_AXIS_LEFT_X, true, 0.1f);
    
    InputAction roll = input_mapping_register_action("roll", true);
    input_mapping_bind_key(roll, SAPP_KEYCODE_Q, false, false, false);
    input_mapping_bind_key(roll, SAPP_KEYCODE_E, false, false, false);
    input_mapping_bind_gamepad_button(roll, GAMEPAD_BUTTON_LB);
    input_mapping_bind_gamepad_button(roll, GAMEPAD_BUTTON_RB);
    
    // Actions
    input_mapping_bind_key(INPUT_ACTION_BOOST, SAPP_KEYCODE_LEFT_SHIFT, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_BOOST, GAMEPAD_BUTTON_A);
    
    input_mapping_bind_key(INPUT_ACTION_BRAKE, SAPP_KEYCODE_LEFT_ALT, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_BRAKE, GAMEPAD_BUTTON_B);
    
    // Camera
    input_mapping_bind_key(INPUT_ACTION_CAMERA_MODE, SAPP_KEYCODE_TAB, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_CAMERA_MODE, GAMEPAD_BUTTON_BACK);
    
    // UI
    input_mapping_bind_key(INPUT_ACTION_MENU, SAPP_KEYCODE_ESCAPE, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_MENU, GAMEPAD_BUTTON_START);
    
    // Debug
    input_mapping_bind_key(INPUT_ACTION_DEBUG_TOGGLE, SAPP_KEYCODE_GRAVE_ACCENT, false, false, false);
    
    printf("✅ Loaded default flight controls\n");
}

// Default menu controls
void input_mapping_load_default_menu_controls(void) {
    // Navigation
    input_mapping_bind_key(INPUT_ACTION_NAV_UP, SAPP_KEYCODE_UP, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_NAV_UP, SAPP_KEYCODE_W, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_NAV_UP, GAMEPAD_BUTTON_DPAD_UP);
    input_mapping_bind_gamepad_axis(INPUT_ACTION_NAV_UP, GAMEPAD_AXIS_LEFT_Y, false, 0.5f);
    
    input_mapping_bind_key(INPUT_ACTION_NAV_DOWN, SAPP_KEYCODE_DOWN, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_NAV_DOWN, SAPP_KEYCODE_S, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_NAV_DOWN, GAMEPAD_BUTTON_DPAD_DOWN);
    input_mapping_bind_gamepad_axis(INPUT_ACTION_NAV_DOWN, GAMEPAD_AXIS_LEFT_Y, true, 0.5f);
    
    input_mapping_bind_key(INPUT_ACTION_NAV_LEFT, SAPP_KEYCODE_LEFT, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_NAV_LEFT, SAPP_KEYCODE_A, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_NAV_LEFT, GAMEPAD_BUTTON_DPAD_LEFT);
    input_mapping_bind_gamepad_axis(INPUT_ACTION_NAV_LEFT, GAMEPAD_AXIS_LEFT_X, false, 0.5f);
    
    input_mapping_bind_key(INPUT_ACTION_NAV_RIGHT, SAPP_KEYCODE_RIGHT, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_NAV_RIGHT, SAPP_KEYCODE_D, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_NAV_RIGHT, GAMEPAD_BUTTON_DPAD_RIGHT);
    input_mapping_bind_gamepad_axis(INPUT_ACTION_NAV_RIGHT, GAMEPAD_AXIS_LEFT_X, true, 0.5f);
    
    // Actions
    input_mapping_bind_key(INPUT_ACTION_CONFIRM, SAPP_KEYCODE_ENTER, false, false, false);
    input_mapping_bind_key(INPUT_ACTION_CONFIRM, SAPP_KEYCODE_SPACE, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_CONFIRM, GAMEPAD_BUTTON_A);
    
    input_mapping_bind_key(INPUT_ACTION_CANCEL, SAPP_KEYCODE_ESCAPE, false, false, false);
    input_mapping_bind_gamepad_button(INPUT_ACTION_CANCEL, GAMEPAD_BUTTON_B);
    
    printf("✅ Loaded default menu controls\n");
}

// Debug print
void input_mapping_debug_print(void) {
    printf("\n=== Input Mapping Debug ===\n");
    printf("Gamepad: %s\n", g_input_mapping.gamepad_connected ? "Connected" : "Disconnected");
    
    if (g_input_mapping.gamepad_connected) {
        printf("  Axes: LX=%.2f LY=%.2f RX=%.2f RY=%.2f LT=%.2f RT=%.2f\n",
               g_input_mapping.gamepad_axes[GAMEPAD_AXIS_LEFT_X],
               g_input_mapping.gamepad_axes[GAMEPAD_AXIS_LEFT_Y],
               g_input_mapping.gamepad_axes[GAMEPAD_AXIS_RIGHT_X],
               g_input_mapping.gamepad_axes[GAMEPAD_AXIS_RIGHT_Y],
               g_input_mapping.gamepad_axes[GAMEPAD_AXIS_LEFT_TRIGGER],
               g_input_mapping.gamepad_axes[GAMEPAD_AXIS_RIGHT_TRIGGER]);
    }
    
    printf("\nActive Actions:\n");
    for (uint32_t i = 0; i < MAX_INPUT_ACTIONS; i++) {
        InputActionDef* action = &g_input_mapping.actions[i];
        if (action->name[0] != '\0' && action->is_pressed) {
            printf("  %s: %.2f (held %.2fs)\n", action->name, action->axis_value, action->hold_time);
        }
    }
    printf("========================\n");
}