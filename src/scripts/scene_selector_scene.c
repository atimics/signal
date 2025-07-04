/**
 * @file scene_selector_scene.c
 * @brief Navigation menu scene script - FTL route planning interface
 */

#include "../scene_script.h"
#include "../graphics_api.h" 
#include "../ui_api.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// NAVIGATION MENU LIFECYCLE
// ============================================================================

static void scene_selector_enter(struct World* world, SceneStateManager* state)
{
    (void)world; // Unused parameter
    printf("🧭 Navigation Menu: Entered FTL navigation interface\n");
    
    // Set scene state to menu
    scene_state_set(state, SCENE_STATE_MENU);
    
    // The UI will be rendered automatically via the scene UI module system
    printf("🎨 Scene UI module will handle navigation interface\n");
}

static void scene_selector_exit(struct World* world, SceneStateManager* state)
{
    (void)world; // Unused parameter
    (void)state; // Unused parameter
    printf("🎬 Scene Selector: Exiting scene selector\n");
    
    // No need to manually hide UI - scene UI modules handle this automatically
}

static void scene_selector_update(struct World* world, SceneStateManager* state, float dt)
{
    (void)world; // Unused parameter
    (void)dt; // Unused parameter
    
    // Don't automatically reopen the scene selector - let it close after selection
    // The scene selector will be hidden automatically when a scene is selected
    
    // Handle scene change requests from UI
    if (ui_has_scene_change_request())
    {
        const char* requested_scene = ui_get_requested_scene();
        
        // Don't transition to ourselves
        if (strcmp(requested_scene, "navigation_menu") != 0)
        {
            printf("🧭 Navigation Menu: Transitioning to %s\n", requested_scene);
            scene_state_request_transition(state, requested_scene);
            ui_clear_scene_change_request();
        }
        else
        {
            ui_clear_scene_change_request();
        }
    }
}

static bool scene_selector_handle_event(struct World* world, SceneStateManager* state, const void* event)
{
    (void)world; // Unused parameter
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
    {
        switch (ev->key_code)
        {
            case SAPP_KEYCODE_ESCAPE:
                // ESC from scene selector doesn't handle the event, allowing main to exit the app
                return false;
                
            case SAPP_KEYCODE_ENTER:
            case SAPP_KEYCODE_KP_ENTER:
                // ENTER can also be used to close if no selection
                printf("🎬 Scene Selector: ENTER pressed, returning to logo\n");
                scene_state_request_transition(state, "logo");
                return true;
                
            default:
                break;
        }
    }
    
    return false; // Event not handled, pass to UI
}

// ============================================================================
// SCENE SELECTOR SCRIPT DEFINITION
// ============================================================================

const SceneScript scene_selector_script = {
    .scene_name = "navigation_menu",
    .on_enter = scene_selector_enter,
    .on_exit = scene_selector_exit,
    .on_update = scene_selector_update,
    .on_input = scene_selector_handle_event
};
