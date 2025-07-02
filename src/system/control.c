#include "control.h"
#include "input.h"
#include "thrusters.h"
#include "../component/look_target.h"
#include <stdio.h>
#include <math.h>

// Global player entity for input processing
static EntityID g_player_entity = INVALID_ENTITY;

// ============================================================================
// CANYON RACING CONTROL FUNCTIONS
// ============================================================================

// Calculate thrust direction based on look target
static Vector3 calculate_look_based_thrust(const LookTarget* look_target, 
                                         const Vector3* ship_position,
                                         float thrust_magnitude) {
    if (!look_target || !ship_position) {
        return (Vector3){0, 0, thrust_magnitude}; // Default forward (positive Z)
    }
    
    // Get direction from ship to look target
    Vector3 to_target = look_target_get_direction(look_target, ship_position);
    
    // Scale by thrust magnitude
    return vector3_multiply_scalar(to_target, thrust_magnitude);
}

// Calculate torque to turn ship towards look direction
static Vector3 calculate_look_alignment_torque(const LookTarget* look_target,
                                             const Vector3* ship_position,
                                             const Quaternion* ship_orientation,
                                             float alignment_strength) {
    if (!look_target || !ship_position || !ship_orientation) {
        return (Vector3){0, 0, 0};
    }
    
    // Get desired forward direction
    Vector3 desired_forward = look_target_get_direction(look_target, ship_position);
    
    // Get current forward direction
    Vector3 current_forward = quaternion_rotate_vector(*ship_orientation, (Vector3){0, 0, 1});
    
    // Calculate rotation axis and angle
    Vector3 rotation_axis = vector3_cross(current_forward, desired_forward);
    float dot = vector3_dot(current_forward, desired_forward);
    dot = fmaxf(-1.0f, fminf(1.0f, dot)); // Clamp to avoid NaN from acos
    float angle = acosf(dot);
    
    // If vectors are nearly aligned, no torque needed
    if (angle < 0.01f) {
        return (Vector3){0, 0, 0};
    }
    
    // Normalize rotation axis
    rotation_axis = vector3_normalize(rotation_axis);
    
    // Apply proportional torque
    return vector3_multiply_scalar(rotation_axis, angle * alignment_strength);
}

// Apply auto-leveling to keep ship upright
static Vector3 apply_auto_level_torque(const Quaternion* ship_orientation, float strength) {
    if (!ship_orientation || strength <= 0.0f) {
        return (Vector3){0, 0, 0};
    }
    
    // Get ship's current up vector
    Vector3 current_up = quaternion_rotate_vector(*ship_orientation, (Vector3){0, 1, 0});
    
    // World up
    Vector3 world_up = {0, 1, 0};
    
    // Calculate correction torque
    Vector3 torque = vector3_cross(current_up, world_up);
    
    // Scale by strength
    return vector3_multiply_scalar(torque, strength);
}

// Process linear input for canyon racing
static Vector3 process_canyon_racing_linear(const InputState* input, 
                                          struct ControlAuthority* control,
                                          const Vector3* ship_position) {
    if (!input || !control) return (Vector3){0, 0, 0};
    
    Vector3 linear_commands = {0, 0, 0};
    
    // Check if we're using look-based thrust
    if (input->look_based_thrust && input->thrust > 0.0f) {
        // Update player position for look target
        input_update_player_position(ship_position);
        
        // Calculate thrust direction based on look target
        linear_commands = calculate_look_based_thrust(&input->look_target, 
                                                    ship_position, 
                                                    input->thrust);
    } else {
        // Traditional thrust (for reverse, vertical, etc.)
        linear_commands = (Vector3){
            0.0f,                // No strafe in canyon racing mode
            input->vertical,     // Up/down
            input->thrust        // Forward/backward (positive Z = forward)
        };
    }
    
    // Apply boost
    if (input->boost > 0.0f) {
        float boost_factor = 1.0f + input->boost * 3.0f; // 4x boost
        linear_commands = vector3_multiply_scalar(linear_commands, boost_factor);
    }
    
    // Apply brake
    if (input->brake) {
        linear_commands = vector3_multiply_scalar(linear_commands, 0.05f);
    }
    
    return linear_commands;
}

