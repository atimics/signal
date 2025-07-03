#include "ode_thrusters.h"
#include "ode_physics.h"
#include "../core.h"
#include <ode/ode.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Thruster configuration for different ship types
typedef struct {
    const char* ship_type;
    int num_thrusters;
    ThrusterConfig thrusters[MAX_THRUSTERS_PER_SHIP];
} ShipThrusterLayout;

// Default thruster layout for wedge_ship_mk2
static const ShipThrusterLayout wedge_ship_layout = {
    .ship_type = "wedge_ship_mk2",
    .num_thrusters = 6,
    .thrusters = {
        // Main engines (gimballed)
        {
            .mount_position = {-2.0f, -0.5f, -3.0f},
            .mount_orientation = {0, 0, 0, 1},  // Identity quaternion
            .thrust_direction = {0, 0, -1},
            .max_thrust = 5000.0f,
            .type = THRUSTER_TYPE_GIMBAL,
            .gimbal_range = 15.0f * (M_PI / 180.0f),  // 15 degrees
            .gimbal_speed = 2.0f,  // rad/s
            .nozzle_length = 1.5f,
            .nozzle_radius = 0.4f
        },
        {
            .mount_position = {2.0f, -0.5f, -3.0f},
            .mount_orientation = {0, 0, 0, 1},
            .thrust_direction = {0, 0, -1},
            .max_thrust = 5000.0f,
            .type = THRUSTER_TYPE_GIMBAL,
            .gimbal_range = 15.0f * (M_PI / 180.0f),
            .gimbal_speed = 2.0f,
            .nozzle_length = 1.5f,
            .nozzle_radius = 0.4f
        },
        // RCS thrusters (fixed)
        // Forward
        {
            .mount_position = {0, 0, 3.0f},
            .mount_orientation = {0, 1, 0, 0},  // 180 degree rotation
            .thrust_direction = {0, 0, 1},
            .max_thrust = 500.0f,
            .type = THRUSTER_TYPE_FIXED,
            .nozzle_length = 0.5f,
            .nozzle_radius = 0.15f
        },
        // Up
        {
            .mount_position = {0, 1.5f, 0},
            .mount_orientation = {0.707f, 0, 0, 0.707f},  // 90 degree rotation
            .thrust_direction = {0, 1, 0},
            .max_thrust = 500.0f,
            .type = THRUSTER_TYPE_FIXED,
            .nozzle_length = 0.5f,
            .nozzle_radius = 0.15f
        },
        // Down
        {
            .mount_position = {0, -1.5f, 0},
            .mount_orientation = {-0.707f, 0, 0, 0.707f},  // -90 degree rotation
            .thrust_direction = {0, -1, 0},
            .max_thrust = 500.0f,
            .type = THRUSTER_TYPE_FIXED,
            .nozzle_length = 0.5f,
            .nozzle_radius = 0.15f
        },
        // Lateral
        {
            .mount_position = {2.5f, 0, 0},
            .mount_orientation = {0, 0.707f, 0, 0.707f},  // 90 degree Y rotation
            .thrust_direction = {1, 0, 0},
            .max_thrust = 500.0f,
            .type = THRUSTER_TYPE_FIXED,
            .nozzle_length = 0.5f,
            .nozzle_radius = 0.15f
        }
    }
};

