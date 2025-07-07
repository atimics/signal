/**
 * @file ode_physics.h
 * @brief ODE (Open Dynamics Engine) physics system integration
 * 
 * This module provides a wrapper around ODE to integrate it with the ECS
 * architecture while maintaining the existing physics component interface.
 */

#ifndef ODE_PHYSICS_H
#define ODE_PHYSICS_H

#include "../core.h"
#include <ode/ode.h>
#include <stdbool.h>

// ============================================================================
// ODE CONFIGURATION
// ============================================================================

// Physics constants
#define ODE_GRAVITY 0.0f           // Zero gravity for space
#define ODE_STEP_SIZE 0.01f        // 100Hz physics updates
#define ODE_MAX_ITERATIONS 20      // Solver iterations
#define ODE_MAX_CONTACTS 64        // Max contacts per collision

// Solver parameters
#define ODE_ERP 0.2f               // Error reduction parameter (0.1-0.8)
#define ODE_CFM 1e-5f              // Constraint force mixing (softness)

// Performance settings
#define ODE_ENABLE_QUICKSTEP 1     // Use faster QuickStep solver
#define ODE_ENABLE_ISLAND_SLEEPING 1  // Auto-disable inactive bodies

// ============================================================================
// ODE PHYSICS SYSTEM
// ============================================================================

/**
 * @brief Main ODE physics system structure
 */
typedef struct ODEPhysicsSystem {
    // Core ODE objects
    dWorldID world;                 // The ODE world
    dSpaceID space;                 // Collision space (quadtree/hash)
    dJointGroupID contact_group;    // Temporary contact joints
    
    // Configuration
    float fixed_timestep;           // Fixed timestep for simulation
    float accumulator;              // Time accumulator for fixed steps
    int max_iterations;             // Solver iteration limit
    
    // Entity mapping (sparse arrays indexed by EntityID)
    dBodyID* body_map;              // Maps EntityID -> ODE body
    dGeomID* geom_map;              // Maps EntityID -> ODE geometry
    uint32_t map_capacity;          // Size of mapping arrays
    
    // Statistics
    uint32_t active_bodies;         // Number of active ODE bodies
    uint32_t total_bodies;          // Total ODE bodies created
    float last_step_time;           // Time taken for last physics step
    
    // Callbacks
    void* collision_user_data;      // User data for collision callback
} ODEPhysicsSystem;

// ============================================================================
// SYSTEM LIFECYCLE
// ============================================================================

/**
 * @brief Initialize the ODE physics system
 * @param system The ODE physics system to initialize
 * @param max_entities Maximum number of entities that can have ODE bodies
 * @return true on success, false on failure
 */
bool ode_physics_init(ODEPhysicsSystem* system, uint32_t max_entities);

/**
 * @brief Shutdown the ODE physics system and free resources
 * @param system The ODE physics system to shutdown
 */
void ode_physics_shutdown(ODEPhysicsSystem* system);

/**
 * @brief Reset the physics system to initial state
 * @param system The ODE physics system to reset
 */
void ode_physics_reset(ODEPhysicsSystem* system);

// ============================================================================
// SIMULATION
// ============================================================================

/**
 * @brief Step the ODE physics simulation
 * @param system The ODE physics system
 * @param world The ECS world for component access
 * @param delta_time Time since last update in seconds
 */
void ode_physics_step(ODEPhysicsSystem* system, struct World* world, float delta_time);

/**
 * @brief Force an immediate physics update (bypasses fixed timestep)
 * @param system The ODE physics system
 * @param world The ECS world
 */
void ode_physics_force_update(ODEPhysicsSystem* system, struct World* world);

// ============================================================================
// BODY MANAGEMENT
// ============================================================================

/**
 * @brief Create an ODE body for an entity
 * @param system The ODE physics system
 * @param world The ECS world
 * @param entity_id The entity to create a body for
 * @return The created dBodyID, or 0 on failure
 */
dBodyID ode_create_body(ODEPhysicsSystem* system, struct World* world, EntityID entity_id);

/**
 * @brief Destroy an ODE body for an entity
 * @param system The ODE physics system
 * @param entity_id The entity whose body to destroy
 */
void ode_destroy_body(ODEPhysicsSystem* system, EntityID entity_id);

/**
 * @brief Get the ODE body for an entity
 * @param system The ODE physics system
 * @param entity_id The entity ID
 * @return The dBodyID, or 0 if not found
 */
dBodyID ode_get_body(ODEPhysicsSystem* system, EntityID entity_id);

// ============================================================================
// COLLISION GEOMETRY
// ============================================================================

/**
 * @brief Create collision geometry for an entity
 * @param system The ODE physics system
 * @param world The ECS world
 * @param entity_id The entity to create geometry for
 * @return The created dGeomID, or 0 on failure
 */
