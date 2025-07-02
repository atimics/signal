/**
 * @file ui_scene.c
 * @brief Scene-specific UI module system implementation
 */

#include "ui_scene.h"
#include "ui_components.h"
#include "ui_api.h"
#include "graphics_api.h"
#include "config.h"
#include "sokol_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST_MODE
#include "../tests/stubs/ui_test_stubs.h"
#endif

#define MAX_SCENE_UI_MODULES 32

// ============================================================================
// SCENE UI REGISTRY
// ============================================================================

static SceneUIModule* g_scene_ui_modules[MAX_SCENE_UI_MODULES];
static int g_scene_ui_count = 0;
static bool g_scene_ui_initialized = false;

void scene_ui_init(void)
{
    if (g_scene_ui_initialized) {
        return;
    }
    
    // Initialize registry
    for (int i = 0; i < MAX_SCENE_UI_MODULES; i++) {
        g_scene_ui_modules[i] = NULL;
    }
    g_scene_ui_count = 0;
    
    // Register built-in UI modules
    scene_ui_register(create_scene_selector_ui_module());
    scene_ui_register(create_debug_ui_module());
    scene_ui_register(create_logo_ui_module());
    
    g_scene_ui_initialized = true;
    printf("🎨 Scene UI system initialized with %d modules\n", g_scene_ui_count);
}

void scene_ui_shutdown(void)
{
    if (!g_scene_ui_initialized) {
        return;
    }
    
    // Shutdown all modules
    for (int i = 0; i < g_scene_ui_count; i++) {
        if (g_scene_ui_modules[i] && g_scene_ui_modules[i]->shutdown) {
            g_scene_ui_modules[i]->shutdown(NULL); // Pass NULL world for global shutdown
        }
        free(g_scene_ui_modules[i]);
        g_scene_ui_modules[i] = NULL;
    }
    
    g_scene_ui_count = 0;
    g_scene_ui_initialized = false;
    printf("🎨 Scene UI system shut down\n");
}

void scene_ui_register(SceneUIModule* module)
{
    if (!module || g_scene_ui_count >= MAX_SCENE_UI_MODULES) {
        printf("❌ Failed to register scene UI module: %s\n", 
               module ? module->scene_name : "NULL");
        return;
    }
    
    g_scene_ui_modules[g_scene_ui_count] = module;
    g_scene_ui_count++;
    
    printf("🎨 Registered scene UI module: %s\n", module->scene_name);
}

void scene_ui_unregister(const char* scene_name)
{
    for (int i = 0; i < g_scene_ui_count; i++) {
        if (g_scene_ui_modules[i] && 
            strcmp(g_scene_ui_modules[i]->scene_name, scene_name) == 0) {
            
            if (g_scene_ui_modules[i]->shutdown) {
                g_scene_ui_modules[i]->shutdown(NULL);
            }
            
            free(g_scene_ui_modules[i]);
            
            // Shift remaining modules
            for (int j = i; j < g_scene_ui_count - 1; j++) {
                g_scene_ui_modules[j] = g_scene_ui_modules[j + 1];
            }
            g_scene_ui_modules[g_scene_ui_count - 1] = NULL;
            g_scene_ui_count--;
            
            printf("🎨 Unregistered scene UI module: %s\n", scene_name);
            return;
        }
    }
}

SceneUIModule* scene_ui_get_module(const char* scene_name)
{
    for (int i = 0; i < g_scene_ui_count; i++) {
        if (g_scene_ui_modules[i] && 
            strcmp(g_scene_ui_modules[i]->scene_name, scene_name) == 0) {
            return g_scene_ui_modules[i];
        }
    }
    return NULL;
}

void scene_ui_render(struct nk_context* ctx, const char* current_scene, 
                     struct World* world, struct SystemScheduler* scheduler, float delta_time)
{
    if (!g_scene_ui_initialized || !current_scene) {
        return;
    }
    
    SceneUIModule* module = scene_ui_get_module(current_scene);
    if (module && module->render) {
        module->render(ctx, world, scheduler, delta_time);
    }
}

