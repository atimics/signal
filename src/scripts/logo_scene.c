/**
 * @file logo_scene.c
 * @brief Logo scene script - spinning cube that transitions to game
 */

#include "../scene_script.h"
#include "../graphics_api.h"
#include "../ui.h"
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
    
    // Set 8-second timer for transition (more time to enjoy the effect)
    state->state_timer = 0.0f;
    
    printf("🎬 Logo scene: UI hidden, 8-second timer started\n");
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
            // Fix orientation: add 180-degree rotation around X-axis to correct upside-down issue
            float orientation_fix = M_PI; // 180 degrees
            
            // Rotate around Y axis (1.5 radians per second for smoother rotation)
            float rotation_speed = 1.5f; // radians per second
            float total_rotation = state->state_timer * rotation_speed;
            
            // Create quaternion combining X-axis flip and Y-axis rotation
            // First, create X-axis rotation (180 degrees to fix orientation)
            float x_half_angle = orientation_fix * 0.5f;
            Quaternion x_rotation = {
                .x = sinf(x_half_angle),
                .y = 0.0f,
                .z = 0.0f,
                .w = cosf(x_half_angle)
            };
            
            // Then, create Y-axis rotation (spinning animation)
            float y_half_angle = total_rotation * 0.5f;
            Quaternion y_rotation = {
                .x = 0.0f,
                .y = sinf(y_half_angle),
                .z = 0.0f,
                .w = cosf(y_half_angle)
            };
            
            // Combine rotations: Y-axis rotation * X-axis rotation
            transform->rotation.x = y_rotation.w * x_rotation.x + y_rotation.x * x_rotation.w +
                                   y_rotation.y * x_rotation.z - y_rotation.z * x_rotation.y;
            transform->rotation.y = y_rotation.w * x_rotation.y - y_rotation.x * x_rotation.z +
                                   y_rotation.y * x_rotation.w + y_rotation.z * x_rotation.x;
            transform->rotation.z = y_rotation.w * x_rotation.z + y_rotation.x * x_rotation.y -
                                   y_rotation.y * x_rotation.x + y_rotation.z * x_rotation.w;
            transform->rotation.w = y_rotation.w * x_rotation.w - y_rotation.x * x_rotation.x -
                                   y_rotation.y * x_rotation.y - y_rotation.z * x_rotation.z;
            
            transform->dirty = true;
        }
    }
    
    // Check if 8 seconds have passed for automatic transition (more time to see the effect)
    if (state->state_timer >= 8.0f)
    {
        printf("🎬 Logo scene: 8 seconds elapsed, transitioning to navigation menu\n");
        scene_transition_to("navigation_menu", world, state);
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
    (void)world; // Unused parameter
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
    {
        if (ev->key_code == SAPP_KEYCODE_ENTER || ev->key_code == SAPP_KEYCODE_KP_ENTER)
        {
            printf("🎬 Logo scene: ENTER pressed, transitioning to navigation menu\n");
            scene_state_request_transition(state, "navigation_menu");
            return true; // Event handled
        }
        else if (ev->key_code == SAPP_KEYCODE_ESCAPE)
        {
            printf("🎬 Logo scene: ESC pressed, transitioning to navigation menu\n");
            scene_state_request_transition(state, "navigation_menu");
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
