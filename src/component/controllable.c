// src/component/controllable.c
// Implementation of controllable component

#include "controllable.h"
#include "../game_input.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Create controllable component
Controllable* controllable_create(void) {
    Controllable* controllable = (Controllable*)malloc(sizeof(Controllable));
    if (controllable) {
        controllable_reset(controllable);
    }
    return controllable;
}

// Destroy controllable component
void controllable_destroy(Controllable* controllable) {
    if (controllable) {
        free(controllable);
    }
}

// Reset to defaults
void controllable_reset(Controllable* controllable) {
    if (!controllable) return;
    
    memset(controllable, 0, sizeof(Controllable));
    
    // Default configuration
    controllable->enabled = true;
    controllable->player_id = 0;
    controllable->scheme_type = INPUT_SCHEME_CUSTOM;
    
    // Default force scales
    controllable->linear_force_scale = 1.0f;
    controllable->angular_force_scale = 1.0f;
    controllable->boost_multiplier = 2.0f;
    controllable->brake_multiplier = 0.1f;
    
    // Default axis configs
    controllable->forward_axis.sensitivity = 1.0f;
    controllable->forward_axis.dead_zone = 0.1f;
    controllable->forward_axis.curve = RESPONSE_LINEAR;
    controllable->forward_axis.max_value = 1.0f;
    
    controllable->strafe_axis = controllable->forward_axis;
    controllable->vertical_axis = controllable->forward_axis;
    controllable->pitch_axis = controllable->forward_axis;
    controllable->yaw_axis = controllable->forward_axis;
    controllable->roll_axis = controllable->forward_axis;
    
    // No smoothing by default
    controllable->input_smoothing = 0.0f;
}

// Set input scheme
void controllable_set_scheme(Controllable* controllable, InputSchemeType scheme) {
    if (!controllable) return;
    
    controllable->scheme_type = scheme;
    
    // Apply preset based on scheme
    switch (scheme) {
        case INPUT_SCHEME_SPACESHIP_6DOF:
            controllable_setup_spaceship_6dof(controllable);
            break;
        case INPUT_SCHEME_SPACESHIP_ARCADE:
            controllable_setup_spaceship_arcade(controllable);
            break;
        case INPUT_SCHEME_FPS_CAMERA:
            controllable_setup_fps_camera(controllable);
            break;
        case INPUT_SCHEME_ORBIT_CAMERA:
            controllable_setup_orbit_camera(controllable);
            break;
        default:
            break;
    }
}

// Configure specific axis
void controllable_configure_axis(Controllable* controllable, const char* axis_name, 
                               InputAction action, float sensitivity, bool inverted) {
    if (!controllable) return;
    
    AxisConfig* axis = NULL;
    
    if (strcmp(axis_name, "forward") == 0) {
        axis = &controllable->forward_axis;
    } else if (strcmp(axis_name, "strafe") == 0) {
        axis = &controllable->strafe_axis;
    } else if (strcmp(axis_name, "vertical") == 0) {
        axis = &controllable->vertical_axis;
    } else if (strcmp(axis_name, "pitch") == 0) {
        axis = &controllable->pitch_axis;
    } else if (strcmp(axis_name, "yaw") == 0) {
        axis = &controllable->yaw_axis;
    } else if (strcmp(axis_name, "roll") == 0) {
        axis = &controllable->roll_axis;
    }
    
    if (axis) {
        axis->action = action;
        axis->sensitivity = sensitivity;
        axis->inverted = inverted;
    }
}

// Set force scales
void controllable_set_force_scales(Controllable* controllable, float linear, float angular) {
    if (!controllable) return;
    controllable->linear_force_scale = linear;
    controllable->angular_force_scale = angular;
}

// Apply response curve
float controllable_apply_response_curve(float input, ResponseCurveType curve) {
    float sign = input < 0 ? -1.0f : 1.0f;
    float abs_input = fabsf(input);
    
    switch (curve) {
        case RESPONSE_LINEAR:
            return input;
            
        case RESPONSE_QUADRATIC:
            return sign * abs_input * abs_input;
            
        case RESPONSE_CUBIC:
            return input * abs_input * abs_input;
            
        case RESPONSE_EXPONENTIAL:
            return sign * (expf(abs_input) - 1.0f) / (expf(1.0f) - 1.0f);
            
        default:
            return input;
    }
}

// Apply dead zone
float controllable_apply_dead_zone(float input, float dead_zone) {
    if (fabsf(input) < dead_zone) {
        return 0.0f;
    }
    
    // Rescale to maintain full range outside dead zone
    float sign = input < 0 ? -1.0f : 1.0f;
    float abs_input = fabsf(input);
    return sign * (abs_input - dead_zone) / (1.0f - dead_zone);
}

