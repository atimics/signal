/**
 * @file ui.c
 * @brief Core UI system - coordinates modular scene UI and global overlays
 */

#include "ui.h"
#include "ui_api.h"
#include "ui_scene.h"
#include "ui_components.h"
#include "ui_adaptive_controls.h"
#include "graphics_api.h"
#include "sokol_app.h"
#include "system/input.h"
#include "component/look_target.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

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
    // Initialize Nuklear
    snk_setup(&(snk_desc_t){ 0 });
    
    // Initialize scene UI system
    scene_ui_init();
    
    // Initialize adaptive controls
    ui_adaptive_controls_init();
    
    printf("✅ Core UI system initialized\n");
}

void ui_shutdown(void)
{
    // Shutdown scene UI system
    scene_ui_shutdown();
    
    // Shutdown Nuklear
    snk_shutdown();
    
    printf("✅ Core UI system shut down\n");
}

void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene)
{
    // Update adaptive controls
    ui_adaptive_controls_update(delta_time);
    
    // Early exit if UI is not visible
    if (!g_ui_visible) return;
    
    // Get new frame context from sokol_nuklear
    struct nk_context* ctx = snk_new_frame();
    
    // Render scene-specific UI
    scene_ui_render(ctx, current_scene, world, scheduler, delta_time);
    
    // Render flight HUD for flight_test scene - must be rendered LAST to be on top
    if (current_scene && strcmp(current_scene, "flight_test") == 0) {
        ui_render_flight_hud(ctx, world);
    }
    
    // Render debug overlay if enabled
    if (g_debug_ui_visible) {
        scene_ui_render(ctx, "debug", world, scheduler, delta_time);
    }
    
    // Render the UI
    snk_render(sapp_width(), sapp_height());
}

bool ui_handle_event(const void* ev)
{
    const sapp_event* event = (const sapp_event*)ev;
    
    // Handle global UI hotkeys before passing to scene UI or Nuklear
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
    
    // Try scene-specific UI event handling first
    // Note: current_scene would need to be passed here in a real implementation
    // For now, we'll skip scene-specific event handling
    
    // Pass event to Nuklear and return whether it was captured
    return snk_handle_event(event);
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
// FLIGHT HUD SYSTEM
// ============================================================================

void ui_render_flight_hud(struct nk_context* ctx, struct World* world)
{
    if (!ctx || !world) return;
    
    const InputState* input = input_get_state();
    if (!input) return;
    
    float screen_width = (float)sapp_width();
    float screen_height = (float)sapp_height();
    
    // Calculate targeting reticle position from look target
    float reticle_x = screen_width * 0.5f;  // Center by default
    float reticle_y = screen_height * 0.5f;
    
    // Apply look target offset to reticle position
    if (input->look_target.distance > 0.0f) {
        // Convert azimuth/elevation to screen offset
        float offset_x = sinf(input->look_target.azimuth) * 100.0f;
        float offset_y = -sinf(input->look_target.elevation) * 100.0f;
        
        reticle_x = screen_width * 0.5f + offset_x;
        reticle_y = screen_height * 0.5f + offset_y;
        
        // Clamp to screen bounds
        reticle_x = fmaxf(20.0f, fminf(screen_width - 20.0f, reticle_x));
        reticle_y = fmaxf(20.0f, fminf(screen_height - 20.0f, reticle_y));
    }
    
    // Draw reticle using immediate mode commands after all other UI
    // This ensures it's always on top and doesn't interfere with other windows
    
    // Create a minimal window that covers the entire screen for the overlay
    struct nk_rect overlay_area = nk_rect(0, 0, screen_width, screen_height);
    
    // Store the current window background style and make it transparent
    struct nk_style_item original_bg = ctx->style.window.fixed_background;
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0,0,0,0)); // Fully transparent
    
    if (nk_begin(ctx, "Flight_Overlay", overlay_area, 
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NOT_INTERACTIVE)) {
        
        struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
        
        // Draw targeting reticle
        struct nk_color reticle_color = nk_rgba(255, 100, 100, 255); // Solid red for visibility
        
        // Main crosshair
        float reticle_size = 15.0f;
        nk_stroke_line(canvas, 
                      reticle_x - reticle_size, reticle_y,
                      reticle_x + reticle_size, reticle_y,
                      3.0f, reticle_color);
        nk_stroke_line(canvas,
                      reticle_x, reticle_y - reticle_size,
                      reticle_x, reticle_y + reticle_size,
                      3.0f, reticle_color);
        
        // Outer circle for targeting feedback
        if (input->thrust > 0.0f) {
            struct nk_color thrust_color = nk_rgba(100, 255, 100, 255); // Solid green when thrusting
            nk_stroke_circle(canvas, nk_rect(reticle_x - 20, reticle_y - 20, 40, 40), 2.0f, thrust_color);
        } else {
            nk_stroke_circle(canvas, nk_rect(reticle_x - 15, reticle_y - 15, 30, 30), 1.0f, reticle_color);
        }
        
        // Debug: Draw a small circle at screen center to verify coordinates
        struct nk_color debug_color = nk_rgba(0, 255, 255, 255); // Cyan
        nk_stroke_circle(canvas, nk_rect(screen_width * 0.5f - 5, screen_height * 0.5f - 5, 10, 10), 1.0f, debug_color);
    }
    nk_end(ctx);
    
    // Restore the original window background style
    ctx->style.window.fixed_background = original_bg;
}
