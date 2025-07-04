/**
 * @file ui.c
 * @brief Core UI system using Microui
 */

#include "ui.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "ui_microui_adapter.h"
#include "graphics_api.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include <stdio.h>

// ============================================================================
// GLOBAL UI STATE
// ============================================================================

static bool g_ui_visible = true;
static bool g_debug_ui_visible = false;

// ============================================================================
// CORE UI SYSTEM
// ============================================================================

void ui_init(void)
{
    // Initialize Microui
    ui_microui_init();
    
    printf("✅ Core UI system initialized with Microui\n");
}

void ui_shutdown(void)
{
    // Shutdown Microui
    ui_microui_shutdown();
    
    printf("✅ Core UI system shut down\n");
}

void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene)
{
    printf("🎨 ui_render called: g_ui_visible=%d, scene=%s\n", g_ui_visible, current_scene);
    
    // Early exit if UI is not visible - absolutely no UI calls should happen
    if (!g_ui_visible) {
        printf("🎨 UI not visible, skipping render\n");
        return;
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
    ui_microui_begin_frame();
    
    // Additional safety check: ensure clip stack is properly set up
    if (ctx->clip_stack.idx <= 0) {
        printf("❌ Error: MicroUI clip stack not properly initialized! Skipping UI render.\n");
        ui_microui_end_frame();  // Clean up
        return;
    }
    
    // Render scene-specific UI using Microui adapter
    scene_ui_render_microui(ctx, current_scene, world, scheduler, delta_time);
    
    // Render debug overlay if enabled
    if (g_debug_ui_visible) {
        // Debug UI with Microui
        if (mu_begin_window(ctx, "Debug", mu_rect(10, 10, 200, 100))) {
            mu_layout_row(ctx, 1, (int[]){-1}, 0);
            mu_label(ctx, "Debug Mode Active");
            mu_end_window(ctx);
        }
    }
    
    // End frame
    ui_microui_end_frame();
    
    // Start UI render pass with blend for transparency
    sg_pass_action ui_pass_action = {
        .colors[0] = { 
            .load_action = SG_LOADACTION_LOAD  // Load existing framebuffer (don't clear)
        }
    };
    
    sg_begin_pass(&(sg_pass){ .swapchain = sglue_swapchain(), .action = ui_pass_action });
    ui_microui_render(sapp_width(), sapp_height());
    sg_end_pass();
}

bool ui_handle_event(const void* ev)
{
    const sapp_event* event = (const sapp_event*)ev;
    
    // Handle global UI hotkeys before passing to Microui
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
    
    // Pass event to Microui
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