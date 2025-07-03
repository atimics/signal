/**
 * @file ui_components.c
 * @brief Modular UI component system implementation
 */

#include "ui_components.h"
#include "config.h"
#include "graphics_api.h"  // This includes nuklear properly
#include "systems.h"       // For SystemScheduler definition
#include "sokol_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#ifdef TEST_MODE
#include "../tests/stubs/ui_test_stubs.h"
#endif

// ============================================================================
// SCENE LIST WIDGET
// ============================================================================

void scene_list_widget_init(SceneListWidget* widget)
{
    widget->scene_names = NULL;
    widget->scene_descriptions = NULL;
    widget->scene_count = 0;
    widget->selected_index = -1;
    widget->scenes_loaded = false;
}

void scene_list_widget_shutdown(SceneListWidget* widget)
{
    if (widget->scene_names) {
        for (int i = 0; i < widget->scene_count; i++) {
            free(widget->scene_names[i]);
            free(widget->scene_descriptions[i]);
        }
        free(widget->scene_names);
        free(widget->scene_descriptions);
    }
    widget->scene_names = NULL;
    widget->scene_descriptions = NULL;
    widget->scene_count = 0;
    widget->scenes_loaded = false;
}

static const char* get_scene_description(const char* scene_name)
{
    if (strcmp(scene_name, "logo") == 0) {
        return "System Boot - Core engine validation sequence";
    } else if (strcmp(scene_name, "system_overview") == 0) {
        return "System Overview - Sector-wide FTL navigation hub";
    } else if (strcmp(scene_name, "slipstream_nav") == 0) {
        return "Slipstream Navigation - FTL threadline planning testbed";
    } else if (strcmp(scene_name, "derelict_alpha") == 0) {
        return "Derelict Alpha - Ancient station excavation site";
    } else if (strcmp(scene_name, "derelict_beta") == 0) {
        return "Derelict Beta - Deep-space archaeological exploration";
    } else if (strcmp(scene_name, "flight_test") == 0) {
        return "Flight Test - Open plain flight training ground";
    } else if (strcmp(scene_name, "navigation_menu") == 0) {
        return "Threadline Planner - Primary FTL navigation interface";
    } else {
        static char default_desc[128];
        snprintf(default_desc, sizeof(default_desc), "%s - Uncharted location", scene_name);
        return default_desc;
    }
}

void scene_list_widget_refresh(SceneListWidget* widget)
{
    // Free existing data
    scene_list_widget_shutdown(widget);
    
    DIR* dir = opendir("data/scenes");
    if (!dir) {
        printf("⚠️  Could not open data/scenes directory\n");
        return;
    }
    
    // Count valid scene files
    widget->scene_count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".yaml") && 
            strcmp(entry->d_name, "navigation_menu.yaml") != 0) {
            widget->scene_count++;
        }
    }
    
    if (widget->scene_count == 0) {
        closedir(dir);
        return;
    }
    
    // Allocate arrays
    widget->scene_names = malloc(widget->scene_count * sizeof(char*));
    widget->scene_descriptions = malloc(widget->scene_count * sizeof(char*));
    
    // Collect scene names
    rewinddir(dir);
    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < widget->scene_count) {
        if (strstr(entry->d_name, ".yaml") && 
            strcmp(entry->d_name, "navigation_menu.yaml") != 0) {
            
            // Remove .yaml extension for scene name
            char* scene_name = malloc(strlen(entry->d_name) + 1);
            strcpy(scene_name, entry->d_name);
            char* dot = strrchr(scene_name, '.');
            if (dot) *dot = '\0';
            
            widget->scene_names[index] = scene_name;
            
            // Generate description
            const char* desc = get_scene_description(scene_name);
            widget->scene_descriptions[index] = malloc(strlen(desc) + 1);
            strcpy(widget->scene_descriptions[index], desc);
            
            index++;
        }
    }
    
    closedir(dir);
    widget->scenes_loaded = true;
    widget->selected_index = -1; // Reset selection
}

bool scene_list_widget_render(struct nk_context* ctx, SceneListWidget* widget, 
                               const char* current_scene, char* selected_scene_out)
{
    if (!widget->scenes_loaded) {
        scene_list_widget_refresh(widget);
    }
    
    bool scene_selected = false;
    
    if (widget->scene_count == 0) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "No scenes found", NK_TEXT_CENTERED);
        return false;
    }
    
    const char* startup_scene = config_get_startup_scene();
    
    nk_layout_row_dynamic(ctx, 200, 1); // Fixed height for scrollable list
    if (nk_group_begin(ctx, "scene_list", NK_WINDOW_BORDER)) {
        for (int i = 0; i < widget->scene_count; i++) {
            bool is_current = (current_scene && strcmp(current_scene, widget->scene_names[i]) == 0);
            bool is_startup = (strcmp(startup_scene, widget->scene_names[i]) == 0);
            bool is_selected = (widget->selected_index == i);
            int selected_state = is_selected ? 1 : 0;
            
            nk_layout_row_dynamic(ctx, 25, 1);
            
            // Style for different scene states
            bool style_pushed = ui_push_button_style(ctx, is_current, is_startup);
            
            // Scene name with indicators
            char scene_label[256];
            snprintf(scene_label, sizeof(scene_label), "%s%s%s", 
                     widget->scene_descriptions[i],
                     is_current ? " ●" : "",
                     is_startup ? " ⚡" : "");
            
            if (nk_selectable_label(ctx, scene_label, NK_TEXT_LEFT, &selected_state)) {
                widget->selected_index = i;
                if (selected_scene_out) {
                    strcpy(selected_scene_out, widget->scene_names[i]);
                    scene_selected = true;
                }
            }
            
            ui_pop_button_style(ctx, style_pushed);
        }
        nk_group_end(ctx);
    }
    
    return scene_selected;
}

