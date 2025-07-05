/**
 * @file ui.c
 * @brief Core UI system using Microui
 */

#include "ui.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "ui_microui_adapter.h"
#include "graphics_api.h"
#include "ui_scene.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// GLOBAL UI STATE
// ============================================================================

static bool g_ui_visible = true;
static bool g_debug_ui_visible = false;
static char g_current_scene[64] = {0};

// ============================================================================
// CORE UI SYSTEM
// ============================================================================

void ui_init(void)
{
    // Initialize Microui
    ui_microui_init();
    
    // Initialize scene UI system
    scene_ui_init();
    
    printf("✅ Core UI system initialized with Microui\n");
}

void ui_shutdown(void)
{
    // Shutdown scene UI system
    scene_ui_shutdown();
    
    // Shutdown Microui
    ui_microui_shutdown();
    
    printf("✅ Core UI system shut down\n");
}

void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene, int screen_width, int screen_height)
{
    // DEBUG: Always log ui_render calls to track the navigation menu issue
    static int ui_render_call_count = 0;
    ui_render_call_count++;
    // Only log occasionally
    if (ui_render_call_count % 300 == 1) {
        printf("🎨 UI Render #%d: scene='%s', visible=%s, debug_visible=%s, screen=%dx%d\n", 
               ui_render_call_count, current_scene ? current_scene : "null", 
               g_ui_visible ? "yes" : "no", g_debug_ui_visible ? "yes" : "no", 
               screen_width, screen_height);
    }
    
    // Store current scene for event handling
    if (current_scene) {
        strncpy(g_current_scene, current_scene, sizeof(g_current_scene) - 1);
        g_current_scene[sizeof(g_current_scene) - 1] = '\0';
    }
    
    // Early exit if UI is not visible - absolutely no UI calls should happen
    if (!g_ui_visible) {
        // printf("🎨 UI: Skipping render - UI not visible\n");
        return;
    }
    
    // Guard against multiple UI processing per frame
    static int last_frame_processed = -1;
    if (ui_render_call_count == last_frame_processed) {
        printf("⚠️ UI: Duplicate ui_render call in same frame %d - skipping\n", ui_render_call_count);
        return;
    }
    last_frame_processed = ui_render_call_count;
    
    // TEMPORARY: Ensure navigation menu module exists when in navigation_menu scene
    if (current_scene && strcmp(current_scene, "navigation_menu") == 0) {
        SceneUIModule* nav_module = scene_ui_get_module("navigation_menu");
        if (!nav_module) {
            // Create and register the navigation menu module
            extern SceneUIModule* create_navigation_menu_ui_module(void);
            SceneUIModule* new_module = create_navigation_menu_ui_module();
            if (new_module) {
                scene_ui_register(new_module);
                if (new_module->init) {
                    new_module->init(world);
                }
            }
        }
    }
    
    // Get Microui context
    UIContext* ui_ctx = ui_microui_get_context();
    if (!ui_ctx) {
        printf("⚠️ Warning: Cannot render UI - context not initialized\n");
        return;
    }

    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("⚠️ Warning: Cannot render UI - MicroUI context not initialized\n");
        return;
    }
    
    // Begin Microui frame (this sets up the clip stack)
    // printf("🎨 UI: Calling ui_microui_begin_frame()...\n");
    ui_microui_begin_frame();
    
    // Additional safety check: ensure clip stack is properly set up
    if (ctx->clip_stack.idx <= 0) {
        printf("❌ Error: MicroUI clip stack not properly initialized! Skipping UI render.\n");
        ui_microui_end_frame();  // Clean up
        return;
    }
    
    // printf("🎨 UI: MicroUI frame begun successfully, clip_stack.idx=%d\n", ctx->clip_stack.idx);
    
    // Render scene-specific UI using Microui adapter
    // printf("🎨 UI: Rendering scene-specific UI for '%s'...\n", current_scene ? current_scene : "null");
    scene_ui_render_microui(ctx, current_scene, world, scheduler, delta_time, screen_width, screen_height);
    
    // Render debug overlay if enabled
    if (g_debug_ui_visible) {
        printf("🎨 UI: Rendering debug overlay...\n");
        // Debug UI with Microui
        if (mu_begin_window(ctx, "Debug", mu_rect(10, 10, 200, 100))) {
            mu_layout_row(ctx, 1, (int[]){-1}, 0);
            mu_label(ctx, "Debug Mode Active");
            mu_end_window(ctx);
        }
    }
    
    // End frame and process commands
    // printf("🎨 UI: Calling ui_microui_end_frame()...\n");
    ui_microui_end_frame();
    // printf("🎨 UI: MicroUI frame ended successfully\n");
    
    // UI rendering is now handled within the main render pass in main.c
    // This function only manages MicroUI context and prepares vertices
    // No separate render pass needed - UI will be drawn in the same pass as 3D
}

bool ui_handle_event(const void* ev)
{
    const sapp_event* event = (const sapp_event*)ev;
    
    // Handle global UI hotkeys before passing to scene modules
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch (event->key_code) {
            case SAPP_KEYCODE_F1:
                ui_toggle_debug_panel();
                return true;  // UI captured this event
                
            case SAPP_KEYCODE_F3:
                ui_toggle_wireframe();
                return true;  // UI captured this event
                
            case SAPP_KEYCODE_ESCAPE:
            case SAPP_KEYCODE_TAB:
                // Let scene handle these
                return false;
                
            default:
                break;
        }
    }
    
    // First, check if current scene's UI module wants to handle this event
    if (strlen(g_current_scene) > 0) {
        SceneUIModule* module = scene_ui_get_module(g_current_scene);
        if (module && module->handle_event) {
            // Pass NULL for world since we don't have it here
            if (module->handle_event(ev, NULL)) {
                return true;  // Scene UI module handled this event
            }
        }
    }
    
    // If scene UI didn't handle it, pass to Microui for general UI handling
    return ui_microui_handle_event(event);
}

// ============================================================================
// UI VISIBILITY CONTROL
// ============================================================================

void ui_set_visible(bool visible)
{
    g_ui_visible = visible;
}

void ui_set_debug_visible(bool visible)
{
    g_debug_ui_visible = visible;
}

bool ui_is_visible(void)
{
    return g_ui_visible;
}

bool ui_is_debug_visible(void)
{
    return g_debug_ui_visible;
}

// ============================================================================
// LEGACY COMPATIBILITY (forwarded to ui_api.h)
// ============================================================================

// Note: These functions are now implemented in ui_api.c
// The main application should use ui_api.h directly for new code