// Process axis input
static float process_axis_input(const AxisConfig* axis) {
    if (axis->action == INPUT_ACTION_NONE) {
        return 0.0f;
    }
    
    struct InputService* service = game_input_get_service();
    float value = service->get_action_value(service, axis->action);
    
    // Apply dead zone
    value = controllable_apply_dead_zone(value, axis->dead_zone);
    
    // Apply response curve
    value = controllable_apply_response_curve(value, axis->curve);
    
    // Apply sensitivity and inversion
    value *= axis->sensitivity;
    if (axis->inverted) {
        value = -value;
    }
    
    // Clamp to max value
    return fmaxf(-axis->max_value, fminf(axis->max_value, value));
}

// Update input
void controllable_update_input(Controllable* controllable, float delta_time) {
    if (!controllable || !controllable->enabled) return;
    
    // Save previous input
    controllable->previous_input = controllable->current_input;
    
    // Process axes
    InputForceMapping new_input = {0};
    
    // Linear axes
    new_input.forward_backward = process_axis_input(&controllable->forward_axis);
    new_input.left_right = process_axis_input(&controllable->strafe_axis);
    new_input.up_down = process_axis_input(&controllable->vertical_axis);
    
    // Angular axes
    new_input.pitch = process_axis_input(&controllable->pitch_axis);
    new_input.yaw = process_axis_input(&controllable->yaw_axis);
    new_input.roll = process_axis_input(&controllable->roll_axis);
    
    // Actions
    if (controllable->boost_action != INPUT_ACTION_NONE) {
        struct InputService* service = game_input_get_service();
        new_input.boost = service->is_action_pressed(service, controllable->boost_action) ? 1.0f : 0.0f;
    }
    
    if (controllable->brake_action != INPUT_ACTION_NONE) {
        new_input.brake = service->is_action_pressed(service, controllable->brake_action) ? 1.0f : 0.0f;
    }
    
    if (controllable->stabilize_action != INPUT_ACTION_NONE) {
        new_input.stabilize = service->is_action_pressed(service, controllable->stabilize_action);
    }
    
    // Apply smoothing if enabled
    if (controllable->input_smoothing > 0.0f) {
        float smooth = controllable->input_smoothing;
        controllable->current_input.forward_backward = 
            controllable->current_input.forward_backward * smooth + new_input.forward_backward * (1.0f - smooth);
        controllable->current_input.left_right = 
            controllable->current_input.left_right * smooth + new_input.left_right * (1.0f - smooth);
        controllable->current_input.up_down = 
            controllable->current_input.up_down * smooth + new_input.up_down * (1.0f - smooth);
        controllable->current_input.pitch = 
            controllable->current_input.pitch * smooth + new_input.pitch * (1.0f - smooth);
        controllable->current_input.yaw = 
            controllable->current_input.yaw * smooth + new_input.yaw * (1.0f - smooth);
        controllable->current_input.roll = 
            controllable->current_input.roll * smooth + new_input.roll * (1.0f - smooth);
        
        // Don't smooth boolean/instant actions
        controllable->current_input.boost = new_input.boost;
        controllable->current_input.brake = new_input.brake;
        controllable->current_input.stabilize = new_input.stabilize;
    } else {
        controllable->current_input = new_input;
    }
    
    // Call custom processing if available
    if (controllable->process_input) {
        controllable->process_input(controllable, delta_time);
    }
}

// Get input forces
InputForceMapping controllable_get_input_forces(const Controllable* controllable) {
    if (!controllable) {
        return (InputForceMapping){0};
    }
    
    InputForceMapping forces = controllable->current_input;
    
    // Apply force scales
    forces.forward_backward *= controllable->linear_force_scale;
    forces.left_right *= controllable->linear_force_scale;
    forces.up_down *= controllable->linear_force_scale;
    
    forces.pitch *= controllable->angular_force_scale;
    forces.yaw *= controllable->angular_force_scale;
    forces.roll *= controllable->angular_force_scale;
    
    // Apply boost
    if (forces.boost > 0.0f) {
        float boost_factor = 1.0f + (controllable->boost_multiplier - 1.0f) * forces.boost;
        forces.forward_backward *= boost_factor;
        forces.left_right *= boost_factor;
        forces.up_down *= boost_factor;
    }
    
    // Apply brake
    if (forces.brake > 0.0f) {
        float brake_factor = 1.0f - (1.0f - controllable->brake_multiplier) * forces.brake;
        forces.forward_backward *= brake_factor;
        forces.left_right *= brake_factor;
        forces.up_down *= brake_factor;
    }
    
    return forces;
}

