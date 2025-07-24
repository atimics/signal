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
    
    // Progressive input configuration (keyboard feels like controller)
    control->input_config.enable_progressive_input = true;
    control->input_config.yaw_acceleration_rate = 2.5f;     // Start slow, build up speed
    control->input_config.roll_acceleration_rate = 2.0f;    // Slightly slower for roll
    control->input_config.max_yaw_velocity = 3.0f;          // Maximum turn rate multiplier
    control->input_config.max_roll_velocity = 2.5f;         // Maximum roll rate multiplier  
    control->input_config.decay_rate = 8.0f;                // Fast decay when released
    
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
    
    // Flight Assist defaults (Sprint 26)
    control->assist_enabled = false;  // Off by default, player enables when ready
    control->assist_target_position = (Vector3){0, 0, 0};
    control->assist_target_velocity = (Vector3){0, 0, 0};
    control->assist_sphere_radius = 50.0f;  // 50m default sphere
    control->assist_responsiveness = 0.7f;  // 70% responsiveness
    control->assist_bank_angle = 0.0f;
    control->assist_target_bank_angle = 0.0f;
    control->assist_computation_time = 0.0f;
    
    // PD Controller defaults - tuned for good response
    control->assist_kp = 2.0f;  // Position gain
    control->assist_kd = 0.5f;  // Velocity damping
    control->assist_max_acceleration = 30.0f;  // 30 m/s² max
    
    // Initialize progressive input state
    control->state.current_yaw_velocity = 0.0f;
    control->state.current_roll_velocity = 0.0f;
    control->state.yaw_input_duration = 0.0f;
    control->state.roll_input_duration = 0.0f;
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
                control->assist_enabled = false;  // Disable flight assist in manual mode
                break;
                
            case FLIGHT_CONTROL_ASSISTED:
                control->state.assistance_level = 0.8f;
                control->assist_enabled = true;  // Enable flight assist in this mode
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

void unified_flight_control_process_input(UnifiedFlightControl* control, InputService* input_service, float delta_time) {
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
    float vertical_up = input_service->get_action_value(input_service, INPUT_ACTION_VERTICAL_UP);
    float vertical_down = input_service->get_action_value(input_service, INPUT_ACTION_VERTICAL_DOWN);
    float vertical = (fabsf(vertical_up) > fabsf(vertical_down)) ? vertical_up : -vertical_down;
    float strafe = 0.0f;   // TODO: Add strafe input actions
    
    // Get angular input - for analog axes, use single action (gamepad gives -1 to +1)
    // For digital input (keyboard), use difference between actions
    float pitch_up = input_service->get_action_value(input_service, config->pitch_up);
    float pitch_down = input_service->get_action_value(input_service, config->pitch_down);
    float yaw_left = input_service->get_action_value(input_service, config->yaw_left);
    float yaw_right = input_service->get_action_value(input_service, config->yaw_right);
    float roll_left = input_service->get_action_value(input_service, config->roll_left);
    float roll_right = input_service->get_action_value(input_service, config->roll_right);
    
    // For analog input (gamepad), the axis gives both directions
    // For digital input (keyboard), calculate difference
    float pitch = (fabsf(pitch_up) > fabsf(pitch_down)) ? pitch_up : -pitch_down;
    float yaw = yaw_left - yaw_right;  // Fixed: A (left) = positive, D (right) = negative  
    float roll = roll_right - roll_left;
    
    // Debug: Log raw input values only when there's actual input
    static uint32_t input_debug_counter = 0;
    bool has_input = (fabsf(thrust) > 0.01f || fabsf(pitch) > 0.01f || fabsf(yaw) > 0.01f || fabsf(roll) > 0.01f || fabsf(vertical) > 0.01f);
    if (has_input && ++input_debug_counter % 30 == 0) {
        printf("🎮 INPUT: thrust=%.2f, pitch=%.2f, yaw=%.2f, roll=%.2f, vertical=%.2f\n",
               thrust, pitch, yaw, roll, vertical);
        printf("🎮 Raw values: P[↑%.2f ↓%.2f] Y[←%.2f →%.2f] R[←%.2f →%.2f] V[↑%.2f ↓%.2f]\n",
               pitch_up, pitch_down, yaw_left, yaw_right, roll_left, roll_right, vertical_up, vertical_down);
    }
    
    // Apply inversion
    if (config->invert_pitch) pitch = -pitch;
    if (config->invert_yaw) yaw = -yaw;
    
    // Progressive input acceleration for keyboard (simulate controller dynamic range)
    if (config->enable_progressive_input) {
        float dt = delta_time;
        
        // Track yaw input duration and build up velocity
        if (fabsf(yaw) > 0.01f) {
            control->state.yaw_input_duration += dt;
            // Accelerate towards target, starting slow and building up
            float target_yaw_velocity = yaw * config->max_yaw_velocity;
            control->state.current_yaw_velocity += (target_yaw_velocity - control->state.current_yaw_velocity) * 
                                                   config->yaw_acceleration_rate * dt;
        } else {
            // Decay when no input
            control->state.current_yaw_velocity *= expf(-config->decay_rate * dt);
            control->state.yaw_input_duration = 0.0f;
        }
        
        // Track roll input duration and build up velocity  
        if (fabsf(roll) > 0.01f) {
            control->state.roll_input_duration += dt;
            // Accelerate towards target, starting slow and building up
            float target_roll_velocity = roll * config->max_roll_velocity;
            control->state.current_roll_velocity += (target_roll_velocity - control->state.current_roll_velocity) * 
                                                    config->roll_acceleration_rate * dt;
        } else {
            // Decay when no input
            control->state.current_roll_velocity *= expf(-config->decay_rate * dt);
            control->state.roll_input_duration = 0.0f;
        }
        
        // Apply progressive velocities (normalized to -1 to +1 range)
        yaw = fmaxf(-1.0f, fminf(1.0f, control->state.current_yaw_velocity));
        roll = fmaxf(-1.0f, fminf(1.0f, control->state.current_roll_velocity));
    }
    
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
    
    // Add banking (coordinated turn) - automatically roll when yawing
    if (control->flight_assist_enabled && fabsf(yaw) > 0.01f) {
        // Banking ratio: configurable per mode, default 1.8f for stronger banking turns
        float banking_ratio = (control->mode == FLIGHT_CONTROL_AUTONOMOUS) ? 1.2f : 1.8f;
        roll -= yaw * banking_ratio;  // Negative because we want to roll into the turn
        
        // Clamp total roll to prevent over-rotation
        roll = fmaxf(-1.0f, fminf(1.0f, roll));
        
        // Debug banking
        static uint32_t bank_debug = 0;
        if (++bank_debug % 60 == 0 && fabsf(yaw) > 0.1f) {
            printf("🏁 Banking: mode=%d, yaw=%.2f → added roll=%.2f (ratio=%.1f)\n", 
                   control->mode, yaw, -yaw * banking_ratio, banking_ratio);
        }
    }
    
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
    control->stability_assist = 0.02f;  // Much reduced from 0.1f for true manual feel
    control->inertia_dampening = 0.0f;
    control->flight_assist_enabled = false;
    control->state.assistance_level = 0.0f;
}