bool scene_ui_handle_event(const void* event, const char* current_scene, struct World* world)
{
    if (!g_scene_ui_initialized || !current_scene) {
        return false;
    }
    
    SceneUIModule* module = scene_ui_get_module(current_scene);
    if (module && module->handle_event) {
        return module->handle_event(event, world);
    }
    
    return false;
}

// ============================================================================
// SCENE SELECTOR UI MODULE
// ============================================================================

typedef struct {
    SceneListWidget scene_list;
    ConfigWidget config;
    char selected_scene[64];
    bool scene_change_requested;
} SceneSelectorUIData;

static void scene_selector_ui_init(struct World* world)
{
    (void)world; // Unused
}

static void scene_selector_ui_shutdown(struct World* world)
{
    (void)world; // Unused
}

static void scene_selector_ui_render(struct nk_context* ctx, struct World* world, 
                                   struct SystemScheduler* scheduler, float delta_time)
{
    (void)world; // Unused
    (void)scheduler; // Unused
    (void)delta_time; // Unused
    
    SceneUIModule* module = scene_ui_get_module("navigation_menu");
    if (!module || !module->data) return;
    
    SceneSelectorUIData* data = (SceneSelectorUIData*)module->data;
    
    // Scene browser panel on the left side
    int panel_width = 320;
    int panel_height = sapp_height() - 40;
    
    if (nk_begin(ctx, "Scene Manager", nk_rect(10, 30, panel_width, panel_height),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE))
    {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "FTL Navigation Interface", NK_TEXT_CENTERED);
        
        ui_draw_separator(ctx);
        ui_draw_spacer(ctx, 5);
        
        // Configuration section
        config_widget_sync_from_config(&data->config);
        if (config_widget_render(ctx, &data->config)) {
            config_widget_apply_to_config(&data->config);
        }
        
        ui_draw_spacer(ctx, 10);
        
        // Scene list
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Available Destinations:", NK_TEXT_LEFT);
        
        char selected_scene[64] = {0};
        if (scene_list_widget_render(ctx, &data->scene_list, "navigation_menu", selected_scene)) {
            strcpy(data->selected_scene, selected_scene);
            data->scene_change_requested = true;
            ui_request_scene_change(selected_scene);  // Forward to UI API
            printf("🎬 Scene launch requested: %s\n", selected_scene);
        }
        
        ui_draw_spacer(ctx, 10);
        
        // Action buttons
        if (data->scene_list.selected_index >= 0 && 
            data->scene_list.selected_index < data->scene_list.scene_count) {
            
            const char* selected_name = data->scene_list.scene_names[data->scene_list.selected_index];
            
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Selected: %s", 
                      data->scene_list.scene_descriptions[data->scene_list.selected_index]);
            
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_button_label(ctx, "Launch")) {
                strcpy(data->selected_scene, selected_name);
                data->scene_change_requested = true;
                ui_request_scene_change(selected_name);  // Forward to UI API
                printf("🎬 Scene launch requested: %s\n", selected_name);
            }
            
            if (nk_button_label(ctx, "Set Default")) {
                config_set_startup_scene(selected_name);
                config_save();
                config_widget_sync_from_config(&data->config);
                printf("⚙️  Set startup scene: %s\n", selected_name);
            }
            
            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Scene Details")) {
                printf("📊 Scene details for: %s (TODO: implement)\n", selected_name);
            }
        }
        
        ui_draw_spacer(ctx, 10);
        
        // Utility buttons
        nk_layout_row_dynamic(ctx, 25, 2);
        if (nk_button_label(ctx, "Refresh")) {
            scene_list_widget_refresh(&data->scene_list);
        }
        
        if (nk_button_label(ctx, "Save Config")) {
            config_save();
            printf("💾 Configuration saved\n");
        }
    }
    nk_end(ctx);
}

static bool scene_selector_ui_handle_event(const void* event, struct World* world)
{
    (void)event; // Unused
    (void)world; // Unused
    return false; // Let other systems handle events
}

