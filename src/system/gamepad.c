// src/system/gamepad.c
// Real hidapi-based gamepad implementation for Xbox controllers
// Cross-platform support with emphasis on macOS Bluetooth Xbox controllers

#include "gamepad.h"
#include "gamepad_hotplug.h"
#include "input.h"  // For input_get_last_device_type
#include "../hidapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Static gamepad state array
static GamepadState gamepads[MAX_GAMEPADS];
static bool gamepad_system_initialized = false;
static float axis_deadzone = 0.20f;  // Increased to handle stick drift

// hidapi device handles
static hid_device* devices[MAX_GAMEPADS];

// Hot-plug detection state
static GamepadHotplugState hotplug_state = {
    .enabled = true,
    .check_interval = 1.0f,  // Check every second
    .time_since_check = 0.0f,
    .last_connected_count = 0,
    .on_connected = NULL,
    .on_disconnected = NULL
};

// Input device tracking - now handled by input.c

// Utility function to normalize axis values
static float normalize_axis(int16_t raw_value) {
    return (float)raw_value / 32767.0f;
}

// Apply deadzone to analog stick value
static float apply_deadzone(float value, float deadzone) {
    if (fabsf(value) < deadzone) {
        return 0.0f;
    }
    
    // Scale remaining range to full 0-1 range
    float sign = (value < 0.0f) ? -1.0f : 1.0f;
    float abs_value = fabsf(value);
    return sign * ((abs_value - deadzone) / (1.0f - deadzone));
}

// Check if device is a supported gamepad
static bool is_supported_gamepad(struct hid_device_info* device_info) {
    // Xbox controllers
    if (device_info->vendor_id == 0x045e) {
        return (device_info->product_id == 0x02ea ||  // Xbox One
                device_info->product_id == 0x028e ||  // Xbox 360
                device_info->product_id == 0x02e3 ||  // Xbox Elite
                device_info->product_id == 0x0b13);   // Xbox Wireless Controller
    }
    
    // PlayStation controllers
    if (device_info->vendor_id == 0x054c) {
        return (device_info->product_id == 0x09cc ||  // DualShock 4
                device_info->product_id == 0x0ce6);   // DualSense
    }
    
    // 8BitDo controllers
    if (device_info->vendor_id == 0x2dc8) {
        return true;  // Most 8BitDo controllers work with Xbox protocol
    }
    
    return false;
}

