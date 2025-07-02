#include "thrusters.h"
#include "physics.h"
#include "../core.h"
#include <stdio.h>
#include <math.h>

// ============================================================================
// THRUSTER FORCE CALCULATION
// ============================================================================

// Future enhancement: Thrust response time modeling
// static Vector3 apply_thrust_response(Vector3 current, Vector3 target, float response_time, float delta_time)

static Vector3 calculate_linear_force(struct ThrusterSystem* thrusters, struct Physics* physics, float efficiency)
{
    if (!thrusters->thrusters_enabled) {
        return (Vector3){ 0.0f, 0.0f, 0.0f };
    }
    
    Vector3 thrust_force = {
        thrusters->current_linear_thrust.x * thrusters->max_linear_force.x * efficiency,
        thrusters->current_linear_thrust.y * thrusters->max_linear_force.y * efficiency,
        thrusters->current_linear_thrust.z * thrusters->max_linear_force.z * efficiency
    };
    
    // Add automatic deceleration when no thrust commanded
    if (physics && thrusters->auto_deceleration) {
        const float decel_strength = 0.05f; // 5% of max thrust for gentle deceleration
        const float velocity_threshold = 2.0f; // Only decelerate above this speed
        
        // If no thrust commanded and velocity exists, apply counter-thrust
        if (fabsf(thrusters->current_linear_thrust.x) < 0.1f && fabsf(physics->velocity.x) > velocity_threshold) {
            float decel_factor = fminf(fabsf(physics->velocity.x) / 50.0f, 1.0f); // Scale with velocity
            thrust_force.x -= physics->velocity.x * thrusters->max_linear_force.x * decel_strength * decel_factor;
        }
        if (fabsf(thrusters->current_linear_thrust.y) < 0.1f && fabsf(physics->velocity.y) > velocity_threshold) {
            float decel_factor = fminf(fabsf(physics->velocity.y) / 50.0f, 1.0f);
            thrust_force.y -= physics->velocity.y * thrusters->max_linear_force.y * decel_strength * decel_factor;
        }
        if (fabsf(thrusters->current_linear_thrust.z) < 0.1f && fabsf(physics->velocity.z) > velocity_threshold) {
            float decel_factor = fminf(fabsf(physics->velocity.z) / 50.0f, 1.0f);
            thrust_force.z -= physics->velocity.z * thrusters->max_linear_force.z * decel_strength * decel_factor;
        }
    }
    
    return thrust_force;
}

static Vector3 calculate_angular_torque(struct ThrusterSystem* thrusters, struct Physics* physics, float efficiency)
{
    if (!thrusters->thrusters_enabled || !physics->has_6dof) {
        return (Vector3){ 0.0f, 0.0f, 0.0f };
    }
    
    return (Vector3){
        thrusters->current_angular_thrust.x * thrusters->max_angular_torque.x * efficiency,
        thrusters->current_angular_thrust.y * thrusters->max_angular_torque.y * efficiency,
        thrusters->current_angular_thrust.z * thrusters->max_angular_torque.z * efficiency
    };
}

// ============================================================================
// MAIN THRUSTER SYSTEM UPDATE
// ============================================================================

