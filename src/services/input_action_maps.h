#ifndef INPUT_ACTION_MAPS_H
#define INPUT_ACTION_MAPS_H

#include <stdbool.h>
#include <stdint.h>

// Forward declare the InputService to avoid circular dependency
struct InputService;

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
    
    // Flight controls
    INPUT_ACTION_THRUST_FORWARD,
    INPUT_ACTION_THRUST_BACK,
    INPUT_ACTION_PITCH_UP,
    INPUT_ACTION_PITCH_DOWN,
    INPUT_ACTION_YAW_LEFT,
    INPUT_ACTION_YAW_RIGHT,
    INPUT_ACTION_ROLL_LEFT,
    INPUT_ACTION_ROLL_RIGHT,

    // Control actions
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_PRIMARY,
    INPUT_ACTION_SECONDARY,
    
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

bool action_maps_load_and_apply(struct InputService* service, const char* file_path);

#endif // INPUT_ACTION_MAPS_H
