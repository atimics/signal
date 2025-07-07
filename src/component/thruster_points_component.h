#ifndef THRUSTER_POINTS_COMPONENT_H
#define THRUSTER_POINTS_COMPONENT_H

#include "../thruster_points.h"
#include "../core.h"
#include "../assets.h"  // For Mesh definition

// Component for simplified thruster system
typedef struct ThrusterPointsComponent {
    ThrusterPointSystem* system;  // The thruster system
    char* definition_file;        // Path to .thrusters file
    bool initialized;            // Has been loaded and validated
    bool visual_debug;           // Show debug visualization
} ThrusterPointsComponent;

// Component creation/destruction
ThrusterPointsComponent* thruster_points_component_create(const char* definition_file);
void thruster_points_component_destroy(ThrusterPointsComponent* component);

// Initialize from file (loads and validates)
bool thruster_points_component_init(ThrusterPointsComponent* component, const Mesh* mesh);

// Update thrust based on control input
void thruster_points_component_update(ThrusterPointsComponent* component, 
                                     const Vector3* linear_command,
                                     const Vector3* angular_command);

// Apply forces to physics component
void thruster_points_component_apply_forces(const ThrusterPointsComponent* component,
                                           const struct Transform* transform,
                                           struct Physics* physics);

#endif // THRUSTER_POINTS_COMPONENT_H