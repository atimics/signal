#include "thruster_points_component.h"
#include "../system/physics.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Component creation/destruction
ThrusterPointsComponent* thruster_points_component_create(const char* definition_file) {
    ThrusterPointsComponent* component = calloc(1, sizeof(ThrusterPointsComponent));
    if (!component) return NULL;
    
    component->system = calloc(1, sizeof(ThrusterPointSystem));
    if (!component->system) {
        free(component);
        return NULL;
    }
    
    if (definition_file) {
        component->definition_file = strdup(definition_file);
    }
    
    component->initialized = false;
    component->visual_debug = false;
    
    return component;
}

void thruster_points_component_destroy(ThrusterPointsComponent* component) {
    if (!component) return;
    
    if (component->system) {
        thruster_point_system_destroy(component->system);
        free(component->system);
    }
    
    if (component->definition_file) {
        free(component->definition_file);
    }
    
    free(component);
}

// Initialize from file (loads and validates)
bool thruster_points_component_init(ThrusterPointsComponent* component, const Mesh* mesh) {
    if (!component || !component->definition_file) return false;
    
    // Load thruster definitions
    if (!thruster_points_load(&component->system->thrusters, component->definition_file)) {
        printf("❌ Failed to load thruster definitions from %s\n", component->definition_file);
        return false;
    }
    
    // Validate against mesh if provided
    if (mesh) {
        if (!thruster_points_validate(&component->system->thrusters, mesh, 0.1f)) {
            printf("⚠️  Warning: Some thrusters failed mesh validation\n");
            // Continue anyway - might be using a different mesh
        }
    }
    
    // Create control groups
    thruster_points_create_groups(component->system);
    
    component->initialized = true;
    
    printf("✅ Thruster system initialized with %d thrusters\n", 
           component->system->thrusters.count);
    
    return true;
}

// Update thrust based on control input
void thruster_points_component_update(ThrusterPointsComponent* component, 
                                     const Vector3* linear_command,
                                     const Vector3* angular_command) {
    if (!component || !component->initialized) return;
    
    // Set linear thrust commands
    if (linear_command) {
        thruster_points_set_linear_thrust(component->system, linear_command);
    }
    
    // Set angular thrust commands (TODO: implement in thruster_points.c)
    if (angular_command) {
        // thruster_points_set_angular_thrust(component->system, angular_command);
    }
}

// Apply forces to physics component
void thruster_points_component_apply_forces(const ThrusterPointsComponent* component,
                                           const struct Transform* transform,
                                           struct Physics* physics) {
    if (!component || !component->initialized || !transform || !physics) return;
    
    Vector3 total_force = {0, 0, 0};
    Vector3 total_torque = {0, 0, 0};
    
    // Calculate forces and torques from all thrusters
    thruster_points_calculate_forces(component->system, transform, 
                                    &total_force, &total_torque);
    
    // Apply to physics
    physics_add_force(physics, total_force);
    physics_add_torque(physics, total_torque);
    
    // Debug output
    if (component->visual_debug && (vector3_length(total_force) > 0.1f)) {
        printf("🚀 Thrust: Force=(%.1f,%.1f,%.1f)N Torque=(%.1f,%.1f,%.1f)Nm\n",
               total_force.x, total_force.y, total_force.z,
               total_torque.x, total_torque.y, total_torque.z);
    }
}