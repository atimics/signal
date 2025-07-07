#ifndef SYSTEM_THRUSTER_POINTS_H
#define SYSTEM_THRUSTER_POINTS_H

#include "../core.h"
#include "../render.h"

// New simplified thruster system update
void thruster_points_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Helper to convert old ThrusterSystem commands to new system
void thruster_points_apply_legacy_commands(struct World* world, EntityID entity_id);

#endif // SYSTEM_THRUSTER_POINTS_H