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

// Process linear input for canyon racing with auto-stop
static Vector3 process_canyon_racing_linear(const InputState* input, 
                                          struct ControlAuthority* control,
                                          const Vector3* ship_position,
                                          const Quaternion* ship_orientation,
                                          const Vector3* current_velocity,
                                          const Quaternion* ship_rotation) {
    (void)ship_position; // Unused for now
    (void)ship_orientation; // Unused for now
    if (!input || !control) return (Vector3){0, 0, 0};
    
    Vector3 linear_commands = {0, 0, 0};
    
    // Check if we have thrust input
    bool has_thrust_input = (fabsf(input->thrust) > 0.01f || fabsf(input->vertical) > 0.01f);
    
    if (has_thrust_input) {
        // Manual thrust control
        linear_commands = (Vector3){
            0.0f,                // X: No strafe for now
            input->vertical,     // Y: Up/down  
            input->thrust        // Z: Forward/backward (positive Z = forward)
        };
        
        // Apply boost
        if (input->boost > 0.0f) {
            float boost_factor = 1.0f + input->boost * 2.0f; // 3x boost (reduced from 4x)
            linear_commands = vector3_multiply_scalar(linear_commands, boost_factor);
        }
    } else if (current_velocity && ship_rotation) {
        // AUTO-STOP: Apply counter-thrust to slow down when no input
        // Transform world velocity to ship-local space
        Vector3 ship_forward = quaternion_rotate_vector(*ship_rotation, (Vector3){0, 0, 1});
        Vector3 ship_right = quaternion_rotate_vector(*ship_rotation, (Vector3){1, 0, 0});
        Vector3 ship_up = quaternion_rotate_vector(*ship_rotation, (Vector3){0, 1, 0});
        
        // Calculate velocity in ship's local coordinate system
        float forward_velocity = vector3_dot(*current_velocity, ship_forward);
        float right_velocity = vector3_dot(*current_velocity, ship_right);
        float up_velocity = vector3_dot(*current_velocity, ship_up);
        
        // Auto-stop parameters - extremely gentle to eliminate jittering
        float auto_stop_strength = 0.05f; // Extremely gentle auto-stop to prevent oscillations
        float velocity_threshold = 1.0f; // Much higher threshold to prevent micro-corrections
        
        // Apply counter-thrust proportional to velocity
        if (fabsf(forward_velocity) > velocity_threshold) {
            linear_commands.z = -forward_velocity * auto_stop_strength;
        }
        if (fabsf(right_velocity) > velocity_threshold) {
            linear_commands.x = -right_velocity * auto_stop_strength;
        }
        if (fabsf(up_velocity) > velocity_threshold) {
            linear_commands.y = -up_velocity * auto_stop_strength;
        }
        
        // Clamp auto-stop forces to reasonable limits
        linear_commands.x = fmaxf(-0.5f, fminf(0.5f, linear_commands.x));
        linear_commands.y = fmaxf(-0.5f, fminf(0.5f, linear_commands.y));
        linear_commands.z = fmaxf(-0.5f, fminf(0.5f, linear_commands.z));
    }
    
    // Apply brake with intensity (overrides auto-stop)
    if (input->brake) {
        float brake_strength = 2.0f; // Base brake strength
        
        // Use brake intensity if available (from analog trigger)
        if (input->brake_intensity > 0.0f) {
            brake_strength *= input->brake_intensity;
        }
        
        // Enhanced braking - stronger than auto-stop
        if (current_velocity && ship_rotation) {
            Vector3 ship_forward = quaternion_rotate_vector(*ship_rotation, (Vector3){0, 0, 1});
            Vector3 ship_right = quaternion_rotate_vector(*ship_rotation, (Vector3){1, 0, 0});
            Vector3 ship_up = quaternion_rotate_vector(*ship_rotation, (Vector3){0, 1, 0});
            
            float forward_velocity = vector3_dot(*current_velocity, ship_forward);
            float right_velocity = vector3_dot(*current_velocity, ship_right);
            float up_velocity = vector3_dot(*current_velocity, ship_up);
            
            // Proportional braking based on brake intensity
            linear_commands.x = -right_velocity * brake_strength;
            linear_commands.y = -up_velocity * brake_strength;
            linear_commands.z = -forward_velocity * brake_strength;
        } else {
            // Fallback if no velocity data
            linear_commands = vector3_multiply_scalar(linear_commands, 0.1f);
        }
    }
    
    // Add strafe controls for zero-g maneuvering
    if (input->strafe_left > 0.0f) {
        linear_commands.x -= input->strafe_left * 0.5f; // Strafe left
    }
    if (input->strafe_right > 0.0f) {
        linear_commands.x += input->strafe_right * 0.5f; // Strafe right
    }
    
    return linear_commands;
}

