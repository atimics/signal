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

// REMOVED: Competing control system functions that caused conflicts
// - calculate_look_based_thrust: Part of look-based thrust system
// - calculate_look_alignment_torque: Part of auto-alignment system  
// - apply_auto_level_torque: Part of auto-leveling system
// All disabled for direct manual control only

// Process linear input for canyon racing
static Vector3 process_canyon_racing_linear(const InputState* input, 
                                          struct ControlAuthority* control,
                                          const Vector3* ship_position,
                                          const Quaternion* ship_orientation) {
    (void)ship_position; // Unused for now
    (void)ship_orientation; // Unused for now
    if (!input || !control) return (Vector3){0, 0, 0};
    
    Vector3 linear_commands = {0, 0, 0};
    
    // SIMPLIFIED: Direct thrust control only
    // For now, stick with traditional forward thrust until we can implement proper target calculations
    linear_commands = (Vector3){
        0.0f,                // X: No strafe for now
        input->vertical,     // Y: Up/down  
        input->thrust        // Z: Forward/backward (positive Z = forward)
    };
    
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

// Process angular input for canyon racing with gyroscopic stabilization
static Vector3 process_canyon_racing_angular(const InputState* input,
                                           struct ControlAuthority* control,
                                           const Vector3* ship_position,
                                           const Quaternion* ship_orientation,
                                           const Vector3* current_angular_velocity) {
    (void)ship_position; // Unused parameter
    (void)ship_orientation; // Unused parameter
    if (!input || !control) return (Vector3){0, 0, 0};
    
    Vector3 angular_commands = {0, 0, 0};
    
    // Direct ship control from input
    float sensitivity = control->control_sensitivity * 0.8f;
    angular_commands.x = input->pitch * sensitivity;
    angular_commands.y = input->yaw * sensitivity;
    angular_commands.z = input->roll * sensitivity;
    
    // GYROSCOPIC STABILIZATION: Apply counter-thrust when no input
    // This simulates RCS thrusters firing to stop rotation
    if (current_angular_velocity) {
        float stabilization_strength = 3.0f; // How aggressively to counter rotation
        float input_deadzone = 0.1f;
        
        // If no pitch input, counter pitch rotation
        if (fabsf(input->pitch) < input_deadzone && fabsf(current_angular_velocity->x) > 0.01f) {
            angular_commands.x = -current_angular_velocity->x * stabilization_strength;
        }
        
        // If no yaw input, counter yaw rotation
        if (fabsf(input->yaw) < input_deadzone && fabsf(current_angular_velocity->y) > 0.01f) {
            angular_commands.y = -current_angular_velocity->y * stabilization_strength;
        }
        
        // If no roll input, counter roll rotation
        if (fabsf(input->roll) < input_deadzone && fabsf(current_angular_velocity->z) > 0.01f) {
            angular_commands.z = -current_angular_velocity->z * stabilization_strength;
        }
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
        struct Physics* physics = entity->physics;
        
        if (!control || !thrusters) continue;

        // Only process input for player-controlled entities
        if (control->controlled_by != INVALID_ENTITY && control->controlled_by == g_player_entity) {
            control_updates++;
            
            // Check if we have any actual input to process
            bool has_input = (input->thrust != 0.0f || 
                            input->pitch != 0.0f || 
                            input->yaw != 0.0f || 
                            input->roll != 0.0f ||
                            input->brake ||
                            input->boost > 0.0f);
            
            // Only override thrust commands if there's actual player input
            // This allows scripted flight to work when player isn't providing input
            if (has_input) {
                // Get ship position and orientation
                Vector3 ship_position = transform ? transform->position : (Vector3){0, 0, 0};
                Quaternion ship_orientation = transform ? transform->rotation : 
                                             (Quaternion){0, 0, 0, 1};
                
                // Get current angular velocity for stabilization
                Vector3* angular_velocity_ptr = NULL;
                if (physics && physics->has_6dof) {
                    angular_velocity_ptr = &physics->angular_velocity;
                }
                
                // Process linear input (thrust)
                Vector3 linear_commands = process_canyon_racing_linear(input, control, 
                                                                     &ship_position, &ship_orientation);
                control->input_linear = linear_commands;
                thruster_set_linear_command(thrusters, linear_commands);
                
                // Process angular input (rotation) with gyroscopic stabilization
                Vector3 angular_commands = process_canyon_racing_angular(input, control, 
                                                                       &ship_position, 
                                                                       &ship_orientation,
                                                                       angular_velocity_ptr);
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
            }
        } else {
            // For non-player entities, only clear commands if they're not being
            // controlled by another system (like scripted flight)
            // Don't override commands that may have been set by other systems
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

// ============================================================================
// UNIFIED SHIP CONFIGURATION
// ============================================================================

void control_configure_ship(struct World* world, EntityID ship_id, ShipConfigPreset preset) {
    if (!world || ship_id == INVALID_ENTITY) return;
    
    struct Physics* physics = entity_get_physics(world, ship_id);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, ship_id);
    struct ControlAuthority* control = entity_get_control_authority(world, ship_id);
    
    if (!physics || !thrusters || !control) {
        printf("⚠️ Ship %d missing required components for configuration\n", ship_id);
        return;
    }
    
    // Always enable 6DOF physics
    physics->has_6dof = true;
    
    // Always set control to be self-controlled by default
    control->controlled_by = ship_id;
    
    switch (preset) {
        case SHIP_CONFIG_FIGHTER:
            // Agile fighter configuration
            physics->mass = 50.0f;
            physics->drag_linear = 0.01f;
            physics->drag_angular = 0.05f;
            physics->moment_of_inertia = (Vector3){0.3f, 0.3f, 0.3f};
            
            thrusters->ship_type = SHIP_TYPE_FIGHTER;
            thrusters->max_linear_force = (Vector3){500, 500, 1000};
            thrusters->max_angular_torque = (Vector3){100, 100, 100};
            thrusters->thrust_response_time = 0.1f;
            thrusters->vacuum_efficiency = 1.0f;
            thrusters->thrusters_enabled = true;
            
            control->control_sensitivity = 1.5f;
            control->stability_assist = 0.3f;
            control->flight_assist_enabled = true;
            control->control_mode = CONTROL_ASSISTED;
            break;
            
        case SHIP_CONFIG_RACER:
            // Canyon racing configuration
            physics->mass = 80.0f;
            physics->drag_linear = 0.99f;  // High drag for arcade feel
            physics->drag_angular = 0.15f;
            physics->moment_of_inertia = (Vector3){0.5f, 0.4f, 0.5f};
            
            thrusters->ship_type = SHIP_TYPE_FIGHTER;
            thrusters->max_linear_force = (Vector3){400, 400, 1200};
            thrusters->max_angular_torque = (Vector3){80, 100, 60};
            thrusters->thrust_response_time = 0.05f;
            thrusters->vacuum_efficiency = 1.0f;
            thrusters->thrusters_enabled = true;
            
            control->control_sensitivity = 2.0f;
            control->stability_assist = 0.5f;
            control->flight_assist_enabled = true;
            control->control_mode = CONTROL_ASSISTED;
            break;
            
        case SHIP_CONFIG_FREIGHTER:
            // Heavy cargo ship
            physics->mass = 500.0f;
            physics->drag_linear = 0.02f;
            physics->drag_angular = 0.1f;
            physics->moment_of_inertia = (Vector3){2.0f, 2.0f, 2.0f};
            
            thrusters->ship_type = SHIP_TYPE_CARGO;
            thrusters->max_linear_force = (Vector3){200, 200, 800};
            thrusters->max_angular_torque = (Vector3){50, 50, 30};
            thrusters->thrust_response_time = 0.3f;
            thrusters->vacuum_efficiency = 0.8f;
            thrusters->thrusters_enabled = true;
            
            control->control_sensitivity = 0.8f;
            control->stability_assist = 0.8f;
            control->flight_assist_enabled = true;
            control->control_mode = CONTROL_ASSISTED;
            break;
            
        case SHIP_CONFIG_RC_ROCKET:
            // RC model rocket (test configuration)
            physics->mass = 10.0f;
            physics->drag_linear = 0.02f;
            physics->drag_angular = 0.3f;
            physics->moment_of_inertia = (Vector3){0.5f, 0.3f, 0.5f};
            
            thrusters->ship_type = SHIP_TYPE_FIGHTER;
            thrusters->max_linear_force = (Vector3){300, 300, 800};
            thrusters->max_angular_torque = (Vector3){50, 80, 50};
            thrusters->thrust_response_time = 0.05f;
            thrusters->vacuum_efficiency = 1.0f;
            thrusters->thrusters_enabled = true;
            
            control->control_sensitivity = 2.0f;
            control->stability_assist = 0.0f;
            control->flight_assist_enabled = false;
            control->control_mode = CONTROL_MANUAL;
            break;
    }
    
    printf("✅ Ship %d configured as %s\n", ship_id,
           preset == SHIP_CONFIG_FIGHTER ? "FIGHTER" :
           preset == SHIP_CONFIG_RACER ? "RACER" :
           preset == SHIP_CONFIG_FREIGHTER ? "FREIGHTER" :
           "RC_ROCKET");
}