void thruster_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    (void)render_config;  // Unused parameter
    (void)delta_time;     // Unused parameter
    if (!world) return;

    uint32_t thruster_updates = 0;
    uint32_t force_applications = 0;

    // Update all entities with thruster systems
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];

        if (!(entity->component_mask & COMPONENT_THRUSTER_SYSTEM) ||
            !(entity->component_mask & COMPONENT_PHYSICS) ||
            !(entity->component_mask & COMPONENT_TRANSFORM))
        {
            continue;
        }

        struct ThrusterSystem* thrusters = entity->thruster_system;
        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;
        
        if (!thrusters || !physics || !transform) continue;

        thruster_updates++;

        // Calculate environmental efficiency
        float efficiency = thruster_calculate_efficiency(thrusters, physics->environment);
        
        // Calculate linear forces in ship-local space
        Vector3 linear_force = calculate_linear_force(thrusters, physics, efficiency);
        if (linear_force.x != 0.0f || linear_force.y != 0.0f || linear_force.z != 0.0f) {
            // Transform force from ship-local space to world space
            Vector3 world_force = quaternion_rotate_vector(transform->rotation, linear_force);
            physics_add_force(physics, world_force);
            force_applications++;
        }
        
        // Calculate angular torques (6DOF)
        Vector3 angular_torque = calculate_angular_torque(thrusters, physics, efficiency);
        if (angular_torque.x != 0.0f || angular_torque.y != 0.0f || angular_torque.z != 0.0f) {
            physics_add_torque(physics, angular_torque);
            force_applications++;
        }
    }

    // Debug logging (more frequent for troubleshooting)
    static uint32_t log_counter = 0;
    if (++log_counter % 120 == 0)  // Every 2 seconds at 60 FPS
    {
        printf("🚀 Thrusters: %d updates, %d force applications\n", 
               thruster_updates, force_applications);
    }
}

// ============================================================================
// THRUSTER CONTROL FUNCTIONS
// ============================================================================

void thruster_set_linear_command(struct ThrusterSystem* thrusters, Vector3 command)
{
    if (!thrusters) return;
    
    // Clamp command values to [-1, 1]
    thrusters->current_linear_thrust = (Vector3){
        fmaxf(-1.0f, fminf(1.0f, command.x)),
        fmaxf(-1.0f, fminf(1.0f, command.y)),
        fmaxf(-1.0f, fminf(1.0f, command.z))
    };
}

void thruster_set_angular_command(struct ThrusterSystem* thrusters, Vector3 command)
{
    if (!thrusters) return;
    
    // Clamp command values to [-1, 1]
    thrusters->current_angular_thrust = (Vector3){
        fmaxf(-1.0f, fminf(1.0f, command.x)),
        fmaxf(-1.0f, fminf(1.0f, command.y)),
        fmaxf(-1.0f, fminf(1.0f, command.z))
    };
}

void thruster_set_enabled(struct ThrusterSystem* thrusters, bool enabled)
{
    if (!thrusters) return;
    thrusters->thrusters_enabled = enabled;
    
    if (!enabled) {
        // Clear thrust commands when disabled
        thrusters->current_linear_thrust = (Vector3){ 0.0f, 0.0f, 0.0f };
        thrusters->current_angular_thrust = (Vector3){ 0.0f, 0.0f, 0.0f };
    }
}

float thruster_calculate_efficiency(struct ThrusterSystem* thrusters, int environment)
{
    if (!thrusters) return 0.0f;
    
    switch (environment) {
        case PHYSICS_SPACE:
            return thrusters->vacuum_efficiency;
        case PHYSICS_ATMOSPHERE:
            return thrusters->atmosphere_efficiency;
        default:
            return 1.0f;
    }
}

// ============================================================================
// SPRINT 21: SHIP TYPE CONFIGURATION
// ============================================================================