// ============================================================================
// CONFIGURATION WIDGET
// ============================================================================

void config_widget_init(ConfigWidget* widget)
{
    config_widget_sync_from_config(widget);
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
    config_save();
}

bool config_widget_render(struct nk_context* ctx, ConfigWidget* widget)
{
    bool changed = false;
    
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_label(ctx, "Startup Configuration:", NK_TEXT_LEFT);
    
    nk_layout_row_dynamic(ctx, 20, 1);
    nk_labelf(ctx, NK_TEXT_LEFT, "Startup Scene: %s", widget->startup_scene);
    
    static int temp_auto_start = 0;
    temp_auto_start = widget->auto_start ? 1 : 0;
    
    nk_layout_row_dynamic(ctx, 25, 1);
    if (nk_checkbox_label(ctx, "Auto-start", &temp_auto_start)) {
        widget->auto_start = (temp_auto_start != 0);
        changed = true;
    }
    
    return changed;
}

// ============================================================================
// PERFORMANCE WIDGET
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
    
    if (widget->update_timer >= 1.0f) {
        widget->fps = widget->frame_count / widget->update_timer;
        widget->frame_count = 0;
        widget->update_timer = 0.0f;
    }
}

void performance_widget_render(struct nk_context* ctx, PerformanceWidget* widget, 
                              struct SystemScheduler* scheduler)
{
    nk_layout_row_dynamic(ctx, 20, 2);
    nk_label(ctx, "FPS:", NK_TEXT_LEFT);
    nk_labelf(ctx, NK_TEXT_LEFT, "%.1f", widget->fps);
    
    if (scheduler) {
        nk_label(ctx, "Frame Count:", NK_TEXT_LEFT);
        nk_labelf(ctx, NK_TEXT_LEFT, "%d", scheduler->frame_count);
        
        nk_label(ctx, "Total Time:", NK_TEXT_LEFT);
        nk_labelf(ctx, NK_TEXT_LEFT, "%.1fs", scheduler->total_time);
    }
}

// ============================================================================
// ENTITY BROWSER WIDGET
// ============================================================================

void entity_browser_widget_init(EntityBrowserWidget* widget)
{
    widget->selected_entity = -1;
    widget->show_components = true;
}

void entity_browser_widget_render(struct nk_context* ctx, EntityBrowserWidget* widget, 
                                 struct World* world)
{
    (void)widget; // Mark as unused for now - widget state could be added later
    
    nk_layout_row_dynamic(ctx, 20, 1);
    nk_labelf(ctx, NK_TEXT_LEFT, "Total Entities: %d", world->entity_count);
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        char entity_name[64];
        snprintf(entity_name, sizeof(entity_name), "Entity %d", entity->id);
        
        if (nk_tree_push_id(ctx, NK_TREE_NODE, entity_name, NK_MINIMIZED, entity->id)) {
            nk_layout_row_dynamic(ctx, 15, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "  Mask: 0x%08X", entity->component_mask);
            
            if (entity->component_mask & COMPONENT_TRANSFORM) {
                struct Transform* t = entity_get_transform(world, entity->id);
                if (t) {
                    nk_labelf(ctx, NK_TEXT_LEFT, "  Pos: (%.1f, %.1f, %.1f)", 
                              t->position.x, t->position.y, t->position.z);
                }
            }
            
            if (entity->component_mask & COMPONENT_PHYSICS) {
                struct Physics* p = entity_get_physics(world, entity->id);
                if (p) {
                    nk_labelf(ctx, NK_TEXT_LEFT, "  Mass: %.1f", p->mass);
                }
            }
            
            if (entity->component_mask & COMPONENT_CAMERA) {
                nk_labelf(ctx, NK_TEXT_LEFT, "  Type: Camera");
            }
            
            if (entity->component_mask & COMPONENT_PLAYER) {
                nk_labelf(ctx, NK_TEXT_LEFT, "  Type: Player");
            }
            
            nk_tree_pop(ctx);
        }
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool ui_push_button_style(struct nk_context* ctx, bool is_current, bool is_special)
{
    if (is_current) {
#ifdef TEST_MODE
        nk_style_push_color(ctx, NK_STYLE_COLOR_PTR(ctx, selectable.normal.data.color), nk_rgb(70, 120, 200));
#else
        nk_style_push_color(ctx, &ctx->style.selectable.normal.data.color, nk_rgb(70, 120, 200));
#endif
        return true;
    } else if (is_special) {
#ifdef TEST_MODE
        nk_style_push_color(ctx, NK_STYLE_COLOR_PTR(ctx, selectable.normal.data.color), nk_rgb(120, 120, 70));
#else
        nk_style_push_color(ctx, &ctx->style.selectable.normal.data.color, nk_rgb(120, 120, 70));
#endif
        return true;
    }
    return false;
}

void ui_pop_button_style(struct nk_context* ctx, bool was_pushed)
{
    if (was_pushed) {
        nk_style_pop_color(ctx);
    }
}

void ui_draw_separator(struct nk_context* ctx)
{
    nk_layout_row_dynamic(ctx, 1, 1);
    nk_label(ctx, "────────────────────────────────────", NK_TEXT_CENTERED);
}

void ui_draw_spacer(struct nk_context* ctx, int height)
{
    nk_layout_row_dynamic(ctx, height, 1);
    nk_spacing(ctx, 1);
}
