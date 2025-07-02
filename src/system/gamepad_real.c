// src/system/gamepad.c
// Real hidapi-based gamepad implementation for Xbox controllers
// Cross-platform support with emphasis on macOS Bluetooth Xbox controllers

#include "gamepad.h"
#include "../hidapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Static gamepad state array
static GamepadState gamepads[MAX_GAMEPADS];
static bool gamepad_system_initialized = false;
static float axis_deadzone = 0.15f;

// hidapi device handles
static hid_device* devices[MAX_GAMEPADS];

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
                device_info->product_id == 0x02e3);   // Xbox Elite
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
    if (++debug_counter % 60 == 0) {  // Once per second at 60fps
        printf("🎮 HID Report (size=%d): ", size);
        for (int i = 0; i < (size < 20 ? size : 20); i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
        
        // Log interpreted values
        if (size >= 14) {
            printf("   Triggers: LT=%d RT=%d (raw bytes 4,5)\n", data[4], data[5]);
            printf("   Sticks: LX=%d LY=%d RX=%d RY=%d (bytes 6-13)\n",
                   *((int16_t*)(data + 6)), *((int16_t*)(data + 8)),
                   *((int16_t*)(data + 10)), *((int16_t*)(data + 12)));
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
    
    // Try alternate mapping for Xbox controllers on macOS
    int16_t left_x, left_y, right_x, right_y;
    
    if (size >= 14) {
        // Standard mapping
        left_x = *((int16_t*)(data + 6));
        left_y = *((int16_t*)(data + 8));
        right_x = *((int16_t*)(data + 10));
        right_y = *((int16_t*)(data + 12));
    } else if (size >= 13) {
        // Some controllers have 13-byte reports
        left_x = *((int16_t*)(data + 5));
        left_y = *((int16_t*)(data + 7));
        right_x = *((int16_t*)(data + 9));
        right_y = *((int16_t*)(data + 11));
    } else {
        return; // Report too small
    }
    
    // Apply deadzone and normalize
    gamepad->left_stick_x = apply_deadzone(normalize_axis(left_x), axis_deadzone);
    gamepad->left_stick_y = apply_deadzone(normalize_axis(-left_y), axis_deadzone); // Invert Y
    gamepad->right_stick_x = apply_deadzone(normalize_axis(right_x), axis_deadzone);
    gamepad->right_stick_y = apply_deadzone(normalize_axis(-right_y), axis_deadzone); // Invert Y
    
    // Triggers - check if bytes 4 and 5 look like triggers (0-255 range)
    if (data[4] <= 255 && data[5] <= 255) {
        gamepad->left_trigger = data[4] / 255.0f;
        gamepad->right_trigger = data[5] / 255.0f;
    } else {
        // Alternate trigger locations for some controllers
        gamepad->left_trigger = 0.0f;
        gamepad->right_trigger = 0.0f;
    }
    
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
