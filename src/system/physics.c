#include "physics.h"

#ifdef USE_ODE_PHYSICS
#include "ode_physics.h"
#endif

#include <stdio.h>
#include <math.h>

// ============================================================================
// PHYSICS UTILITY FUNCTIONS
// ============================================================================

static Vector3 vector3_cross_product(Vector3 a, Vector3 b)
{
    return (Vector3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static Quaternion quaternion_from_angular_velocity(Vector3 angular_velocity, float delta_time)
{
    float angle = vector3_length(angular_velocity) * delta_time;
    if (angle < 0.0001f) {
        return (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f };  // Identity quaternion
    }
    
    Vector3 axis = vector3_normalize(angular_velocity);
    float half_angle = angle * 0.5f;
    float sin_half = sinf(half_angle);
    
    return (Quaternion){
        axis.x * sin_half,
        axis.y * sin_half,
        axis.z * sin_half,
        cosf(half_angle)
    };
}

static Quaternion quaternion_multiply(Quaternion a, Quaternion b)
{
    return (Quaternion){
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

static Quaternion quaternion_normalize(Quaternion q)
{
    float length = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (length < 0.0001f) {
        return (Quaternion){ 0.0f, 0.0f, 0.0f, 1.0f };
    }
    
    return (Quaternion){
        q.x / length,
        q.y / length,
        q.z / length,
        q.w / length
    };
}

// ============================================================================
// FORCE AND TORQUE APPLICATION
// ============================================================================

static void physics_apply_forces(struct Physics* physics, float delta_time)
{
    (void)delta_time;  // Unused parameter
    if (physics->mass <= 0.0f) {
        printf("❌ Physics: Mass is zero or negative! Mass: %.3f\n", physics->mass);
        return;
    }
    
    // Linear dynamics: F = ma -> a = F/m
    Vector3 linear_acceleration = vector3_multiply(physics->force_accumulator, 1.0f / physics->mass);
    
    // Clamp acceleration to prevent numerical instability
    const float max_acceleration = 1000.0f; // Maximum acceleration
    float accel_magnitude = vector3_length(linear_acceleration);
    if (accel_magnitude > max_acceleration) {
        linear_acceleration = vector3_multiply(vector3_normalize(linear_acceleration), max_acceleration);
    }
    
    physics->acceleration = linear_acceleration;
    
    // Debug output for non-zero forces (reduced frequency)
    static uint32_t force_log_counter = 0;
    float force_mag = sqrtf(physics->force_accumulator.x * physics->force_accumulator.x + 
                          physics->force_accumulator.y * physics->force_accumulator.y + 
                          physics->force_accumulator.z * physics->force_accumulator.z);
    if (force_mag > 0.1f && ++force_log_counter % 60 == 0) {  // Every second at 60 FPS
        printf("⚡ LINEAR: Force:[%.0f,%.0f,%.0f] -> Accel:[%.2f,%.2f,%.2f] -> Vel:[%.2f,%.2f,%.2f]\n",
               physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z,
               linear_acceleration.x, linear_acceleration.y, linear_acceleration.z,
               physics->velocity.x, physics->velocity.y, physics->velocity.z);
    }
    
    // Clear force accumulator for next frame
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
}

static void physics_apply_torques(struct Physics* physics, float delta_time)
{
    (void)delta_time;  // Unused parameter
    if (!physics->has_6dof) return;
    
    // Angular dynamics: τ = I·α -> α = τ/I (component-wise)
    Vector3 angular_accel = {
        physics->moment_of_inertia.x > 0.0f ? physics->torque_accumulator.x / physics->moment_of_inertia.x : 0.0f,
        physics->moment_of_inertia.y > 0.0f ? physics->torque_accumulator.y / physics->moment_of_inertia.y : 0.0f,
        physics->moment_of_inertia.z > 0.0f ? physics->torque_accumulator.z / physics->moment_of_inertia.z : 0.0f
    };
    
    physics->angular_acceleration = angular_accel;
    
    // Debug angular forces
    static uint32_t torque_log_counter = 0;
    float torque_mag = sqrtf(physics->torque_accumulator.x * physics->torque_accumulator.x + 
                           physics->torque_accumulator.y * physics->torque_accumulator.y + 
                           physics->torque_accumulator.z * physics->torque_accumulator.z);
    if (torque_mag > 0.1f && ++torque_log_counter % 60 == 0) {  // Every second at 60 FPS
        printf("🌀 ANGULAR: Torque:[%.0f,%.0f,%.0f] -> AngAccel:[%.2f,%.2f,%.2f] -> AngVel:[%.2f,%.2f,%.2f]\n",
               physics->torque_accumulator.x, physics->torque_accumulator.y, physics->torque_accumulator.z,
               angular_accel.x, angular_accel.y, angular_accel.z,
               physics->angular_velocity.x, physics->angular_velocity.y, physics->angular_velocity.z);
    }
    
    // Clear torque accumulator for next frame
    physics->torque_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
}

// ============================================================================
// INTEGRATION FUNCTIONS
// ============================================================================

void physics_integrate_linear(struct Physics* physics, struct Transform* transform, float delta_time)
{
    // Store old position for debug
    Vector3 old_pos = transform->position;
    
    // DEBUG: Track velocity at each step with more detailed info
    static uint32_t debug_counter = 0;
    bool should_debug = (++debug_counter % 60 == 0);
    
    Vector3 vel_before = physics->velocity;
    
    // Enhanced debug: track mass and delta_time
    if (should_debug) {
        printf("🔬 PHYSICS DEBUG: mass=%.1f, drag=%.4f, dt=%.5f\n", 
               physics->mass, physics->drag_linear, delta_time);
    }
    
    // Apply acceleration to velocity
    physics->velocity = vector3_add(physics->velocity, vector3_multiply(physics->acceleration, delta_time));
    
    // Apply drag
    physics->velocity = vector3_multiply(physics->velocity, 1.0f - physics->drag_linear);
    
    // Clamp velocity to prevent numerical overflow
    const float max_speed = 500.0f; // Maximum speed in units per second
    float speed = vector3_length(physics->velocity);
    if (speed > max_speed) {
        physics->velocity = vector3_multiply(vector3_normalize(physics->velocity), max_speed);
    }
    
    Vector3 vel_after_drag = physics->velocity;
    
    // Apply velocity to position
    transform->position = vector3_add(transform->position, vector3_multiply(physics->velocity, delta_time));
    
    // DEBUG: Show velocity changes step by step (reduced frequency)
    if (should_debug && (physics->acceleration.x != 0.0f || physics->acceleration.y != 0.0f || physics->acceleration.z != 0.0f)) {
        printf("🔍 VEL: Before:[%.2f,%.2f,%.2f] After:[%.2f,%.2f,%.2f]\n",
               vel_before.x, vel_before.y, vel_before.z,
               vel_after_drag.x, vel_after_drag.y, vel_after_drag.z);
    }
    
    // Debug position changes - track any movement
    static uint32_t pos_log_counter = 0;
    Vector3 pos_change = vector3_subtract(transform->position, old_pos);
    float pos_change_mag = sqrtf(pos_change.x * pos_change.x + pos_change.y * pos_change.y + pos_change.z * pos_change.z);
    
    if (pos_change_mag > 0.001f && ++pos_log_counter % 60 == 0) {
        printf("📍 POSITION: OldPos:[%.3f,%.3f,%.3f] NewPos:[%.3f,%.3f,%.3f] Change:[%.4f,%.4f,%.4f]\n",
               old_pos.x, old_pos.y, old_pos.z,
               transform->position.x, transform->position.y, transform->position.z,
               pos_change.x, pos_change.y, pos_change.z);
        printf("📍 VELOCITY: [%.4f,%.4f,%.4f] dt=%.5f\n",
               physics->velocity.x, physics->velocity.y, physics->velocity.z, delta_time);
    }
}

void physics_integrate_angular(struct Physics* physics, struct Transform* transform, float delta_time)
{
    if (!physics->has_6dof) return;
    
    // Apply angular acceleration to angular velocity
    physics->angular_velocity = vector3_add(physics->angular_velocity, 
                                          vector3_multiply(physics->angular_acceleration, delta_time));
    
    // Apply angular drag
    physics->angular_velocity = vector3_multiply(physics->angular_velocity, 1.0f - physics->drag_angular);
    
    // Clamp angular velocity to prevent excessive spinning
    const float max_angular_speed = 5.0f; // radians per second
    float angular_speed = vector3_length(physics->angular_velocity);
    if (angular_speed > max_angular_speed) {
        physics->angular_velocity = vector3_multiply(
            vector3_normalize(physics->angular_velocity), 
            max_angular_speed
        );
    }
    
    // Convert angular velocity to quaternion rotation
    Quaternion rotation_delta = quaternion_from_angular_velocity(physics->angular_velocity, delta_time);
    
    // Apply rotation to transform
    transform->rotation = quaternion_normalize(quaternion_multiply(transform->rotation, rotation_delta));
}

// ============================================================================
// ENVIRONMENTAL PHYSICS
// ============================================================================

static void physics_apply_environmental_effects(struct Physics* physics, struct Transform* transform, float delta_time)
{
    (void)transform;  // Unused parameter
    (void)delta_time;  // Unused parameter
    
    switch (physics->environment) {
        case PHYSICS_SPACE:
            // No environmental forces in space
            break;
            
        case PHYSICS_ATMOSPHERE:
            {
                // Apply reduced gravity for better flight feel (space station or low gravity)
                Vector3 gravity = { 0.0f, -3.0f * physics->mass, 0.0f };  // Much lighter gravity
                physics->force_accumulator = vector3_add(physics->force_accumulator, gravity);
                
                // Debug gravity application
                static uint32_t gravity_debug_counter = 0;
                if (++gravity_debug_counter % 300 == 0) {  // Every 5 seconds
                    printf("🌍 GRAVITY: Applied %.1fN to %.1fkg entity\n", 
                           -3.0f * physics->mass, physics->mass);
                }
                break;
            }
    }
}

// ============================================================================
// MAIN PHYSICS UPDATE FUNCTION
// ============================================================================

void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    (void)render_config;  // Unused
    if (!world) return;

    uint32_t linear_updates = 0;
    uint32_t angular_updates = 0;
    uint32_t force_applications = 0;

    // Update all entities with physics + transform components
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];

        if (!(entity->component_mask & COMPONENT_PHYSICS) ||
            !(entity->component_mask & COMPONENT_TRANSFORM))
        {
            continue;
        }

        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;

        if (physics->kinematic) continue;  // Skip kinematic objects

        // Apply environmental effects
        physics_apply_environmental_effects(physics, transform, delta_time);
        
        // Debug: Track force applications for player entity
        bool is_player = (entity->component_mask & COMPONENT_PLAYER);
        if (is_player) {
            float force_mag = sqrtf(physics->force_accumulator.x * physics->force_accumulator.x + 
                                  physics->force_accumulator.y * physics->force_accumulator.y + 
                                  physics->force_accumulator.z * physics->force_accumulator.z);
            if (force_mag > 0.1f) {
                force_applications++;
            }
        }
        
        // Apply accumulated forces and torques
        physics_apply_forces(physics, delta_time);
        physics_apply_torques(physics, delta_time);
        
        // Integrate linear dynamics
        physics_integrate_linear(physics, transform, delta_time);
        linear_updates++;
        
        // Integrate angular dynamics (6DOF)
        if (physics->has_6dof) {
            physics_integrate_angular(physics, transform, delta_time);
            angular_updates++;
        }

        // Prevent going below ground level (space navigation)
        if (transform->position.y < -50.0f) {
            transform->position.y = -50.0f;
            physics->velocity.y = fmaxf(0.0f, physics->velocity.y); // Stop downward velocity
        }

        transform->dirty = true;
    }

    // Debug logging
    static uint32_t log_counter = 0;
    if (++log_counter % 120 == 0)  // Every 2 seconds at 60 FPS
    {
        printf("🔧 Physics: %d linear + %d angular updates, %d force applications\n", 
               linear_updates, angular_updates, force_applications);
    }
}

// ============================================================================
// PHYSICS UTILITY FUNCTIONS FOR EXTERNAL USE
// ============================================================================

void physics_add_force(struct Physics* physics, Vector3 force)
{
    if (!physics) return;
    
    // Debug significant forces
    static int force_counter = 0;
    if (++force_counter % 60 == 0 && 
        (fabsf(force.x) > 1.0f || fabsf(force.y) > 1.0f || fabsf(force.z) > 1.0f)) {
        printf("⚡ DEBUG: physics_add_force called with [%.1f,%.1f,%.1f]\n",
               force.x, force.y, force.z);
        printf("  Current accumulator: [%.1f,%.1f,%.1f]\n",
               physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z);
    }
    
    // Clamp individual force to prevent numerical issues
    const float max_force = 100000.0f; // Maximum force component
    force.x = fmaxf(-max_force, fminf(max_force, force.x));
    force.y = fmaxf(-max_force, fminf(max_force, force.y));
    force.z = fmaxf(-max_force, fminf(max_force, force.z));
    
    physics->force_accumulator = vector3_add(physics->force_accumulator, force);
}

void physics_add_torque(struct Physics* physics, Vector3 torque)
{
    if (!physics || !physics->has_6dof) return;
    physics->torque_accumulator = vector3_add(physics->torque_accumulator, torque);
}

void physics_add_force_at_point(struct Physics* physics, Vector3 force, Vector3 point, Vector3 center_of_mass)
{
    if (!physics) return;
    
    // Add the force
    physics_add_force(physics, force);
    
    // Calculate and add torque if 6DOF is enabled
    if (physics->has_6dof) {
        Vector3 offset = vector3_subtract(point, center_of_mass);
        Vector3 torque = vector3_cross_product(offset, force);
        physics_add_torque(physics, torque);
    }
}

void physics_set_6dof_enabled(struct Physics* physics, bool enabled)
{
    if (!physics) return;
    physics->has_6dof = enabled;
    
    if (!enabled) {
        // Clear angular dynamics when disabling 6DOF
        physics->angular_velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
        physics->angular_acceleration = (Vector3){ 0.0f, 0.0f, 0.0f };
        physics->torque_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    }
}

// ============================================================================
// ODE INTEGRATION
// ============================================================================

void physics_system_update_with_ode(struct World* world, struct ODEPhysicsSystem* ode_system, 
                                   RenderConfig* render_config, float delta_time)
{
#ifdef USE_ODE_PHYSICS
    (void)render_config;  // Unused
    if (!world) return;
    
    // If no ODE system provided, fall back to regular physics
    if (!ode_system) {
        physics_system_update(world, render_config, delta_time);
        return;
    }
    
    // Let ODE handle the physics simulation
    ode_physics_step(ode_system, world, delta_time);
    
    // For entities not using ODE, use custom physics
    uint32_t custom_physics_updates = 0;
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_PHYSICS) ||
            !(entity->component_mask & COMPONENT_TRANSFORM)) {
            continue;
        }
        
        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;
        
        // Skip if using ODE
        if (physics->use_ode) continue;
        if (physics->kinematic) continue;
        
        // Use custom physics for this entity
        physics_apply_environmental_effects(physics, transform, delta_time);
        physics_apply_forces(physics, delta_time);
        physics_apply_torques(physics, delta_time);
        physics_integrate_linear(physics, transform, delta_time);
        
        if (physics->has_6dof) {
            physics_integrate_angular(physics, transform, delta_time);
        }
        
        // Prevent going below ground
        if (transform->position.y < -50.0f) {
            transform->position.y = -50.0f;
            physics->velocity.y = fmaxf(0.0f, physics->velocity.y);
        }
        
        transform->dirty = true;
        custom_physics_updates++;
    }
    
    // Debug logging
    static uint32_t log_counter = 0;
    if (++log_counter % 120 == 0) {  // Every 2 seconds
        uint32_t active_bodies = 0;
        uint32_t total_bodies = 0;
        float step_time = 0.0f;
        ode_get_statistics(ode_system, &active_bodies, &total_bodies, &step_time);
        
        printf("🔧 Physics: ODE bodies: %u/%u (%.2fms), Custom updates: %u\n", 
               active_bodies, total_bodies, step_time, custom_physics_updates);
    }
#else
    // ODE not available, use regular physics
    physics_system_update(world, render_config, delta_time);
    (void)ode_system;
#endif
}

void physics_set_use_ode(struct Physics* physics, bool use_ode)
{
    if (!physics) return;
    physics->use_ode = use_ode;
    
    // Clear ODE handles if disabling
    if (!use_ode) {
        physics->ode_body = NULL;
        physics->ode_geom = NULL;
    }
}

bool physics_is_using_ode(const struct Physics* physics)
{
    return physics ? physics->use_ode : false;
}