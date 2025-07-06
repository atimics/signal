// src/scene_input.c
// Implementation of high-level scene input API

#include "scene_input.h"
#include <stdio.h>

// Forward declarations to avoid including sokol_app.h
#define SAPP_KEYCODE_W 87
#define SAPP_KEYCODE_A 65
#define SAPP_KEYCODE_S 83
#define SAPP_KEYCODE_D 68
#define SAPP_KEYCODE_Q 81
#define SAPP_KEYCODE_E 69
#define SAPP_KEYCODE_UP 262
#define SAPP_KEYCODE_DOWN 263
#define SAPP_KEYCODE_LEFT 260
#define SAPP_KEYCODE_RIGHT 261
#define SAPP_KEYCODE_SPACE 32
#define SAPP_KEYCODE_ENTER 257
#define SAPP_KEYCODE_ESCAPE 256
#define SAPP_KEYCODE_TAB 258
#define SAPP_KEYCODE_C 67
#define SAPP_KEYCODE_R 82
#define SAPP_KEYCODE_F 70
#define SAPP_KEYCODE_X 88
#define SAPP_KEYCODE_P 80
#define SAPP_KEYCODE_0 48
#define SAPP_KEYCODE_1 49
#define SAPP_KEYCODE_2 50
#define SAPP_KEYCODE_3 51

// Event structure forward declaration
struct sapp_event;

static bool g_debug_overlay = false;

// Initialize scene input system
void scene_input_init(void) {
    // Initialize the underlying input mapping system
    input_mapping_init();
    
    // Register common actions
    input_mapping_register_action("forward", true);
    input_mapping_register_action("backward", true);
    input_mapping_register_action("strafe_left", true);
    input_mapping_register_action("strafe_right", true);
    input_mapping_register_action("vertical_up", true);
    input_mapping_register_action("vertical_down", true);
    input_mapping_register_action("pitch", true);
    input_mapping_register_action("yaw", true);
    input_mapping_register_action("roll", true);
    
    printf("✅ Scene input system initialized\n");
}

void scene_input_shutdown(void) {
    input_mapping_shutdown();
    printf("✅ Scene input system shut down\n");
}

// Load flight scene profile
void scene_input_load_flight_profile(void) {
    // Load default flight controls
    input_mapping_load_default_flight_controls();
    
    // Register scene-specific actions
    InputAction circuit = input_mapping_register_action("flight_circuit", false);
    input_mapping_bind_key(circuit, SAPP_KEYCODE_1, false, false, false);
    
    InputAction figure8 = input_mapping_register_action("flight_figure8", false);
    input_mapping_bind_key(figure8, SAPP_KEYCODE_2, false, false, false);
    
    InputAction landing = input_mapping_register_action("flight_landing", false);
    input_mapping_bind_key(landing, SAPP_KEYCODE_3, false, false, false);
    
    InputAction stop = input_mapping_register_action("flight_stop", false);
    input_mapping_bind_key(stop, SAPP_KEYCODE_0, false, false, false);
    
    InputAction pause = input_mapping_register_action("flight_pause", false);
    input_mapping_bind_key(pause, SAPP_KEYCODE_P, false, false, false);
    
    // Movement axes
    InputAction forward = input_mapping_register_action("forward", true);
    input_mapping_bind_key(forward, SAPP_KEYCODE_W, false, false, false);
    input_mapping_bind_key(forward, SAPP_KEYCODE_SPACE, false, false, false);
    input_mapping_bind_gamepad_axis(forward, GAMEPAD_AXIS_RIGHT_TRIGGER, true, 0.1f);
    
    InputAction backward = input_mapping_register_action("backward", true);
    input_mapping_bind_key(backward, SAPP_KEYCODE_S, false, false, false);
    input_mapping_bind_key(backward, SAPP_KEYCODE_X, false, false, false);
    input_mapping_bind_gamepad_axis(backward, GAMEPAD_AXIS_LEFT_TRIGGER, true, 0.1f);
    
    InputAction strafe_left = input_mapping_register_action("strafe_left", true);
    input_mapping_bind_key(strafe_left, SAPP_KEYCODE_A, false, false, false);
    
    InputAction strafe_right = input_mapping_register_action("strafe_right", true);
    input_mapping_bind_key(strafe_right, SAPP_KEYCODE_D, false, false, false);
    
    InputAction vertical_up = input_mapping_register_action("vertical_up", true);
    input_mapping_bind_key(vertical_up, SAPP_KEYCODE_R, false, false, false);
    input_mapping_bind_gamepad_axis(vertical_up, GAMEPAD_AXIS_RIGHT_Y, false, 0.1f);
    
    InputAction vertical_down = input_mapping_register_action("vertical_down", true);
    input_mapping_bind_key(vertical_down, SAPP_KEYCODE_F, false, false, false);
    input_mapping_bind_gamepad_axis(vertical_down, GAMEPAD_AXIS_RIGHT_Y, true, 0.1f);
    
    // Already registered in default flight controls
    printf("✅ Loaded flight scene input profile\n");
}