// Initialize ODE thruster system for an entity
bool ode_thrusters_init(ODEThrusterSystem* system, struct World* world, EntityID entity_id) {
    if (!system || !world) return false;
    
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return false;
    
    // Get the ship's ODE body
    struct ODEPhysics* ode_physics = entity_get_ode_physics(world, entity_id);
    if (!ode_physics || !ode_physics->body) {
        printf("❌ Entity %d has no ODE physics body for thrusters\n", entity_id);
        return false;
    }
    
    system->entity_id = entity_id;
    system->ship_body = ode_physics->body;
    system->num_thrusters = 0;
    
    // Get thruster layout for this ship type
    const ShipThrusterLayout* layout = &wedge_ship_layout;  // TODO: Select based on ship type
    
    printf("🚀 Initializing ODE thruster system for entity %d\n", entity_id);
    printf("   Ship type: %s\n", layout->ship_type);
    printf("   Num thrusters: %d\n", layout->num_thrusters);
    
    // Create ODE bodies for each thruster
    for (int i = 0; i < layout->num_thrusters && i < MAX_THRUSTERS_PER_SHIP; i++) {
        const ThrusterConfig* config = &layout->thrusters[i];
        ODEThruster* thruster = &system->thrusters[i];
        
        // Copy configuration
        thruster->config = *config;
        thruster->current_thrust = 0.0f;
        thruster->current_gimbal = (Vector2){0, 0};
        
        // Create thruster nozzle body
        thruster->nozzle_body = dBodyCreate(ode_get_world());
        
        // Set nozzle mass (small compared to ship)
        dMass nozzle_mass;
        dMassSetCylinderTotal(&nozzle_mass, 10.0f, 3,  // 10kg, Z-axis
                             config->nozzle_radius, config->nozzle_length);
        dBodySetMass(thruster->nozzle_body, &nozzle_mass);
        
        // Position nozzle relative to ship
        dReal ship_pos[3];
        const dReal* ship_rot = dBodyGetRotation(system->ship_body);
        dBodyGetRelPointPos(system->ship_body, 
                           config->mount_position.x,
                           config->mount_position.y,
                           config->mount_position.z,
                           ship_pos);
        dBodySetPosition(thruster->nozzle_body, ship_pos[0], ship_pos[1], ship_pos[2]);
        
        // Set nozzle orientation
        dMatrix3 R;
        dRFromAxisAndAngle(R, 0, 1, 0, 0);  // TODO: Use mount_orientation
        dBodySetRotation(thruster->nozzle_body, R);
        
        // Create attachment joint based on thruster type
        if (config->type == THRUSTER_TYPE_FIXED) {
            // Fixed joint - no movement
            thruster->mount_joint = dJointCreateFixed(ode_get_world(), 0);
            dJointAttach(thruster->mount_joint, system->ship_body, thruster->nozzle_body);
            dJointSetFixed(thruster->mount_joint);
        } else if (config->type == THRUSTER_TYPE_GIMBAL) {
            // Universal joint - 2 DOF rotation
            thruster->mount_joint = dJointCreateUniversal(ode_get_world(), 0);
            dJointAttach(thruster->mount_joint, system->ship_body, thruster->nozzle_body);
            
            // Set joint anchor at mount position
            dJointSetUniversalAnchor(thruster->mount_joint,
                                   ship_pos[0], ship_pos[1], ship_pos[2]);
            
            // Set joint axes
            dJointSetUniversalAxis1(thruster->mount_joint, 1, 0, 0);  // Pitch
            dJointSetUniversalAxis2(thruster->mount_joint, 0, 1, 0);  // Yaw
            
            // Set joint limits
            dJointSetUniversalParam(thruster->mount_joint, dParamLoStop1, -config->gimbal_range);
            dJointSetUniversalParam(thruster->mount_joint, dParamHiStop1, config->gimbal_range);
            dJointSetUniversalParam(thruster->mount_joint, dParamLoStop2, -config->gimbal_range);
            dJointSetUniversalParam(thruster->mount_joint, dParamHiStop2, config->gimbal_range);
            
            // Add motors for gimbal control
            thruster->gimbal_motor = dJointCreateAMotor(ode_get_world(), 0);
            dJointAttach(thruster->gimbal_motor, system->ship_body, thruster->nozzle_body);
            dJointSetAMotorMode(thruster->gimbal_motor, dAMotorUser);
            dJointSetAMotorNumAxes(thruster->gimbal_motor, 2);
            
            // Configure motor axes
            dJointSetAMotorAxis(thruster->gimbal_motor, 0, 1, 1, 0, 0);
            dJointSetAMotorAxis(thruster->gimbal_motor, 1, 1, 0, 1, 0);
            
            // Set motor parameters
            dJointSetAMotorParam(thruster->gimbal_motor, dParamFMax1, 100.0f);
            dJointSetAMotorParam(thruster->gimbal_motor, dParamFMax2, 100.0f);
        }
        
        system->num_thrusters++;
        
        printf("   ✅ Thruster %d: type=%s pos=(%.1f,%.1f,%.1f) thrust=%.0fN\n",
               i, config->type == THRUSTER_TYPE_FIXED ? "FIXED" : "GIMBAL",
               config->mount_position.x, config->mount_position.y, config->mount_position.z,
               config->max_thrust);
    }
    
    return true;
}

