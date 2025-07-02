/**
 * @file engine_test_stubs.c
 * @brief Test stubs for engine functions not needed for UI API testing
 */

#ifdef TEST_MODE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../../src/system/gamepad.h"

// Config stubs
bool config_get_auto_start(void) { return false; }
const char* config_get_startup_scene(void) { return "test_scene"; }
void config_save(void) {}
void config_set_auto_start(bool enabled) { (void)enabled; }
void config_set_startup_scene(const char* scene) { (void)scene; }

// App/window stubs
int sapp_width(void) { return 1920; }
int sapp_height(void) { return 1080; }

// Gamepad stubs for testing
static GamepadState test_gamepad_state = {0};

bool gamepad_init(void) { return true; }
void gamepad_shutdown(void) {}
void gamepad_poll(void) {}
GamepadState* gamepad_get_state(int gamepad_index) { 
    (void)gamepad_index; 
    return &test_gamepad_state; 
}
bool gamepad_is_connected(int gamepad_index) { 
    (void)gamepad_index; 
    return false; 
}
bool gamepad_button_pressed(int gamepad_index, GamepadButton button) { 
    (void)gamepad_index; 
    (void)button; 
    return false; 
}
bool gamepad_button_just_pressed(int gamepad_index, GamepadButton button) { 
    (void)gamepad_index; 
    (void)button; 
    return false; 
}
bool gamepad_button_just_released(int gamepad_index, GamepadButton button) { 
    (void)gamepad_index; 
    (void)button; 
    return false; 
}
float gamepad_get_axis(int gamepad_index, const char* axis_name) { 
    (void)gamepad_index; 
    (void)axis_name; 
    return 0.0f; 
}
const char* gamepad_get_button_name(GamepadButton button) { 
    (void)button; 
    return "test_button"; 
}
void gamepad_set_deadzone(float deadzone) { 
    (void)deadzone; 
}

// System scheduler stub (minimal)
typedef struct {
    int frame_count;
    float total_time;
} SystemScheduler;

#endif // TEST_MODE
