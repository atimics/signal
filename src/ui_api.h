/**
 * @file ui_api.h
 * @brief Public UI API for scene transitions and state management
 */

#ifndef UI_API_H
#define UI_API_H

#include <stdbool.h>

// ============================================================================
// SCENE TRANSITION API
// ============================================================================

// Request a scene change (called by UI modules)
void ui_request_scene_change(const char* scene_name);

// Check if there's a pending scene change request
bool ui_has_scene_change_request(void);

// Get the requested scene name (returns NULL if no request)
const char* ui_get_requested_scene(void);

// Clear the scene change request
void ui_clear_scene_change_request(void);

// ============================================================================
// DEBUG UI CONTROL
// ============================================================================

// Toggle debug panel visibility
void ui_toggle_debug_panel(void);

// Set debug panel visibility
void ui_set_debug_panel_visible(bool visible);

// Check if debug panel is visible
bool ui_is_debug_panel_visible(void);

// ============================================================================
// GLOBAL UI STATE
// ============================================================================

// Toggle HUD visibility
void ui_toggle_hud(void);

// Set HUD visibility
void ui_set_hud_visible(bool visible);

// Check if HUD is visible
bool ui_is_hud_visible(void);

// Toggle wireframe mode
void ui_toggle_wireframe(void);

// Get current wireframe state
bool ui_is_wireframe_enabled(void);

#endif // UI_API_H
