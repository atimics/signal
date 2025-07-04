/**
 * @file navigation_menu_scene.c
 * @brief Navigation menu scene script - initializes the navigation UI module
 */

#include "../scene_script.h"
#include "../scene_state.h"
#include "../ui_scene.h"
#include "../ui.h"
#include <stdio.h>
#include <string.h>

// External function to create the navigation menu UI module
extern SceneUIModule* create_navigation_menu_ui_module(void);

// ============================================================================
// NAVIGATION MENU LIFECYCLE
// ============================================================================

static void navigation_menu_enter(struct World* world, SceneStateManager* state)
{
    (void)world; // Unused parameter
    printf("🧭 Navigation Menu: Entering navigation menu scene\n");
    
    // Set scene state to menu
    scene_state_set(state, SCENE_STATE_MENU);
    
    // Enable UI for the navigation menu
    scene_state_set_ui_visible(state, true);
    ui_set_visible(true);  // Synchronize with UI system
    
    // Create and register the navigation menu UI module
    SceneUIModule* nav_module = create_navigation_menu_ui_module();
    if (nav_module) {
        scene_ui_register(nav_module);
        
        // Initialize the module
        if (nav_module->init) {
            nav_module->init(world);
        }
        
        printf("🧭 Navigation Menu: UI module registered and initialized\n");
    } else {
        printf("❌ Navigation Menu: Failed to create UI module\n");
    }
}

static void navigation_menu_exit(struct World* world, SceneStateManager* state)
{
    (void)state; // Unused parameter
    printf("🧭 Navigation Menu: Exiting navigation menu scene\n");
    
    // Get the module and shutdown
    SceneUIModule* module = scene_ui_get_module("navigation_menu");
    if (module && module->shutdown) {
        module->shutdown(world);
    }
    
    // Unregister the UI module
    scene_ui_unregister("navigation_menu");
}

static void navigation_menu_update(struct World* world, SceneStateManager* state, float dt)
{
    (void)world; // Unused parameter
    (void)state; // Unused parameter
    (void)dt; // Unused parameter
    
    // Navigation menu doesn't need per-frame updates
    // All interaction is handled through the UI module
}

static bool navigation_menu_handle_event(struct World* world, SceneStateManager* state, const void* event)
{
    (void)world; // Unused parameter
    (void)state; // Unused parameter
    (void)event; // Unused parameter
    
    // Let the UI module handle all events
    // Return false so events are passed to the UI system
    return false;
}

// ============================================================================
// NAVIGATION MENU SCRIPT DEFINITION
// ============================================================================

const SceneScript navigation_menu_script = {
    .scene_name = "navigation_menu",
    .on_enter = navigation_menu_enter,
    .on_exit = navigation_menu_exit,
    .on_update = navigation_menu_update,
    .on_input = navigation_menu_handle_event
};