// Process angular input for canyon racing with enhanced stabilization
static Vector3 process_canyon_racing_angular(const InputState* input,
                                           struct ControlAuthority* control,
                                           const Vector3* ship_position,
                                           const Quaternion* ship_orientation,
                                           const Vector3* current_angular_velocity) {
    (void)ship_position; // Unused parameter
    (void)ship_orientation; // Unused parameter
    if (!input || !control) return (Vector3){0, 0, 0};
    
    Vector3 angular_commands = {0, 0, 0};
    
    // Extremely gentle sensitivity for zero-g stability
    float base_sensitivity = control->control_sensitivity * 0.15f; // Extremely gentle to prevent jittering
    
    // Direct linear input for predictable control
    angular_commands.x = input->pitch * base_sensitivity;
    angular_commands.y = input->yaw * base_sensitivity;
    angular_commands.z = input->roll * base_sensitivity;
    
    // EXTREMELY GENTLE ZERO-G STABILIZATION - prevent oscillations
    if (current_angular_velocity) {
        float stabilization_strength = 0.5f; // Extremely gentle stabilization
        float input_deadzone = 0.15f; // Large deadzone to prevent fighting user input
        float velocity_threshold = 0.1f; // Only stabilize significant rotations
        
        // Only apply stabilization if there's no user input and significant velocity
        if (fabsf(input->pitch) < input_deadzone && fabsf(current_angular_velocity->x) > velocity_threshold) {
            float damping = -current_angular_velocity->x * stabilization_strength;
            angular_commands.x += damping;
        }
        
        if (fabsf(input->yaw) < input_deadzone && fabsf(current_angular_velocity->y) > velocity_threshold) {
            float damping = -current_angular_velocity->y * stabilization_strength;
            angular_commands.y += damping;
        }
        
        if (fabsf(input->roll) < input_deadzone && fabsf(current_angular_velocity->z) > velocity_threshold) {
            float damping = -current_angular_velocity->z * stabilization_strength;
            angular_commands.z += damping;
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
        // Check if this entity is the player (either by ID match or self-controlled player)
        bool is_player_controlled = (entity->id == g_player_entity) || 
                                   (control->controlled_by == entity->id && entity->id == g_player_entity);
        
        if (is_player_controlled) {
            control_updates++;
            
            // Debug output
            static int entity_debug_counter = 0;
            if (++entity_debug_counter % 60 == 0) {
                printf("🎮 DEBUG: Control processing entity %d (player=%d, controlled_by=%d)\n", 
                       entity->id, g_player_entity, control->controlled_by);
            }
            
            // Check if we have any actual input to process (including zero-g controls)
            bool has_input = (input->thrust != 0.0f || 
                            input->pitch != 0.0f || 
                            input->yaw != 0.0f || 
                            input->roll != 0.0f ||
                            input->brake ||
                            input->boost > 0.0f ||
                            input->strafe_left > 0.0f ||
                            input->strafe_right > 0.0f);
            
            // Only override thrust commands if there's actual player input
            // This allows scripted flight to work when player isn't providing input
            if (has_input) {
                // Get ship position, orientation, and velocity for enhanced control
                Vector3 ship_position = transform ? transform->position : (Vector3){0, 0, 0};
                Quaternion ship_orientation = transform ? transform->rotation : 
                                             (Quaternion){0, 0, 0, 1};
                
                // Get current velocities for stabilization and auto-stop
                Vector3* angular_velocity_ptr = NULL;
                Vector3* linear_velocity_ptr = NULL;
                if (physics && physics->has_6dof) {
                    angular_velocity_ptr = &physics->angular_velocity;
                    linear_velocity_ptr = &physics->velocity;
                }
                
                // Process linear input (thrust) with auto-stop functionality
                Vector3 linear_commands = process_canyon_racing_linear(input, control, 
                                                                     &ship_position, &ship_orientation,
                                                                     linear_velocity_ptr, &ship_orientation);
                control->input_linear = linear_commands;
                thruster_set_linear_command(thrusters, linear_commands);
                
                // Process angular input (rotation) with enhanced stabilization
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
            } else {
                // No player input - log this
                static int no_input_counter = 0;
                if (++no_input_counter % 60 == 0) {
                    printf("🎮 No player input - scripted flight can control entity %d\n", entity->id);
                    // Check current thruster state
                    printf("  Current thrust: [%.2f,%.2f,%.2f]\n",
                           thrusters->current_linear_thrust.x,
                           thrusters->current_linear_thrust.y,
                           thrusters->current_linear_thrust.z);
                }
            }
        } else {
            // For non-player entities, DO NOT clear thrust commands
            // This allows scripted flight and other systems to control the entity
            static int non_player_counter = 0;
            if (++non_player_counter % 300 == 0 && entity->id < 10) { // Log every 5 seconds for first 10 entities
                printf("🎮 Non-player entity %d (controlled_by=%d, player=%d) - NOT clearing thrust\n",
                       entity->id, control->controlled_by, g_player_entity);
            }
            // Remove the clearing of thrust commands - let other systems control the entity
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
            // Zero-G STABILITY optimized configuration
            physics->mass = 120.0f; // Increased mass for stability
            physics->drag_linear = 0.08f;   // Higher drag for natural damping
            physics->drag_angular = 0.25f;  // Much higher angular drag for stability
            physics->moment_of_inertia = (Vector3){0.8f, 0.6f, 0.8f}; // Higher inertia for stability
            
            thrusters->ship_type = SHIP_TYPE_FIGHTER;
            thrusters->max_linear_force = (Vector3){400, 400, 600}; // Reduced thrust for gentleness
            thrusters->max_angular_torque = (Vector3){80, 90, 60}; // Much lower torque for stability
            thrusters->thrust_response_time = 0.1f; // Slower response for smoothness
            thrusters->vacuum_efficiency = 1.0f;
            thrusters->thrusters_enabled = true;
            
            control->control_sensitivity = 0.6f; // Lower sensitivity for stability
            control->stability_assist = 0.9f;    // Very strong assist for zero-g
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
            // Zero-G test rocket (lightweight and agile)
            physics->mass = 8.0f;
            physics->drag_linear = 0.005f;  // Minimal drag for true zero-g
            physics->drag_angular = 0.02f;  // Minimal angular drag
            physics->moment_of_inertia = (Vector3){0.2f, 0.15f, 0.2f}; // Very low inertia
            
            thrusters->ship_type = SHIP_TYPE_FIGHTER;
            thrusters->max_linear_force = (Vector3){400, 400, 600}; // Balanced for test flights
            thrusters->max_angular_torque = (Vector3){80, 100, 60}; // Good maneuverability
            thrusters->thrust_response_time = 0.02f; // Instant response
            thrusters->vacuum_efficiency = 1.0f;
            thrusters->thrusters_enabled = true;
            
            control->control_sensitivity = 0.8f; // Stable but responsive
            control->stability_assist = 0.5f;    // Some assist but not overwhelming
            control->flight_assist_enabled = true;
            control->control_mode = CONTROL_ASSISTED;
            break;
    }
    
    printf("✅ Ship %d configured as %s\n", ship_id,
           preset == SHIP_CONFIG_FIGHTER ? "FIGHTER" :
           preset == SHIP_CONFIG_RACER ? "RACER" :
           preset == SHIP_CONFIG_FREIGHTER ? "FREIGHTER" :
           "RC_ROCKET");
}