dGeomID ode_create_geometry(ODEPhysicsSystem* system, struct World* world, EntityID entity_id);

/**
 * @brief Destroy collision geometry for an entity
 * @param system The ODE physics system
 * @param entity_id The entity whose geometry to destroy
 */
void ode_destroy_geometry(ODEPhysicsSystem* system, EntityID entity_id);

// ============================================================================
// COMPONENT SYNCHRONIZATION
// ============================================================================

/**
 * @brief Sync ECS component data to ODE body (before simulation)
 * @param body The ODE body
 * @param physics The Physics component
 * @param transform The Transform component
 */
void ode_sync_to_body(dBodyID body, const struct Physics* physics, const struct Transform* transform);

/**
 * @brief Sync ODE body data back to ECS components (after simulation)
 * @param body The ODE body
 * @param physics The Physics component to update
 * @param transform The Transform component to update
 */
void ode_sync_from_body(dBodyID body, struct Physics* physics, struct Transform* transform);

// ============================================================================
// FORCE APPLICATION
// ============================================================================

/**
 * @brief Apply forces from thruster system to ODE body
 * @param body The ODE body
 * @param thrusters The thruster system component
 * @param orientation The entity's orientation quaternion
 */
void ode_apply_thruster_forces(dBodyID body, const struct ThrusterSystem* thrusters, const Quaternion* orientation);

/**
 * @brief Apply environmental forces (drag, etc.) to ODE body
 * @param body The ODE body
 * @param physics The physics component with drag coefficients
 */
void ode_apply_environmental_forces(dBodyID body, const struct Physics* physics);

// ============================================================================
// COLLISION HANDLING
// ============================================================================

/**
 * @brief ODE near callback for collision detection
 * @param data User data (ODEPhysicsSystem*)
 * @param o1 First geometry
 * @param o2 Second geometry
 */
void ode_near_callback(void* data, dGeomID o1, dGeomID o2);

/**
 * @brief Set collision layers for a geometry
 * @param geom The ODE geometry
 * @param category_bits What this geometry is
 * @param collide_bits What this geometry collides with
 */
void ode_set_collision_layers(dGeomID geom, uint32_t category_bits, uint32_t collide_bits);

// ============================================================================
// CONSTRAINTS AND JOINTS
// ============================================================================

/**
 * @brief Create a fixed joint between two entities
 * @param system The ODE physics system
 * @param entity1 First entity
 * @param entity2 Second entity
 * @return The created joint ID, or 0 on failure
 */
dJointID ode_create_fixed_joint(ODEPhysicsSystem* system, EntityID entity1, EntityID entity2);

/**
 * @brief Create a hinge joint between two entities
 * @param system The ODE physics system
 * @param entity1 First entity
 * @param entity2 Second entity
 * @param anchor Anchor point in world space
 * @param axis Hinge axis in world space
 * @return The created joint ID, or 0 on failure
 */
dJointID ode_create_hinge_joint(ODEPhysicsSystem* system, EntityID entity1, EntityID entity2,
                               Vector3 anchor, Vector3 axis);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Convert Vector3 to ODE format
 */
static inline void vector3_to_ode(const Vector3* v, dReal* out) {
    out[0] = v->x;
    out[1] = v->y;
    out[2] = v->z;
}

/**
 * @brief Convert ODE vector to Vector3
 */
static inline Vector3 ode_to_vector3(const dReal* v) {
    return (Vector3){v[0], v[1], v[2]};
}

/**
 * @brief Convert Quaternion to ODE format
 */
static inline void quaternion_to_ode(const Quaternion* q, dQuaternion out) {
    out[0] = q->w;  // ODE uses w,x,y,z order
    out[1] = q->x;
    out[2] = q->y;
    out[3] = q->z;
}

/**
 * @brief Convert ODE quaternion to Quaternion
 */
static inline Quaternion ode_to_quaternion(const dQuaternion q) {
    return (Quaternion){q[1], q[2], q[3], q[0]};  // Convert from w,x,y,z to x,y,z,w
}

// ============================================================================
// DEBUG AND VISUALIZATION
// ============================================================================

/**
 * @brief Get physics statistics
 * @param system The ODE physics system
 * @param active_bodies Output: number of active bodies
 * @param total_bodies Output: total number of bodies
 * @param step_time Output: last physics step time in ms
 */
void ode_get_statistics(const ODEPhysicsSystem* system, uint32_t* active_bodies, 
                       uint32_t* total_bodies, float* step_time);

/**
 * @brief Enable/disable ODE debug drawing
 * @param system The ODE physics system
 * @param enable Whether to enable debug drawing
 */
void ode_set_debug_draw(ODEPhysicsSystem* system, bool enable);

#endif // ODE_PHYSICS_H