#ifndef SYSTEM_PHYSICS_H
#define SYSTEM_PHYSICS_H

#include "../core.h"
#include "../render.h"

// Forward declaration for ODE integration
struct ODEPhysicsSystem;

// Core physics system update
void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// ODE-integrated physics system update
void physics_system_update_with_ode(struct World* world, struct ODEPhysicsSystem* ode_system, 
                                   RenderConfig* render_config, float delta_time);

// Physics utility functions for force/torque application
void physics_add_force(struct Physics* physics, Vector3 force);
void physics_add_torque(struct Physics* physics, Vector3 torque);
void physics_add_force_at_point(struct Physics* physics, Vector3 force, Vector3 point, Vector3 center_of_mass);

// 6DOF control
void physics_set_6dof_enabled(struct Physics* physics, bool enabled);

// Gravity alignment control (for spatial orientation)
void physics_set_gravity_alignment(struct Physics* physics, bool enabled, float strength);

// ODE integration control
void physics_set_use_ode(struct Physics* physics, bool use_ode);
bool physics_is_using_ode(const struct Physics* physics);

// Testing functions (exposed for unit tests)
#ifdef UNITY_TESTING
void physics_integrate_linear(struct Physics* physics, struct Transform* transform, float delta_time);
void physics_integrate_angular(struct Physics* physics, struct Transform* transform, float delta_time);
#endif

#endif // SYSTEM_PHYSICS_H