// Preset: 6DOF Spaceship
void controllable_setup_spaceship_6dof(Controllable* controllable) {
    if (!controllable) return;
    
    // Configure axis actions using InputService enums
    controllable->forward_axis.action = INPUT_ACTION_MOVE_FORWARD;
    controllable->strafe_axis.action = INPUT_ACTION_MOVE_RIGHT; // Will handle negative for left
    controllable->vertical_axis.action = INPUT_ACTION_MOVE_UP; // Will handle negative for down
    
    // Rotation
    controllable->pitch_axis.action = INPUT_ACTION_CAMERA_PITCH;
    controllable->yaw_axis.action = INPUT_ACTION_CAMERA_YAW;
    controllable->roll_axis.action = INPUT_ACTION_NONE; // Roll not available in basic set
    
    // Actions
    controllable->boost_action = INPUT_ACTION_BOOST;
    controllable->brake_action = INPUT_ACTION_BRAKE;
    
    // Configure response curves
    controllable->pitch_axis.curve = RESPONSE_QUADRATIC;
    controllable->yaw_axis.curve = RESPONSE_QUADRATIC;
    controllable->roll_axis.curve = RESPONSE_LINEAR;
    
    // Set sensitivities
    controllable->pitch_axis.sensitivity = 0.8f;
    controllable->yaw_axis.sensitivity = 0.8f;
    controllable->roll_axis.sensitivity = 1.0f;
    
    printf("✅ Configured 6DOF spaceship controls\n");
}

// Preset: Arcade Spaceship
void controllable_setup_spaceship_arcade(Controllable* controllable) {
    if (!controllable) return;
    
    // Simplified controls - forward only, banking turns
    controllable->forward_axis.action = INPUT_ACTION_MOVE_FORWARD;
    controllable->yaw_axis.action = INPUT_ACTION_CAMERA_YAW;
    controllable->pitch_axis.action = INPUT_ACTION_CAMERA_PITCH;
    
    // Auto-roll based on yaw (banking)
    controllable->roll_axis.action = INPUT_ACTION_NONE; // Will be calculated from yaw
    
    // Actions
    controllable->boost_action = INPUT_ACTION_BOOST;
    controllable->brake_action = INPUT_ACTION_BRAKE;
    
    // Arcade-style response
    controllable->pitch_axis.curve = RESPONSE_LINEAR;
    controllable->yaw_axis.curve = RESPONSE_LINEAR;
    
    // Higher sensitivity for arcade feel
    controllable->pitch_axis.sensitivity = 1.2f;
    controllable->yaw_axis.sensitivity = 1.2f;
    
    // Add some input smoothing for arcade feel
    controllable->input_smoothing = 0.1f;
    
    printf("✅ Configured arcade spaceship controls\n");
}

// Preset: FPS Camera
void controllable_setup_fps_camera(Controllable* controllable) {
    if (!controllable) return;
    
    // Movement
    controllable->forward_axis.action = INPUT_ACTION_MOVE_FORWARD;
    controllable->strafe_axis.action = INPUT_ACTION_MOVE_RIGHT;
    
    // Look (pitch/yaw only for FPS)
    controllable->pitch_axis.action = INPUT_ACTION_CAMERA_PITCH;
    controllable->yaw_axis.action = INPUT_ACTION_CAMERA_YAW;
    
    // FPS cameras typically have different sensitivities
    controllable->pitch_axis.sensitivity = 0.5f;
    controllable->yaw_axis.sensitivity = 0.5f;
    
    // Linear response for direct control
    controllable->pitch_axis.curve = RESPONSE_LINEAR;
    controllable->yaw_axis.curve = RESPONSE_LINEAR;
    
    printf("✅ Configured FPS camera controls\n");
}

// Preset: Orbit Camera
void controllable_setup_orbit_camera(Controllable* controllable) {
    if (!controllable) return;
    
    // Orbit controls
    controllable->yaw_axis.action = INPUT_ACTION_CAMERA_YAW;
    controllable->pitch_axis.action = INPUT_ACTION_CAMERA_PITCH;
    
    // Zoom (using forward/backward)
    controllable->forward_axis.action = INPUT_ACTION_CAMERA_ZOOM;
    
    // Orbit cameras need inverted pitch usually
    controllable->pitch_axis.inverted = true;
    
    // Smooth orbit movement
    controllable->input_smoothing = 0.15f;
    
    printf("✅ Configured orbit camera controls\n");
}