// Parse Xbox-style controller input report
static void parse_xbox_report(GamepadState* gamepad, const unsigned char* data, int size) {
    if (size < 14) return;  // Minimum expected size for Xbox controller report
    
    // Store previous button states for edge detection
    memcpy(gamepad->buttons_previous, gamepad->buttons, sizeof(gamepad->buttons));
    
    // Debug: Log raw HID report data periodically with more detail
    static int debug_counter = 0;
    if (++debug_counter % 3 == 0) {  // 20 times per second at 60fps
        printf("🎮 HID Report (size=%d): ", size);
        for (int i = 0; i < (size < 20 ? size : 20); i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
        
        // Compact byte display for easier reading
        printf("   Bytes: ");
        for (int i = 0; i < (size < 20 ? size : 20); i++) {
            printf("[%d]=%d ", i, data[i]);
        }
        printf("\n");
        
        // Show interpretation
        printf("   Interpreted: LT=%.2f RT=%.2f LS(%.2f,%.2f) RS(%.2f,%.2f)\n",
               gamepad->left_trigger, gamepad->right_trigger,
               gamepad->left_stick_x, gamepad->left_stick_y,
               gamepad->right_stick_x, gamepad->right_stick_y);
        
        // Check for unusual values that might indicate wrong mapping
        if (fabsf(gamepad->left_stick_x) > 0.9f || fabsf(gamepad->left_stick_y) > 0.9f ||
            fabsf(gamepad->right_stick_x) > 0.9f || fabsf(gamepad->right_stick_y) > 0.9f) {
            printf("   ⚠️ WARNING: Stick at extreme value - might be reading trigger as stick!\n");
        }
    }
    
    // Xbox controller report format
    // Common Xbox One controller on macOS format:
    // Byte 0: Report ID (usually 0x01)
    // Byte 1: D-pad and some buttons
    // Byte 2-3: More buttons
    // Byte 4: Left trigger (0-255)
    // Byte 5: Right trigger (0-255)
    // Byte 6-7: Left stick X (16-bit signed)
    // Byte 8-9: Left stick Y (16-bit signed)
    // Byte 10-11: Right stick X (16-bit signed)
    // Byte 12-13: Right stick Y (16-bit signed)
    
    // Xbox Wireless Controller on macOS typical layout
    // After analyzing many controllers, the most common layout is:
    // Bytes 0-1: Report ID and buttons
    // Bytes 2-3: More buttons
    // Bytes 4-11: Sticks (16-bit signed integers)
    // Bytes 12-13: Triggers (8-bit unsigned)
    
    int16_t left_x = 0, left_y = 0, right_x = 0, right_y = 0;
    float left_trigger = 0.0f, right_trigger = 0.0f;
    
    if (size >= 14) {
        // Standard Xbox controller layout on macOS
        left_x = *((int16_t*)(data + 4));
        left_y = *((int16_t*)(data + 6));
        right_x = *((int16_t*)(data + 8));
        right_y = *((int16_t*)(data + 10));
        
        // Raw trigger values
        uint8_t lt_raw = data[12];
        uint8_t rt_raw = data[13];
        
        // Xbox controllers often have a resting position around 127-128
        // We need to calibrate for this
        const uint8_t trigger_center = 127;
        const uint8_t trigger_deadzone = 20;
        
        // Process left trigger
        if (lt_raw > trigger_center + trigger_deadzone) {
            left_trigger = (lt_raw - trigger_center) / 128.0f;
        } else {
            left_trigger = 0.0f;
        }
        
        // Process right trigger
        if (rt_raw > trigger_center + trigger_deadzone) {
            right_trigger = (rt_raw - trigger_center) / 128.0f;
        } else {
            right_trigger = 0.0f;
        }
        
        // Clamp to 0-1 range
        left_trigger = fminf(fmaxf(left_trigger, 0.0f), 1.0f);
        right_trigger = fminf(fmaxf(right_trigger, 0.0f), 1.0f);
        
        // Debug specific for trigger issue
        static int trigger_debug = 0;
        if (++trigger_debug % 30 == 0 && (lt_raw != trigger_center || rt_raw != trigger_center)) {
            printf("🎮 TRIGGER DEBUG: LT_raw=%d RT_raw=%d → LT=%.2f RT=%.2f\n",
                   lt_raw, rt_raw, left_trigger, right_trigger);
        }
    } else {
        return; // Report too small
    }
    
    // Apply deadzone and normalize
    gamepad->left_stick_x = apply_deadzone(normalize_axis(left_x), axis_deadzone);
    gamepad->left_stick_y = apply_deadzone(normalize_axis(-left_y), axis_deadzone); // Invert Y
    gamepad->right_stick_x = apply_deadzone(normalize_axis(right_x), axis_deadzone);
    gamepad->right_stick_y = apply_deadzone(normalize_axis(-right_y), axis_deadzone); // Invert Y
    
    // Set triggers from detected values
    gamepad->left_trigger = left_trigger;
    gamepad->right_trigger = right_trigger;
    
    // Buttons (byte layout may vary)
    uint8_t buttons1 = data[2];
    uint8_t buttons2 = data[3];
    
    gamepad->buttons[GAMEPAD_BUTTON_A] = (buttons1 & 0x10) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_B] = (buttons1 & 0x20) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_X] = (buttons1 & 0x40) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_Y] = (buttons1 & 0x80) != 0;
    
    gamepad->buttons[GAMEPAD_BUTTON_LB] = (buttons2 & 0x01) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_RB] = (buttons2 & 0x02) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_BACK] = (buttons2 & 0x10) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_START] = (buttons2 & 0x20) != 0;
    
    gamepad->buttons[GAMEPAD_BUTTON_LS] = (buttons2 & 0x40) != 0;
    gamepad->buttons[GAMEPAD_BUTTON_RS] = (buttons2 & 0x80) != 0;
    
    // D-pad (often in lower 4 bits of a byte)
    uint8_t dpad = data[1] & 0x0F;
    gamepad->buttons[GAMEPAD_BUTTON_DPAD_UP] = (dpad == 0 || dpad == 1 || dpad == 7);
    gamepad->buttons[GAMEPAD_BUTTON_DPAD_RIGHT] = (dpad == 1 || dpad == 2 || dpad == 3);
    gamepad->buttons[GAMEPAD_BUTTON_DPAD_DOWN] = (dpad == 3 || dpad == 4 || dpad == 5);
    gamepad->buttons[GAMEPAD_BUTTON_DPAD_LEFT] = (dpad == 5 || dpad == 6 || dpad == 7);
}

