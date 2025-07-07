/**
 * @file template_scene.c
 * @brief Template scene for testing and development
 * 
 * This scene provides a blank template with basic setup for testing
 * new features or as a starting point for new scenes.
 */

#include "../scene_script.h"
#include "../scene_state.h"
#include "../system/camera.h"
#include "../core.h"
#include <stdio.h>

// ============================================================================
// TEMPLATE SCENE LIFECYCLE
// ============================================================================

static void template_enter(struct World* world, SceneStateManager* state)
{
    (void)world;
    printf("🔧 Template scene: Entering template scene\n");
    
    // Set scene state to playing
    scene_state_set(state, SCENE_STATE_GAME);
    
    // Show UI for development/debugging
    scene_state_set_ui_visible(state, true);
    
    printf("🔧 Template scene: Ready for testing\n");
}

static void template_exit(struct World* world, SceneStateManager* state)
{
    (void)world;
    (void)state;
    printf("🔧 Template scene: Exiting template scene\n");
}

static void template_update(struct World* world, SceneStateManager* state, float dt)
{
    (void)world;
    (void)state;
    (void)dt;
    
    // Template scene has no special update logic
    // Add custom update logic here as needed
}

static bool template_handle_event(struct World* world, SceneStateManager* state, const void* event)
{
    (void)world;
    (void)state;
    (void)event;
    
    // Let default handler process ESC to return to menu
    return false;
}

// ============================================================================
// TEMPLATE SCENE SCRIPT DEFINITION
// ============================================================================

const SceneScript template_script = {
    .scene_name = "template",
    .on_enter = template_enter,
    .on_exit = template_exit,
    .on_update = template_update,
    .on_input = template_handle_event
};