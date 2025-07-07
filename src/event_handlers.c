/**
 * @file event_handlers.c
 * @brief Implementation of standard event handlers
 */

#include "event_handlers.h"
#include "hal/input_hal.h"
#include "ui.h"
#include "ui_api.h"
#include "scene_script.h"
#include "services/input_service.h"
#include "game_input.h"
#include "graphics_api.h"  // For sokol_app.h
#include <stdio.h>
#include <string.h>

// ============================================================================
// HAL EVENT HANDLER (Priority 0 - Always gets events first)
// ============================================================================

bool hal_event_handler(const sapp_event* event, void* context) {
    (void)context;
    
    // Forward to HAL for input capture - this never consumes events
    extern void input_hal_sokol_event_handler(const sapp_event* e);
    input_hal_sokol_event_handler(event);
    
    return false;  // Never consume - HAL just captures input data
}

// ============================================================================
// UI EVENT HANDLER (Priority 1 - UI can capture events)
// ============================================================================

bool ui_event_handler(const sapp_event* event, void* context) {
    (void)context;
    
    // Forward to UI system - returns true if UI captured the event
    return ui_handle_event(event);
}

// ============================================================================
// SCENE EVENT HANDLER (Priority 2 - Scene-specific logic)
// ============================================================================

bool scene_event_handler(const sapp_event* event, void* context) {
    SceneEventContext* ctx = (SceneEventContext*)context;
    if (!ctx || !ctx->scene_state || !ctx->world) return false;
    
    // Forward to scene script - returns true if scene handled the event
    return scene_script_execute_input(ctx->scene_state->current_scene_name, 
                                    ctx->world, 
                                    ctx->scene_state, 
                                    event);
}

// ============================================================================
// GLOBAL EVENT HANDLER (Priority 3 - Fallback global handling)
// ============================================================================

bool global_event_handler(const sapp_event* event, void* context) {
    GlobalEventContext* ctx = (GlobalEventContext*)context;
    if (!ctx || !ctx->scene_state) return false;
    
    switch (event->type) {
        case SAPP_EVENTTYPE_KEY_DOWN: {
            // ESC key - exit application only in navigation menu
            if (event->key_code == SAPP_KEYCODE_ESCAPE) {
                if (strcmp(ctx->scene_state->current_scene_name, "navigation_menu") == 0) {
                    printf("⎋ Escape key pressed in navigation menu - exiting\n");
                    sapp_request_quit();
                    return true;  // Consumed
                }
                // For other scenes, ESC is handled by scene scripts
                return false;
            }
            
            // Tab key - navigation menu (handled through input service)
            else if (event->key_code == SAPP_KEYCODE_TAB) {
                // Tab key handling is now processed through InputService
                // The actual scene transition logic should be in scene scripts
                // This is just a fallback debug message
                printf("🧭 TAB pressed - navigation menu request pending\n");
                return false;  // Don't consume - let input service handle
            }
            
            // Tilde (~) key - toggle debug UI
            else if (event->key_code == SAPP_KEYCODE_GRAVE_ACCENT) {
                bool current_visible = scene_state_is_debug_ui_visible(ctx->scene_state);
                scene_state_set_debug_ui_visible(ctx->scene_state, !current_visible);
                ui_set_debug_visible(!current_visible);
                ui_toggle_hud();
                printf("🔧 Debug UI & HUD: %s\n", !current_visible ? "ON" : "OFF");
                return true;  // Consumed
            }
            
            // Number keys 1-9 - reserved for scene scripts
            else if (event->key_code >= SAPP_KEYCODE_1 && event->key_code <= SAPP_KEYCODE_9) {
                return false;  // Let scene scripts handle these
            }
            
            break;
        }
        
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            // Right mouse button for camera control
            if (event->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
                sapp_lock_mouse(true);
                return false;  // Don't consume - let input system handle
            }
            break;
        }
        
        case SAPP_EVENTTYPE_MOUSE_UP: {
            if (event->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
                sapp_lock_mouse(false);
                return false;  // Don't consume - let input system handle
            }
            break;
        }
        
        default:
            break;
    }
    
    return false;  // Event not handled
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static SceneEventContext g_scene_context = {0};
static GlobalEventContext g_global_context = {0};

void register_standard_event_handlers(EventRouter* router, 
                                     SceneStateManager* scene_state, 
                                     struct World* world) {
    if (!router || !scene_state || !world) return;
    
    // Set up contexts
    g_scene_context.scene_state = scene_state;
    g_scene_context.world = world;
    g_global_context.scene_state = scene_state;
    
    // Register handlers in priority order
    event_router_register_handler(router, hal_event_handler, NULL, EVENT_PRIORITY_HAL, "HAL");
    event_router_register_handler(router, ui_event_handler, NULL, EVENT_PRIORITY_UI, "UI");
    event_router_register_handler(router, scene_event_handler, &g_scene_context, EVENT_PRIORITY_SCENE, "Scene");
    event_router_register_handler(router, global_event_handler, &g_global_context, EVENT_PRIORITY_GLOBAL, "Global");
    
    printf("✅ Standard event handlers registered\n");
}

void unregister_standard_event_handlers(EventRouter* router) {
    if (!router) return;
    
    event_router_unregister_handler(router, hal_event_handler);
    event_router_unregister_handler(router, ui_event_handler);
    event_router_unregister_handler(router, scene_event_handler);
    event_router_unregister_handler(router, global_event_handler);
    
    printf("✅ Standard event handlers unregistered\n");
}