SceneUIModule* create_scene_selector_ui_module(void)
{
    SceneUIModule* module = malloc(sizeof(SceneUIModule));
    SceneSelectorUIData* data = malloc(sizeof(SceneSelectorUIData));
    
    // Initialize data
    scene_list_widget_init(&data->scene_list);
    config_widget_init(&data->config);
    strcpy(data->selected_scene, "");
    data->scene_change_requested = false;
    
    // Setup module
    module->scene_name = "navigation_menu";
    module->init = scene_selector_ui_init;
    module->shutdown = scene_selector_ui_shutdown;
    module->render = scene_selector_ui_render;
    module->handle_event = scene_selector_ui_handle_event;
    module->data = data;
    
    return module;
}

// ============================================================================
// DEBUG UI MODULE
// ============================================================================

typedef struct {
    PerformanceWidget performance;
    EntityBrowserWidget entity_browser;
    bool show_debug_panel;
    bool show_wireframe;
} DebugUIData;

static void debug_ui_init(struct World* world)
{
    (void)world; // Unused
}

static void debug_ui_shutdown(struct World* world)
{
    (void)world; // Unused
}

static void debug_ui_render(struct nk_context* ctx, struct World* world, 
                          struct SystemScheduler* scheduler, float delta_time)
{
    SceneUIModule* module = scene_ui_get_module("debug");
    if (!module || !module->data) return;
    
    DebugUIData* data = (DebugUIData*)module->data;
    
    if (!data->show_debug_panel) return;
    
    performance_widget_update(&data->performance, delta_time);
    
    if (nk_begin(ctx, "Debug Panel", nk_rect(50, 250, 400, 500),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | 
                 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
    {
        // Performance section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Performance", NK_MAXIMIZED)) {
            performance_widget_render(ctx, &data->performance, scheduler);
            nk_tree_pop(ctx);
        }
        
        // Entity browser section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Entities", NK_MAXIMIZED)) {
            entity_browser_widget_render(ctx, &data->entity_browser, world);
            nk_tree_pop(ctx);
        }
        
        // Render settings
        if (nk_tree_push(ctx, NK_TREE_TAB, "Render Settings", NK_MAXIMIZED)) {
            int wireframe_int = data->show_wireframe ? 1 : 0;
            if (nk_checkbox_label(ctx, "Wireframe Mode", &wireframe_int)) {
                data->show_wireframe = wireframe_int ? true : false;
                // TODO: Apply wireframe setting to render config
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
}

SceneUIModule* create_debug_ui_module(void)
{
    SceneUIModule* module = malloc(sizeof(SceneUIModule));
    DebugUIData* data = malloc(sizeof(DebugUIData));
    
    // Initialize data
    performance_widget_init(&data->performance);
    entity_browser_widget_init(&data->entity_browser);
    data->show_debug_panel = false;
    data->show_wireframe = false;
    
    // Setup module
    module->scene_name = "debug";
    module->init = debug_ui_init;
    module->shutdown = debug_ui_shutdown;
    module->render = debug_ui_render;
    module->handle_event = NULL;
    module->data = data;
    
    return module;
}

// ============================================================================
// LOGO UI MODULE
// ============================================================================

static void logo_ui_render(struct nk_context* ctx, struct World* world, 
                         struct SystemScheduler* scheduler, float delta_time)
{
    (void)world; // Unused
    (void)scheduler; // Unused
    (void)delta_time; // Unused
    
    // Create a centered overlay window for the logo scene
    int screen_width = sapp_width();
    int screen_height = sapp_height();
    
    int overlay_width = 300;
    int overlay_height = 80;
    int x = (screen_width - overlay_width) / 2;
    int y = screen_height - overlay_height - 50;
    
    if (nk_begin(ctx, "[Press ENTER to begin]", nk_rect(x, y, overlay_width, overlay_height),
                 NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "[Press ENTER for Scene Selector]", NK_TEXT_CENTERED);
        nk_label(ctx, "[Press TAB from any scene]", NK_TEXT_CENTERED);
    }
    nk_end(ctx);
}

SceneUIModule* create_logo_ui_module(void)
{
    SceneUIModule* module = malloc(sizeof(SceneUIModule));
    
    module->scene_name = "logo";
    module->init = NULL;
    module->shutdown = NULL;
    module->render = logo_ui_render;
    module->handle_event = NULL;
    module->data = NULL;
    
    return module;
}
