// src/system/gamepad.c
// Cross-platform gamepad input implementation
// Currently using stub implementation - will integrate hidapi next

#include "gamepad.h"
#include <stdio.h>
#include <string.h>

// Internal gamepad state storage
static GamepadState gamepads[MAX_GAMEPADS];
static bool gamepad_system_initialized = false;
static int connected_count = 0;

// Stub implementation - will be replaced with hidapi
bool gamepad_init(void) {
    if (gamepad_system_initialized) {
        return true;
    }
    
    // Clear all gamepad states
    memset(gamepads, 0, sizeof(gamepads));
    connected_count = 0;
    
    printf("🎮 Gamepad system initialized (stub implementation)\n");
    printf("🎮 Note: Connect Xbox controller to test - hidapi integration coming next\n");
    
    gamepad_system_initialized = true;
    return true;
}

void gamepad_shutdown(void) {
    if (!gamepad_system_initialized) {
        return;
    }
    
    memset(gamepads, 0, sizeof(gamepads));
    connected_count = 0;
    gamepad_system_initialized = false;
    
    printf("🎮 Gamepad system shutdown\n");
}

void gamepad_poll(void) {
    if (!gamepad_system_initialized) {
        return;
    }
    
    // TODO: Replace with hidapi polling
    // For now, this is a stub that does nothing
    // When hidapi is integrated, this will:
    // 1. Enumerate connected HID devices
    // 2. Read input reports from Xbox controllers
    // 3. Parse reports into GamepadState structures
    // 4. Update the gamepads[] array
}

GamepadState gamepad_get_state(int gamepad_index) {
    if (!gamepad_system_initialized || gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        GamepadState empty = {0};
        return empty;
    }
    
    return gamepads[gamepad_index];
}

int gamepad_get_count(void) {
    return connected_count;
}

bool gamepad_is_connected(int gamepad_index) {
    if (!gamepad_system_initialized || gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        return false;
    }
    
    return gamepads[gamepad_index].connected;
}

const char* gamepad_get_name(int gamepad_index) {
    if (!gamepad_system_initialized || gamepad_index < 0 || gamepad_index >= MAX_GAMEPADS) {
        return "Invalid";
    }
    
    if (gamepads[gamepad_index].connected) {
        return "Xbox Controller (Stub)";
    }
    
    return "Not Connected";
}