// Load menu profile
void scene_input_load_menu_profile(void) {
    input_mapping_load_default_menu_controls();
    printf("✅ Loaded menu input profile\n");
}

// Load derelict profile
void scene_input_load_derelict_profile(void) {
    // Load flight controls as base
    scene_input_load_flight_profile();
    
    // Add derelict-specific actions
    InputAction toggle_magnet = input_mapping_register_action("toggle_magnet", false);
    input_mapping_bind_key(toggle_magnet, SAPP_KEYCODE_SPACE, false, false, false);
    input_mapping_bind_gamepad_button(toggle_magnet, GAMEPAD_BUTTON_X);
    
    InputAction scan = input_mapping_register_action("scan", false);
    input_mapping_bind_key(scan, SAPP_KEYCODE_E, false, false, false);
    input_mapping_bind_gamepad_button(scan, GAMEPAD_BUTTON_Y);
    
    printf("✅ Loaded derelict scene input profile\n");
}

// Update input
void scene_input_update(float delta_time) {
    input_mapping_update(delta_time);
    
    // Show debug overlay if enabled
    if (g_debug_overlay && input_mapping_just_pressed(INPUT_ACTION_DEBUG_TOGGLE)) {
        input_mapping_debug_print();
    }
}

// Handle events
bool scene_input_handle_event(const struct sapp_event* event) {
    return input_mapping_handle_event(event);
}

// Get movement input
MovementInput scene_input_get_movement(void) {
    MovementInput input = {0};
    
    // Forward/backward (combine multiple inputs)
    float forward = input_mapping_get_axis(input_mapping_register_action("forward", true));
    float backward = input_mapping_get_axis(input_mapping_register_action("backward", true));
    input.forward = forward - backward;
    
    // Strafe
    float strafe_left = input_mapping_get_axis(input_mapping_register_action("strafe_left", true));
    float strafe_right = input_mapping_get_axis(input_mapping_register_action("strafe_right", true));
    input.strafe = strafe_right - strafe_left;
    
    // Vertical
    float vertical_up = input_mapping_get_axis(input_mapping_register_action("vertical_up", true));
    float vertical_down = input_mapping_get_axis(input_mapping_register_action("vertical_down", true));
    input.vertical = vertical_up - vertical_down;
    
    // Rotation
    input.pitch = input_mapping_get_axis(input_mapping_register_action("pitch", true));
    input.yaw = input_mapping_get_axis(input_mapping_register_action("yaw", true));
    input.roll = input_mapping_get_axis(input_mapping_register_action("roll", true));
    
    // Actions
    input.boost = input_mapping_is_pressed(INPUT_ACTION_BOOST);
    input.brake = input_mapping_is_pressed(INPUT_ACTION_BRAKE);
    
    return input;
}

// Get menu input
MenuInput scene_input_get_menu(void) {
    MenuInput input = {0};
    
    input.up = input_mapping_just_pressed(INPUT_ACTION_NAV_UP);
    input.down = input_mapping_just_pressed(INPUT_ACTION_NAV_DOWN);
    input.left = input_mapping_just_pressed(INPUT_ACTION_NAV_LEFT);
    input.right = input_mapping_just_pressed(INPUT_ACTION_NAV_RIGHT);
    input.confirm = input_mapping_just_pressed(INPUT_ACTION_CONFIRM);
    input.cancel = input_mapping_just_pressed(INPUT_ACTION_CANCEL);
    input.back = input_mapping_just_pressed(INPUT_ACTION_MENU);
    
    return input;
}

