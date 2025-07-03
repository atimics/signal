// src/system/input_mapping.h
// Configurable input mapping system for keyboard and gamepad

#ifndef INPUT_MAPPING_H
#define INPUT_MAPPING_H

#include "../sokol_app.h"
#include <stdbool.h>
#include <stdint.h>

// Maximum number of actions and bindings
#define MAX_INPUT_ACTIONS 64
#define MAX_BINDINGS_PER_ACTION 4
#define MAX_INPUT_CONTEXTS 8

// Input action identifier
typedef uint32_t InputAction;

// Pre-defined common actions
enum CommonInputActions {
    INPUT_ACTION_NONE = 0,
    
    // Movement
    INPUT_ACTION_MOVE_FORWARD,
    INPUT_ACTION_MOVE_BACKWARD,
    INPUT_ACTION_MOVE_LEFT,
    INPUT_ACTION_MOVE_RIGHT,
    INPUT_ACTION_MOVE_UP,
    INPUT_ACTION_MOVE_DOWN,
    
    // Rotation
    INPUT_ACTION_PITCH_UP,
    INPUT_ACTION_PITCH_DOWN,
    INPUT_ACTION_YAW_LEFT,
    INPUT_ACTION_YAW_RIGHT,
    INPUT_ACTION_ROLL_LEFT,
    INPUT_ACTION_ROLL_RIGHT,
    
    // Actions
    INPUT_ACTION_PRIMARY,      // Primary action (fire, confirm, etc)
    INPUT_ACTION_SECONDARY,    // Secondary action (alt fire, cancel, etc)
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_JUMP,
    INPUT_ACTION_CROUCH,
    
    // UI/Menu
    INPUT_ACTION_MENU,
    INPUT_ACTION_PAUSE,
    INPUT_ACTION_CONFIRM,
    INPUT_ACTION_CANCEL,
    INPUT_ACTION_NAV_UP,
    INPUT_ACTION_NAV_DOWN,
    INPUT_ACTION_NAV_LEFT,
    INPUT_ACTION_NAV_RIGHT,
    
    // Camera
    INPUT_ACTION_CAMERA_NEXT,
    INPUT_ACTION_CAMERA_PREV,
    INPUT_ACTION_CAMERA_MODE,
    
    // Special/Debug
    INPUT_ACTION_DEBUG_TOGGLE,
    INPUT_ACTION_SCREENSHOT,
    INPUT_ACTION_QUICKSAVE,
    INPUT_ACTION_QUICKLOAD,
    
    // Scene-specific actions start at 1000
    INPUT_ACTION_CUSTOM_BASE = 1000
};

// Input source types
typedef enum {
    INPUT_SOURCE_KEYBOARD,
    INPUT_SOURCE_GAMEPAD_BUTTON,
    INPUT_SOURCE_GAMEPAD_AXIS,
    INPUT_SOURCE_MOUSE_BUTTON,
    INPUT_SOURCE_MOUSE_AXIS
} InputSourceType;

// Gamepad axis identifiers
typedef enum {
    GAMEPAD_AXIS_LEFT_X = 0,
    GAMEPAD_AXIS_LEFT_Y,
    GAMEPAD_AXIS_RIGHT_X,
    GAMEPAD_AXIS_RIGHT_Y,
    GAMEPAD_AXIS_LEFT_TRIGGER,
    GAMEPAD_AXIS_RIGHT_TRIGGER,
    GAMEPAD_AXIS_COUNT
} GamepadAxis;

// Gamepad button identifiers (Xbox layout)
typedef enum {
    GAMEPAD_BUTTON_A = 0,
    GAMEPAD_BUTTON_B,
    GAMEPAD_BUTTON_X,
    GAMEPAD_BUTTON_Y,
    GAMEPAD_BUTTON_LEFT_BUMPER,
    GAMEPAD_BUTTON_RIGHT_BUMPER,
    GAMEPAD_BUTTON_BACK,
    GAMEPAD_BUTTON_START,
    GAMEPAD_BUTTON_LEFT_STICK,
    GAMEPAD_BUTTON_RIGHT_STICK,
    GAMEPAD_BUTTON_DPAD_UP,
    GAMEPAD_BUTTON_DPAD_DOWN,
    GAMEPAD_BUTTON_DPAD_LEFT,
    GAMEPAD_BUTTON_DPAD_RIGHT,
    GAMEPAD_BUTTON_GUIDE,
    GAMEPAD_BUTTON_COUNT
} GamepadButton;