// Process angular input for canyon racing
static Vector3 process_canyon_racing_angular(const InputState* input,
                                           struct ControlAuthority* control,
                                           const Vector3* ship_position,
                                           const Quaternion* ship_orientation) {
    if (!input || !control) return (Vector3){0, 0, 0};
    
    Vector3 angular_commands = {0, 0, 0};
    
    // Direct ship control from input (tuned for responsive but smooth control)
    float sensitivity = control->control_sensitivity * 0.8f; // Increased for better responsiveness
    angular_commands.x = input->pitch * sensitivity;
    angular_commands.y = input->yaw * sensitivity;
    angular_commands.z = input->roll * sensitivity;
    
    // Add look-direction alignment when thrusting
    if (input->look_based_thrust && input->thrust > 0.0f) {
        Vector3 look_torque = calculate_look_alignment_torque(&input->look_target,
                                                            ship_position,
                                                            ship_orientation,
                                                            0.5f); // Reduced alignment strength for smoother control
        angular_commands = vector3_add(angular_commands, look_torque);
    }
    
    // Add auto-leveling
    if (input->auto_level > 0.0f) {
        Vector3 level_torque = apply_auto_level_torque(ship_orientation, input->auto_level);
        angular_commands = vector3_add(angular_commands, level_torque);
    }
    
    // Clamp to reasonable values
    angular_commands.x = fmaxf(-1.0f, fminf(1.0f, angular_commands.x));
    angular_commands.y = fmaxf(-1.0f, fminf(1.0f, angular_commands.y));
    angular_commands.z = fmaxf(-1.0f, fminf(1.0f, angular_commands.z));
    
    return angular_commands;
}

// ============================================================================
// MAIN CONTROL SYSTEM UPDATE
// ============================================================================

void control_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config;
    (void)delta_time;
    if (!world) return;

    // Get current input state
    const InputState* input = input_get_state();
    if (!input) return;

    uint32_t control_updates = 0;
    (void)control_updates; // Suppress unused warning

    // Process all entities with control authority
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];

        if (!(entity->component_mask & COMPONENT_CONTROL_AUTHORITY) ||
            !(entity->component_mask & COMPONENT_THRUSTER_SYSTEM)) {
            continue;
        }

        struct ControlAuthority* control = entity->control_authority;
        struct ThrusterSystem* thrusters = entity->thruster_system;
        struct Transform* transform = entity->transform;
        
        if (!control || !thrusters) continue;

        // Only process input for player-controlled entities
        if (control->controlled_by != INVALID_ENTITY && control->controlled_by == g_player_entity) {
            control_updates++;
            
            // Get ship position and orientation
            Vector3 ship_position = transform ? transform->position : (Vector3){0, 0, 0};
            Quaternion ship_orientation = transform ? transform->rotation : 
                                         (Quaternion){0, 0, 0, 1};
            
            // Process linear input (thrust)
            Vector3 linear_commands = process_canyon_racing_linear(input, control, &ship_position);
            control->input_linear = linear_commands;
            thruster_set_linear_command(thrusters, linear_commands);
            
            // Process angular input (rotation)
            Vector3 angular_commands = process_canyon_racing_angular(input, control, 
                                                                   &ship_position, 
                                                                   &ship_orientation);
            control->input_angular = angular_commands;
            thruster_set_angular_command(thrusters, angular_commands);
            
            // Store boost and brake state
            control->input_boost = input->boost;
            control->input_brake = input->brake;
            
            // Debug output
            static uint32_t debug_counter = 0;
            if (++debug_counter % 60 == 0 && 
                (input->thrust != 0.0f || fabsf(angular_commands.x) > 0.1f || 
                 fabsf(angular_commands.y) > 0.1f || fabsf(angular_commands.z) > 0.1f)) {
                
                printf("🏎️ Canyon Control: ");
                if (input->look_based_thrust) {
                    printf("LOOK-THRUST ");
                }
                if (input->auto_level > 0.0f) {
                    printf("AUTO-LEVEL(%.1f) ", input->auto_level);
                }
                printf("Lin:(%.2f,%.2f,%.2f) Ang:(%.2f,%.2f,%.2f)\n",
                       linear_commands.x, linear_commands.y, linear_commands.z,
                       angular_commands.x, angular_commands.y, angular_commands.z);
            }
        } else {
            // Clear commands for non-player entities
            thruster_set_linear_command(thrusters, (Vector3){0, 0, 0});
            thruster_set_angular_command(thrusters, (Vector3){0, 0, 0});
        }
    }
}

