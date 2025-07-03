#ifndef THRUSTER_POINTS_H
#define THRUSTER_POINTS_H

#include "core.h"
#include "assets.h"  // For Mesh definition

// Single thruster attachment point
typedef struct {
    char name[32];         // Thruster identifier
    Vector3 position;      // Position on mesh (local space)
    Vector3 direction;     // Thrust direction (normalized)
    float max_force;       // Maximum thrust force (N)
    float current_thrust;  // Current thrust level (0-1)
} ThrusterPoint;

// Collection of thrusters for a ship
typedef struct {
    ThrusterPoint* points;
    int count;
    int capacity;
} ThrusterArray;

// Thruster control groups
typedef enum {
    THRUSTER_GROUP_MAIN,      // Main engines
    THRUSTER_GROUP_FORWARD,   // Forward RCS
    THRUSTER_GROUP_BACKWARD,  // Backward RCS
    THRUSTER_GROUP_LEFT,      // Left RCS
    THRUSTER_GROUP_RIGHT,     // Right RCS
    THRUSTER_GROUP_UP,        // Up RCS
    THRUSTER_GROUP_DOWN,      // Down RCS
    THRUSTER_GROUP_PITCH_UP,  // Pitch up
    THRUSTER_GROUP_PITCH_DOWN,// Pitch down
    THRUSTER_GROUP_YAW_LEFT,  // Yaw left
    THRUSTER_GROUP_YAW_RIGHT, // Yaw right
    THRUSTER_GROUP_ROLL_LEFT, // Roll left
    THRUSTER_GROUP_ROLL_RIGHT,// Roll right
    THRUSTER_GROUP_COUNT
} ThrusterGroup;

// Thruster group mapping
typedef struct {
    int* thruster_indices;    // Indices into ThrusterArray
    int count;
    float activation;         // Current activation level (0-1)
} ThrusterGroupMapping;

// Complete thruster system for an entity
typedef struct {
    ThrusterArray thrusters;
    ThrusterGroupMapping groups[THRUSTER_GROUP_COUNT];
    
    // Quick access for common operations
    float linear_thrust_available[3];   // Max thrust per axis
    float angular_thrust_available[3];  // Max torque per axis
} ThrusterPointSystem;

// Load thruster definitions from file
bool thruster_points_load(ThrusterArray* thrusters, const char* filename);

// Validate thruster positions against mesh vertices
bool thruster_points_validate(const ThrusterArray* thrusters, 
                             const Mesh* mesh,
                             float tolerance);

// Create thruster groups based on position and direction
void thruster_points_create_groups(ThrusterPointSystem* system);

// Apply thrust commands to groups
void thruster_points_set_linear_thrust(ThrusterPointSystem* system,
                                      const Vector3* thrust_command);

void thruster_points_set_angular_thrust(ThrusterPointSystem* system,
                                       const Vector3* torque_command);

// Calculate total forces and torques
void thruster_points_calculate_forces(const ThrusterPointSystem* system,
                                     const struct Transform* transform,
                                     Vector3* total_force,
                                     Vector3* total_torque);

// Visual debugging
void thruster_points_debug_draw(const ThrusterPointSystem* system,
                               const struct Transform* transform);

// Cleanup
void thruster_points_destroy(ThrusterArray* thrusters);
void thruster_point_system_destroy(ThrusterPointSystem* system);

#endif // THRUSTER_POINTS_H