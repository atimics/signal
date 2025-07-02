#include "physics.h"

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
    if (physics->mass <= 0.0f) return;
    
    // Linear dynamics: F = ma -> a = F/m
    Vector3 linear_acceleration = vector3_multiply(physics->force_accumulator, 1.0f / physics->mass);
    physics->acceleration = linear_acceleration;
    
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
    
    // Clear torque accumulator for next frame
    physics->torque_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
}

// ============================================================================
// INTEGRATION FUNCTIONS
// ============================================================================

static void physics_integrate_linear(struct Physics* physics, struct Transform* transform, float delta_time)
{
    // Apply acceleration to velocity
    physics->velocity = vector3_add(physics->velocity, vector3_multiply(physics->acceleration, delta_time));
    
    // Apply drag
    physics->velocity = vector3_multiply(physics->velocity, physics->drag_linear);
    
    // Apply velocity to position
    transform->position = vector3_add(transform->position, vector3_multiply(physics->velocity, delta_time));
}

static void physics_integrate_angular(struct Physics* physics, struct Transform* transform, float delta_time)
{
    if (!physics->has_6dof) return;
    
    // Apply angular acceleration to angular velocity
    physics->angular_velocity = vector3_add(physics->angular_velocity, 
                                          vector3_multiply(physics->angular_acceleration, delta_time));
    
    // Apply angular drag
    physics->angular_velocity = vector3_multiply(physics->angular_velocity, physics->drag_angular);
    
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
            // Apply atmospheric drag (stronger than space)
            physics->drag_linear *= 0.98f;  // Additional atmospheric drag
            
            // Apply gravity if in atmosphere
            Vector3 gravity = { 0.0f, -9.81f * physics->mass, 0.0f };
            physics->force_accumulator = vector3_add(physics->force_accumulator, gravity);
            break;
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

    // Performance logging
    static uint32_t log_counter = 0;
    if (++log_counter % 600 == 0)  // Every 10 seconds at 60 FPS
    {
        printf("🔧 Physics: %d linear + %d angular updates (6DOF enabled)\n", 
               linear_updates, angular_updates);
    }
}

// ============================================================================
// PHYSICS UTILITY FUNCTIONS FOR EXTERNAL USE
// ============================================================================

void physics_add_force(struct Physics* physics, Vector3 force)
{
    if (!physics) return;
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