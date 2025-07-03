#ifndef ODE_THRUSTERS_H
#define ODE_THRUSTERS_H

#include "../core.h"
#include <ode/ode.h>

#define MAX_THRUSTERS_PER_SHIP 12

// Thruster types
typedef enum {
    THRUSTER_TYPE_FIXED,    // Fixed direction (RCS)
    THRUSTER_TYPE_GIMBAL,   // Can rotate for thrust vectoring
    THRUSTER_TYPE_VERNIER   // Small precision thrusters
} ThrusterType;

// Configuration for a single thruster
typedef struct {
    Vector3 mount_position;      // Where thruster attaches to ship (local space)
    Quaternion mount_orientation; // Default orientation (local space)
    Vector3 thrust_direction;    // Which way thrust points (thruster local)
    float max_thrust;           // Maximum thrust force (N)
    ThrusterType type;
    float gimbal_range;         // Max gimbal angle (radians) for GIMBAL type
    float gimbal_speed;         // Gimbal rotation speed (rad/s)
    float nozzle_length;        // Physical nozzle dimensions
    float nozzle_radius;
} ThrusterConfig;

// Runtime state for a single thruster
typedef struct {
    ThrusterConfig config;
    
    // ODE bodies and joints
    dBodyID nozzle_body;        // Physical nozzle body
    dJointID mount_joint;       // Joint connecting to ship (fixed or universal)
    dJointID gimbal_motor;      // Motor for gimbal control (if applicable)
    
    // Current state
    float current_thrust;       // Current thrust level (0-1)
    Vector2 current_gimbal;     // Current gimbal angles (pitch, yaw)
} ODEThruster;

// ODE-based thruster system for a ship
typedef struct {
    EntityID entity_id;
    dBodyID ship_body;          // Reference to ship's main body
    
    int num_thrusters;
    ODEThruster thrusters[MAX_THRUSTERS_PER_SHIP];
} ODEThrusterSystem;

// Data for rendering thrusters
typedef struct {
    Vector3 position;
    Vector3 direction;
    float intensity;
    ThrusterType type;
    float size;
} ThrusterRenderData;

// Initialize ODE thruster system for an entity
bool ode_thrusters_init(ODEThrusterSystem* system, struct World* world, EntityID entity_id);

// Update thruster forces and gimbals
void ode_thrusters_update(ODEThrusterSystem* system, struct World* world, float delta_time);

// Cleanup ODE thruster system
void ode_thrusters_cleanup(ODEThrusterSystem* system);

// Get current thruster states for rendering
void ode_thrusters_get_render_data(const ODEThrusterSystem* system, 
                                  ThrusterRenderData* render_data,
                                  int max_thrusters);

#endif // ODE_THRUSTERS_H