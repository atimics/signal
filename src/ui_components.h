/**
 * @file ui_components.h
 * @brief Modular UI component system - reusable widgets
 */

#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "core.h"

// Forward declarations
struct nk_context;
struct World;
struct SystemScheduler;

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
bool scene_list_widget_render(struct nk_context* ctx, SceneListWidget* widget, 
                               const char* current_scene, char* selected_scene_out);
void scene_list_widget_refresh(SceneListWidget* widget);

// Configuration widget
void config_widget_init(ConfigWidget* widget);
bool config_widget_render(struct nk_context* ctx, ConfigWidget* widget);
void config_widget_sync_from_config(ConfigWidget* widget);
void config_widget_apply_to_config(ConfigWidget* widget);

// Performance widget
void performance_widget_init(PerformanceWidget* widget);
void performance_widget_update(PerformanceWidget* widget, float delta_time);
void performance_widget_render(struct nk_context* ctx, PerformanceWidget* widget, 
                              struct SystemScheduler* scheduler);

// Entity browser widget
void entity_browser_widget_init(EntityBrowserWidget* widget);
void entity_browser_widget_render(struct nk_context* ctx, EntityBrowserWidget* widget, 
                                 struct World* world);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Common UI styling helpers
bool ui_push_button_style(struct nk_context* ctx, bool is_current, bool is_special);
void ui_pop_button_style(struct nk_context* ctx, bool was_pushed);
void ui_draw_separator(struct nk_context* ctx);
void ui_draw_spacer(struct nk_context* ctx, int height);

#endif // UI_COMPONENTS_H
