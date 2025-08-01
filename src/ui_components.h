/**
 * @file ui_components.h
 * @brief Modular UI component system - reusable widgets for Microui
 */

#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "core.h"

// Forward declarations
struct World;
struct SystemScheduler;
struct mu_Context;

// ============================================================================
// REUSABLE UI COMPONENTS
// ============================================================================

// Scene list component with selection and actions
typedef struct {
    char** scene_names;
    char** scene_descriptions;
    int scene_count;
    int selected_index;
    bool scenes_loaded;
} SceneListWidget;

// Configuration panel for startup settings
typedef struct {
    bool auto_start;
    char startup_scene[64];
} ConfigWidget;

// Performance metrics display
typedef struct {
    float fps;
    int frame_count;
    float update_timer;
} PerformanceWidget;

// Entity browser for debugging
typedef struct {
    int selected_entity;
    bool show_components;
} EntityBrowserWidget;

// ============================================================================
// WIDGET FUNCTIONS
// ============================================================================

// Scene list widget
void scene_list_widget_init(SceneListWidget* widget);
void scene_list_widget_shutdown(SceneListWidget* widget);
void scene_list_widget_refresh(SceneListWidget* widget);
bool scene_list_widget_render_microui(struct mu_Context* ctx, SceneListWidget* widget, 
                                     const char* exclude_scene, char* selected_scene_out);

// Config widget
void config_widget_init(ConfigWidget* widget);
bool config_widget_render_microui(struct mu_Context* ctx, ConfigWidget* widget);
void config_widget_sync_from_config(ConfigWidget* widget);
void config_widget_apply_to_config(ConfigWidget* widget);

// Performance widget
void performance_widget_init(PerformanceWidget* widget);
void performance_widget_update(PerformanceWidget* widget, float delta_time);
void performance_widget_render_microui(struct mu_Context* ctx, PerformanceWidget* widget, 
                                      struct SystemScheduler* scheduler);

// Entity browser widget
void entity_browser_widget_init(EntityBrowserWidget* widget);
void entity_browser_widget_render_microui(struct mu_Context* ctx, EntityBrowserWidget* widget, 
                                         struct World* world);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Helper functions for common UI patterns
void ui_draw_separator_microui(struct mu_Context* ctx);
void ui_draw_spacer_microui(struct mu_Context* ctx, int height);

#endif // UI_COMPONENTS_H