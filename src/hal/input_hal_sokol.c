/**
 * @file input_hal_sokol.c
 * @brief Sokol implementation of Input HAL
 */

#include "input_hal.h"
#include "../sokol_app.h"
#include "../hidapi.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MAX_QUEUED_EVENTS 256
#define MAX_GAMEPADS 4

// Xbox controller USB IDs - comprehensive support
#define XBOX_VENDOR_ID 0x045E

// Xbox 360 controller PIDs
#define XBOX_360_WIRED_PID      0x028E
#define XBOX_360_WIRELESS_PID   0x0291

// Xbox One controller PIDs  
#define XBOX_ONE_WIRED_PID      0x02DD
#define XBOX_ONE_WIRELESS_PID   0x02E0
#define XBOX_ONE_S_PID          0x02EA
#define XBOX_ONE_ELITE_PID      0x02E3

// Xbox Series X|S controller PIDs
#define XBOX_SERIES_PID         0x0B12

// Dead zone configuration
#define DEFAULT_STICK_DEADZONE  0.15f
#define DEFAULT_TRIGGER_DEADZONE 0.05f

typedef enum {
    XBOX_CONTROLLER_UNKNOWN,
    XBOX_CONTROLLER_360,
    XBOX_CONTROLLER_ONE,
    XBOX_CONTROLLER_SERIES
} XboxControllerType;

typedef struct {
    hid_device* device;
    bool connected;
    XboxControllerType type;
    uint8_t last_report[64];
    float axes[6];  // Left X, Left Y, Right X, Right Y, LT, RT
    uint16_t buttons;
    float stick_deadzone;
    float trigger_deadzone;
} GamepadState;

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
    
    // Gamepad state
    GamepadState gamepads[MAX_GAMEPADS];
    bool hidapi_initialized;
    
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

// Helper: Identify Xbox controller type from PID
static XboxControllerType identify_xbox_controller(uint16_t product_id) {
    switch (product_id) {
        case XBOX_360_WIRED_PID:
        case XBOX_360_WIRELESS_PID:
            return XBOX_CONTROLLER_360;
            
        case XBOX_ONE_WIRED_PID:
        case XBOX_ONE_WIRELESS_PID:
        case XBOX_ONE_S_PID:
        case XBOX_ONE_ELITE_PID:
            return XBOX_CONTROLLER_ONE;
            
        case XBOX_SERIES_PID:
            return XBOX_CONTROLLER_SERIES;
            
        default:
            return XBOX_CONTROLLER_UNKNOWN;
    }
}

// Helper: Apply dead zone to axis value
static float apply_deadzone(float value, float deadzone) {
    if (fabsf(value) < deadzone) {
        return 0.0f;
    }
    
    // Scale the remaining range to maintain full output
    float sign = (value < 0.0f) ? -1.0f : 1.0f;
    return sign * (fabsf(value) - deadzone) / (1.0f - deadzone);
}

// Helper: Convert raw gamepad value to normalized float with dead zone
static float normalize_axis_with_deadzone(int16_t raw_value, float deadzone) {
    if (raw_value == 0) return 0.0f;
    
    float normalized;
    if (raw_value > 0) {
        normalized = (float)raw_value / 32767.0f;
    } else {
        normalized = (float)raw_value / 32768.0f;
    }
    
    return apply_deadzone(normalized, deadzone);
}

