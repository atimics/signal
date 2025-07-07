/**
 * @file unified_flight_control.c
 * @brief Implementation of unified flight control component
 */

#include "unified_flight_control.h"
#include "../game_input.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// ============================================================================
// COMPONENT MANAGEMENT
// ============================================================================

UnifiedFlightControl* unified_flight_control_create(void) {
    UnifiedFlightControl* control = (UnifiedFlightControl*)calloc(1, sizeof(UnifiedFlightControl));
    if (control) {
        unified_flight_control_reset(control);
    }
    return control;
}

void unified_flight_control_destroy(UnifiedFlightControl* control) {
    if (control) {
        free(control);
    }
}

void unified_flight_control_reset(UnifiedFlightControl* control) {
    if (!control) return;
    
    memset(control, 0, sizeof(UnifiedFlightControl));
    
    // Set defaults
    control->mode = FLIGHT_CONTROL_MANUAL;
    control->authority_level = AUTHORITY_NONE;
    control->controlled_by = INVALID_ENTITY;
    control->enabled = true;
    
    // Default input configuration - using InputActionID enum values
    control->input_config.thrust_forward = INPUT_ACTION_THRUST_FORWARD;
    control->input_config.thrust_back = INPUT_ACTION_THRUST_BACK;
    control->input_config.pitch_up = INPUT_ACTION_PITCH_UP;
    control->input_config.pitch_down = INPUT_ACTION_PITCH_DOWN;
    control->input_config.yaw_left = INPUT_ACTION_YAW_LEFT;
    control->input_config.yaw_right = INPUT_ACTION_YAW_RIGHT;
    control->input_config.roll_left = INPUT_ACTION_ROLL_LEFT;
    control->input_config.roll_right = INPUT_ACTION_ROLL_RIGHT;
    control->input_config.boost = INPUT_ACTION_BOOST;
    control->input_config.brake = INPUT_ACTION_BRAKE;
    
    // Default sensitivity
    control->input_config.linear_sensitivity = 1.0f;
    control->input_config.angular_sensitivity = 1.0f;
    control->input_config.dead_zone = 0.1f;
    
    // Default assistance settings
    control->stability_assist = 0.3f;
    control->inertia_dampening = 0.2f;
    control->flight_assist_enabled = true;
    
    // Default control limits
    control->state.max_linear_acceleration = 50.0f;
    control->state.max_angular_acceleration = 3.0f;
    control->state.g_force_limit = 8.0f;
    control->state.collision_avoidance = true;
    
    // Default ship parameters
    control->mass_scaling = 1.0f;
    control->thruster_efficiency = 1.0f;
    control->moment_of_inertia = (Vector3){1.0f, 1.0f, 1.0f};
}

// ============================================================================
// MODE MANAGEMENT
// ============================================================================

void unified_flight_control_set_mode(UnifiedFlightControl* control, FlightControlMode mode) {
    if (!control) return;
    
    if (unified_flight_control_can_switch_mode(control, mode)) {
        FlightControlMode old_mode = control->mode;
        control->mode = mode;
        
        printf("✈️ Flight control mode changed: %d → %d\n", old_mode, mode);
        
        // Mode-specific setup
        switch (mode) {
            case FLIGHT_CONTROL_MANUAL:
                control->state.assistance_level = 0.0f;
                break;
                
            case FLIGHT_CONTROL_ASSISTED:
                control->state.assistance_level = 0.8f;
                break;
                
            case FLIGHT_CONTROL_SCRIPTED:
                control->state.assistance_level = 1.0f;
                break;
                
            case FLIGHT_CONTROL_AUTONOMOUS:
                control->state.assistance_level = 1.0f;
                // Initialize autonomous data if needed
                break;
                
            case FLIGHT_CONTROL_FORMATION:
                control->state.assistance_level = 1.0f;
                break;
        }
    }
}

FlightControlMode unified_flight_control_get_mode(const UnifiedFlightControl* control) {
    return control ? control->mode : FLIGHT_CONTROL_MANUAL;
}

bool unified_flight_control_can_switch_mode(const UnifiedFlightControl* control, FlightControlMode new_mode) {
    if (!control || !control->enabled) return false;
    
    // Always allow switching to manual
    if (new_mode == FLIGHT_CONTROL_MANUAL) return true;
    
    // Other modes require appropriate authority
    switch (new_mode) {
        case FLIGHT_CONTROL_ASSISTED:
            return control->authority_level >= AUTHORITY_ASSISTANT;
            
        case FLIGHT_CONTROL_SCRIPTED:
            return control->authority_level >= AUTHORITY_SCRIPT;
            
        case FLIGHT_CONTROL_AUTONOMOUS:
            return control->authority_level >= AUTHORITY_AI;
            
        case FLIGHT_CONTROL_FORMATION:
            return control->authority_level >= AUTHORITY_AI;
            
        default:
            return false;
    }
}

// ============================================================================
// INPUT PROCESSING
// ============================================================================

