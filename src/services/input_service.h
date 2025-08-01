/**
 * @file input_service.h
 * @brief High-level input service that provides action mapping and contexts
 */

#ifndef INPUT_SERVICE_H
#define INPUT_SERVICE_H

#include "../hal/input_hal.h"
#include <stdbool.h>

// Input actions (game-specific)
typedef enum {
    INPUT_ACTION_NONE = 0,
    
    // UI actions
    INPUT_ACTION_UI_UP,
    INPUT_ACTION_UI_DOWN,
    INPUT_ACTION_UI_LEFT,
    INPUT_ACTION_UI_RIGHT,
    INPUT_ACTION_UI_CONFIRM,
    INPUT_ACTION_UI_CANCEL,
    INPUT_ACTION_UI_MENU,
    
    // Movement actions
    INPUT_ACTION_MOVE_FORWARD,
    INPUT_ACTION_MOVE_BACKWARD,
    INPUT_ACTION_MOVE_LEFT,
    INPUT_ACTION_MOVE_RIGHT,
    INPUT_ACTION_MOVE_UP,
    INPUT_ACTION_MOVE_DOWN,
    
    // Control actions
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_PRIMARY,
    INPUT_ACTION_SECONDARY,
    
    // Flight control actions (Sprint 25)
    INPUT_ACTION_PITCH_UP,
    INPUT_ACTION_PITCH_DOWN,
    INPUT_ACTION_YAW_LEFT,
    INPUT_ACTION_YAW_RIGHT,
    INPUT_ACTION_ROLL_LEFT,
    INPUT_ACTION_ROLL_RIGHT,
    INPUT_ACTION_THRUST_FORWARD,
    INPUT_ACTION_THRUST_BACK,
    INPUT_ACTION_VERTICAL_UP,
    INPUT_ACTION_VERTICAL_DOWN,
    
    // Camera actions
    INPUT_ACTION_CAMERA_PITCH,
    INPUT_ACTION_CAMERA_YAW,
    INPUT_ACTION_CAMERA_ZOOM,
    
    // System actions
    INPUT_ACTION_DEBUG_TOGGLE,
    INPUT_ACTION_SCREENSHOT,
    INPUT_ACTION_QUIT,
    
    INPUT_ACTION_COUNT
} InputActionID;

// Input contexts
typedef enum {
    INPUT_CONTEXT_MENU,
    INPUT_CONTEXT_GAMEPLAY,
    INPUT_CONTEXT_DIALOG,
    INPUT_CONTEXT_DEBUG,
    INPUT_CONTEXT_COUNT
} InputContextID;

// High-level input event
typedef struct {
    InputActionID action;
    float value;          // Analog value (-1 to 1 for axes, 0 or 1 for buttons)
    bool just_pressed;
    bool just_released;
    uint32_t timestamp;
} InputEvent;

// Input binding
typedef struct {
    InputDeviceType device;
    union {
        struct {
            uint32_t key;
            uint32_t modifiers;
        } keyboard;
        struct {
            uint8_t button;
        } mouse;
        struct {
            uint8_t button;
            uint8_t axis;
            uint8_t gamepad_id;
            bool is_axis;  // true for axis, false for button
        } gamepad;
    } binding;
    float scale;      // For analog scaling
    bool invert;      // For axis inversion
} InputBinding;

// Service configuration
typedef struct {
    InputHAL* hal;
    const char* bindings_path;
    bool enable_input_logging;
} InputServiceConfig;

// Input service interface
typedef struct InputService {
    // Lifecycle
    bool (*init)(struct InputService* self, const InputServiceConfig* config);
    void (*shutdown)(struct InputService* self);
    
    // Context management
    void (*push_context)(struct InputService* self, InputContextID context);
    void (*pop_context)(struct InputService* self);
    InputContextID (*get_active_context)(struct InputService* self);
    
    // Event processing
    void (*process_frame)(struct InputService* self, float delta_time);
    bool (*get_next_event)(struct InputService* self, InputEvent* event);
    
    // State queries (for compatibility)
    float (*get_action_value)(struct InputService* self, InputActionID action);
    bool (*is_action_pressed)(struct InputService* self, InputActionID action);
    bool (*is_action_just_pressed)(struct InputService* self, InputActionID action);
    bool (*is_action_just_released)(struct InputService* self, InputActionID action);
    
    // Binding management
    void (*bind_action)(struct InputService* self, InputActionID action, InputContextID context, const InputBinding* binding);
    void (*clear_action_bindings)(struct InputService* self, InputActionID action, InputContextID context);
    void (*save_bindings)(struct InputService* self, const char* profile_name);
    void (*load_bindings)(struct InputService* self, const char* profile_name);
    
    // Internal data
    void* internal;
} InputService;

// Factory function
InputService* input_service_create(void);
void input_service_destroy(InputService* service);

// Default bindings setup
void input_service_setup_default_bindings(InputService* service);

#endif // INPUT_SERVICE_H