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

// Enhanced sensitivity curve with velocity-adaptive response
static float apply_adaptive_sensitivity(float input, float base_sensitivity, float current_velocity, float max_velocity) {
    if (fabsf(input) < 0.001f) return 0.0f;
    
    // Velocity-based sensitivity scaling: higher sensitivity at low speeds for precision
    float velocity_factor = 1.0f;
    if (max_velocity > 0.0f) {
        float velocity_ratio = fabsf(current_velocity) / max_velocity;
        velocity_factor = 1.5f - (velocity_ratio * 0.5f); // 1.5x at standstill, 1.0x at max speed
    }
    
    // Adaptive response curve: more linear for small inputs, more aggressive for large inputs
    float abs_input = fabsf(input);
    float sign = input > 0.0f ? 1.0f : -1.0f;
    
    float response;
    if (abs_input < 0.3f) {
        // Linear response for small inputs (precision zone)
        response = abs_input * 1.8f;
    } else {
        // Smooth curve for larger inputs
        float normalized = (abs_input - 0.3f) / 0.7f; // Map 0.3-1.0 to 0-1
        response = 0.54f + (normalized * normalized * 0.46f); // Start at 0.54, curve to 1.0
    }
    
    return sign * response * base_sensitivity * velocity_factor;
}

// Turn rate limiter to prevent over-rotation
static Vector3 apply_turn_rate_limiting(Vector3 input, Vector3 previous_angular, float delta_limit) {
    Vector3 limited = input;
    
    // Limit rate of change for each axis
    limited.x = fmaxf(previous_angular.x - delta_limit, fminf(previous_angular.x + delta_limit, input.x));
    limited.y = fmaxf(previous_angular.y - delta_limit, fminf(previous_angular.y + delta_limit, input.y));
    limited.z = fmaxf(previous_angular.z - delta_limit, fminf(previous_angular.z + delta_limit, input.z));
    
    return limited;
}

// Gamepad-specific input processing with separate tuning
static Vector3 process_gamepad_angular_input(const InputState* input, float sensitivity, Vector3 current_velocity) {
    Vector3 result = { 0.0f, 0.0f, 0.0f };
    
    // Enhanced gamepad sensitivity for canyon racing
    const float gamepad_pitch_sensitivity = sensitivity * 1.2f;
    const float gamepad_yaw_sensitivity = sensitivity * 1.4f;  // Higher for quick turns
    const float gamepad_roll_sensitivity = sensitivity * 1.6f;  // Highest for banking
    
    // Apply adaptive curves to each axis
    result.x = apply_adaptive_sensitivity(input->pitch, gamepad_pitch_sensitivity, current_velocity.x, 5.0f);
    result.y = apply_adaptive_sensitivity(input->yaw, gamepad_yaw_sensitivity, current_velocity.y, 5.0f);
    result.z = apply_adaptive_sensitivity(input->roll, gamepad_roll_sensitivity, current_velocity.z, 5.0f);
    
    return result;
}