void thruster_configure_ship_type(struct ThrusterSystem* thrusters, ShipType ship_type, float base_thrust)
{
    if (!thrusters) return;
    
    thrusters->ship_type = ship_type;
    
    // Configure ship characteristics based on type
    switch (ship_type) {
        case SHIP_TYPE_FIGHTER:
            // Fast, agile, balanced
            thrusters->max_linear_force = (Vector3){
                base_thrust * 1.3f,  // X = Left/right strafe: good strafing  
                base_thrust * 1.5f,  // Y = Up/down: excellent maneuverability
                base_thrust * 1.2f   // Z = Forward/back: slightly above average
            };
            thrusters->max_angular_torque = (Vector3){ 20.0f, 25.0f, 15.0f };  // Much higher for responsiveness
            thrusters->thrust_response_time = 0.05f;  // Very responsive
            thrusters->power_efficiency = 0.9f;       // Good efficiency
            thrusters->heat_generation = 0.7f;        // Moderate heat
            break;
            
        case SHIP_TYPE_INTERCEPTOR:
            // Very fast forward, minimal lateral
            thrusters->max_linear_force = (Vector3){
                base_thrust * 0.6f,  // X = Left/right: poor strafing
                base_thrust * 0.8f,  // Y = Up/down: limited maneuverability
                base_thrust * 2.0f   // Z = Forward/back: exceptional speed
            };
            thrusters->max_angular_torque = (Vector3){ 35.0f, 20.0f, 15.0f };
            thrusters->thrust_response_time = 0.03f;  // Extremely responsive
            thrusters->power_efficiency = 0.8f;       // Less efficient at full power
            thrusters->heat_generation = 1.2f;        // High heat generation
            break;
            
        case SHIP_TYPE_CARGO:
            // Slow but powerful, heavy
            thrusters->max_linear_force = (Vector3){
                base_thrust * 1.0f,  // X = Left/right: standard
                base_thrust * 1.8f,  // Y = Up/down: overcome heavy mass
                base_thrust * 1.5f   // Z = Forward/back: good for mass
            };
            thrusters->max_angular_torque = (Vector3){ 12.0f, 12.0f, 8.0f };
            thrusters->thrust_response_time = 0.15f;  // Slow response
            thrusters->power_efficiency = 1.1f;       // Very efficient
            thrusters->heat_generation = 0.4f;        // Low heat (efficient)
            break;
            
        case SHIP_TYPE_EXPLORER:
            // Balanced, efficient
            thrusters->max_linear_force = (Vector3){
                base_thrust * 1.0f,  // X = Left/right: standard
                base_thrust * 1.0f,  // Y = Up/down: standard
                base_thrust * 1.0f   // Z = Forward/back: standard
            };
            thrusters->max_angular_torque = (Vector3){ 18.0f, 18.0f, 15.0f };
            thrusters->thrust_response_time = 0.08f;  // Good response
            thrusters->power_efficiency = 1.2f;       // Excellent efficiency
            thrusters->heat_generation = 0.5f;        // Low heat
            break;
            
        case SHIP_TYPE_CUSTOM:
        default:
            // Keep existing configuration
            thrusters->power_efficiency = 1.0f;
            thrusters->heat_generation = 0.6f;
            break;
    }
    
    // Common settings
    thrusters->atmosphere_efficiency = 1.0f;
    thrusters->vacuum_efficiency = 1.0f;
    thrusters->thrusters_enabled = true;
}

void thruster_apply_ship_characteristics(struct ThrusterSystem* thrusters, struct Physics* physics)
{
    if (!thrusters || !physics) return;
    
    // Apply ship type effects to physics
    switch (thrusters->ship_type) {
        case SHIP_TYPE_FIGHTER:
            // Fighters have lower drag for agility
            physics->drag_linear = fmaxf(physics->drag_linear * 0.98f, 0.95f);
            physics->drag_angular = fmaxf(physics->drag_angular * 0.95f, 0.85f);
            break;
            
        case SHIP_TYPE_INTERCEPTOR:
            // Interceptors have very low linear drag, higher angular drag
            physics->drag_linear = fmaxf(physics->drag_linear * 0.96f, 0.90f);
            physics->drag_angular = fminf(physics->drag_angular * 1.1f, 0.95f);
            break;
            
        case SHIP_TYPE_CARGO:
            // Cargo ships have higher drag due to mass and design
            physics->drag_linear = fminf(physics->drag_linear * 1.02f, 0.99f);
            physics->drag_angular = fminf(physics->drag_angular * 1.05f, 0.95f);
            break;
            
        case SHIP_TYPE_EXPLORER:
            // Explorers maintain standard drag
            break;
            
        case SHIP_TYPE_CUSTOM:
        default:
            // No modifications for custom ships
            break;
    }
}