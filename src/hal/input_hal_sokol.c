/**
 * @file input_hal_sokol.c
 * @brief Sokol implementation of Input HAL
 */

#include "input_hal.h"
#include "../sokol_app.h"
#include <stdlib.h>
#include <string.h>

#define MAX_QUEUED_EVENTS 256

typedef struct SokolInputHAL {
    InputHAL base;
    
    // Event queue (ring buffer)
    HardwareInputEvent events[MAX_QUEUED_EVENTS];
    volatile int write_pos;
    volatile int read_pos;
    
    // State tracking for compatibility
    bool keys[512];
    float mouse_x, mouse_y;
    bool mouse_captured;
    bool mouse_visible;
    
    // Frame timing
    uint32_t frame_count;
} SokolInputHAL;

// Global instance (temporary until we have proper context management)
static SokolInputHAL* g_sokol_hal = NULL;


// Helper: Queue an event
static void queue_event(SokolInputHAL* hal, const HardwareInputEvent* event) {
    int next_write = (hal->write_pos + 1) % MAX_QUEUED_EVENTS;
    
    // Check for queue full (leave one slot empty to distinguish full/empty)
    if (next_write == hal->read_pos) {
        // Queue full - drop oldest event
        hal->read_pos = (hal->read_pos + 1) % MAX_QUEUED_EVENTS;
    }
    
    hal->events[hal->write_pos] = *event;
    hal->write_pos = next_write;
}

// Sokol event handler - called by Sokol framework
void input_hal_sokol_event_handler(const sapp_event* e) {
    if (!g_sokol_hal) return;
    
    HardwareInputEvent event = {0};
    event.timestamp = g_sokol_hal->frame_count;
    
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
        case SAPP_EVENTTYPE_KEY_UP: {
            event.device = INPUT_DEVICE_KEYBOARD;
            event.data.keyboard.key = e->key_code;
            event.data.keyboard.pressed = (e->type == SAPP_EVENTTYPE_KEY_DOWN);
            event.data.keyboard.modifiers = e->modifiers;
            
            // Update state
            if (e->key_code < 512) {
                g_sokol_hal->keys[e->key_code] = event.data.keyboard.pressed;
            }
            
            queue_event(g_sokol_hal, &event);
            break;
        }
        
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            event.device = INPUT_DEVICE_MOUSE;
            event.data.mouse.x = e->mouse_x;
            event.data.mouse.y = e->mouse_y;
            event.data.mouse.dx = e->mouse_dx;
            event.data.mouse.dy = e->mouse_dy;
            event.data.mouse.buttons = 0;  // Will be set by button events
            
            // Update state
            g_sokol_hal->mouse_x = e->mouse_x;
            g_sokol_hal->mouse_y = e->mouse_y;
            
            queue_event(g_sokol_hal, &event);
            break;
        }
        
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        case SAPP_EVENTTYPE_MOUSE_UP: {
            event.device = INPUT_DEVICE_MOUSE;
            event.data.mouse.x = e->mouse_x;
            event.data.mouse.y = e->mouse_y;
            event.data.mouse.dx = 0;
            event.data.mouse.dy = 0;
            event.data.mouse.buttons = (1 << e->mouse_button);
            
            // For mouse up events, we need to indicate which button was released
            if (e->type == SAPP_EVENTTYPE_MOUSE_UP) {
                event.data.mouse.buttons |= 0x80;  // High bit indicates release
            }
            
            queue_event(g_sokol_hal, &event);
            break;
        }
        
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            event.device = INPUT_DEVICE_MOUSE;
            event.data.mouse.x = e->mouse_x;
            event.data.mouse.y = e->mouse_y;
            event.data.mouse.dx = 0;
            event.data.mouse.dy = 0;
            event.data.mouse.wheel_dx = e->scroll_x;
            event.data.mouse.wheel_dy = e->scroll_y;
            
            queue_event(g_sokol_hal, &event);
            break;
        }
        
        // TODO: Add gamepad support when available
        
        default:
            break;
    }
}

