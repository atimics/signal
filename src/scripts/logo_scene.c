/**
 * @file logo_scene.c
 * @brief Logo scene script - spinning cube that transitions to game
 */

#include "../scene_script.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// LOGO SCENE SCRIPT IMPLEMENTATION
// ============================================================================

static void logo_on_enter(struct World* world, SceneStateManager* state)
{
    printf("🎬 Logo scene script: Entering logo scene\n");
    
    (void)world; // Suppress unused parameter warning
    
    // Hide all UI for logo scene
    scene_state_set_ui_visible(state, false);
    scene_state_set_debug_ui_visible(state, false);
    
    // Set 3-second timer for transition
    state->state_timer = 0.0f;
    
    printf("🎬 Logo scene: UI hidden, 3-second timer started\n");
}

static void logo_on_update(struct World* world, SceneStateManager* state, float dt)
{
    if (!world || !state) return;
    
    (void)dt; // Using state->state_timer instead of dt directly
    
    // Spin the logo cube
    EntityID logo_cube = find_entity_by_name(world, "logo_cube");
    if (logo_cube != INVALID_ENTITY)
    {
        struct Transform* transform = entity_get_transform(world, logo_cube);
        if (transform)
        {
            // Rotate around Y axis (2 radians per second = ~114 degrees/sec)
            float rotation_speed = 2.0f; // radians per second
            float total_rotation = state->state_timer * rotation_speed;
            
            // Create quaternion from Y-axis rotation
            float half_angle = total_rotation * 0.5f;
            transform->rotation.x = 0.0f;
            transform->rotation.y = sinf(half_angle);
            transform->rotation.z = 0.0f;
            transform->rotation.w = cosf(half_angle);
            transform->dirty = true;
        }
    }
    
    // Check if 3 seconds have passed for automatic transition
    if (state->state_timer >= 3.0f)
    {
        printf("🎬 Logo scene: 3 seconds elapsed, transitioning to spaceport\n");
        scene_transition_to("spaceport_alpha", world, state);
    }
}

static void logo_on_exit(struct World* world, SceneStateManager* state)
{
    printf("🎬 Logo scene script: Exiting logo scene\n");
    
    (void)world; // Suppress unused parameter warning
    
    // Restore UI for game scene
    scene_state_set_ui_visible(state, true);
    scene_state_set_debug_ui_visible(state, true);
    
    printf("🎬 Logo scene: UI restored for game scene\n");
}

static bool logo_on_input(struct World* world, SceneStateManager* state, const void* event)
{
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
    {
        if (ev->key_code == SAPP_KEYCODE_ENTER || ev->key_code == SAPP_KEYCODE_KP_ENTER)
        {
            printf("🎬 Logo scene: ENTER pressed, transitioning to spaceport\n");
            scene_transition_to("spaceport_alpha", world, state);
            return true; // Event handled
        }
    }
    
    return false; // Event not handled
}

// ============================================================================
// LOGO SCRIPT DEFINITION
// ============================================================================

const SceneScript logo_script = {
    .scene_name = "logo",
    .on_enter = logo_on_enter,
    .on_update = logo_on_update,
    .on_exit = logo_on_exit,
    .on_input = logo_on_input
};
