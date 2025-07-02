#include "thrusters.h"
#include "physics.h"
#include <stdio.h>
#include <math.h>

// ============================================================================
// THRUSTER FORCE CALCULATION
// ============================================================================

// Future enhancement: Thrust response time modeling
// static Vector3 apply_thrust_response(Vector3 current, Vector3 target, float response_time, float delta_time)

static Vector3 calculate_linear_force(struct ThrusterSystem* thrusters, struct Physics* physics, float efficiency)
{
    (void)physics;  // Unused parameter
    if (!thrusters->thrusters_enabled) {
        return (Vector3){ 0.0f, 0.0f, 0.0f };
    }
    
    return (Vector3){
        thrusters->current_linear_thrust.x * thrusters->max_linear_force.x * efficiency,
        thrusters->current_linear_thrust.y * thrusters->max_linear_force.y * efficiency,
        thrusters->current_linear_thrust.z * thrusters->max_linear_force.z * efficiency
    };
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
            !(entity->component_mask & COMPONENT_PHYSICS))
        {
            continue;
        }

        struct ThrusterSystem* thrusters = entity->thruster_system;
        struct Physics* physics = entity->physics;
        
        if (!thrusters || !physics) continue;

        thruster_updates++;

        // Calculate environmental efficiency
        float efficiency = thruster_calculate_efficiency(thrusters, physics->environment);
        
        // Calculate linear forces
        Vector3 linear_force = calculate_linear_force(thrusters, physics, efficiency);
        if (linear_force.x != 0.0f || linear_force.y != 0.0f || linear_force.z != 0.0f) {
            printf("🚀 Applying force: [%.2f,%.2f,%.2f] from thrust [%.2f,%.2f,%.2f]\n",
                   linear_force.x, linear_force.y, linear_force.z,
                   thrusters->current_linear_thrust.x, thrusters->current_linear_thrust.y, thrusters->current_linear_thrust.z);
            physics_add_force(physics, linear_force);
            force_applications++;
        }
        
        // Calculate angular torques (6DOF)
        Vector3 angular_torque = calculate_angular_torque(thrusters, physics, efficiency);
        if (angular_torque.x != 0.0f || angular_torque.y != 0.0f || angular_torque.z != 0.0f) {
            printf("🌀 Applying torque: [%.2f,%.2f,%.2f] from thrust [%.2f,%.2f,%.2f]\n",
                   angular_torque.x, angular_torque.y, angular_torque.z,
                   thrusters->current_angular_thrust.x, thrusters->current_angular_thrust.y, thrusters->current_angular_thrust.z);
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