/**
 * @file scene_script.c
 * @brief Implementation of scene scripting system
 */

#include "scene_script.h"
#include "graphics_api.h"
#include "system/camera.h"
#include "render.h"
#include <stdio.h>
#include <string.h>

// Forward declarations for scene scripts
extern const SceneScript logo_script;
extern const SceneScript scene_selector_script;
extern const SceneScript navigation_menu_script;
extern const SceneScript derelict_navigation_script;
extern const SceneScript flight_test_script;
extern const SceneScript ship_launch_test_script;

// ============================================================================
// SCENE SCRIPT REGISTRY
// ============================================================================

static const SceneScript* scene_scripts[] = {
    &logo_script,
    &scene_selector_script,
    &navigation_menu_script,
    &derelict_navigation_script,
    &flight_test_script,
    &ship_launch_test_script,
    // Add more scene scripts here as they are created
};

static const int scene_script_count = sizeof(scene_scripts) / sizeof(scene_scripts[0]);

// ============================================================================
// SCENE SCRIPT API IMPLEMENTATION
// ============================================================================

const SceneScript* scene_script_find(const char* scene_name)
{
    if (!scene_name) return NULL;
    
    for (int i = 0; i < scene_script_count; i++)
    {
        if (strcmp(scene_scripts[i]->scene_name, scene_name) == 0)
        {
            return scene_scripts[i];
        }
    }
    
    return NULL;
}

void scene_script_execute_enter(const char* scene_name, struct World* world, SceneStateManager* state)
{
    const SceneScript* script = scene_script_find(scene_name);
    if (script && script->on_enter)
    {
        printf("🎬 Executing scene script enter: %s\n", scene_name);
        script->on_enter(world, state);
    }
}

void scene_script_execute_update(const char* scene_name, struct World* world, SceneStateManager* state, float dt)
{
    const SceneScript* script = scene_script_find(scene_name);
    if (script && script->on_update)
    {
        script->on_update(world, state, dt);
    }
}

void scene_script_execute_exit(const char* scene_name, struct World* world, SceneStateManager* state)
{
    const SceneScript* script = scene_script_find(scene_name);
    if (script && script->on_exit)
    {
        printf("🎬 Executing scene script exit: %s\n", scene_name);
        script->on_exit(world, state);
    }
}

bool scene_script_execute_input(const char* scene_name, struct World* world, SceneStateManager* state, const void* event)
{
    const SceneScript* script = scene_script_find(scene_name);
    if (script && script->on_input)
    {
        return script->on_input(world, state, event);
    }
    
    // Default behavior for scenes without specific scripts: ESC returns to navigation menu
    const sapp_event* ev = (const sapp_event*)event;
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN && ev->key_code == SAPP_KEYCODE_ESCAPE)
    {
        // Don't handle ESC if we're already in the navigation menu
        if (strcmp(scene_name, "navigation_menu") != 0 && strcmp(scene_name, "scene_selector") != 0)
        {
            printf("🎬 Default handler: ESC pressed in %s, returning to navigation menu\n", scene_name);
            scene_state_request_transition(state, "navigation_menu");
            return true; // Event handled
        }
    }
    
    return false; // Event not handled
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

EntityID find_entity_by_name(struct World* world, const char* name)
{
    // TODO: Implement entity name/tag system
    // For now, we'll use a simple heuristic based on component types
    if (!world || !name) return INVALID_ENTITY;
    
    // Look for "logo_cube" entity - it should have Transform and Renderable components
    if (strcmp(name, "logo_cube") == 0)
    {
        for (uint32_t i = 0; i < world->entity_count; i++)
        {
            struct Entity* entity = &world->entities[i];
            if ((entity->component_mask & COMPONENT_TRANSFORM) &&
                (entity->component_mask & COMPONENT_RENDERABLE) &&
                !(entity->component_mask & COMPONENT_CAMERA))
            {
                // This is likely the logo cube (has transform + renderable, but not camera)
                return entity->id;
            }
        }
    }
    
    return INVALID_ENTITY;
}

void scene_transition_to(const char* scene_name, struct World* world, SceneStateManager* state)
{
    if (!scene_name || !state) return;
    
    // Execute exit script for current scene
    scene_script_execute_exit(state->current_scene_name, world, state);
    
    // Request scene transition
    scene_state_request_transition(state, scene_name);
    
    // Update scene state based on target scene
    if (strcmp(scene_name, "system_overview") == 0 || 
        strcmp(scene_name, "slipstream_nav") == 0 ||
        strcmp(scene_name, "derelict_alpha") == 0 ||
        strcmp(scene_name, "derelict_beta") == 0)
    {
        scene_state_set(state, SCENE_STATE_GAME);
    }
    else if (strcmp(scene_name, "logo") == 0)
    {
        scene_state_set(state, SCENE_STATE_LOGO);
    }
    else if (strcmp(scene_name, "navigation_menu") == 0)
    {
        scene_state_set(state, SCENE_STATE_MENU);
    }
    else
    {
        scene_state_set(state, SCENE_STATE_GAME); // Default to game state
    }
}
