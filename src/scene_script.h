/**
 * @file scene_script.h
 * @brief Scene scripting API for C-based scene behaviors
 */

#ifndef SCENE_SCRIPT_H
#define SCENE_SCRIPT_H

#include "core.h"
#include "scene_state.h"

// ============================================================================
// SCENE SCRIPT TYPES
// ============================================================================

typedef struct SceneScript {
    const char* scene_name;
    void (*on_enter)(struct World* world, SceneStateManager* state);
    void (*on_update)(struct World* world, SceneStateManager* state, float dt);
    void (*on_exit)(struct World* world, SceneStateManager* state);
    bool (*on_input)(struct World* world, SceneStateManager* state, const void* event); // Returns true if event handled
} SceneScript;

// ============================================================================
// SCENE SCRIPT API
// ============================================================================

// Script registry and execution
const SceneScript* scene_script_find(const char* scene_name);
void scene_script_execute_enter(const char* scene_name, struct World* world, SceneStateManager* state);
void scene_script_execute_update(const char* scene_name, struct World* world, SceneStateManager* state, float dt);
void scene_script_execute_exit(const char* scene_name, struct World* world, SceneStateManager* state);
bool scene_script_execute_input(const char* scene_name, struct World* world, SceneStateManager* state, const void* event);

// Utility functions for scripts
EntityID find_entity_by_name(struct World* world, const char* name);
void scene_transition_to(const char* scene_name, struct World* world, SceneStateManager* state);

#endif // SCENE_SCRIPT_H
