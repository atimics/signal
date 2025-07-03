/**
 * @file ui.c
 * @brief Core UI system - coordinates modular scene UI and global overlays
 */

#include "ui.h"
#include "ui_api.h"
#include "ui_scene.h"
#include "ui_components.h"
#include "ui_adaptive_controls.h"
#include "hud_system.h"
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
    
    // Initialize modular HUD system
    hud_system_init();
    
    printf("✅ Core UI system initialized\n");
}

void ui_shutdown(void)
{
    // Shutdown modular HUD system
    hud_system_shutdown();
    
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
    
    // Update modular HUD system
    hud_system_update(delta_time);
    
    // Early exit if UI is not visible
    if (!g_ui_visible) return;
    
    // Get new frame context from sokol_nuklear
    struct nk_context* ctx = snk_new_frame();
    
    // Render scene-specific UI
    scene_ui_render(ctx, current_scene, world, scheduler, delta_time);
    
    // Render modular HUD for flight_test scene - must be rendered LAST to be on top
    if (current_scene && strcmp(current_scene, "flight_test") == 0) {
        hud_system_render(ctx, world);
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
    
    // Only pass mouse and touch events to Nuklear
    // Keyboard events should go to the game unless Nuklear has an active text input
    bool should_pass_to_nuklear = false;
    
    switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        case SAPP_EVENTTYPE_MOUSE_UP:
        case SAPP_EVENTTYPE_MOUSE_MOVE:
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
        case SAPP_EVENTTYPE_MOUSE_ENTER:
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
            should_pass_to_nuklear = true;
            break;
            
        case SAPP_EVENTTYPE_KEY_DOWN:
        case SAPP_EVENTTYPE_KEY_UP:
        case SAPP_EVENTTYPE_CHAR:
            // Only pass keyboard events if there's an active text input
            // For now, we'll assume no text inputs are active in the flight test scene
            should_pass_to_nuklear = false;
            break;
            
        default:
            should_pass_to_nuklear = false;
            break;
    }
    
    if (should_pass_to_nuklear) {
        return snk_handle_event(event);
    }
    
    return false;  // Event not captured by UI
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
// FLIGHT HUD SYSTEM (Legacy - now handled by hud_system.c)
// ============================================================================

// NOTE: Flight HUD rendering is now handled by the modular HUD system
// in hud_system.c. This provides better organization and support for
// multiple camera modes with different HUD layouts.