// Public API implementation

bool gamepad_init(void) {
    if (gamepad_system_initialized) {
        printf("⚠️  Gamepad system already initialized\n");
        return true;
    }
    
    printf("🎮 Initializing gamepad system with hidapi...\n");
    
    // Initialize hidapi
    if (hid_init() != 0) {
        printf("❌ Failed to initialize hidapi\n");
        return false;
    }
    
    // Initialize gamepad states and device handles
    memset(gamepads, 0, sizeof(gamepads));
    memset(devices, 0, sizeof(devices));
    
    // Enumerate and connect supported controllers
    struct hid_device_info* device_list = hid_enumerate(0x0, 0x0);
    struct hid_device_info* current_device = device_list;
    
    int gamepad_index = 0;
    
    while (current_device && gamepad_index < MAX_GAMEPADS) {
        if (is_supported_gamepad(current_device)) {
            hid_device* handle = hid_open_path(current_device->path);
            if (handle) {
                devices[gamepad_index] = handle;
                GamepadState* gamepad = &gamepads[gamepad_index];
                
                gamepad->connected = true;
                gamepad->hid_device = handle;
                gamepad->vendor_id = current_device->vendor_id;
                gamepad->product_id = current_device->product_id;
                
                // Get product string
                if (current_device->product_string) {
                    wcstombs(gamepad->product_string, current_device->product_string, 
                            sizeof(gamepad->product_string) - 1);
                    gamepad->product_string[sizeof(gamepad->product_string) - 1] = '\0';
                } else {
                    snprintf(gamepad->product_string, sizeof(gamepad->product_string), 
                            "Gamepad %d", gamepad_index);
                }
                
                // Set non-blocking mode
                hid_set_nonblocking(handle, 1);
                
                printf("🎮 Connected gamepad %d: %s (VID:0x%04X PID:0x%04X)\n", 
                       gamepad_index, gamepad->product_string, 
                       gamepad->vendor_id, gamepad->product_id);
                
                gamepad_index++;
            }
        }
        
        current_device = current_device->next;
    }
    
    hid_free_enumeration(device_list);
    
    if (gamepad_index > 0) {
        printf("🎮 Found %d gamepad(s)\n", gamepad_index);
    } else {
        printf("🎮 No supported gamepads detected\n");
        printf("🎮 Supported: Xbox (VID:0x045E), PlayStation (VID:0x054C), 8BitDo (VID:0x2DC8)\n");
    }
    
    gamepad_system_initialized = true;
    printf("✅ Gamepad system initialized successfully\n");
    return true;
}

void gamepad_shutdown(void) {
    if (!gamepad_system_initialized) {
        return;
    }
    
    printf("🎮 Shutting down gamepad system...\n");
    
    // Close all connected devices
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (devices[i]) {
            hid_close(devices[i]);
            devices[i] = NULL;
            gamepads[i].connected = false;
            gamepads[i].hid_device = NULL;
        }
    }
    
    // Shutdown hidapi
    hid_exit();
    
    gamepad_system_initialized = false;
    printf("✅ Gamepad system shutdown complete\n");
}

