// src/ui_adaptive_controls.c
// Implementation of adaptive UI controls

#include "ui_adaptive_controls.h"
#include "system/gamepad.h"
#include "system/gamepad_hotplug.h"
#include "system/input.h"  // For InputDeviceType
#include "sokol_app.h"
#include <stdio.h>
#include <string.h>

// Forward declaration to avoid including nuklear.h
struct nk_context;

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
    "Select", "Enter", "A", SAPP_KEYCODE_ENTER, 0  // GAMEPAD_BUTTON_A
};

const ControlHint UI_HINT_BACK = {
    "Back", "ESC", "B", SAPP_KEYCODE_ESCAPE, 1  // GAMEPAD_BUTTON_B
};

const ControlHint UI_HINT_NAVIGATE = {
    "Navigate", "↑↓ / WS", "LS/D-Pad", SAPP_KEYCODE_UP, 10  // GAMEPAD_BUTTON_DPAD_UP
};

const ControlHint UI_HINT_PAUSE = {
    "Pause", "ESC", "Start", SAPP_KEYCODE_ESCAPE, 7  // GAMEPAD_BUTTON_START
};

// Callbacks for gamepad connection events
static void on_gamepad_connected(int index) {
    printf("🎮 UI: Gamepad %d connected, switching to gamepad prompts\n", index);
    adaptive_state.target_display_device = INPUT_DEVICE_GAMEPAD;
}

static void on_gamepad_disconnected(int index) {
    printf("🎮 UI: Gamepad %d disconnected\n", index);
    
    // Check if any gamepads are still connected
    bool any_connected = false;
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (gamepad_is_connected(i)) {
            any_connected = true;
            break;
        }
    }
    
    if (!any_connected) {
        printf("🎮 UI: All gamepads disconnected, switching to keyboard prompts\n");
        adaptive_state.target_display_device = INPUT_DEVICE_KEYBOARD;
    }
}

void ui_adaptive_controls_init(void) {
    // Register callbacks for gamepad events
    gamepad_set_connected_callback(on_gamepad_connected);
    gamepad_set_disconnected_callback(on_gamepad_disconnected);
    
    // Check if any gamepads are already connected
    if (gamepad_get_primary() != NULL) {
        adaptive_state.current_display_device = INPUT_DEVICE_GAMEPAD;
        adaptive_state.target_display_device = INPUT_DEVICE_GAMEPAD;
    }
    
    printf("🎮 UI: Adaptive controls initialized (mode: %s)\n",
           adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD ? "Gamepad" : "Keyboard");
}

void ui_adaptive_controls_update(float delta_time) {
    if (adaptive_state.forced_device_type >= 0) {
        // Forced mode - no auto-switching
        adaptive_state.current_display_device = (InputDeviceType)adaptive_state.forced_device_type;
        adaptive_state.target_display_device = (InputDeviceType)adaptive_state.forced_device_type;
        return;
    }
    
    // Update target based on last used device
    InputDeviceType last_device = input_get_last_device_type();
    if (last_device == INPUT_DEVICE_GAMEPAD && gamepad_get_primary() != NULL) {
        adaptive_state.target_display_device = INPUT_DEVICE_GAMEPAD;
    } else if (last_device == INPUT_DEVICE_KEYBOARD) {
        adaptive_state.target_display_device = INPUT_DEVICE_KEYBOARD;
    }
    
    // Smooth transition
    if (adaptive_state.current_display_device != adaptive_state.target_display_device) {
        adaptive_state.fade_timer += delta_time * 3.0f; // 0.33 second transition
        
        if (adaptive_state.fade_timer >= 1.0f) {
            adaptive_state.current_display_device = adaptive_state.target_display_device;
            adaptive_state.fade_timer = 0.0f;
        }
    }
}

const char* ui_adaptive_get_hint_text(const ControlHint* hint) {
    if (!hint) return "";
    
    return (adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD) 
           ? hint->gamepad_hint 
           : hint->keyboard_hint;
}

bool ui_adaptive_hint_is_gamepad(const ControlHint* hint) {
    (void)hint; // Not used, but kept for API consistency
    return adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD;
}

const char* ui_adaptive_get_control_text(const char* action_name, 
                                         const char* keyboard_hint, 
                                         const char* gamepad_hint) {
    static char buffer[256];
    
    const char* hint = (adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD) 
                       ? gamepad_hint 
                       : keyboard_hint;
    
    snprintf(buffer, sizeof(buffer), "%s: %s", action_name, hint);
    return buffer;
}

bool ui_adaptive_should_show_gamepad(void) {
    return adaptive_state.current_display_device == INPUT_DEVICE_GAMEPAD;
}

void ui_adaptive_force_device_type(int device_type) {
    adaptive_state.forced_device_type = device_type;
    
    if (device_type >= 0) {
        adaptive_state.current_display_device = (InputDeviceType)device_type;
        adaptive_state.target_display_device = (InputDeviceType)device_type;
        printf("🎮 UI: Forced control display to %s\n",
               device_type == INPUT_DEVICE_GAMEPAD ? "Gamepad" : "Keyboard");
    } else {
        printf("🎮 UI: Returned to auto-detect control display\n");
    }
}

bool ui_adaptive_menu_navigate(int* selected_index, int menu_item_count) {
    if (!selected_index || menu_item_count <= 0) return false;
    
    bool changed = false;
    
    // Try gamepad navigation first
    if (gamepad_navigate_menu(selected_index, menu_item_count)) {
        input_set_last_device_type(INPUT_DEVICE_GAMEPAD);
        return true;
    }
    
    // Keyboard navigation would be handled by event system
    // This is just for gamepad polling
    
    return changed;
}

bool ui_adaptive_menu_select(void) {
    GamepadState* gamepad = gamepad_get_primary();
    if (gamepad && gamepad->connected) {
        if (gamepad_button_just_pressed(gamepad_get_primary_index(), 0)) {  // GAMEPAD_BUTTON_A
            input_set_last_device_type(INPUT_DEVICE_GAMEPAD);
            return true;
        }
    }
    
    // Keyboard handled by event system
    return false;
}

bool ui_adaptive_menu_back(void) {
    GamepadState* gamepad = gamepad_get_primary();
    if (gamepad && gamepad->connected) {
        if (gamepad_button_just_pressed(gamepad_get_primary_index(), 1)) {  // GAMEPAD_BUTTON_B
            input_set_last_device_type(INPUT_DEVICE_GAMEPAD);
            return true;
        }
    }
    
    // Keyboard handled by event system
    return false;
}