void unified_flight_control_setup_assisted_flight(UnifiedFlightControl* control) {
    if (!control) return;
    
    unified_flight_control_set_mode(control, FLIGHT_CONTROL_ASSISTED);
    control->stability_assist = 0.15f;  // Much reduced from 0.5f for manual feel 
    control->inertia_dampening = 0.1f;  // Reduced from 0.3f
    control->flight_assist_enabled = true;
    control->state.assistance_level = 0.3f;  // Reduced from 0.8f
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

// ============================================================================
// FLIGHT ASSIST FUNCTIONS (Sprint 26)
// ============================================================================

void unified_flight_control_enable_assist(UnifiedFlightControl* control, bool enabled) {
    if (!control) return;
    
    control->assist_enabled = enabled;
    
    // Reset assist state when toggling
    if (!enabled) {
        control->assist_target_position = (Vector3){0, 0, 0};
        control->assist_target_velocity = (Vector3){0, 0, 0};
        control->assist_bank_angle = 0.0f;
        control->assist_target_bank_angle = 0.0f;
    }
}

void unified_flight_control_set_assist_params(UnifiedFlightControl* control, float kp, float kd, float max_accel) {
    if (!control) return;
    
    control->assist_kp = fmaxf(0.0f, kp);
    control->assist_kd = fmaxf(0.0f, kd);
    control->assist_max_acceleration = fmaxf(1.0f, max_accel);
}

void unified_flight_control_set_assist_responsiveness(UnifiedFlightControl* control, float responsiveness) {
    if (!control) return;
    
    control->assist_responsiveness = fmaxf(0.0f, fminf(1.0f, responsiveness));
}

Vector3 unified_flight_control_calculate_assist_target(const UnifiedFlightControl* control, 
                                                      const struct Transform* transform,
                                                      Vector3 input_direction) {
    if (!control || !transform) {
        return (Vector3){0, 0, 0};
    }
    
    // Normalize input direction
    float input_magnitude = vector3_length(input_direction);
    if (input_magnitude < 0.001f) {
        // No input - target current position (station keeping)
        return transform->position;
    }
    
    // Scale by sphere radius (adjusted by responsiveness)
    float effective_radius = control->assist_sphere_radius * control->assist_responsiveness;
    Vector3 normalized_input = vector3_multiply(input_direction, 1.0f / input_magnitude);
    Vector3 scaled_input = vector3_multiply(normalized_input, effective_radius);
    
    // Transform to world space using ship's rotation
    Vector3 world_direction = quaternion_rotate_vector(transform->rotation, scaled_input);
    
    // Target position is current position + world space offset
    return vector3_add(transform->position, world_direction);
}

Vector3 unified_flight_control_get_assist_acceleration(const UnifiedFlightControl* control,
                                                       const struct Transform* transform,
                                                       const struct Physics* physics) {
    if (!control || !transform || !physics || !control->assist_enabled) {
        return (Vector3){0, 0, 0};
    }
    
    // Calculate position error
    Vector3 position_error = vector3_subtract(control->assist_target_position, transform->position);
    
    // Calculate velocity error (we want to match target velocity)
    Vector3 velocity_error = vector3_subtract(control->assist_target_velocity, physics->velocity);
    
    // PD control law: acceleration = Kp * position_error + Kd * velocity_error
    Vector3 p_term = vector3_multiply(position_error, control->assist_kp);
    Vector3 d_term = vector3_multiply(velocity_error, control->assist_kd);
    Vector3 desired_acceleration = vector3_add(p_term, d_term);
    
    // Clamp to maximum acceleration
    float accel_magnitude = vector3_length(desired_acceleration);
    if (accel_magnitude > control->assist_max_acceleration && accel_magnitude > 0.001f) {
        desired_acceleration = vector3_multiply(desired_acceleration, 
                                              control->assist_max_acceleration / accel_magnitude);
    }
    
    return desired_acceleration;
}