// ============================================================================
// CONTROL CONFIGURATION FUNCTIONS
// ============================================================================

void control_set_player_entity(struct World* world, EntityID player_entity) {
    (void)world;
    g_player_entity = player_entity;
    printf("🎮 Canyon Racing Control: Player entity set to %d\n", player_entity);
}

void control_set_sensitivity(struct ControlAuthority* control, float sensitivity) {
    if (!control) return;
    control->control_sensitivity = fmaxf(0.1f, fminf(5.0f, sensitivity));
}

void control_set_flight_assist(struct ControlAuthority* control, bool enabled) {
    if (!control) return;
    control->flight_assist_enabled = enabled;
}

void control_toggle_flight_assist(struct ControlAuthority* control) {
    if (!control) return;
    control->flight_assist_enabled = !control->flight_assist_enabled;
    printf("🎮 Flight Assist: %s\n", control->flight_assist_enabled ? "ON" : "OFF");
}

// ============================================================================
// FLIGHT ASSISTANCE FUNCTIONS  
// ============================================================================

Vector3 apply_stability_assist(Vector3 input, Vector3 current_angular_velocity, float assist_strength) {
    Vector3 assisted_input = input;
    
    if (assist_strength > 0.0f) {
        // Apply counter-rotation to stabilize
        if (fabsf(input.x) < 0.1f) {
            assisted_input.x -= current_angular_velocity.x * assist_strength * 0.5f;
        }
        if (fabsf(input.y) < 0.1f) {
            assisted_input.y -= current_angular_velocity.y * assist_strength * 0.5f;
        }
        if (fabsf(input.z) < 0.1f) {
            assisted_input.z -= current_angular_velocity.z * assist_strength * 0.5f;
        }
    }
    
    // Clamp
    assisted_input.x = fmaxf(-1.0f, fminf(1.0f, assisted_input.x));
    assisted_input.y = fmaxf(-1.0f, fminf(1.0f, assisted_input.y));
    assisted_input.z = fmaxf(-1.0f, fminf(1.0f, assisted_input.z));
    
    return assisted_input;
}

Vector3 apply_sensitivity_curve(Vector3 input, float sensitivity) {
    // Simple quadratic curve for smooth response
    Vector3 result;
    
    float sign_x = input.x > 0 ? 1.0f : -1.0f;
    float sign_y = input.y > 0 ? 1.0f : -1.0f;
    float sign_z = input.z > 0 ? 1.0f : -1.0f;
    
    result.x = input.x * fabsf(input.x) * sign_x * sensitivity;
    result.y = input.y * fabsf(input.y) * sign_y * sensitivity;
    result.z = input.z * fabsf(input.z) * sign_z * sensitivity;
    
    // Clamp
    result.x = fmaxf(-1.0f, fminf(1.0f, result.x));
    result.y = fmaxf(-1.0f, fminf(1.0f, result.y));
    result.z = fmaxf(-1.0f, fminf(1.0f, result.z));
    
    return result;
}