// Get camera input
CameraInput scene_input_get_camera(void) {
    CameraInput input = {0};
    
    input.next_camera = input_mapping_just_pressed(INPUT_ACTION_CAMERA_NEXT);
    input.prev_camera = input_mapping_just_pressed(INPUT_ACTION_CAMERA_PREV);
    input.toggle_mode = input_mapping_just_pressed(INPUT_ACTION_CAMERA_MODE);
    
    // Mouse/right stick look would be handled here
    // For now, returning zeros
    input.look_x = 0.0f;
    input.look_y = 0.0f;
    
    return input;
}

// Scene-specific actions
bool scene_input_action_pressed(SceneInputAction action) {
    switch (action) {
        case SCENE_ACTION_FLIGHT_CIRCUIT:
            return input_mapping_is_pressed(input_mapping_register_action("flight_circuit", false));
        case SCENE_ACTION_FLIGHT_FIGURE8:
            return input_mapping_is_pressed(input_mapping_register_action("flight_figure8", false));
        case SCENE_ACTION_FLIGHT_LANDING:
            return input_mapping_is_pressed(input_mapping_register_action("flight_landing", false));
        case SCENE_ACTION_FLIGHT_STOP:
            return input_mapping_is_pressed(input_mapping_register_action("flight_stop", false));
        case SCENE_ACTION_FLIGHT_PAUSE:
            return input_mapping_is_pressed(input_mapping_register_action("flight_pause", false));
        case SCENE_ACTION_DERELICT_TOGGLE_MAGNET:
            return input_mapping_is_pressed(input_mapping_register_action("toggle_magnet", false));
        case SCENE_ACTION_DERELICT_SCAN:
            return input_mapping_is_pressed(input_mapping_register_action("scan", false));
        default:
            return false;
    }
}

bool scene_input_action_just_pressed(SceneInputAction action) {
    switch (action) {
        case SCENE_ACTION_FLIGHT_CIRCUIT:
            return input_mapping_just_pressed(input_mapping_register_action("flight_circuit", false));
        case SCENE_ACTION_FLIGHT_FIGURE8:
            return input_mapping_just_pressed(input_mapping_register_action("flight_figure8", false));
        case SCENE_ACTION_FLIGHT_LANDING:
            return input_mapping_just_pressed(input_mapping_register_action("flight_landing", false));
        case SCENE_ACTION_FLIGHT_STOP:
            return input_mapping_just_pressed(input_mapping_register_action("flight_stop", false));
        case SCENE_ACTION_FLIGHT_PAUSE:
            return input_mapping_just_pressed(input_mapping_register_action("flight_pause", false));
        case SCENE_ACTION_DERELICT_TOGGLE_MAGNET:
            return input_mapping_just_pressed(input_mapping_register_action("toggle_magnet", false));
        case SCENE_ACTION_DERELICT_SCAN:
            return input_mapping_just_pressed(input_mapping_register_action("scan", false));
        default:
            return false;
    }
}

bool scene_input_action_just_released(SceneInputAction action) {
    switch (action) {
        case SCENE_ACTION_FLIGHT_CIRCUIT:
            return input_mapping_just_released(input_mapping_register_action("flight_circuit", false));
        case SCENE_ACTION_FLIGHT_FIGURE8:
            return input_mapping_just_released(input_mapping_register_action("flight_figure8", false));
        case SCENE_ACTION_FLIGHT_LANDING:
            return input_mapping_just_released(input_mapping_register_action("flight_landing", false));
        case SCENE_ACTION_FLIGHT_STOP:
            return input_mapping_just_released(input_mapping_register_action("flight_stop", false));
        case SCENE_ACTION_FLIGHT_PAUSE:
            return input_mapping_just_released(input_mapping_register_action("flight_pause", false));
        case SCENE_ACTION_DERELICT_TOGGLE_MAGNET:
            return input_mapping_just_released(input_mapping_register_action("toggle_magnet", false));
        case SCENE_ACTION_DERELICT_SCAN:
            return input_mapping_just_released(input_mapping_register_action("scan", false));
        default:
            return false;
    }
}

// Debug overlay
void scene_input_show_debug_overlay(bool show) {
    g_debug_overlay = show;
}

bool scene_input_is_debug_visible(void) {
    return g_debug_overlay;
}