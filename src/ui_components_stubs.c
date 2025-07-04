/**
 * @file ui_components_stubs.c
 * @brief Stub implementations for UI components to allow building without Nuklear
 */

#include "ui_components.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// SCENE LIST WIDGET STUBS
// ============================================================================

void scene_list_widget_init(SceneListWidget* widget)
{
    widget->scene_names = NULL;
    widget->scene_descriptions = NULL;
    widget->scene_count = 0;
    widget->selected_index = -1;
    widget->scenes_loaded = false;
}

void scene_list_widget_refresh(SceneListWidget* widget)
{
    widget->scenes_loaded = true;
}

bool scene_list_widget_render_microui(struct mu_Context* ctx, SceneListWidget* widget, 
                                     const char* exclude_scene, char* selected_scene_out)
{
    (void)ctx;
    (void)widget;
    (void)exclude_scene;
    (void)selected_scene_out;
    return false;
}

// ============================================================================
// CONFIG WIDGET STUBS
// ============================================================================

void config_widget_init(ConfigWidget* widget)
{
    widget->auto_start = false;
    strcpy(widget->startup_scene, "logo");
}

bool config_widget_render_microui(struct mu_Context* ctx, ConfigWidget* widget)
{
    (void)ctx;
    (void)widget;
    return false;
}

void config_widget_sync_from_config(ConfigWidget* widget)
{
    widget->auto_start = config_get_auto_start();
    strncpy(widget->startup_scene, config_get_startup_scene(), sizeof(widget->startup_scene) - 1);
}

void config_widget_apply_to_config(ConfigWidget* widget)
{
    config_set_auto_start(widget->auto_start);
    config_set_startup_scene(widget->startup_scene);
}

// ============================================================================
// PERFORMANCE WIDGET STUBS
// ============================================================================

void performance_widget_init(PerformanceWidget* widget)
{
    widget->fps = 0.0f;
    widget->frame_count = 0;
    widget->update_timer = 0.0f;
}

void performance_widget_update(PerformanceWidget* widget, float delta_time)
{
    (void)widget;
    (void)delta_time;
}

void performance_widget_render_microui(struct mu_Context* ctx, PerformanceWidget* widget, 
                                      struct SystemScheduler* scheduler)
{
    (void)ctx;
    (void)widget;
    (void)scheduler;
}

// ============================================================================
// ENTITY BROWSER WIDGET STUBS
// ============================================================================

void entity_browser_widget_init(EntityBrowserWidget* widget)
{
    widget->selected_entity = -1;
    widget->show_components = false;
}

void entity_browser_widget_render_microui(struct mu_Context* ctx, EntityBrowserWidget* widget, 
                                         struct World* world)
{
    (void)ctx;
    (void)widget;
    (void)world;
}

// ============================================================================
// UTILITY FUNCTION STUBS
// ============================================================================

void ui_draw_separator_microui(struct mu_Context* ctx)
{
    (void)ctx;
}

void ui_draw_spacer_microui(struct mu_Context* ctx, int height)
{
    (void)ctx;
    (void)height;
}