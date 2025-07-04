// src/system/gamepad_wasm.c
// Simplified gamepad implementation for WebAssembly (no HID support)

#include "gamepad.h"
#include "gamepad_hotplug.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Static gamepad state array
static GamepadState gamepads[MAX_GAMEPADS];
static bool gamepad_system_initialized = false;
static float axis_deadzone = 0.08f;

// Initialize the gamepad system
bool gamepad_init(void) {
    printf("🎮 Initializing WASM gamepad system (keyboard-only)\n");
    
    // Initialize all gamepads as disconnected
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        memset(&gamepads[i], 0, sizeof(GamepadState));
        gamepads[i].connected = false;
        gamepads[i].hid_device = NULL;
    }
    
    gamepad_system_initialized = true;
    return true;
}

// Shutdown the gamepad system
void gamepad_shutdown(void) {
    if (!gamepad_system_initialized) {
        return;
    }
    
    printf("🎮 Shutting down WASM gamepad system\n");
    gamepad_system_initialized = false;
}

// Update gamepad states
void gamepad_update(float delta_time) {
    (void)delta_time;
    // No-op for WASM - browser handles gamepad API
}

// Poll gamepad states
void gamepad_poll(void) {
    // No-op for WASM
}

// Get gamepad state by index
GamepadState* gamepad_get_state(int gamepad_index) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        return NULL;
    }
    return &gamepads[gamepad_index];
}

// Check if gamepad is connected
bool gamepad_is_connected(int gamepad_index) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        return false;
    }
    return gamepads[gamepad_index].connected;
}

// Check button states
bool gamepad_button_pressed(int gamepad_index, GamepadButton button) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || button >= GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    return gamepads[gamepad_index].buttons[button];
}

bool gamepad_button_just_pressed(int gamepad_index, GamepadButton button) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || button >= GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    return gamepads[gamepad_index].buttons[button] && !gamepads[gamepad_index].buttons_previous[button];
}

bool gamepad_button_just_released(int gamepad_index, GamepadButton button) {
    if (gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS || button >= GAMEPAD_BUTTON_COUNT) {
        return false;
    }
    return !gamepads[gamepad_index].buttons[button] && gamepads[gamepad_index].buttons_previous[button];
}

// Get button name
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
        case GAMEPAD_BUTTON_DPAD_UP: return "DPad Up";
        case GAMEPAD_BUTTON_DPAD_DOWN: return "DPad Down";
        case GAMEPAD_BUTTON_DPAD_LEFT: return "DPad Left";
        case GAMEPAD_BUTTON_DPAD_RIGHT: return "DPad Right";
        default: return "Unknown";
    }
}

// Get gamepad state by index
GamepadState* gamepad_get(int index) {
    if (index < 0 || index >= MAX_GAMEPADS) {
        return NULL;
    }
    return &gamepads[index];
}

// Get first connected gamepad
GamepadState* gamepad_get_first_connected(void) {
    return NULL;  // No gamepad support in WASM yet
}

// Get count of connected gamepads
int gamepad_get_connected_count(void) {
    return 0;  // No gamepad support in WASM yet
}

// Check if a specific button is pressed
bool gamepad_is_button_pressed(int gamepad_index, GamepadButton button) {
    (void)gamepad_index;
    (void)button;
    return false;
}

// Check if a specific button was just pressed this frame
bool gamepad_is_button_just_pressed(int gamepad_index, GamepadButton button) {
    (void)gamepad_index;
    (void)button;
    return false;
}

// Get axis value with deadzone applied
float gamepad_get_axis(int gamepad_index, const char* axis_name) {
    (void)gamepad_index;
    (void)axis_name;
    return 0.0f;
}

// Set axis deadzone
void gamepad_set_deadzone(float deadzone) {
    axis_deadzone = fmaxf(0.0f, fminf(1.0f, deadzone));
}

// Gamepad hot-plug detection
void gamepad_hotplug_init(void* on_connected, void* on_disconnected) {
    (void)on_connected;
    (void)on_disconnected;
    // No hot-plug support in WASM
}

void gamepad_hotplug_shutdown(void) {
    // No-op
}

void gamepad_hotplug_enable(bool enabled) {
    (void)enabled;
    // No-op
}

bool gamepad_hotplug_is_enabled(void) {
    return false;
}

void gamepad_hotplug_set_interval(float seconds) {
    (void)seconds;
    // No-op
}

// Detailed gamepad info
const char* gamepad_get_name(int gamepad_index) {
    (void)gamepad_index;
    return "WASM Keyboard";
}

// Vibration/rumble support
void gamepad_set_vibration(int gamepad_index, float left_motor, float right_motor) {
    (void)gamepad_index;
    (void)left_motor;
    (void)right_motor;
    // No vibration support in WASM
}

// Per-gamepad calibration
void gamepad_calibrate(int gamepad_index) {
    (void)gamepad_index;
    // No calibration needed for WASM
}

// Battery level
int gamepad_get_battery_level(int gamepad_index) {
    (void)gamepad_index;
    return -1;  // Not supported
}