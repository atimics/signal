/**
 * @file scene_state.c
 * @brief Implementation of scene state management
 */

#include "scene_state.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// SCENE STATE MANAGEMENT
// ============================================================================

void scene_state_init(SceneStateManager* manager)
{
    if (!manager) return;
    
    memset(manager, 0, sizeof(SceneStateManager));
    manager->current_state = SCENE_STATE_LOGO;
    manager->previous_state = SCENE_STATE_LOGO;
    manager->state_timer = 0.0f;
    manager->ui_visible = false;        // Start with UI hidden for logo
    manager->debug_ui_visible = false;  // Start with debug UI hidden
    manager->transition_pending = false;
    strcpy(manager->current_scene_name, "logo");
    
    printf("🎬 Scene state manager initialized - starting in LOGO state\n");
}

void scene_state_update(SceneStateManager* manager, float dt)
{
    if (!manager) return;
    
    manager->state_timer += dt;
}

void scene_state_set(SceneStateManager* manager, SceneStateType new_state)
{
    if (!manager || manager->current_state == new_state) return;
    
    const char* state_names[] = {
        "LOGO", "MENU", "GAME", "CUTSCENE", "PAUSE"
    };
    
    printf("🎬 Scene state changing: %s -> %s\n", 
           state_names[manager->current_state], 
           state_names[new_state]);
    
    manager->previous_state = manager->current_state;
    manager->current_state = new_state;
    manager->state_timer = 0.0f;
    
    // Update UI visibility based on scene state
    switch (new_state)
    {
        case SCENE_STATE_LOGO:
        case SCENE_STATE_CUTSCENE:
            manager->ui_visible = false;
            manager->debug_ui_visible = false;
            break;
            
        case SCENE_STATE_MENU:
            manager->ui_visible = true;
            manager->debug_ui_visible = false;
            break;
            
        case SCENE_STATE_GAME:
            manager->ui_visible = true;
            manager->debug_ui_visible = true;
            break;
            
        case SCENE_STATE_PAUSE:
            manager->ui_visible = true;
            manager->debug_ui_visible = true;
            break;
    }
}

SceneStateType scene_state_get(const SceneStateManager* manager)
{
    return manager ? manager->current_state : SCENE_STATE_LOGO;
}

// ============================================================================
// UI VISIBILITY CONTROL
// ============================================================================

void scene_state_set_ui_visible(SceneStateManager* manager, bool visible)
{
    if (!manager) return;
    manager->ui_visible = visible;
}

void scene_state_set_debug_ui_visible(SceneStateManager* manager, bool visible)
{
    if (!manager) return;
    manager->debug_ui_visible = visible;
}

bool scene_state_is_ui_visible(const SceneStateManager* manager)
{
    return manager ? manager->ui_visible : false;
}

bool scene_state_is_debug_ui_visible(const SceneStateManager* manager)
{
    return manager ? manager->debug_ui_visible : false;
}

// ============================================================================
// SCENE TRANSITIONS
// ============================================================================

void scene_state_request_transition(SceneStateManager* manager, const char* scene_name)
{
    if (!manager || !scene_name) return;
    
    strncpy(manager->next_scene_name, scene_name, sizeof(manager->next_scene_name) - 1);
    manager->next_scene_name[sizeof(manager->next_scene_name) - 1] = '\0';
    manager->transition_pending = true;
    
    printf("🎬 Scene transition requested: %s -> %s\n", 
           manager->current_scene_name, manager->next_scene_name);
}

bool scene_state_has_pending_transition(const SceneStateManager* manager)
{
    return manager ? manager->transition_pending : false;
}

const char* scene_state_get_next_scene(const SceneStateManager* manager)
{
    return manager ? manager->next_scene_name : "";
}
