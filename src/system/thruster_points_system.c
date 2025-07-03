#include "thruster_points_system.h"
#include "../component/thruster_points_component.h"
#include "physics.h"
#include "control.h"
#include <stdio.h>

// System update function that processes all entities with thruster points
void thruster_points_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config;
    (void)delta_time;
    
    if (!world) return;
    
    // For now, we'll handle the legacy ThrusterSystem component
    // In the future, we can migrate to a pure thruster points approach
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Skip if entity doesn't have the required components
        if (!(entity->component_mask & COMPONENT_TRANSFORM) ||
            !(entity->component_mask & COMPONENT_PHYSICS) ||
            !(entity->component_mask & COMPONENT_THRUSTER_SYSTEM)) {
            continue;
        }
        
        thruster_points_apply_legacy_commands(world, entity->id);
    }
}

// Convert legacy ThrusterSystem commands to thruster points
void thruster_points_apply_legacy_commands(struct World* world, EntityID entity_id) {
    struct ThrusterSystem* old_thrusters = entity_get_thruster_system(world, entity_id);
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Physics* physics = entity_get_physics(world, entity_id);
    
    if (!old_thrusters || !transform || !physics || !old_thrusters->thrusters_enabled) {
        return;
    }
    
    // Calculate efficiency based on environment
    float efficiency = (physics->environment == PHYSICS_SPACE) ? 
                      old_thrusters->vacuum_efficiency : 
                      old_thrusters->atmosphere_efficiency;
    
    // Apply linear forces
    Vector3 linear_force = {
        old_thrusters->current_linear_thrust.x * old_thrusters->max_linear_force.x * efficiency,
        old_thrusters->current_linear_thrust.y * old_thrusters->max_linear_force.y * efficiency,
        old_thrusters->current_linear_thrust.z * old_thrusters->max_linear_force.z * efficiency
    };
    
    // Transform to world space
    Vector3 world_force = quaternion_rotate_vector(transform->rotation, linear_force);
    physics_add_force(physics, world_force);
    
    // Apply angular torques
    if (physics->has_6dof) {
        Vector3 angular_torque = {
            old_thrusters->current_angular_thrust.x * old_thrusters->max_angular_torque.x * efficiency,
            old_thrusters->current_angular_thrust.y * old_thrusters->max_angular_torque.y * efficiency,
            old_thrusters->current_angular_thrust.z * old_thrusters->max_angular_torque.z * efficiency
        };
        
        physics_add_torque(physics, angular_torque);
    }
    
    // Debug output for high thrust
    static int debug_counter = 0;
    if (++debug_counter % 60 == 0) {  // Once per second
        float thrust_magnitude = vector3_length(linear_force);
        if (thrust_magnitude > 100.0f) {
            printf("🚀 Legacy Thrust: %.1fN efficiency:%.2f\n", thrust_magnitude, efficiency);
        }
    }
}