void gamepad_poll(void) {
    if (!gamepad_system_initialized) {
        return;
    }
    
    unsigned char buffer[64];
    
    // Poll each connected gamepad
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (!devices[i]) {
            gamepads[i].connected = false;
            continue;
        }
        
        GamepadState* gamepad = &gamepads[i];
        
        // Read input report
        int bytes_read = hid_read(devices[i], buffer, sizeof(buffer));
        
        if (bytes_read > 0) {
            parse_xbox_report(gamepad, buffer, bytes_read);
            gamepad->connected = true;
        } else if (bytes_read < 0) {
            // Device disconnected
            printf("🎮 Gamepad %d disconnected\n", i);
            hid_close(devices[i]);
            devices[i] = NULL;
            gamepad->connected = false;
            gamepad->hid_device = NULL;
            
            // Call disconnected callback
            if (hotplug_state.on_disconnected) {
                hotplug_state.on_disconnected(i);
            }
        }
    }
}

GamepadState* gamepad_get_state(int gamepad_index) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        return NULL;
    }
    return &gamepads[gamepad_index];
}

bool gamepad_is_connected(int gamepad_index) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        return false;
    }
    return gamepads[gamepad_index].connected;
}

bool gamepad_button_pressed(int gamepad_index, GamepadButton button) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || 
        button < 0 || button >= GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    
    GamepadState* gamepad = &gamepads[gamepad_index];
    if (!gamepad->connected) {
        return false;
    }
    
    return gamepad->buttons[button];
}

bool gamepad_button_just_pressed(int gamepad_index, GamepadButton button) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || 
        button < 0 || button >= GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    
    GamepadState* gamepad = &gamepads[gamepad_index];
    if (!gamepad->connected) {
        return false;
    }
    
    return gamepad->buttons[button] && !gamepad->buttons_previous[button];
}

bool gamepad_button_just_released(int gamepad_index, GamepadButton button) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || 
        button < 0 || button >= GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    
    GamepadState* gamepad = &gamepads[gamepad_index];
    if (!gamepad->connected) {
        return false;
    }
    
    return !gamepad->buttons[button] && gamepad->buttons_previous[button];
}

float gamepad_get_axis(int gamepad_index, const char* axis_name) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || !axis_name) {
        return 0.0f;
    }
    
    GamepadState* gamepad = &gamepads[gamepad_index];
    if (!gamepad->connected) {
        return 0.0f;
    }
    
    if (strcmp(axis_name, "left_x") == 0) return gamepad->left_stick_x;
    if (strcmp(axis_name, "left_y") == 0) return gamepad->left_stick_y;
    if (strcmp(axis_name, "right_x") == 0) return gamepad->right_stick_x;
    if (strcmp(axis_name, "right_y") == 0) return gamepad->right_stick_y;
    if (strcmp(axis_name, "left_trigger") == 0) return gamepad->left_trigger;
    if (strcmp(axis_name, "right_trigger") == 0) return gamepad->right_trigger;
    
    return 0.0f;
}

const char* gamepad_get_button_name(GamepadButton button) {
    switch (button) {
        case GAMEPAD_BUTTON_A: return "A";
        case GAMEPAD_BUTTON_B: return "B";
        case GAMEPAD_BUTTON_X: return "X";
        case GAMEPAD_BUTTON_Y: return "Y";
        case GAMEPAD_BUTTON_LB: return "LB";
        case GAMEPAD_BUTTON_RB: return "RB";
        case GAMEPAD_BUTTON_BACK: return "Back";
        case GAMEPAD_BUTTON_START: return "Start";
        case GAMEPAD_BUTTON_LS: return "LS";
        case GAMEPAD_BUTTON_RS: return "RS";
        case GAMEPAD_BUTTON_DPAD_UP: return "D-Up";
        case GAMEPAD_BUTTON_DPAD_DOWN: return "D-Down";
        case GAMEPAD_BUTTON_DPAD_LEFT: return "D-Left";
        case GAMEPAD_BUTTON_DPAD_RIGHT: return "D-Right";
        default: return "Unknown";
    }
}