void unified_flight_control_process_input(UnifiedFlightControl* control, InputService* input_service) {
    if (!control || !input_service || !control->enabled) {
        static uint32_t null_debug = 0;
        if (++null_debug % 300 == 0) {
            printf("🎮 PROCESS_INPUT: control=%p, input_service=%p, enabled=%d\n",
                   (void*)control, (void*)input_service, control ? control->enabled : -1);
        }
        return;
    }
    
    // Only process manual input in manual and assisted modes
    if (control->mode != FLIGHT_CONTROL_MANUAL && control->mode != FLIGHT_CONTROL_ASSISTED) {
        static uint32_t mode_debug = 0;
        if (++mode_debug % 300 == 0) {
            printf("🎮 PROCESS_INPUT: Wrong mode %d (need MANUAL or ASSISTED)\n", control->mode);
        }
        return;
    }
    
    const InputConfiguration* config = &control->input_config;
    
    // Get linear input
    float thrust = input_service->get_action_value(input_service, config->thrust_forward) - 
                   input_service->get_action_value(input_service, config->thrust_back);
    float vertical = 0.0f; // TODO: Add vertical input actions
    float strafe = 0.0f;   // TODO: Add strafe input actions
    
    // Get angular input
    float pitch = input_service->get_action_value(input_service, config->pitch_up) - 
                  input_service->get_action_value(input_service, config->pitch_down);
    float yaw = input_service->get_action_value(input_service, config->yaw_right) - 
                input_service->get_action_value(input_service, config->yaw_left);
    float roll = input_service->get_action_value(input_service, config->roll_right) - 
                 input_service->get_action_value(input_service, config->roll_left);
    
    // Debug: Log raw input values (always log every second, even if zero)
    static uint32_t input_debug_counter = 0;
    if (++input_debug_counter % 60 == 0) {
        printf("🎮 INPUT DEBUG: thrust=%.2f, pitch=%.2f, yaw=%.2f, roll=%.2f\n",
               thrust, pitch, yaw, roll);
        printf("   Mode=%d, Authority=%d, Enabled=%d\n",
               control->mode, control->authority_level, control->enabled);
    }
    
    // Apply inversion
    if (config->invert_pitch) pitch = -pitch;
    if (config->invert_yaw) yaw = -yaw;
    
    // Apply sensitivity
    thrust *= config->linear_sensitivity;
    vertical *= config->linear_sensitivity;
    strafe *= config->linear_sensitivity;
    pitch *= config->angular_sensitivity;
    yaw *= config->angular_sensitivity;
    roll *= config->angular_sensitivity;
    
    // Apply dead zone
    if (fabsf(thrust) < config->dead_zone) thrust = 0.0f;
    if (fabsf(vertical) < config->dead_zone) vertical = 0.0f;
    if (fabsf(strafe) < config->dead_zone) strafe = 0.0f;
    if (fabsf(pitch) < config->dead_zone) pitch = 0.0f;
    if (fabsf(yaw) < config->dead_zone) yaw = 0.0f;
    if (fabsf(roll) < config->dead_zone) roll = 0.0f;
    
    // Apply response curve
    if (config->use_quadratic_curve) {
        thrust = thrust * fabsf(thrust);
        vertical = vertical * fabsf(vertical);
        strafe = strafe * fabsf(strafe);
        pitch = pitch * fabsf(pitch);
        yaw = yaw * fabsf(yaw);
        roll = roll * fabsf(roll);
    }
    
    // Clamp to [-1, 1]
    control->state.linear_input.x = fmaxf(-1.0f, fminf(1.0f, strafe));
    control->state.linear_input.y = fmaxf(-1.0f, fminf(1.0f, vertical));
    control->state.linear_input.z = fmaxf(-1.0f, fminf(1.0f, thrust));
    
    control->state.angular_input.x = fmaxf(-1.0f, fminf(1.0f, pitch));
    control->state.angular_input.y = fmaxf(-1.0f, fminf(1.0f, yaw));
    control->state.angular_input.z = fmaxf(-1.0f, fminf(1.0f, roll));
    
    // Get boost/brake
    control->state.boost_input = input_service->get_action_value(input_service, config->boost);
    control->state.brake_input = input_service->get_action_value(input_service, config->brake);
}

// ============================================================================
// CONTROL STATE ACCESS
// ============================================================================

const ControlState* unified_flight_control_get_state(const UnifiedFlightControl* control) {
    return control ? &control->state : NULL;
}

Vector3 unified_flight_control_get_linear_command(const UnifiedFlightControl* control) {
    if (!control || !control->enabled) {
        return (Vector3){0, 0, 0};
    }
    
    Vector3 command = control->state.linear_input;
    
    // Apply boost
    if (control->state.boost_input > 0.0f) {
        float boost_factor = 1.0f + control->state.boost_input * 2.0f; // 3x max boost
        command = (Vector3){
            command.x * boost_factor,
            command.y * boost_factor,
            command.z * boost_factor
        };
    }
    
    return command;
}