static Vector3 process_linear_input(const InputState* input, struct ControlAuthority* control)
{
    if (!input || !control) return (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Canyon racer flight model - banking creates natural turn forces
    float banking_input = input->strafe;
    
    Vector3 linear_commands = {
        banking_input * 0.2f, // Slight lateral force from banking (like centrifugal force)
        input->vertical,      // Up/down -> Y-axis (vertical thrust)
        -input->thrust        // Forward/backward -> Z-axis (negative Z = forward away from camera)
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
    
    static Vector3 previous_angular = { 0.0f, 0.0f, 0.0f };
    
    // Check if this is gamepad input (more sophisticated detection)
    bool is_gamepad_input = (fabsf(input->pitch) < 1.0f && fabsf(input->pitch) > 0.0f) ||
                           (fabsf(input->yaw) < 1.0f && fabsf(input->yaw) > 0.0f) ||
                           (fabsf(input->roll) < 1.0f && fabsf(input->roll) > 0.0f);
    
    Vector3 angular_commands;
    
    if (is_gamepad_input) {
        // Use enhanced gamepad processing
        Vector3 current_angular_velocity = physics ? physics->angular_velocity : (Vector3){ 0.0f, 0.0f, 0.0f };
        angular_commands = process_gamepad_angular_input(input, control->control_sensitivity, current_angular_velocity);
        
        // Apply turn rate limiting for smooth gamepad control
        angular_commands = apply_turn_rate_limiting(angular_commands, previous_angular, 0.15f); // Limit to 15% change per frame
    } else {
        // Canyon racer banking flight model for keyboard
        float banking_input = input->strafe;
        float banking_strength = 1.5f;
        
        angular_commands = (Vector3){
            input->pitch,                    // X = Pitch (nose up/down)
            banking_input * banking_strength + input->yaw, // Y = Banking + direct yaw
            banking_input * banking_strength + input->roll  // Z = Banking + direct roll
        };
        
        // Apply standard sensitivity curve for keyboard
        angular_commands = apply_sensitivity_curve(angular_commands, control->control_sensitivity);
    }
    
    // Store for next frame's rate limiting
    previous_angular = angular_commands;
    
    // Debug angular commands (less frequent)
    static uint32_t angular_debug = 0;
    if ((fabsf(angular_commands.x) > 0.01f || fabsf(angular_commands.y) > 0.01f || fabsf(angular_commands.z) > 0.01f) && ++angular_debug % 60 == 0) {
        printf("🏎️ Angular [%s]: P:%.2f Y:%.2f R:%.2f\n", 
               is_gamepad_input ? "GAMEPAD" : "KEYBOARD",
               angular_commands.x, angular_commands.y, angular_commands.z);
    }
    
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
            
            thruster_set_linear_command(thrusters, linear_commands);
            
            // Process angular input (pitch, yaw, roll) - 6DOF
            Vector3 angular_commands = process_angular_input(input, control, physics);
            control->input_angular = angular_commands;
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
    if (++log_counter % 60 == 0)  // Every second at 60 FPS for more frequent debugging
    {
        printf("🎮 Control: %d entities processed, Player: %d\n", 
               control_updates, g_player_entity);
        
        // Print input state for debugging
        if (input) {
            printf("🎮 Input: T:%.2f S:%.2f V:%.2f P:%.2f Y:%.2f R:%.2f B:%s Boost:%.2f\n",
                   input->thrust, input->strafe, input->vertical,
                   input->pitch, input->yaw, input->roll,
                   input->brake ? "ON" : "OFF", input->boost);
            
            // Debug any non-zero input
            if (input->thrust != 0.0f || input->strafe != 0.0f || input->vertical != 0.0f) {
                printf("🚀 Movement input detected: T:%.2f S:%.2f V:%.2f\n", 
                       input->thrust, input->strafe, input->vertical);
            }
            
            // Debug strafe specifically since left/right asymmetry reported
            if (input->strafe != 0.0f) {
                printf("↔️ STRAFE: %.2f (A=LEFT=-1, D=RIGHT=+1)\n", input->strafe);
            }
            
            // Debug angular controls specifically
            if (input->pitch != 0.0f || input->yaw != 0.0f || input->roll != 0.0f) {
                printf("🔄 Angular input: P:%.2f Y:%.2f R:%.2f\n", 
                       input->pitch, input->yaw, input->roll);
            }
            
            // Debug yaw specifically since it's not working
            if (input->yaw != 0.0f) {
                printf("➡️ YAW: %.2f (LEFT=-1, RIGHT=+1)\n", input->yaw);
            }
        } else {
            printf("❌ Control: No input state available!\n");
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
    // Simplified sensitivity curve for keyboard input (less aggressive than before)
    Vector3 result;
    
    // Use quadratic curve for smooth response - less aggressive than cubic
    float sign_x = input.x > 0 ? 1.0f : -1.0f;
    float sign_y = input.y > 0 ? 1.0f : -1.0f;
    float sign_z = input.z > 0 ? 1.0f : -1.0f;
    
    result.x = input.x * fabsf(input.x) * sign_x * sensitivity * 0.8f; // Slightly reduced
    result.y = input.y * fabsf(input.y) * sign_y * sensitivity * 0.8f;
    result.z = input.z * fabsf(input.z) * sign_z * sensitivity * 0.8f;
    
    // Clamp to valid range
    result.x = fmaxf(-1.0f, fminf(1.0f, result.x));
    result.y = fmaxf(-1.0f, fminf(1.0f, result.y));
    result.z = fmaxf(-1.0f, fminf(1.0f, result.z));
    
    return result;
}