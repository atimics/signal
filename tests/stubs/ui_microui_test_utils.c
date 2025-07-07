/**
 * @file ui_microui_test_utils.c
 * @brief Test utilities implementation for MicroUI testing
 */

#include "ui_microui_test_utils.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include <string.h>

// Test state tracking
static struct {
    int vertex_count;
    int command_count;
    int draw_call_count;
    size_t memory_usage;
    bool font_texture_bound;
    bool ui_visible;
    unsigned char vertex_buffer[8192 * 20]; // Max vertices * vertex size
    size_t vertex_buffer_size;
} test_state = {
    .ui_visible = true
};

// Forward declaration of actual render state getter
extern int ui_microui_get_rendered_vertex_count(void);

// Vertex count is now provided by ui_microui.c directly

// Command count is now provided by ui_microui.c directly

// Draw call count is now provided by ui_microui.c directly

// Vertex data functions are now provided by ui_microui.c directly

// Font texture and memory usage functions are now provided by ui_microui.c directly

// UI input queries
bool ui_wants_mouse_input(void) {
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) return false;
    
    // Check if mouse is over any window
    return ctx->hover != 0 || ctx->focus != 0;
}

bool ui_wants_keyboard_input(void) {
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) return false;
    
    // Check if any input has focus
    return ctx->focus != 0;
}

// UI visibility control
// Note: ui_set_visible is defined in engine_test_stubs.c

bool ui_is_visible(void) {
    return test_state.ui_visible;
}

// Frame management wrappers
void ui_begin_frame(void) {
    ui_microui_begin_frame();
    
    // Reset test state for new frame
    test_state.vertex_count = 0;
    test_state.command_count = 0;
    test_state.draw_call_count = 0;
}

void ui_end_frame(void) {
    ui_microui_end_frame();
}

void ui_render(void) {
    if (!test_state.ui_visible) {
        return;
    }
    
    // Would call ui_microui_render() in real implementation
}

// Event handling
void ui_handle_event(ui_event_t* event) {
    if (!event) return;
    
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) return;
    
    switch (event->type) {
        case UI_EVENT_MOUSE_BUTTON:
            mu_input_mousedown(ctx, (int)event->mouse_button.x, 
                             (int)event->mouse_button.y, 
                             event->mouse_button.button);
            if (event->mouse_button.action == 0) {
                mu_input_mouseup(ctx, (int)event->mouse_button.x,
                               (int)event->mouse_button.y,
                               event->mouse_button.button);
            }
            break;
            
        case UI_EVENT_MOUSE_MOVE:
            mu_input_mousemove(ctx, (int)event->mouse_move.x, 
                              (int)event->mouse_move.y);
            break;
            
        case UI_EVENT_KEY:
            if (event->key.action == 1) {
                mu_input_keydown(ctx, event->key.key);
            } else {
                mu_input_keyup(ctx, event->key.key);
            }
            break;
            
        case UI_EVENT_SCROLL:
            mu_input_scroll(ctx, 0, (int)event->scroll.y);
            break;
            
        default:
            break;
    }
}

// Initialize UI system if needed
void ui_init(void) {
    static bool initialized = false;
    if (!initialized) {
        ui_microui_init();
        initialized = true;
    }
}

// Shutdown UI system
void ui_shutdown(void) {
    ui_microui_shutdown();
}