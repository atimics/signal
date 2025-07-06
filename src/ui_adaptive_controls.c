// src/ui_adaptive_controls_simple.c
// Simplified implementation of adaptive UI controls without legacy gamepad API

#include "ui_adaptive_controls.h"
#include "game_input.h"
#include "services/input_service.h"
#include "sokol_app.h"
#include <stdio.h>
#include <string.h>

// Include the InputDeviceType from HAL
#include "hal/input_hal.h"

// Add keyboard+mouse alias if needed
#ifndef INPUT_DEVICE_KEYBOARD_MOUSE
#define INPUT_DEVICE_KEYBOARD_MOUSE INPUT_DEVICE_KEYBOARD
#endif

// State tracking
static struct {
    int forced_device_type;  // -1 for auto, or INPUT_DEVICE_* value
    float fade_timer;        // For smooth transitions
    InputDeviceType current_display_device;
    InputDeviceType target_display_device;
} adaptive_state = {
    .forced_device_type = -1,
    .fade_timer = 0.0f,
    .current_display_device = INPUT_DEVICE_KEYBOARD,
    .target_display_device = INPUT_DEVICE_KEYBOARD
};

// Common control hints
const ControlHint UI_HINT_SELECT = {
    "Select", "Enter", "A", SAPP_KEYCODE_ENTER, 0
};

const ControlHint UI_HINT_BACK = {
    "Back", "ESC", "B", SAPP_KEYCODE_ESCAPE, 1
};

const ControlHint UI_HINT_NAVIGATE = {
    "Navigate", "↑↓ / WS", "LS/D-Pad", SAPP_KEYCODE_UP, 10
};

const ControlHint UI_HINT_PAUSE = {
    "Pause", "ESC", "Start", SAPP_KEYCODE_ESCAPE, 7
};

// Public API implementation
void ui_adaptive_init(void) {
    // For now, always show keyboard controls
    // TODO: Detect gamepad through new input system
    adaptive_state.current_display_device = INPUT_DEVICE_KEYBOARD;
    adaptive_state.target_display_device = INPUT_DEVICE_KEYBOARD;
}

void ui_adaptive_update(float dt) {
    (void)dt;
    // Simple update - no animation for now
    adaptive_state.current_display_device = adaptive_state.target_display_device;
}

void ui_adaptive_set_device_type(int device_type) {
    adaptive_state.forced_device_type = device_type;
    if (device_type >= 0) {
        adaptive_state.target_display_device = (InputDeviceType)device_type;
    }
}

bool ui_adaptive_should_show_gamepad(void) {
    return adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD;
}

const char* ui_adaptive_get_hint(const ControlHint* hint) {
    if (!hint) return "";
    
    if (adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD) {
        return hint->gamepad_hint;
    } else {
        return hint->keyboard_hint;
    }
}

const char* ui_adaptive_get_action_hint(const char* action_name,
                                       const char* keyboard_hint,
                                       const char* gamepad_hint) {
    (void)action_name;
    
    if (adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD) {
        return gamepad_hint;
    } else {
        return keyboard_hint;
    }
}

// Simplified navigation - no gamepad support yet
bool ui_adaptive_navigate_vertical(int* selected_index, int menu_item_count) {
    (void)selected_index;
    (void)menu_item_count;
    return false; // Let keyboard/mouse handle it
}

bool ui_adaptive_navigate_horizontal(float* value, float step) {
    (void)value;
    (void)step;
    return false; // Let keyboard/mouse handle it
}

bool ui_adaptive_select(void) {
    return false; // Let keyboard/mouse handle it
}

bool ui_adaptive_cancel(void) {
    return false; // Let keyboard/mouse handle it
}

// Legacy compatibility function
const char* ui_adaptive_get_hint_text(const ControlHint* hint) {
    return ui_adaptive_get_hint(hint);
}