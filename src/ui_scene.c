/**
 * @file ui_scene.c
 * @brief Scene-specific UI module system implementation using Microui
 */

#include "ui_scene.h"
#include "ui_components.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "graphics_api.h"
#include "config.h"
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
    
    g_scene_ui_initialized = true;
    printf("🎨 Scene UI system initialized with Microui\n");
}

void scene_ui_shutdown(void)
{
    if (!g_scene_ui_initialized) {
        return;
    }
    
    // Shutdown all modules
    for (int i = 0; i < g_scene_ui_count; i++) {
        if (g_scene_ui_modules[i]) {
            free(g_scene_ui_modules[i]);
            g_scene_ui_modules[i] = NULL;
        }
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

// ============================================================================
// MICROUI SCENE RENDERING (from adapter)
// ============================================================================

void scene_ui_render_microui(mu_Context* ctx, const char* scene_name, 
                            struct World* world, struct SystemScheduler* scheduler, 
                            float delta_time)
{
    if (!ctx || !scene_name) return;
    
    // Mark unused parameters
    (void)world;
    (void)scheduler;
    (void)delta_time;
    
    // Navigation menu scene
    if (strcmp(scene_name, "navigation_menu") == 0 || 
        strcmp(scene_name, "scene_selector") == 0) {
        
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