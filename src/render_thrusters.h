#ifndef RENDER_THRUSTERS_H
#define RENDER_THRUSTERS_H

#include "core.h"
#include "render.h"

// Thruster types
typedef enum {
    THRUSTER_MAIN,      // Main propulsion
    THRUSTER_RCS,       // Reaction Control System
    THRUSTER_GIMBAL     // Gimballed main thruster
} ThrusterType;

// Physical thruster mount point
typedef struct {
    Vector3 position;       // Attachment point on ship (local space)
    Vector3 direction;      // Default thrust direction (local space)
    ThrusterType type;
    float size;            // Visual size multiplier
    float max_gimbal_angle; // Maximum gimbal angle in radians (for THRUSTER_GIMBAL)
    
    // ODE physics bodies (if using physical simulation)
    void* ode_body;        // dBodyID for the thruster nozzle
    void* ode_joint;       // dJointID for the attachment joint
    void* ode_motor;       // dJointID for gimbal motor (if gimballed)
} ThrusterMount;

// Render thruster effects
void render_thrusters(struct World* world, RenderConfig* render_config, EntityID entity_id);

// Get thruster configuration for a ship type
const ThrusterMount* get_thruster_mounts(const char* ship_type, int* count);

// Check if a specific thruster should be firing
bool is_thruster_firing(const ThrusterMount* mount, 
                       const Vector3* linear_thrust, 
                       const Vector3* angular_thrust);

// Initialize ODE thruster bodies for an entity
void init_ode_thrusters(struct World* world, EntityID entity_id);

// Update thruster gimbal angles based on control input
void update_thruster_gimbals(struct World* world, EntityID entity_id, float delta_time);

#endif // RENDER_THRUSTERS_H