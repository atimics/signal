/**
 * @file input_state.h
 * @brief Shared input state structure for control systems
 */

#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <stdbool.h>

// Input state structure for movement controls
typedef struct InputState {
    // Movement axes
    float thrust;       // Forward/backward
    float vertical;     // Up/down movement
    float pitch;        // Pitch rotation
    float yaw;          // Yaw rotation  
    float roll;         // Roll rotation
    float strafe_left;  // Strafe left
    float strafe_right; // Strafe right
    
    // Action buttons
    float boost;          // Boost multiplier
    bool boost_pressed;   // Boost just pressed this frame
    bool brake;           // Brake active
    bool brake_pressed;   // Brake just pressed this frame
    float brake_intensity;// Brake intensity (0-1)
    
    // UI/System controls
    bool menu;          // Menu/pause
    bool confirm;       // Confirm/select
    bool cancel;        // Cancel/back
    
    // Camera controls
    bool camera_next;   // Next camera mode
    bool camera_prev;   // Previous camera mode
} InputState;

// Get current input state from the new input service
const InputState* input_get_state(void);

// Debug functions (Sprint 25)
void input_set_debug_enabled(bool enabled);
bool input_is_debug_enabled(void);

#endif // INPUT_STATE_H