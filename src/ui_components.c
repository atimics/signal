/**
 * @file ui_components.c
 * @brief MicroUI widget implementations for the CGame UI system
 */

#include "ui_components.h"
#include "ui_microui.h"
#include "config.h"
#include "data.h"
#include "scene_state.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// SCENE LIST WIDGET IMPLEMENTATION
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
    // Free existing data
    if (widget->scene_names) {
        for (int i = 0; i < widget->scene_count; i++) {
            free(widget->scene_names[i]);
            free(widget->scene_descriptions[i]);
        }
        free(widget->scene_names);
        free(widget->scene_descriptions);
    }
    
    // Load scene list - for now, use hardcoded scenes
    // TODO: Implement scene discovery from file system
    const char* scene_list[] = {
        "logo", "scene_selector", "flight_test", "derelict_navigation", "ship_launch_test"
    };
    const char* scene_descriptions[] = {
        "Logo animation and startup sequence",
        "Scene selection menu",
        "Flight mechanics testing arena", 
        "Navigate through derelict structures",
        "Test ship launch and physics"
    };
    
    widget->scene_count = sizeof(scene_list) / sizeof(scene_list[0]);
    widget->scene_names = malloc(widget->scene_count * sizeof(char*));
    widget->scene_descriptions = malloc(widget->scene_count * sizeof(char*));
    
    for (int i = 0; i < widget->scene_count; i++) {
        widget->scene_names[i] = malloc(strlen(scene_list[i]) + 1);
        strcpy(widget->scene_names[i], scene_list[i]);
        
        widget->scene_descriptions[i] = malloc(strlen(scene_descriptions[i]) + 1);
        strcpy(widget->scene_descriptions[i], scene_descriptions[i]);
    }
    
    widget->scenes_loaded = true;
}

bool scene_list_widget_render_microui(struct mu_Context* ctx, SceneListWidget* widget, 
                                     const char* exclude_scene, char* selected_scene_out)
{
    bool scene_selected = false;
    
    if (!widget->scenes_loaded) {
        mu_label(ctx, "Loading scenes...");
        return false;
    }
    
    if (widget->scene_count == 0) {
        mu_label(ctx, "No scenes available");
        return false;
    }
    
    mu_label(ctx, "Available Scenes:");
    
    for (int i = 0; i < widget->scene_count; i++) {
        // Skip excluded scene
        if (exclude_scene && strcmp(widget->scene_names[i], exclude_scene) == 0) {
            continue;
        }
        
        // Create button for each scene
        char button_text[128];
        snprintf(button_text, sizeof(button_text), "%s", widget->scene_names[i]);
        
        if (mu_button(ctx, button_text)) {
            widget->selected_index = i;
            if (selected_scene_out) {
                strncpy(selected_scene_out, widget->scene_names[i], 63);
                selected_scene_out[63] = '\0';
            }
            scene_selected = true;
        }
        
        // Show description as smaller text
        if (widget->scene_descriptions[i] && strlen(widget->scene_descriptions[i]) > 0) {
            char desc_text[256];
            snprintf(desc_text, sizeof(desc_text), "  %s", widget->scene_descriptions[i]);
            mu_label(ctx, desc_text);
        }
    }
    
    return scene_selected;
}

// ============================================================================
// CONFIG WIDGET IMPLEMENTATION
// ============================================================================

void config_widget_init(ConfigWidget* widget)
{
    widget->auto_start = false;
    strcpy(widget->startup_scene, "logo");
}

bool config_widget_render_microui(struct mu_Context* ctx, ConfigWidget* widget)
{
    bool changed = false;
    
    mu_label(ctx, "Configuration");
    ui_draw_separator_microui(ctx);
    
    // Auto-start checkbox
    int auto_start_int = widget->auto_start ? 1 : 0;
    if (mu_checkbox(ctx, "Auto-start enabled", &auto_start_int)) {
        widget->auto_start = auto_start_int ? true : false;
        changed = true;
    }
    
    // Startup scene text input
    mu_label(ctx, "Startup Scene:");
    if (mu_textbox(ctx, widget->startup_scene, sizeof(widget->startup_scene))) {
        changed = true;
    }
    
    ui_draw_spacer_microui(ctx, 10);
    
    // Action buttons
    if (mu_button(ctx, "Apply Settings")) {
        config_widget_apply_to_config(widget);
        changed = true;
    }
    
    if (mu_button(ctx, "Reset to Defaults")) {
        widget->auto_start = false;
        strcpy(widget->startup_scene, "logo");
        changed = true;
    }
    
    return changed;
}

void config_widget_sync_from_config(ConfigWidget* widget)
{
    widget->auto_start = config_get_auto_start();
    strncpy(widget->startup_scene, config_get_startup_scene(), sizeof(widget->startup_scene) - 1);
    widget->startup_scene[sizeof(widget->startup_scene) - 1] = '\0';
}

void config_widget_apply_to_config(ConfigWidget* widget)
{
    config_set_auto_start(widget->auto_start);
    config_set_startup_scene(widget->startup_scene);
}

