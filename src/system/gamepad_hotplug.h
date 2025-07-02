// src/system/gamepad_hotplug.h
// Dynamic gamepad detection and UI integration

#pragma once

#include "gamepad.h"
#include <stdbool.h>

// Input device tracking moved to input.h to avoid duplicate definitions

// Gamepad hot-plug detection
typedef struct {
    bool enabled;
    float check_interval;      // How often to check for new devices (seconds)
    float time_since_check;    // Time accumulator
    int last_connected_count;  // Track connection changes
    
    // Callbacks for connection events
    void (*on_connected)(int gamepad_index);
    void (*on_disconnected)(int gamepad_index);
} GamepadHotplugState;

// Extended gamepad functions for hot-plug support
void gamepad_enable_hotplug(bool enable);
void gamepad_set_hotplug_interval(float seconds);
void gamepad_check_connections(void);  // Force immediate check
void gamepad_update_hotplug(float delta_time);

// Callbacks for connection events
void gamepad_set_connected_callback(void (*callback)(int));
void gamepad_set_disconnected_callback(void (*callback)(int));

// Input device tracking functions moved to input.h
bool input_was_gamepad_used_last(void);

// UI helper functions
const char* gamepad_get_button_icon(GamepadButton button);  // Returns icon/glyph name
const char* gamepad_get_axis_icon(const char* axis_name);
bool gamepad_navigate_menu(int* selected_index, int menu_item_count);  // Handle D-pad/stick navigation

// Get primary gamepad (first connected)
GamepadState* gamepad_get_primary(void);
int gamepad_get_primary_index(void);