// Forward declarations with proper signatures
static bool sokol_init(InputHAL* self, void* platform_data);
static void sokol_shutdown(InputHAL* self);
static void sokol_poll_events(InputHAL* self);
static bool sokol_get_next_event(InputHAL* self, HardwareInputEvent* event);
static bool sokol_is_key_pressed(InputHAL* self, uint32_t key);
static void sokol_get_mouse_position(InputHAL* self, float* x, float* y);
static void sokol_set_mouse_capture(InputHAL* self, bool captured);
static void sokol_set_mouse_visible(InputHAL* self, bool visible);
static void sokol_vibrate_gamepad(InputHAL* self, uint8_t gamepad_id, float left, float right);

// Implementation functions
static bool sokol_init(InputHAL* self, void* platform_data) {
    (void)platform_data;
    
    SokolInputHAL* hal = (SokolInputHAL*)calloc(1, sizeof(SokolInputHAL));
    if (!hal) return false;
    
    // Initialize state
    hal->mouse_visible = true;
    hal->frame_count = 0;
    
    // Store platform data
    self->platform_data = hal;
    
    g_sokol_hal = hal;
    return true;
}

static void sokol_shutdown(InputHAL* self) {
    if (self && self->platform_data) {
        free(self->platform_data);
        self->platform_data = NULL;
    }
    g_sokol_hal = NULL;
}

static void sokol_poll_events(InputHAL* self) {
    // In Sokol, events are pushed to us via callbacks
    // We just increment the frame counter here
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (hal) {
        hal->frame_count++;
    }
}

static bool sokol_get_next_event(InputHAL* self, HardwareInputEvent* event) {
    if (!self || !event) return false;
    
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (!hal) return false;
    
    // Check if queue is empty
    if (hal->read_pos == hal->write_pos) {
        return false;
    }
    
    // Read event from queue
    *event = hal->events[hal->read_pos];
    hal->read_pos = (hal->read_pos + 1) % MAX_QUEUED_EVENTS;
    
    return true;
}

static bool sokol_is_key_pressed(InputHAL* self, uint32_t key) {
    if (!self || key >= 512) return false;
    
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (!hal) return false;
    
    return hal->keys[key];
}

static void sokol_get_mouse_position(InputHAL* self, float* x, float* y) {
    if (!self) return;
    
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (!hal) return;
    
    if (x) *x = hal->mouse_x;
    if (y) *y = hal->mouse_y;
}

static void sokol_set_mouse_capture(InputHAL* self, bool captured) {
    if (!self) return;
    
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (!hal) return;
    
    hal->mouse_captured = captured;
    sapp_lock_mouse(captured);
}

static void sokol_set_mouse_visible(InputHAL* self, bool visible) {
    if (!self) return;
    
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (!hal) return;
    
    hal->mouse_visible = visible;
    sapp_show_mouse(visible);
}

static void sokol_vibrate_gamepad(InputHAL* self, uint8_t gamepad_id, float left, float right) {
    (void)self;
    (void)gamepad_id;
    (void)left;
    (void)right;
    // TODO: Implement when gamepad support is added
}

// Factory function
InputHAL* input_hal_create_sokol(void) {
    InputHAL* hal = (InputHAL*)calloc(1, sizeof(InputHAL));
    if (!hal) return NULL;
    
    hal->init = sokol_init;
    hal->shutdown = sokol_shutdown;
    hal->poll_events = sokol_poll_events;
    hal->get_next_event = sokol_get_next_event;
    hal->is_key_pressed = sokol_is_key_pressed;
    hal->get_mouse_position = sokol_get_mouse_position;
    hal->set_mouse_capture = sokol_set_mouse_capture;
    hal->set_mouse_visible = sokol_set_mouse_visible;
    hal->vibrate_gamepad = sokol_vibrate_gamepad;
    
    return hal;
}

// Get the global HAL instance (temporary)
SokolInputHAL* input_hal_sokol_get_instance(void) {
    return g_sokol_hal;
}