// src/system/gamepad.h
// Cross-platform gamepad input using hidapi
// Supports Xbox controllers via Bluetooth on macOS

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MAX_GAMEPADS 4

// Xbox controller button mappings
typedef enum {
    GAMEPAD_BUTTON_A = 0,
    GAMEPAD_BUTTON_B = 1,
    GAMEPAD_BUTTON_X = 2,
    GAMEPAD_BUTTON_Y = 3,
    GAMEPAD_BUTTON_LB = 4,
    GAMEPAD_BUTTON_RB = 5,
    GAMEPAD_BUTTON_BACK = 6,
    GAMEPAD_BUTTON_START = 7,
    GAMEPAD_BUTTON_LS = 8,      // Left stick click
    GAMEPAD_BUTTON_RS = 9,      // Right stick click
    GAMEPAD_BUTTON_DPAD_UP = 10,
    GAMEPAD_BUTTON_DPAD_DOWN = 11,
    GAMEPAD_BUTTON_DPAD_LEFT = 12,
    GAMEPAD_BUTTON_DPAD_RIGHT = 13,
    GAMEPAD_BUTTON_COUNT = 14
} GamepadButton;

// Gamepad state structure
typedef struct {
    bool connected;
    
    // Analog sticks (normalized -1.0 to 1.0)
    float left_stick_x;
    float left_stick_y;
    float right_stick_x;
    float right_stick_y;
    
    // Triggers (0.0 to 1.0)
    float left_trigger;
    float right_trigger;
    
    // Buttons (true = pressed)
    bool buttons[GAMEPAD_BUTTON_COUNT];
    
    // Raw button state for edge detection
    bool buttons_previous[GAMEPAD_BUTTON_COUNT];
    
    // Internal hidapi device handle
    void* hid_device;
    
    // Controller info
    char product_string[128];
    unsigned short vendor_id;
    unsigned short product_id;
} GamepadState;

// Xbox controller vendor/product IDs
#define XBOX_VENDOR_ID 0x045E
#define XBOX_ONE_PRODUCT_ID 0x02EA
#define XBOX_360_PRODUCT_ID 0x028E
#define XBOX_ELITE_PRODUCT_ID 0x02E3

// Gamepad system functions
bool gamepad_init(void);
void gamepad_shutdown(void);
void gamepad_poll(void);

// Per-gamepad functions
GamepadState* gamepad_get_state(int gamepad_index);
bool gamepad_is_connected(int gamepad_index);
bool gamepad_button_pressed(int gamepad_index, GamepadButton button);
bool gamepad_button_just_pressed(int gamepad_index, GamepadButton button);
bool gamepad_button_just_released(int gamepad_index, GamepadButton button);
float gamepad_get_axis(int gamepad_index, const char* axis_name);

// Utility functions
const char* gamepad_get_button_name(GamepadButton button);
void gamepad_set_deadzone(float deadzone);  // Default 0.15f
