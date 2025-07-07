// src/ui_adaptive_controls.h
// Adaptive UI controls that switch between keyboard and gamepad prompts

#pragma once

#include "core.h"

// Forward declarations
struct nk_context;

// Control hint structure
typedef struct {
    const char* action_name;      // e.g., "Select", "Back", "Move"
    const char* keyboard_hint;    // e.g., "Enter", "ESC", "WASD"
    const char* gamepad_hint;     // e.g., "A", "B", "LS"
    int keyboard_key;             // Sokol key code
    int gamepad_button;           // GamepadButton enum value
} ControlHint;

// Initialize adaptive controls system
void ui_adaptive_controls_init(void);

// Update adaptive controls (call each frame)
void ui_adaptive_controls_update(float delta_time);

// Get the appropriate hint text based on current device
const char* ui_adaptive_get_hint_text(const ControlHint* hint);

// Helper to check if hint should show gamepad version
bool ui_adaptive_hint_is_gamepad(const ControlHint* hint);

// Get formatted text for a control (returns static buffer)
const char* ui_adaptive_get_control_text(const char* action_name, 
                                         const char* keyboard_hint, 
                                         const char* gamepad_hint);

// Check if we should show gamepad controls
bool ui_adaptive_should_show_gamepad(void);

// Force show a specific control type
void ui_adaptive_force_device_type(int device_type); // -1 for auto

// Common control hints
extern const ControlHint UI_HINT_SELECT;
extern const ControlHint UI_HINT_BACK;
extern const ControlHint UI_HINT_NAVIGATE;
extern const ControlHint UI_HINT_PAUSE;

// Flight control hints (Sprint 25)
extern const ControlHint UI_HINT_THRUST;
extern const ControlHint UI_HINT_PITCH;
extern const ControlHint UI_HINT_YAW;
extern const ControlHint UI_HINT_ROLL;
extern const ControlHint UI_HINT_BOOST;
extern const ControlHint UI_HINT_BRAKE;

// Menu navigation with adaptive controls
bool ui_adaptive_menu_navigate(int* selected_index, int menu_item_count);
bool ui_adaptive_menu_select(void);
bool ui_adaptive_menu_back(void);