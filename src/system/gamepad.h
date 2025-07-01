// src/system/gamepad.h
// Cross-platform gamepad input system using hidapi
// Designed to align with Sokol's minimal, pure-C philosophy

#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of gamepads supported
#define MAX_GAMEPADS 4

// Gamepad state structure - Xbox controller layout
typedef struct {
    bool connected;
    
    // Analog sticks (-1.0 to 1.0)
    float left_stick_x;
    float left_stick_y;
    float right_stick_x;
    float right_stick_y;
    
    // Triggers (0.0 to 1.0)
    float left_trigger;
    float right_trigger;
    
    // Face buttons
    bool button_a;          // Xbox A / PS Cross
    bool button_b;          // Xbox B / PS Circle  
    bool button_x;          // Xbox X / PS Square
    bool button_y;          // Xbox Y / PS Triangle
    
    // Shoulder buttons
    bool left_bumper;       // LB / L1
    bool right_bumper;      // RB / R1
    
    // D-pad
    bool dpad_up;
    bool dpad_down;
    bool dpad_left;
    bool dpad_right;
    
    // System buttons
    bool start;             // Menu / Options
    bool back;              // View / Share
    bool left_stick_click;  // LS / L3
    bool right_stick_click; // RS / R3
    
    // Xbox guide button (if available)
    bool guide;
    
} GamepadState;

// Initialize gamepad system
// Must be called before any other gamepad functions
bool gamepad_init(void);

// Shutdown gamepad system
void gamepad_shutdown(void);

// Poll all connected gamepads
// Call this once per frame to update states
void gamepad_poll(void);

// Get the current state of a gamepad (0-3)
// Returns a gamepad state struct. Check .connected to see if valid.
GamepadState gamepad_get_state(int gamepad_index);

// Get number of connected gamepads
int gamepad_get_count(void);

// Check if a specific gamepad is connected
bool gamepad_is_connected(int gamepad_index);

// Get a human-readable name for debugging
const char* gamepad_get_name(int gamepad_index);

#ifdef __cplusplus
}
#endif

#endif // GAMEPAD_H
