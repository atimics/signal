#include "control.h"
#include "input.h"
#include "thrusters.h"
#include <stdio.h>
#include <math.h>

// Global player entity for input processing
static EntityID g_player_entity = INVALID_ENTITY;

// ============================================================================
// CONTROL PROCESSING FUNCTIONS
// ============================================================================

static Vector3 process_linear_input(const InputState* input, struct ControlAuthority* control)
{
    if (!input || !control) return (Vector3){ 0.0f, 0.0f, 0.0f };
    
    Vector3 linear_commands = {
        input->thrust,   // Forward/backward
        input->strafe,   // Left/right
        input->vertical  // Up/down
    };
    
    // Apply sensitivity
    linear_commands = apply_sensitivity_curve(linear_commands, control->control_sensitivity);
    
    // Apply boost multiplier
    if (input->boost > 0.0f) {
        float boost_factor = 1.0f + input->boost * 0.5f; // Up to 50% boost
        linear_commands.x *= boost_factor;
        linear_commands.y *= boost_factor;
        linear_commands.z *= boost_factor;
    }
    
    // Apply brake
    if (input->brake) {
        linear_commands.x *= 0.1f; // Reduce thrust when braking
        linear_commands.y *= 0.1f;
        linear_commands.z *= 0.1f;
    }
    
    return linear_commands;
}

static Vector3 process_angular_input(const InputState* input, struct ControlAuthority* control, struct Physics* physics)
{
    if (!input || !control) return (Vector3){ 0.0f, 0.0f, 0.0f };
    
    Vector3 angular_commands = {
        input->pitch,  // Pitch (nose up/down)
        input->yaw,    // Yaw (turn left/right)  
        input->roll    // Roll (bank left/right)
    };
    
    // Apply sensitivity
    angular_commands = apply_sensitivity_curve(angular_commands, control->control_sensitivity);
    
    // Apply flight assistance if enabled
    if (control->flight_assist_enabled && physics) {
        angular_commands = apply_stability_assist(angular_commands, physics->angular_velocity, control->stability_assist);
    }
    
    return angular_commands;
}

// ============================================================================
// MAIN CONTROL SYSTEM UPDATE
// ============================================================================

void control_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    (void)render_config;  // Unused parameter
    (void)delta_time;     // Unused parameter
    if (!world) return;

    // Get current input state
    const InputState* input = input_get_state();
    if (!input) return;

    uint32_t control_updates = 0;

    // Process all entities with control authority
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];

        if (!(entity->component_mask & COMPONENT_CONTROL_AUTHORITY) ||
            !(entity->component_mask & COMPONENT_THRUSTER_SYSTEM))
        {
            continue;
        }

        struct ControlAuthority* control = entity->control_authority;
        struct ThrusterSystem* thrusters = entity->thruster_system;
        struct Physics* physics = entity->physics; // Optional for flight assist
        
        if (!control || !thrusters) continue;

        // Only process input for player-controlled entities
        if (control->controlled_by != INVALID_ENTITY && control->controlled_by == g_player_entity) {
            control_updates++;
            
            // Process linear input (thrust, strafe, vertical)
            Vector3 linear_commands = process_linear_input(input, control);
            control->input_linear = linear_commands;
            
            // Debug: Print commands being sent to thrusters
            if (linear_commands.x != 0.0f || linear_commands.y != 0.0f || linear_commands.z != 0.0f) {
                printf("🎮 Sending linear commands: [%.2f,%.2f,%.2f] to thrusters\n",
                       linear_commands.x, linear_commands.y, linear_commands.z);
            }
            
            thruster_set_linear_command(thrusters, linear_commands);
            
            // Process angular input (pitch, yaw, roll) - 6DOF
            Vector3 angular_commands = process_angular_input(input, control, physics);
            control->input_angular = angular_commands;
            
            if (angular_commands.x != 0.0f || angular_commands.y != 0.0f || angular_commands.z != 0.0f) {
                printf("🎮 Sending angular commands: [%.2f,%.2f,%.2f] to thrusters\n",
                       angular_commands.x, angular_commands.y, angular_commands.z);
            }
            
            thruster_set_angular_command(thrusters, angular_commands);
            
            // Store boost and brake state
            control->input_boost = input->boost;
            control->input_brake = input->brake;
        } else {
            // Clear commands for non-player entities
            thruster_set_linear_command(thrusters, (Vector3){ 0.0f, 0.0f, 0.0f });
            thruster_set_angular_command(thrusters, (Vector3){ 0.0f, 0.0f, 0.0f });
        }
    }

    // Debug logging (more frequent for troubleshooting)
    static uint32_t log_counter = 0;
    if (++log_counter % 120 == 0)  // Every 2 seconds at 60 FPS
    {
        printf("🎮 Control: %d entities processed, Player: %d\n", 
               control_updates, g_player_entity);
        
        // Print input state for debugging
        if (input) {
            printf("🎮 Input: T:%.2f S:%.2f V:%.2f P:%.2f Y:%.2f R:%.2f\n",
                   input->thrust, input->strafe, input->vertical,
                   input->pitch, input->yaw, input->roll);
        }
    }
}

