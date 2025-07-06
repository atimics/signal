/**
 * @file input_hal.h
 * @brief Hardware Abstraction Layer for Input
 * 
 * This layer abstracts platform-specific input handling,
 * allowing the game to work with different input backends.
 */

#ifndef INPUT_HAL_H
#define INPUT_HAL_H

#include <stdint.h>
#include <stdbool.h>

// Input device types
typedef enum {
    INPUT_DEVICE_KEYBOARD,
    INPUT_DEVICE_MOUSE,
    INPUT_DEVICE_GAMEPAD,
    INPUT_DEVICE_TOUCH
} InputDeviceType;

// Hardware input event structure
typedef struct {
    uint32_t timestamp;
    InputDeviceType device;
    union {
        struct {
            uint32_t key;
            bool pressed;
            uint32_t modifiers;  // Shift, Ctrl, Alt, etc.
        } keyboard;
        
        struct {
            float x, y;          // Absolute position
            float dx, dy;        // Relative movement
            uint8_t buttons;     // Bitmask of pressed buttons
            float wheel_dx, wheel_dy;
        } mouse;
        
        struct {
            uint8_t id;          // Gamepad ID
            float axes[6];       // Sticks and triggers
            uint32_t buttons;    // Button bitmask
        } gamepad;
        
        struct {
            uint8_t id;          // Touch ID for multi-touch
            float x, y;
            float pressure;
            bool pressed;
        } touch;
    } data;
} HardwareInputEvent;

// HAL interface
typedef struct InputHAL {
    // Lifecycle
    bool (*init)(struct InputHAL* self, void* platform_data);
    void (*shutdown)(struct InputHAL* self);
    
    // Event polling
    void (*poll_events)(struct InputHAL* self);
    bool (*get_next_event)(struct InputHAL* self, HardwareInputEvent* event);
    
    // State queries (for compatibility during migration)
    bool (*is_key_pressed)(struct InputHAL* self, uint32_t key);
    void (*get_mouse_position)(struct InputHAL* self, float* x, float* y);
    
    // Platform-specific features
    void (*set_mouse_capture)(struct InputHAL* self, bool captured);
    void (*set_mouse_visible)(struct InputHAL* self, bool visible);
    void (*vibrate_gamepad)(struct InputHAL* self, uint8_t gamepad_id, float left, float right);
    
    // Internal platform data
    void* platform_data;
} InputHAL;

// Factory function for creating platform-specific HAL
InputHAL* input_hal_create_sokol(void);
InputHAL* input_hal_create_mock(void);  // For testing

// Test helper functions for mock HAL
void mock_input_queue_key_event(InputHAL* hal, uint32_t key, bool pressed);
void mock_input_queue_mouse_move(InputHAL* hal, float x, float y);

#endif // INPUT_HAL_H