// ============================================================================
// PERFORMANCE WIDGET IMPLEMENTATION
// ============================================================================

void performance_widget_init(PerformanceWidget* widget)
{
    widget->fps = 0.0f;
    widget->frame_count = 0;
    widget->update_timer = 0.0f;
}

void performance_widget_update(PerformanceWidget* widget, float delta_time)
{
    widget->frame_count++;
    widget->update_timer += delta_time;
    
    // Update FPS every second
    if (widget->update_timer >= 1.0f) {
        widget->fps = widget->frame_count / widget->update_timer;
        widget->frame_count = 0;
        widget->update_timer = 0.0f;
    }
}

void performance_widget_render_microui(struct mu_Context* ctx, PerformanceWidget* widget, 
                                      struct SystemScheduler* scheduler)
{
    mu_label(ctx, "Performance Metrics");
    ui_draw_separator_microui(ctx);
    
    // FPS display
    char fps_text[64];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", widget->fps);
    mu_label(ctx, fps_text);
    
    // Frame count display
    char frame_text[64];
    snprintf(frame_text, sizeof(frame_text), "Frames: %d", widget->frame_count);
    mu_label(ctx, frame_text);
    
    ui_draw_spacer_microui(ctx, 5);
    
    // System scheduler info if available
    if (scheduler) {
        mu_label(ctx, "System Scheduler:");
        mu_label(ctx, "  Physics: 60Hz");
        mu_label(ctx, "  Render: VSync");
        mu_label(ctx, "  AI: Variable");
    }
    
    ui_draw_spacer_microui(ctx, 5);
    
    // Memory usage placeholder
    mu_label(ctx, "Memory Usage:");
    mu_label(ctx, "  Entities: 64 KB");
    mu_label(ctx, "  Components: 128 KB");
    mu_label(ctx, "  Total: ~192 KB");
}

// ============================================================================
// ENTITY BROWSER WIDGET IMPLEMENTATION
// ============================================================================

void entity_browser_widget_init(EntityBrowserWidget* widget)
{
    widget->selected_entity = -1;
    widget->show_components = false;
}

void entity_browser_widget_render_microui(struct mu_Context* ctx, EntityBrowserWidget* widget, 
                                         struct World* world)
{
    if (!world) {
        mu_label(ctx, "No world loaded");
        return;
    }
    
    mu_label(ctx, "Entity Browser");
    ui_draw_separator_microui(ctx);
    
    // Toggle component view
    int show_components_int = widget->show_components ? 1 : 0;
    if (mu_checkbox(ctx, "Show Components", &show_components_int)) {
        widget->show_components = show_components_int ? true : false;
    }
    ui_draw_spacer_microui(ctx, 5);
    
    // Entity count display
    char entity_count_text[64];
    snprintf(entity_count_text, sizeof(entity_count_text), "Total Entities: %d", world->entity_count);
    mu_label(ctx, entity_count_text);
    
    ui_draw_spacer_microui(ctx, 5);
    
    // Entity list (limit to first 20 for performance)
    int display_count = world->entity_count < 20 ? world->entity_count : 20;
    
    for (int i = 0; i < display_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Entity button
        char entity_text[128];
        snprintf(entity_text, sizeof(entity_text), "Entity %d (Mask: 0x%X)", i, entity->component_mask);
        
        if (mu_button(ctx, entity_text)) {
            widget->selected_entity = i;
        }
        
        // Show components if enabled and entity is selected
        if (widget->show_components && widget->selected_entity == i) {
            if (entity->component_mask & COMPONENT_TRANSFORM) {
                mu_label(ctx, "  - Transform");
            }
            if (entity->component_mask & COMPONENT_PHYSICS) {
                mu_label(ctx, "  - Physics");
            }
            if (entity->component_mask & COMPONENT_RENDERABLE) {
                mu_label(ctx, "  - Renderable");
            }
            if (entity->component_mask & COMPONENT_CAMERA) {
                mu_label(ctx, "  - Camera");
            }
            ui_draw_spacer_microui(ctx, 3);
        }
    }
    
    if (world->entity_count > 20) {
        char more_text[64];
        snprintf(more_text, sizeof(more_text), "... and %d more entities", world->entity_count - 20);
        mu_label(ctx, more_text);
    }
}

// ============================================================================
// UTILITY FUNCTION IMPLEMENTATIONS
// ============================================================================

void ui_draw_separator_microui(struct mu_Context* ctx)
{
    // Draw a horizontal line as a separator
    mu_layout_row(ctx, 1, (int[]){ -1 }, 1);
    mu_draw_rect(ctx, mu_layout_next(ctx), mu_color(100, 100, 100, 255));
}

void ui_draw_spacer_microui(struct mu_Context* ctx, int height)
{
    // Draw an invisible spacer of specified height
    mu_layout_row(ctx, 1, (int[]){ -1 }, height);
    mu_layout_next(ctx);  // Just advance layout without drawing anything
}