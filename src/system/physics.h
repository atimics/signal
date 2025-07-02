#ifndef SYSTEM_PHYSICS_H
#define SYSTEM_PHYSICS_H

#include "../core.h"
#include "../render.h"

// Core physics system update
void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Physics utility functions for force/torque application
void physics_add_force(struct Physics* physics, Vector3 force);
void physics_add_torque(struct Physics* physics, Vector3 torque);
void physics_add_force_at_point(struct Physics* physics, Vector3 force, Vector3 point, Vector3 center_of_mass);

// 6DOF control
void physics_set_6dof_enabled(struct Physics* physics, bool enabled);

#endif // SYSTEM_PHYSICS_H