void gamepad_set_deadzone(float deadzone) {
    if (deadzone >= 0.0f && deadzone <= 1.0f) {
        axis_deadzone = deadzone;
        printf("🎮 Gamepad deadzone set to %.2f\n", deadzone);
    }
}

// ============================================================================
// HOT-PLUG DETECTION
// ============================================================================

// Internal function to scan for new controllers
static void scan_for_new_controllers(void) {
    struct hid_device_info* device_list = hid_enumerate(0x0, 0x0);
    struct hid_device_info* current_device = device_list;
    
    while (current_device) {
        if (is_supported_gamepad(current_device)) {
            // Check if this device is already connected
            bool already_connected = false;
            for (int i = 0; i < MAX_GAMEPADS; i++) {
                if (gamepads[i].connected && 
                    gamepads[i].vendor_id == current_device->vendor_id &&
                    gamepads[i].product_id == current_device->product_id) {
                    // Simple check - could be improved with serial number
                    already_connected = true;
                    break;
                }
            }
            
            if (!already_connected) {
                // Find empty slot
                for (int i = 0; i < MAX_GAMEPADS; i++) {
                    if (!devices[i]) {
                        hid_device* handle = hid_open_path(current_device->path);
                        if (handle) {
                            devices[i] = handle;
                            GamepadState* gamepad = &gamepads[i];
                            
                            gamepad->connected = true;
                            gamepad->hid_device = handle;
                            gamepad->vendor_id = current_device->vendor_id;
                            gamepad->product_id = current_device->product_id;
                            
                            // Get product string
                            if (current_device->product_string) {
                                wcstombs(gamepad->product_string, current_device->product_string, 
                                        sizeof(gamepad->product_string) - 1);
                                gamepad->product_string[sizeof(gamepad->product_string) - 1] = '\0';
                            } else {
                                snprintf(gamepad->product_string, sizeof(gamepad->product_string), 
                                        "Gamepad %d", i);
                            }
                            
                            // Set non-blocking mode
                            hid_set_nonblocking(handle, 1);
                            
                            printf("🎮 Hot-plugged gamepad %d: %s (VID:0x%04X PID:0x%04X)\n", 
                                   i, gamepad->product_string, 
                                   gamepad->vendor_id, gamepad->product_id);
                            
                            // Call connected callback
                            if (hotplug_state.on_connected) {
                                hotplug_state.on_connected(i);
                            }
                            
                            break;
                        }
                    }
                }
            }
        }
        
        current_device = current_device->next;
    }
    
    hid_free_enumeration(device_list);
}

void gamepad_enable_hotplug(bool enable) {
    hotplug_state.enabled = enable;
    if (enable) {
        printf("🎮 Gamepad hot-plug detection enabled (interval: %.1fs)\n", 
               hotplug_state.check_interval);
    } else {
        printf("🎮 Gamepad hot-plug detection disabled\n");
    }
}

void gamepad_set_hotplug_interval(float seconds) {
    if (seconds >= 0.1f && seconds <= 10.0f) {
        hotplug_state.check_interval = seconds;
        printf("🎮 Gamepad hot-plug check interval set to %.1fs\n", seconds);
    }
}

void gamepad_check_connections(void) {
    if (!gamepad_system_initialized) {
        return;
    }
    
    scan_for_new_controllers();
    
    // Update connected count
    int connected_count = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (gamepads[i].connected) {
            connected_count++;
        }
    }
    
    if (connected_count != hotplug_state.last_connected_count) {
        printf("🎮 Connected gamepads: %d\n", connected_count);
        hotplug_state.last_connected_count = connected_count;
    }
}

void gamepad_update_hotplug(float delta_time) {
    if (!gamepad_system_initialized || !hotplug_state.enabled) {
        return;
    }
    
    hotplug_state.time_since_check += delta_time;
    
    if (hotplug_state.time_since_check >= hotplug_state.check_interval) {
        hotplug_state.time_since_check = 0.0f;
        gamepad_check_connections();
    }
}