// Update thruster forces and gimbals
void ode_thrusters_update(ODEThrusterSystem* system, struct World* world, float delta_time) {
    if (!system || !world) return;
    
    struct Entity* entity = entity_get(world, system->entity_id);
    if (!entity) return;
    
    // Get control input
    struct ControlAuthority* control = entity_get_control_authority(world, system->entity_id);
    struct ThrusterSystem* thrust_cmd = entity_get_thruster_system(world, system->entity_id);
    
    if (!control || !thrust_cmd) return;
    
    // Update each thruster
    for (int i = 0; i < system->num_thrusters; i++) {
        ODEThruster* thruster = &system->thrusters[i];
        const ThrusterConfig* config = &thruster->config;
        
        // Calculate thrust command based on type and input
        float thrust_command = 0.0f;
        Vector2 gimbal_target = {0, 0};
        
        // Main engines respond to forward/back thrust
        if (config->type == THRUSTER_TYPE_GIMBAL) {
            // Main thrust from Z-axis command
            if (config->thrust_direction.z < 0) {
                thrust_command = fmaxf(0, -thrust_cmd->current_linear_thrust.z);
            }
            
            // Gimbal for steering
            gimbal_target.x = control->input_angular.x * config->gimbal_range;  // Pitch
            gimbal_target.y = -control->input_angular.y * config->gimbal_range; // Yaw
        }
        // RCS thrusters
        else if (config->type == THRUSTER_TYPE_FIXED) {
            // Map thrust commands to appropriate RCS thrusters
            Vector3 cmd = thrust_cmd->current_linear_thrust;
            float dot = vector3_dot(config->thrust_direction, cmd);
            thrust_command = fmaxf(0, dot);
        }
        
        // Update gimbal angles (smooth interpolation)
        if (config->type == THRUSTER_TYPE_GIMBAL && thruster->gimbal_motor) {
            float gimbal_speed = config->gimbal_speed * delta_time;
            thruster->current_gimbal.x += (gimbal_target.x - thruster->current_gimbal.x) * gimbal_speed;
            thruster->current_gimbal.y += (gimbal_target.y - thruster->current_gimbal.y) * gimbal_speed;
            
            // Set motor velocities to achieve target angles
            dJointSetAMotorParam(thruster->gimbal_motor, dParamVel1, 
                                (gimbal_target.x - thruster->current_gimbal.x) * 5.0f);
            dJointSetAMotorParam(thruster->gimbal_motor, dParamVel2,
                                (gimbal_target.y - thruster->current_gimbal.y) * 5.0f);
        }
        
        // Update thrust
        thruster->current_thrust = thrust_command;
        
        // Apply thrust force
        if (thruster->current_thrust > 0.01f && thruster->nozzle_body) {
            // Get nozzle orientation
            const dReal* R = dBodyGetRotation(thruster->nozzle_body);
            
            // Thrust direction in world space (nozzle Z-axis)
            dReal thrust_dir[3] = {
                R[2] * config->thrust_direction.z,
                R[6] * config->thrust_direction.z,
                R[10] * config->thrust_direction.z
            };
            
            // Apply force at nozzle position
            dReal force = thruster->current_thrust * config->max_thrust;
            const dReal* pos = dBodyGetPosition(thruster->nozzle_body);
            
            // Apply equal and opposite forces (Newton's third law)
            dBodyAddForceAtPos(system->ship_body,
                              thrust_dir[0] * force,
                              thrust_dir[1] * force,
                              thrust_dir[2] * force,
                              pos[0], pos[1], pos[2]);
            
            if (i < 2) {  // Log main engines only
                printf("🔥 Thruster %d: thrust=%.0fN gimbal=(%.1f°,%.1f°)\n",
                       i, force,
                       thruster->current_gimbal.x * 180.0f / M_PI,
                       thruster->current_gimbal.y * 180.0f / M_PI);
            }
        }
    }
}

// Cleanup ODE thruster system
void ode_thrusters_cleanup(ODEThrusterSystem* system) {
    if (!system) return;
    
    // Destroy all thruster bodies and joints
    for (int i = 0; i < system->num_thrusters; i++) {
        ODEThruster* thruster = &system->thrusters[i];
        
        if (thruster->gimbal_motor) {
            dJointDestroy(thruster->gimbal_motor);
            thruster->gimbal_motor = NULL;
        }
        
        if (thruster->mount_joint) {
            dJointDestroy(thruster->mount_joint);
            thruster->mount_joint = NULL;
        }
        
        if (thruster->nozzle_body) {
            dBodyDestroy(thruster->nozzle_body);
            thruster->nozzle_body = NULL;
        }
    }
    
    system->num_thrusters = 0;
}

// Get current thruster states for rendering
void ode_thrusters_get_render_data(const ODEThrusterSystem* system, 
                                  ThrusterRenderData* render_data,
                                  int max_thrusters) {
    if (!system || !render_data) return;
    
    int count = (system->num_thrusters < max_thrusters) ? system->num_thrusters : max_thrusters;
    
    for (int i = 0; i < count; i++) {
        const ODEThruster* thruster = &system->thrusters[i];
        ThrusterRenderData* data = &render_data[i];
        
        // Get nozzle position and orientation
        if (thruster->nozzle_body) {
            const dReal* pos = dBodyGetPosition(thruster->nozzle_body);
            const dReal* R = dBodyGetRotation(thruster->nozzle_body);
            
            data->position = (Vector3){pos[0], pos[1], pos[2]};
            
            // Extract thrust direction from rotation matrix
            data->direction = (Vector3){R[2], R[6], R[10]};
            
            data->intensity = thruster->current_thrust;
            data->type = thruster->config.type;
            data->size = thruster->config.nozzle_radius;
        }
    }
}