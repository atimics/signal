/**
 * @file ui_scene.h
 * @brief Scene-specific UI module system
 */

#ifndef UI_SCENE_H
#define UI_SCENE_H

#include "core.h"
#include "systems.h"

// Forward declarations
struct nk_context;
struct World;
struct SystemScheduler;

// ============================================================================
// SCENE UI MODULE INTERFACE
// ============================================================================

typedef struct SceneUIModule {
    const char* scene_name;
    
    // Lifecycle functions
    void (*init)(struct World* world);
    void (*shutdown)(struct World* world);
    
    // Rendering function
    void (*render)(struct nk_context* ctx, struct World* world, 
                   struct SystemScheduler* scheduler, float delta_time);
    
    // Event handling (optional)
    bool (*handle_event)(const void* event, struct World* world);
    
    // Module-specific data
    void* data;
} SceneUIModule;

// ============================================================================
// SCENE UI REGISTRY
// ============================================================================

// Register a scene UI module
void scene_ui_register(SceneUIModule* module);

// Unregister a scene UI module
void scene_ui_unregister(const char* scene_name);

// Get the UI module for a scene (returns NULL if not found)
SceneUIModule* scene_ui_get_module(const char* scene_name);

// Initialize scene UI system
void scene_ui_init(void);

// Shutdown scene UI system
void scene_ui_shutdown(void);

// Render UI for the current scene
void scene_ui_render(struct nk_context* ctx, const char* current_scene, 
                     struct World* world, struct SystemScheduler* scheduler, float delta_time);

// Handle events for the current scene
bool scene_ui_handle_event(const void* event, const char* current_scene, struct World* world);

// ============================================================================
// BUILT-IN SCENE UI MODULES
// ============================================================================

// Scene selector UI module
SceneUIModule* create_scene_selector_ui_module(void);

// Debug UI module (can be overlaid on any scene)
SceneUIModule* create_debug_ui_module(void);

// Logo scene UI module
SceneUIModule* create_logo_ui_module(void);

// Navigation menu UI module
SceneUIModule* create_navigation_menu_ui_module(void);

#endif // UI_SCENE_H
