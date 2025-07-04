// src/scene_input.h
// High-level scene input API built on top of input mapping system

#ifndef SCENE_INPUT_H
#define SCENE_INPUT_H

#include "system/input_mapping.h"
#include <stdbool.h>

// Forward declarations to avoid multiple sokol includes
#ifndef WASM_BUILD
struct sapp_event;
#endif

// Scene-specific action IDs (start at 1000 to avoid conflicts)
typedef enum {
    // Flight scene actions
    SCENE_ACTION_FLIGHT_CIRCUIT = INPUT_ACTION_CUSTOM_BASE,
    SCENE_ACTION_FLIGHT_FIGURE8,
    SCENE_ACTION_FLIGHT_LANDING,
    SCENE_ACTION_FLIGHT_STOP,
    SCENE_ACTION_FLIGHT_PAUSE,
    
    // Derelict scene actions
    SCENE_ACTION_DERELICT_TOGGLE_MAGNET = INPUT_ACTION_CUSTOM_BASE + 100,
    SCENE_ACTION_DERELICT_SCAN,
    
    // Navigation menu actions
    SCENE_ACTION_NAV_SELECT_SYSTEM = INPUT_ACTION_CUSTOM_BASE + 200,
    SCENE_ACTION_NAV_ZOOM_IN,
    SCENE_ACTION_NAV_ZOOM_OUT,
} SceneInputAction;

// Initialize scene input system
void scene_input_init(void);
void scene_input_shutdown(void);

// Scene input profiles
void scene_input_load_flight_profile(void);
void scene_input_load_menu_profile(void);
void scene_input_load_derelict_profile(void);

// Update input (call once per frame)
void scene_input_update(float delta_time);

// Handle events
bool scene_input_handle_event(const struct sapp_event* event);

// High-level movement queries (combines multiple inputs)
typedef struct {
    float forward;      // -1 to 1 (negative = backward)
    float strafe;       // -1 to 1 (negative = left)
    float vertical;     // -1 to 1 (negative = down)
    float pitch;        // -1 to 1 (negative = down)
    float yaw;          // -1 to 1 (negative = left)
    float roll;         // -1 to 1 (negative = left)
    bool boost;
    bool brake;
} MovementInput;

MovementInput scene_input_get_movement(void);

// High-level menu navigation
typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool confirm;
    bool cancel;
    bool back;
} MenuInput;

MenuInput scene_input_get_menu(void);

// Camera control queries
typedef struct {
    bool next_camera;
    bool prev_camera;
    bool toggle_mode;
    float look_x;       // Mouse/right stick X
    float look_y;       // Mouse/right stick Y
} CameraInput;

CameraInput scene_input_get_camera(void);

// Scene-specific actions
bool scene_input_action_pressed(SceneInputAction action);
bool scene_input_action_just_pressed(SceneInputAction action);
bool scene_input_action_just_released(SceneInputAction action);

// Debug overlay
void scene_input_show_debug_overlay(bool show);
bool scene_input_is_debug_visible(void);

#endif // SCENE_INPUT_H