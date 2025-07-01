/**
 * @file scene_state.h
 * @brief Scene state management and UI visibility control
 */

#ifndef SCENE_STATE_H
#define SCENE_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include "core.h"

// ============================================================================
// SCENE STATE TYPES
// ============================================================================

typedef enum {
    SCENE_STATE_LOGO,       // Logo/splash screen
    SCENE_STATE_MENU,       // Main menu
    SCENE_STATE_GAME,       // Active gameplay
    SCENE_STATE_CUTSCENE,   // Non-interactive cutscene
    SCENE_STATE_PAUSE       // Paused game
} SceneStateType;

typedef struct {
    SceneStateType current_state;
    SceneStateType previous_state;
    float state_timer;
    bool ui_visible;
    bool debug_ui_visible;
    char current_scene_name[64];
    char next_scene_name[64];
    bool transition_pending;
} SceneStateManager;

// ============================================================================
// SCENE STATE API
// ============================================================================

// Scene state management
void scene_state_init(SceneStateManager* manager);
void scene_state_update(SceneStateManager* manager, float dt);
void scene_state_set(SceneStateManager* manager, SceneStateType new_state);
SceneStateType scene_state_get(const SceneStateManager* manager);

// UI visibility control
void scene_state_set_ui_visible(SceneStateManager* manager, bool visible);
void scene_state_set_debug_ui_visible(SceneStateManager* manager, bool visible);
bool scene_state_is_ui_visible(const SceneStateManager* manager);
bool scene_state_is_debug_ui_visible(const SceneStateManager* manager);

// Scene transitions
void scene_state_request_transition(SceneStateManager* manager, const char* scene_name);
bool scene_state_has_pending_transition(const SceneStateManager* manager);
const char* scene_state_get_next_scene(const SceneStateManager* manager);

#endif // SCENE_STATE_H