// Helper: Parse Xbox controller input report
static void parse_xbox_report(GamepadState* gamepad, const uint8_t* data, size_t len) {
    if (len < 17) return;
    
    // Xbox controller report format varies by controller type:
    // All Xbox controllers use 17-byte HID reports
    // Byte 0: Report ID (0x01)
    // Bytes 1-2: Buttons (little endian)
    // Bytes 3-4: Left stick X (little endian, signed)
    // Bytes 5-6: Left stick Y (little endian, signed)
    // Bytes 7-8: Right stick X (little endian, signed) 
    // Bytes 9-10: Right stick Y (little endian, signed)
    // Bytes 11-12: Triggers (format varies by controller)
    
    // Parse buttons (same format for all Xbox controllers)
    gamepad->buttons = data[1] | (data[2] << 8);
    
    // Parse analog sticks (same format for all Xbox controllers)
    int16_t left_x = (int16_t)(data[3] | (data[4] << 8));
    int16_t left_y = (int16_t)(data[5] | (data[6] << 8));
    int16_t right_x = (int16_t)(data[7] | (data[8] << 8));
    int16_t right_y = (int16_t)(data[9] | (data[10] << 8));
    
    // Parse triggers - format depends on controller type
    uint16_t lt_raw, rt_raw;
    
    switch (gamepad->type) {
        case XBOX_CONTROLLER_360:
            // Xbox 360: Triggers are separate bytes
            lt_raw = data[11];
            rt_raw = data[12];
            // Scale 8-bit to 16-bit range
            lt_raw *= 257;  // 257 = 65535/255 (more accurate than *255)
            rt_raw *= 257;
            break;
            
        case XBOX_CONTROLLER_ONE:
        case XBOX_CONTROLLER_SERIES:
            // Xbox One/Series: Triggers can be 16-bit values
            // Based on user feedback, try individual bytes first
            lt_raw = data[11];
            rt_raw = data[12];
            // Scale 8-bit to 16-bit range  
            lt_raw *= 257;
            rt_raw *= 257;
            break;
            
        default:
            // Fallback: treat as separate bytes
            lt_raw = data[11] * 257;
            rt_raw = data[12] * 257;
            break;
    }
    
    // Convert to normalized floats with dead zones
    gamepad->axes[0] = normalize_axis_with_deadzone(right_x, gamepad->stick_deadzone);   // Right X
    gamepad->axes[1] = -normalize_axis_with_deadzone(right_y, gamepad->stick_deadzone);  // Right Y (inverted)
    gamepad->axes[2] = normalize_axis_with_deadzone(left_x, gamepad->stick_deadzone);    // Left X
    gamepad->axes[3] = -normalize_axis_with_deadzone(left_y, gamepad->stick_deadzone);   // Left Y (inverted)
    gamepad->axes[4] = apply_deadzone((float)lt_raw / 65535.0f, gamepad->trigger_deadzone);  // LT
    gamepad->axes[5] = apply_deadzone((float)rt_raw / 65535.0f, gamepad->trigger_deadzone);  // RT
    
    // Debug output - only log when values change significantly
    static float last_rt = -1.0f;
    static int debug_count = 0;
    
    if (fabsf(gamepad->axes[5] - last_rt) > 0.05f || debug_count < 3) {
        printf("🎮 Controller %s: LT=%.2f RT=%.2f (raw: LT=%d RT=%d, bytes: [11]=%02X [12]=%02X)\n",
               gamepad->type == XBOX_CONTROLLER_360 ? "360" :
               gamepad->type == XBOX_CONTROLLER_ONE ? "One" :
               gamepad->type == XBOX_CONTROLLER_SERIES ? "Series" : "Unknown",
               gamepad->axes[4], gamepad->axes[5], lt_raw, rt_raw, data[11], data[12]);
        last_rt = gamepad->axes[5];
        debug_count++;
    }
}

// Helper: Initialize gamepad system
static void init_gamepads(SokolInputHAL* hal) {
    if (hid_init() != 0) {
        printf("⚠️  Failed to initialize HIDAPI\n");
        return;
    }
    
    hal->hidapi_initialized = true;
    printf("🎮 HIDAPI initialized successfully\n");
    
    // Scan for ALL HID devices first for debugging
    struct hid_device_info* all_devs = hid_enumerate(0, 0);
    struct hid_device_info* cur = all_devs;
    int total_devices = 0;
    int gamepad_count = 0;
    
    printf("🎮 Scanning all HID devices...\n");
    while (cur && total_devices < 20) {  // Limit output
        printf("🔍 HID Device: VID=0x%04X PID=0x%04X Path=%s\n", 
               cur->vendor_id, cur->product_id, cur->path ? cur->path : "NULL");
        
        // Check for various gamepad vendor IDs
        bool is_gamepad = false;
        const char* controller_type = "Unknown";
        
        if (cur->vendor_id == 0x045E) {  // Microsoft
            is_gamepad = true;
            controller_type = "Xbox";
        } else if (cur->vendor_id == 0x054C) {  // Sony
            is_gamepad = true;
            controller_type = "PlayStation";
        } else if (cur->vendor_id == 0x057E) {  // Nintendo
            is_gamepad = true;
            controller_type = "Nintendo";
        }
        
        if (is_gamepad && gamepad_count < MAX_GAMEPADS) {
            printf("🎮 Attempting to open %s controller...\n", controller_type);
            hid_device* dev = hid_open_path(cur->path);
            if (dev) {
                hid_set_nonblocking(dev, 1);  // Non-blocking reads
                hal->gamepads[gamepad_count].device = dev;
                hal->gamepads[gamepad_count].connected = true;
                printf("✅ %s controller connected as gamepad %d\n", controller_type, gamepad_count);
                gamepad_count++;
            } else {
                printf("❌ Failed to open %s controller\n", controller_type);
            }
        }
        
        cur = cur->next;
        total_devices++;
    }
    
    hid_free_enumeration(all_devs);
    printf("🎮 Found %d total HID devices, %d gamepads connected\n", total_devices, gamepad_count);
}