// ============================================================================
// CONTROL CONFIGURATION FUNCTIONS
// ============================================================================

void control_set_player_entity(struct World* world, EntityID player_entity)
{
    (void)world;  // Unused for now, but may be needed for validation
    g_player_entity = player_entity;
    printf("🎮 Control: Player entity set to %d\n", player_entity);
}

void control_set_sensitivity(struct ControlAuthority* control, float sensitivity)
{
    if (!control) return;
    control->control_sensitivity = fmaxf(0.1f, fminf(5.0f, sensitivity)); // Clamp 0.1-5.0
}

void control_set_flight_assist(struct ControlAuthority* control, bool enabled)
{
    if (!control) return;
    control->flight_assist_enabled = enabled;
}

void control_toggle_flight_assist(struct ControlAuthority* control)
{
    if (!control) return;
    control->flight_assist_enabled = !control->flight_assist_enabled;
    printf("🎮 Flight Assist: %s\n", control->flight_assist_enabled ? "ON" : "OFF");
}

// ============================================================================
// FLIGHT ASSISTANCE FUNCTIONS  
// ============================================================================

Vector3 apply_stability_assist(Vector3 input, Vector3 current_angular_velocity, float assist_strength)
{
    // Apply counter-rotation to reduce angular velocity when no input is given
    Vector3 assisted_input = input;
    
    if (assist_strength > 0.0f) {
        // If there's no input on an axis, apply counter-rotation to stabilize
        if (fabsf(input.x) < 0.1f) { // Pitch
            assisted_input.x -= current_angular_velocity.x * assist_strength * 0.5f;
        }
        if (fabsf(input.y) < 0.1f) { // Yaw
            assisted_input.y -= current_angular_velocity.y * assist_strength * 0.5f;
        }
        if (fabsf(input.z) < 0.1f) { // Roll
            assisted_input.z -= current_angular_velocity.z * assist_strength * 0.5f;
        }
    }
    
    // Clamp the result to valid input range
    assisted_input.x = fmaxf(-1.0f, fminf(1.0f, assisted_input.x));
    assisted_input.y = fmaxf(-1.0f, fminf(1.0f, assisted_input.y));
    assisted_input.z = fmaxf(-1.0f, fminf(1.0f, assisted_input.z));
    
    return assisted_input;
}

Vector3 apply_sensitivity_curve(Vector3 input, float sensitivity)
{
    // Apply smooth response curve for better control feel
    Vector3 result;
    
    // Use cubic curve for smooth response near center, more aggressive at extremes
    result.x = input.x * input.x * input.x * (input.x > 0 ? 1.0f : -1.0f) * sensitivity;
    result.y = input.y * input.y * input.y * (input.y > 0 ? 1.0f : -1.0f) * sensitivity;
    result.z = input.z * input.z * input.z * (input.z > 0 ? 1.0f : -1.0f) * sensitivity;
    
    // Clamp to valid range
    result.x = fmaxf(-1.0f, fminf(1.0f, result.x));
    result.y = fmaxf(-1.0f, fminf(1.0f, result.y));
    result.z = fmaxf(-1.0f, fminf(1.0f, result.z));
    
    return result;
}