void gamepad_set_connected_callback(void (*callback)(int)) {
    hotplug_state.on_connected = callback;
}

void gamepad_set_disconnected_callback(void (*callback)(int)) {
    hotplug_state.on_disconnected = callback;
}

// ============================================================================
// INPUT DEVICE TRACKING
// ============================================================================
// Note: input_get_last_device_type and input_set_last_device_type have been
// moved to input.c for the canyon racing system to avoid duplicate symbols

bool input_was_gamepad_used_last(void) {
    // Now delegates to the input system
    return input_get_last_device_type() == INPUT_DEVICE_GAMEPAD;
}

// ============================================================================
// UI HELPER FUNCTIONS
// ============================================================================

const char* gamepad_get_button_icon(GamepadButton button) {
    // Return Unicode symbols or icon names for buttons
    switch (button) {
        case GAMEPAD_BUTTON_A: return "Ⓐ";
        case GAMEPAD_BUTTON_B: return "Ⓑ";
        case GAMEPAD_BUTTON_X: return "Ⓧ";
        case GAMEPAD_BUTTON_Y: return "Ⓨ";
        case GAMEPAD_BUTTON_LB: return "LB";
        case GAMEPAD_BUTTON_RB: return "RB";
        case GAMEPAD_BUTTON_BACK: return "⧉";
        case GAMEPAD_BUTTON_START: return "☰";
        case GAMEPAD_BUTTON_LS: return "LS";
        case GAMEPAD_BUTTON_RS: return "RS";
        case GAMEPAD_BUTTON_DPAD_UP: return "↑";
        case GAMEPAD_BUTTON_DPAD_DOWN: return "↓";
        case GAMEPAD_BUTTON_DPAD_LEFT: return "←";
        case GAMEPAD_BUTTON_DPAD_RIGHT: return "→";
        default: return "?";
    }
}

const char* gamepad_get_axis_icon(const char* axis_name) {
    if (!axis_name) return "?";
    
    if (strcmp(axis_name, "left_x") == 0) return "LS→";
    if (strcmp(axis_name, "left_y") == 0) return "LS↑";
    if (strcmp(axis_name, "right_x") == 0) return "RS→";
    if (strcmp(axis_name, "right_y") == 0) return "RS↑";
    if (strcmp(axis_name, "left_trigger") == 0) return "LT";
    if (strcmp(axis_name, "right_trigger") == 0) return "RT";
    
    return "?";
}

bool gamepad_navigate_menu(int* selected_index, int menu_item_count) {
    if (!selected_index || menu_item_count <= 0) {
        return false;
    }
    
    GamepadState* gamepad = gamepad_get_primary();
    if (!gamepad || !gamepad->connected) {
        return false;
    }
    
    bool changed = false;
    static float nav_cooldown = 0.0f;
    
    // Simple cooldown to prevent too-fast navigation
    if (nav_cooldown > 0.0f) {
        nav_cooldown -= 0.016f; // Assume 60 FPS
        return false;
    }
    
    // D-pad navigation
    if (gamepad->buttons[GAMEPAD_BUTTON_DPAD_UP] || gamepad->left_stick_y > 0.5f) {
        (*selected_index)--;
        if (*selected_index < 0) {
            *selected_index = menu_item_count - 1;
        }
        changed = true;
        nav_cooldown = 0.2f;
    } else if (gamepad->buttons[GAMEPAD_BUTTON_DPAD_DOWN] || gamepad->left_stick_y < -0.5f) {
        (*selected_index)++;
        if (*selected_index >= menu_item_count) {
            *selected_index = 0;
        }
        changed = true;
        nav_cooldown = 0.2f;
    }
    
    return changed;
}

GamepadState* gamepad_get_primary(void) {
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (gamepads[i].connected) {
            return &gamepads[i];
        }
    }
    return NULL;
}

int gamepad_get_primary_index(void) {
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (gamepads[i].connected) {
            return i;
        }
    }
    return -1;
}
