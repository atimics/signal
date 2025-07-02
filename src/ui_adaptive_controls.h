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

// Render control hints based on last used device
void ui_adaptive_render_hint(struct nk_context* ctx, const ControlHint* hint);

// Render a set of control hints (e.g., at bottom of screen)
void ui_adaptive_render_hints(struct nk_context* ctx, const ControlHint* hints, int hint_count);

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

// Menu navigation with adaptive controls
bool ui_adaptive_menu_navigate(int* selected_index, int menu_item_count);
bool ui_adaptive_menu_select(void);
bool ui_adaptive_menu_back(void);