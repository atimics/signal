/**
 * @file ui_scene.c
 * @brief Scene-specific UI module system implementation using Microui
 */

#include "ui_scene.h"
#include "ui_components.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "ui_adaptive_controls.h"
#include "scene_state.h"
#include "graphics_api.h"
#include "config.h"
#include "sokol_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Navigation menu data structure (from ui_navigation_menu_impl.c)
typedef struct {
    int selected_index;
    float animation_timer;
    bool gamepad_was_connected;
    
    const char* destinations[9];
    const char* descriptions[9];
    int destination_count;
    
    ControlHint nav_hints[3];
} NavigationMenuData;

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
    
    g_scene_ui_initialized = true;
    printf("🎨 Scene UI system initialized with Microui\n");
}

void scene_ui_shutdown(void)
{
    if (!g_scene_ui_initialized) {
        return;
    }
    
    // Clear module pointers (don't free - they're statically allocated)
    for (int i = 0; i < g_scene_ui_count; i++) {
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
            
            // Don't free the module - it may be statically allocated
            // The module creator is responsible for cleanup
            
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

// ============================================================================
// MICROUI SCENE RENDERING (from adapter)
// ============================================================================

void scene_ui_render_microui(mu_Context* ctx, const char* scene_name, 
                            struct World* world, struct SystemScheduler* scheduler, 
                            float delta_time)
{
    if (!ctx || !scene_name) return;
    
    // Safety check: ensure MicroUI context has a proper clip stack
    if (ctx->clip_stack.idx <= 0) {
        printf("⚠️ Warning: scene_ui_render_microui called with empty clip stack, skipping\n");
        return;
    }
    
    // Mark unused parameters for navigation menu
    (void)world;
    (void)scheduler;
    (void)delta_time;
    
    // Check if there's a registered module for this scene
    SceneUIModule* module = scene_ui_get_module(scene_name);
    if (module && module->render) {
        // Delegate to the module's render function
        module->render(ctx, world, scheduler, delta_time);
        return;
    }
    
    // Navigation menu scene (legacy inline code - should be removed once module works)
    if (strcmp(scene_name, "navigation_menu") == 0) {
        // Fallback if module is not registered
        SceneUIModule* nav_module = scene_ui_get_module(scene_name);
        if (nav_module && nav_module->data) {
            // Get the navigation menu data
            NavigationMenuData* nav_data = (NavigationMenuData*)nav_module->data;
            
            // Render the navigation menu using MicroUI (within the current frame)
            // Main window
            if (mu_begin_window(ctx, "FTL Navigation Interface", mu_rect(50, 50, 700, 500))) {
                
                // Title
                mu_layout_row(ctx, 1, (int[]){-1}, 40);
                mu_label(ctx, "SELECT DESTINATION");
                
                // Show connection status
                if (ui_adaptive_should_show_gamepad()) {
                    mu_layout_row(ctx, 1, (int[]){-1}, 20);
                    mu_label(ctx, "Gamepad Connected");
                }
                
                // Spacer
                mu_layout_row(ctx, 1, (int[]){-1}, 20);
                mu_label(ctx, "");
                
                // Menu items
                for (int i = 0; i < nav_data->destination_count; i++) {
                    mu_layout_row(ctx, 1, (int[]){-1}, 50);
                    
                    // Highlight selected item
                    if (i == nav_data->selected_index) {
                        // Draw a colored background rect for selection
                        mu_Rect item_rect = mu_layout_next(ctx);
                        mu_draw_rect(ctx, item_rect, mu_color(100, 150, 255, 255));
                    }
                    
                    if (mu_button(ctx, nav_data->destinations[i])) {
                        // Handle selection
                        const char* scene_names[] = {
                            "ship_launch_test",
                            "flight_test", 
                            "thruster_test"
                        };
                        
                        if (i < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
                            printf("🧭 Navigation: Selected %s\n", scene_names[i]);
                            scene_state_request_transition(NULL, scene_names[i]);
                        }
                    }
                    
                    // Show description for selected item
                    if (i == nav_data->selected_index && nav_data->descriptions[i]) {
                        mu_layout_row(ctx, 1, (int[]){-1}, 20);
                        mu_label(ctx, nav_data->descriptions[i]);
                    }
                }
                
                // Control hints at bottom
                mu_layout_row(ctx, 1, (int[]){-1}, 40);
                mu_label(ctx, "");  // Spacer
                
                // Render adaptive control hints
                mu_layout_row(ctx, 1, (int[]){-1}, 20);
                mu_label(ctx, "Controls:");
                
                for (int i = 0; i < 3; i++) {
                    mu_layout_row(ctx, 2, (int[]){200, -1}, 20);
                    mu_label(ctx, nav_data->nav_hints[i].action_name);
                    mu_label(ctx, ui_adaptive_get_hint_text(&nav_data->nav_hints[i]));
                }
                
                mu_end_window(ctx);
            }
            return;
        }
    }
    
    // Fallback rendering for scenes without modules
    // Scene selector (fallback for compatibility)
    else if (strcmp(scene_name, "scene_selector") == 0) {
        
        // Create main window
        if (mu_begin_window(ctx, "Navigation", mu_rect(10, 10, 300, 400))) {
            
            // Title
            mu_layout_row(ctx, 1, (int[]){-1}, 30);
            mu_label(ctx, "FTL Navigation Interface");
            
            // Scene buttons
            mu_layout_row(ctx, 1, (int[]){-1}, 40);
            
            if (mu_button(ctx, "System Overview")) {
                ui_request_scene_change("system_overview");
            }
            
            if (mu_button(ctx, "Derelict Alpha")) {
                ui_request_scene_change("derelict_alpha");
            }
            
            if (mu_button(ctx, "Derelict Beta")) {
                ui_request_scene_change("derelict_beta");
            }
            
            if (mu_button(ctx, "Slipstream Navigation")) {
                ui_request_scene_change("slipstream_nav");
            }
            
            if (mu_button(ctx, "Flight Test")) {
                ui_request_scene_change("flight_test");
            }
            
            mu_end_window(ctx);
        }
        
        // Debug overlay (bottom right)
        int screen_width = sapp_width();
        int screen_height = sapp_height();
        
        if (mu_begin_window(ctx, "Debug Info", mu_rect(screen_width - 210, screen_height - 110, 200, 100))) {
            mu_layout_row(ctx, 1, (int[]){-1}, 0);
            mu_label(ctx, "Press ~ to toggle debug");
            mu_label(ctx, "Press TAB to return here");
            mu_label(ctx, "Press ESC to exit");
            mu_end_window(ctx);
        }
    }
    // Logo scene
    else if (strcmp(scene_name, "logo") == 0) {
        int screen_width = sapp_width();
        int screen_height = sapp_height();
        
        int overlay_width = 300;
        int overlay_height = 80;
        int x = (screen_width - overlay_width) / 2;
        int y = screen_height - overlay_height - 50;
        
        if (mu_begin_window_ex(ctx, "[Press ENTER]", mu_rect(x, y, overlay_width, overlay_height), 
                              MU_OPT_NOINTERACT | MU_OPT_NOTITLE)) {
            mu_layout_row(ctx, 1, (int[]){-1}, 20);
            mu_label(ctx, "[Press ENTER for Scene Selector]");
            mu_label(ctx, "[Press TAB from any scene]");
            mu_end_window(ctx);
        }
    }
}

bool scene_ui_handle_event(const void* event, const char* current_scene, struct World* world)
{
    // Currently no scene-specific event handling needed for Microui
    (void)event;
    (void)current_scene;
    (void)world;
    return false;
}