Vector3 unified_flight_control_get_angular_command(const UnifiedFlightControl* control) {
    if (!control || !control->enabled) {
        return (Vector3){0, 0, 0};
    }
    
    return control->state.angular_input;
}

// ============================================================================
// AUTHORITY MANAGEMENT
// ============================================================================

void unified_flight_control_request_authority(UnifiedFlightControl* control, ControlAuthority level, EntityID requester) {
    if (!control) return;
    
    // Higher authority levels can always take control
    if (level > control->authority_level) {
        control->authority_level = level;
        control->controlled_by = requester;
        printf("✈️ Control authority granted: level %d to entity %d\n", level, requester);
    }
}

void unified_flight_control_release_authority(UnifiedFlightControl* control, EntityID releaser) {
    if (!control) return;
    
    if (control->controlled_by == releaser) {
        control->authority_level = AUTHORITY_NONE;
        control->controlled_by = INVALID_ENTITY;
        printf("✈️ Control authority released by entity %d\n", releaser);
    }
}

bool unified_flight_control_has_authority(const UnifiedFlightControl* control, EntityID entity) {
    return control && control->controlled_by == entity;
}

// ============================================================================
// CONFIGURATION FUNCTIONS
// ============================================================================

void unified_flight_control_configure_input(UnifiedFlightControl* control, const InputConfiguration* config) {
    if (!control || !config) return;
    
    control->input_config = *config;
}

void unified_flight_control_set_sensitivity(UnifiedFlightControl* control, float linear, float angular) {
    if (!control) return;
    
    control->input_config.linear_sensitivity = fmaxf(0.1f, fminf(5.0f, linear));
    control->input_config.angular_sensitivity = fmaxf(0.1f, fminf(5.0f, angular));
}

void unified_flight_control_set_assistance(UnifiedFlightControl* control, float stability, float inertia) {
    if (!control) return;
    
    control->stability_assist = fmaxf(0.0f, fminf(1.0f, stability));
    control->inertia_dampening = fmaxf(0.0f, fminf(1.0f, inertia));
}

// ============================================================================
// PRESET CONFIGURATIONS
// ============================================================================

void unified_flight_control_setup_manual_flight(UnifiedFlightControl* control) {
    if (!control) return;
    
    unified_flight_control_set_mode(control, FLIGHT_CONTROL_MANUAL);
    control->stability_assist = 0.1f;
    control->inertia_dampening = 0.0f;
    control->flight_assist_enabled = false;
    control->state.assistance_level = 0.0f;
}

void unified_flight_control_setup_assisted_flight(UnifiedFlightControl* control) {
    if (!control) return;
    
    unified_flight_control_set_mode(control, FLIGHT_CONTROL_ASSISTED);
    control->stability_assist = 0.5f;
    control->inertia_dampening = 0.3f;
    control->flight_assist_enabled = true;
    control->state.assistance_level = 0.8f;
}

void unified_flight_control_setup_autonomous_flight(UnifiedFlightControl* control) {
    if (!control) return;
    
    unified_flight_control_set_mode(control, FLIGHT_CONTROL_AUTONOMOUS);
    control->stability_assist = 1.0f;
    control->inertia_dampening = 0.8f;
    control->flight_assist_enabled = true;
    control->state.assistance_level = 1.0f;
}

// ============================================================================
// SYSTEM INTEGRATION
// ============================================================================

void unified_flight_control_update(UnifiedFlightControl* control, float delta_time) {
    if (!control || !control->enabled) return;
    
    // Update timing
    control->last_update_time = delta_time;
    control->update_count++;
    
    // Mode-specific updates
    switch (control->mode) {
        case FLIGHT_CONTROL_MANUAL:
        case FLIGHT_CONTROL_ASSISTED:
            // Input is processed separately by unified_flight_control_process_input
            break;
            
        case FLIGHT_CONTROL_SCRIPTED:
            // TODO: Update scripted flight path following
            break;
            
        case FLIGHT_CONTROL_AUTONOMOUS:
            // TODO: Update autonomous flight (Sprint 26)
            break;
            
        case FLIGHT_CONTROL_FORMATION:
            // TODO: Update formation flying
            break;
    }
}

// ============================================================================
// MIGRATION FUNCTIONS (for compatibility during transition)
// ============================================================================

void unified_flight_control_migrate_from_control_authority(UnifiedFlightControl* unified, const void* old_control) {
    // TODO: Implement migration from old ControlAuthority component
    (void)unified;
    (void)old_control;
}

void unified_flight_control_migrate_from_controllable(UnifiedFlightControl* unified, const void* old_controllable) {
    // TODO: Implement migration from old Controllable component
    (void)unified;
    (void)old_controllable;
}

void unified_flight_control_migrate_from_scripted_flight(UnifiedFlightControl* unified, const void* old_scripted) {
    // TODO: Implement migration from old ScriptedFlight component
    (void)unified;
    (void)old_scripted;
}