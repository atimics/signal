#ifndef SYSTEM_THRUSTERS_H
#define SYSTEM_THRUSTERS_H

#include "../core.h"
#include "../render.h"

// Thruster system update - processes thrust commands and applies forces
void thruster_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Thruster control functions
void thruster_set_linear_command(struct ThrusterSystem* thrusters, Vector3 command);
void thruster_set_angular_command(struct ThrusterSystem* thrusters, Vector3 command);
void thruster_set_enabled(struct ThrusterSystem* thrusters, bool enabled);

// Calculate effective thrust based on environment
float thruster_calculate_efficiency(struct ThrusterSystem* thrusters, int environment);

#endif // SYSTEM_THRUSTERS_H