// Input binding - maps a physical input to an action
typedef struct {
    InputSourceType source_type;
    union {
        sapp_keycode key_code;          // For keyboard
        GamepadButton gamepad_button;   // For gamepad buttons
        GamepadAxis gamepad_axis;       // For gamepad axes
        uint8_t mouse_button;           // For mouse buttons
    } source;
    
    // For axis inputs
    float axis_threshold;   // Threshold for digital conversion (0.5 default)
    bool axis_positive;     // True for positive direction, false for negative
    
    // Modifiers
    bool require_shift;
    bool require_ctrl;
    bool require_alt;
} InputBinding;

// Input action definition
typedef struct {
    char name[64];                                      // Human-readable name
    InputBinding bindings[MAX_BINDINGS_PER_ACTION];    // Multiple bindings per action
    uint8_t binding_count;
    bool is_axis;                                       // True for analog inputs
    float axis_value;                                   // Current axis value (-1 to 1)
    bool is_pressed;                                    // Current pressed state
    bool was_pressed;                                   // Previous frame pressed state
    float hold_time;                                    // How long the action has been held
} InputActionDef;

// Input context - allows different input mappings for different game states
typedef struct {
    char name[64];
    InputAction actions[MAX_INPUT_ACTIONS];
    uint32_t action_count;
    bool active;
} InputContext;

// Input mapping state
typedef struct {
    InputActionDef actions[MAX_INPUT_ACTIONS];
    InputContext contexts[MAX_INPUT_CONTEXTS];
    uint32_t context_count;
    uint32_t active_context;
    
    // Current input state
    bool keyboard_state[SAPP_MAX_KEYCODES];
    bool gamepad_buttons[GAMEPAD_BUTTON_COUNT];
    float gamepad_axes[GAMEPAD_AXIS_COUNT];
    bool mouse_buttons[3];
    float mouse_delta_x;
    float mouse_delta_y;
    
    // Gamepad info
    bool gamepad_connected;
    uint32_t gamepad_id;
} InputMapping;

// Initialize input mapping system
void input_mapping_init(void);
void input_mapping_shutdown(void);

// Context management
uint32_t input_mapping_create_context(const char* name);
void input_mapping_activate_context(uint32_t context_id);
void input_mapping_deactivate_context(uint32_t context_id);

// Action registration
InputAction input_mapping_register_action(const char* name, bool is_axis);
void input_mapping_bind_key(InputAction action, sapp_keycode key, bool shift, bool ctrl, bool alt);
void input_mapping_bind_gamepad_button(InputAction action, GamepadButton button);
void input_mapping_bind_gamepad_axis(InputAction action, GamepadAxis axis, bool positive, float threshold);
void input_mapping_bind_mouse_button(InputAction action, uint8_t button);
void input_mapping_clear_bindings(InputAction action);

// Update - call once per frame
void input_mapping_update(float delta_time);

// Event handling
bool input_mapping_handle_event(const sapp_event* event);

// Query actions
bool input_mapping_is_pressed(InputAction action);
bool input_mapping_just_pressed(InputAction action);
bool input_mapping_just_released(InputAction action);
float input_mapping_get_axis(InputAction action);
float input_mapping_get_hold_time(InputAction action);

// Load/save mappings
bool input_mapping_load_from_file(const char* filename);
bool input_mapping_save_to_file(const char* filename);

// Default configurations
void input_mapping_load_default_flight_controls(void);
void input_mapping_load_default_menu_controls(void);
void input_mapping_load_default_fps_controls(void);

// Debug
void input_mapping_debug_print(void);

#endif // INPUT_MAPPING_H