// Helper: Update gamepad state and generate events
static void update_gamepads(SokolInputHAL* hal) {
    if (!hal->hidapi_initialized) return;
    
    static int poll_count = 0;
    poll_count++;
    
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        GamepadState* gamepad = &hal->gamepads[i];
        if (!gamepad->connected || !gamepad->device) continue;
        
        uint8_t report[64];
        int bytes_read = hid_read(gamepad->device, report, sizeof(report));
        
        // Debug: Show read attempts and results more frequently when debugging RT
        if (poll_count % 60 == 0) {  // Every 1 second at 60fps
            printf("🎮 Gamepad %d poll: bytes_read=%d\n", i, bytes_read);
        }
        
        if (bytes_read > 0) {
            // Always show raw report data to debug RT trigger
            static uint8_t last_report[64] = {0};
            static bool first_report = true;
            
            // Compare with last report to see what changed
            bool changed = first_report || memcmp(report, last_report, bytes_read) != 0;
            
            if (changed) {
                printf("🎮 Raw report (%d bytes): ", bytes_read);
                for (int j = 0; j < bytes_read && j < 17; j++) {
                    printf("%02X ", report[j]);
                }
                printf("\n");
                
                if (!first_report) {
                    printf("🎮 Changed bytes: ");
                    bool any_changed = false;
                    for (int j = 0; j < bytes_read && j < 17; j++) {
                        if (report[j] != last_report[j]) {
                            printf("[%d]:%02X→%02X ", j, last_report[j], report[j]);
                            any_changed = true;
                        }
                    }
                    if (!any_changed) {
                        printf("NO CHANGES DETECTED!");
                    }
                    printf("\n");
                }
                
                memcpy(last_report, report, bytes_read);
                first_report = false;
            } else if (bytes_read > 0) {
                // Even if no changes, check if RT might be in button data
                static int no_change_debug = 0;
                if (no_change_debug < 2) {
                    printf("🎮 No changes detected, but got %d bytes. Button data: [1]=%02X [2]=%02X\n", 
                           bytes_read, report[1], report[2]);
                    no_change_debug++;
                }
            }
            
            // Parse the report
            float prev_axes[6];
            memcpy(prev_axes, gamepad->axes, sizeof(prev_axes));
            
            parse_xbox_report(gamepad, report, bytes_read);
            
            // Generate events for axis changes
            for (int axis = 0; axis < 6; axis++) {
                if (fabsf(gamepad->axes[axis] - prev_axes[axis]) > 0.01f) {
                    HardwareInputEvent event = {0};
                    event.device = INPUT_DEVICE_GAMEPAD;
                    event.timestamp = hal->frame_count;
                    event.data.gamepad.id = i;
                    memcpy(event.data.gamepad.axes, gamepad->axes, sizeof(gamepad->axes));
                    event.data.gamepad.buttons = gamepad->buttons;
                    
                    // Debug: Show first few gamepad events
                    static int debug_count = 0;
                    if (debug_count < 10) {
                        printf("🎮 Gamepad %d Axis %d: %.2f → %.2f\n", 
                               i, axis, prev_axes[axis], gamepad->axes[axis]);
                        debug_count++;
                    }
                    
                    queue_event(hal, &event);
                }
            }
        } else if (bytes_read < 0) {
            // Error reading
            if (poll_count % 300 == 0) {
                printf("⚠️  Error reading from gamepad %d: %d\n", i, bytes_read);
            }
        }
    }
}

// Helper: Clean up gamepad resources
static void cleanup_gamepads(SokolInputHAL* hal) {
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (hal->gamepads[i].device) {
            hid_close(hal->gamepads[i].device);
            hal->gamepads[i].device = NULL;
            hal->gamepads[i].connected = false;
        }
    }
    
    if (hal->hidapi_initialized) {
        hid_exit();
        hal->hidapi_initialized = false;
    }
}

// Sokol event handler - called by Sokol framework
void input_hal_sokol_event_handler(const sapp_event* e) {
    if (!g_sokol_hal) {
        return;  // Silently ignore if not initialized
    }
    
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
    if (!hal) {
        return false;
    }
    
    // Initialize state
    hal->mouse_visible = true;
    hal->frame_count = 0;
    
    // Initialize gamepad system
    init_gamepads(hal);
    
    // Store platform data
    self->platform_data = hal;
    
    g_sokol_hal = hal;
    return true;
}

static void sokol_shutdown(InputHAL* self) {
    if (self && self->platform_data) {
        SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
        cleanup_gamepads(hal);
        free(self->platform_data);
        self->platform_data = NULL;
    }
    g_sokol_hal = NULL;
}

static void sokol_poll_events(InputHAL* self) {
    // In Sokol, events are pushed to us via callbacks
    // We just increment the frame counter here and poll gamepads
    SokolInputHAL* hal = (SokolInputHAL*)self->platform_data;
    if (hal) {
        hal->frame_count++;
        // Poll gamepad state
        update_gamepads(hal);
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