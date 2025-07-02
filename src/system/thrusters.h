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

// Sprint 21: Ship type configuration
void thruster_configure_ship_type(struct ThrusterSystem* thrusters, ShipType ship_type, float base_thrust);
void thruster_apply_ship_characteristics(struct ThrusterSystem* thrusters, struct Physics* physics);

#endif // SYSTEM_THRUSTERS_H