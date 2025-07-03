/**
 * @file ode_physics.c
 * @brief ODE physics system implementation
 */

#include "ode_physics.h"
#include "thrusters.h"
#include "../component/physics.h"
#include "../core/math_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// SYSTEM LIFECYCLE
// ============================================================================

bool ode_physics_init(ODEPhysicsSystem* system, uint32_t max_entities) {
    if (!system) return false;
    
    printf("🌐 Initializing ODE Physics System...\n");
    
    // Initialize ODE
    if (!dInitODE2(0)) {
        printf("❌ Failed to initialize ODE\n");
        return false;
    }
    
    // Create the world
    system->world = dWorldCreate();
    if (!system->world) {
        printf("❌ Failed to create ODE world\n");
        return false;
    }
    
    // Configure world parameters
    dWorldSetGravity(system->world, 0, ODE_GRAVITY, 0);  // Zero gravity for space
    dWorldSetERP(system->world, ODE_ERP);
    dWorldSetCFM(system->world, ODE_CFM);
    
    // Set solver parameters
    dWorldSetQuickStepNumIterations(system->world, ODE_MAX_ITERATIONS);
    
    #if ODE_ENABLE_ISLAND_SLEEPING
    dWorldSetAutoDisableFlag(system->world, 1);
    dWorldSetAutoDisableLinearThreshold(system->world, 0.01);
    dWorldSetAutoDisableAngularThreshold(system->world, 0.01);
    dWorldSetAutoDisableSteps(system->world, 10);
    dWorldSetAutoDisableTime(system->world, 0.5);
    #endif
    
    // Create collision space (hash space for now, can optimize later)
    system->space = dHashSpaceCreate(0);
    if (!system->space) {
        printf("❌ Failed to create ODE space\n");
        dWorldDestroy(system->world);
        return false;
    }
    
    // Create contact group
    system->contact_group = dJointGroupCreate(0);
    
    // Allocate entity mapping arrays
    system->map_capacity = max_entities;
    system->body_map = calloc(max_entities, sizeof(dBodyID));
    system->geom_map = calloc(max_entities, sizeof(dGeomID));
    
    if (!system->body_map || !system->geom_map) {
        printf("❌ Failed to allocate entity mapping arrays\n");
        ode_physics_shutdown(system);
        return false;
    }
    
    // Initialize timing
    system->fixed_timestep = ODE_STEP_SIZE;
    system->accumulator = 0.0f;
    system->max_iterations = ODE_MAX_ITERATIONS;
    
    // Initialize statistics
    system->active_bodies = 0;
    system->total_bodies = 0;
    system->last_step_time = 0.0f;
    
    printf("✅ ODE Physics initialized:\n");
    printf("   - Gravity: %.1f\n", ODE_GRAVITY);
    printf("   - Step size: %.3fs (%.0f Hz)\n", ODE_STEP_SIZE, 1.0f/ODE_STEP_SIZE);
    printf("   - Max iterations: %d\n", ODE_MAX_ITERATIONS);
    printf("   - Entity capacity: %u\n", max_entities);
    
    return true;
}

void ode_physics_shutdown(ODEPhysicsSystem* system) {
    if (!system) return;
    
    printf("🌐 Shutting down ODE Physics System...\n");
    
    // Destroy all bodies and geometries
    if (system->body_map && system->geom_map) {
        for (uint32_t i = 0; i < system->map_capacity; i++) {
            if (system->geom_map[i]) {
                dGeomDestroy(system->geom_map[i]);
            }
            if (system->body_map[i]) {
                dBodyDestroy(system->body_map[i]);
            }
        }
    }
    
    // Free mapping arrays
    free(system->body_map);
    free(system->geom_map);
    
    // Destroy ODE objects
    if (system->contact_group) {
        dJointGroupDestroy(system->contact_group);
    }
    if (system->space) {
        dSpaceDestroy(system->space);
    }
    if (system->world) {
        dWorldDestroy(system->world);
    }
    
    // Close ODE
    dCloseODE();
    
    // Clear the structure
    memset(system, 0, sizeof(ODEPhysicsSystem));
    
    printf("✅ ODE Physics shutdown complete\n");
}

void ode_physics_reset(ODEPhysicsSystem* system) {
    if (!system || !system->world) return;
    
    // Reset all bodies to initial state
    for (uint32_t i = 0; i < system->map_capacity; i++) {
        if (system->body_map[i]) {
            dBodySetPosition(system->body_map[i], 0, 0, 0);
            dBodySetLinearVel(system->body_map[i], 0, 0, 0);
            dBodySetAngularVel(system->body_map[i], 0, 0, 0);
            dBodySetForce(system->body_map[i], 0, 0, 0);
            dBodySetTorque(system->body_map[i], 0, 0, 0);
        }
    }
    
    // Reset timing
    system->accumulator = 0.0f;
    
    printf("🔄 ODE Physics system reset\n");
}

// ============================================================================
// SIMULATION
// ============================================================================

void ode_physics_step(ODEPhysicsSystem* system, struct World* world, float delta_time) {
    if (!system || !system->world || !world) return;
    
    // Track step timing
    float step_start = world->total_time;
    
    // Fixed timestep with interpolation
    system->accumulator += delta_time;
    
    int steps = 0;
    while (system->accumulator >= system->fixed_timestep) {
        // Pre-step: Sync components to ODE and apply forces
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            
            // Skip entities without physics
            if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
            
            struct Physics* physics = entity->physics;
            if (!physics || !physics->use_ode) continue;
            
            EntityID entity_id = entity->id;
            dBodyID body = ode_get_body(system, entity_id);
            if (!body) continue;
            
            // Sync component state to ODE
            struct Transform* transform = entity->transform;
            if (transform) {
                ode_sync_to_body(body, physics, transform);
            }
            
            // Apply forces
            if (entity->component_mask & COMPONENT_THRUSTER_SYSTEM) {
                struct ThrusterSystem* thrusters = entity->thruster_system;
                if (thrusters && thrusters->thrusters_enabled) {
                    ode_apply_thruster_forces(body, thrusters, &transform->rotation);
                }
            }
            
            // Apply environmental forces
            ode_apply_environmental_forces(body, physics);
        }
        
        // Collision detection
        dSpaceCollide(system->space, system, ode_near_callback);
        
        // Step the world
        #if ODE_ENABLE_QUICKSTEP
        dWorldQuickStep(system->world, system->fixed_timestep);
        #else
        dWorldStep(system->world, system->fixed_timestep);
        #endif
        
        // Clear contact joints
        dJointGroupEmpty(system->contact_group);
        
        system->accumulator -= system->fixed_timestep;
        steps++;
        
        // Prevent spiral of death
        if (steps > 5) {
            system->accumulator = 0;
            break;
        }
    }
    
    // Post-step: Sync ODE state back to components
    system->active_bodies = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
        
        struct Physics* physics = entity->physics;
        if (!physics || !physics->use_ode) continue;
        
        EntityID entity_id = entity->id;
        dBodyID body = ode_get_body(system, entity_id);
        if (!body) continue;
        
        // Sync ODE state back to components
        struct Transform* transform = entity->transform;
        if (transform) {
            ode_sync_from_body(body, physics, transform);
        }
        
        // Count active bodies
        if (dBodyIsEnabled(body)) {
            system->active_bodies++;
        }
    }
    
    // Update timing stats
    system->last_step_time = (world->total_time - step_start) * 1000.0f; // Convert to ms
}

void ode_physics_force_update(ODEPhysicsSystem* system, struct World* world) {
    if (!system || !world) return;
    
    // Force a single step
    float old_accumulator = system->accumulator;
    system->accumulator = system->fixed_timestep;
    ode_physics_step(system, world, 0);
    system->accumulator = old_accumulator;
}

// ============================================================================
// BODY MANAGEMENT
// ============================================================================

dBodyID ode_create_body(ODEPhysicsSystem* system, struct World* world, EntityID entity_id) {
    if (!system || !world || entity_id == INVALID_ENTITY) return 0;
    if (entity_id >= system->map_capacity) {
        printf("⚠️ Entity ID %u exceeds ODE mapping capacity\n", entity_id);
        return 0;
    }
    
    // Check if body already exists
    if (system->body_map[entity_id]) {
        return system->body_map[entity_id];
    }
    
    // Get entity and components
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return 0;
    
    struct Physics* physics = entity->physics;
    struct Transform* transform = entity->transform;
    
    if (!physics || !transform) {
        printf("⚠️ Entity %u missing required components for ODE body\n", entity_id);
        return 0;
    }
    
    // Create ODE body
    dBodyID body = dBodyCreate(system->world);
    if (!body) {
        printf("❌ Failed to create ODE body for entity %u\n", entity_id);
        return 0;
    }
    
    // Set mass properties
    dMass mass;
    dMassSetZero(&mass);
    
    // For now, use a sphere mass distribution
    // TODO: Support different mass distributions based on ship type
    dMassSetSphere(&mass, 1.0, 1.0);  // density=1, radius=1
    dMassAdjust(&mass, physics->mass);
    
    // Apply moment of inertia if specified
    if (physics->has_6dof && physics->moment_of_inertia.x > 0) {
        mass.I[0] = physics->moment_of_inertia.x;
        mass.I[5] = physics->moment_of_inertia.y;
        mass.I[10] = physics->moment_of_inertia.z;
    }
    
    dBodySetMass(body, &mass);
    
    // Set initial position and orientation
    dBodySetPosition(body, transform->position.x, transform->position.y, transform->position.z);
    dQuaternion q;
    quaternion_to_ode(&transform->rotation, q);
    dBodySetQuaternion(body, q);
    
    // Set initial velocities
    dBodySetLinearVel(body, physics->velocity.x, physics->velocity.y, physics->velocity.z);
    dBodySetAngularVel(body, physics->angular_velocity.x, physics->angular_velocity.y, physics->angular_velocity.z);
    
    // Configure damping
    dBodySetLinearDamping(body, physics->drag_linear);
    dBodySetAngularDamping(body, physics->drag_angular);
    
    // Set kinematic flag
    if (physics->kinematic) {
        dBodySetKinematic(body);
    }
    
    // Store in mapping
    system->body_map[entity_id] = body;
    system->total_bodies++;
    
    // Store entity ID in body user data
    dBodySetData(body, (void*)(uintptr_t)entity_id);
    
    printf("✅ Created ODE body for entity %u (mass: %.1f kg)\n", entity_id, physics->mass);
    
    return body;
}

void ode_destroy_body(ODEPhysicsSystem* system, EntityID entity_id) {
    if (!system || entity_id >= system->map_capacity) return;
    
    dBodyID body = system->body_map[entity_id];
    if (body) {
        dBodyDestroy(body);
        system->body_map[entity_id] = 0;
        system->total_bodies--;
        printf("🗑️ Destroyed ODE body for entity %u\n", entity_id);
    }
}

dBodyID ode_get_body(ODEPhysicsSystem* system, EntityID entity_id) {
    if (!system || entity_id >= system->map_capacity) return 0;
    return system->body_map[entity_id];
}

// ============================================================================
// COLLISION GEOMETRY
// ============================================================================

dGeomID ode_create_geometry(ODEPhysicsSystem* system, struct World* world, EntityID entity_id) {
    if (!system || !world || entity_id == INVALID_ENTITY) return 0;
    if (entity_id >= system->map_capacity) return 0;
    
    // Check if geometry already exists
    if (system->geom_map[entity_id]) {
        return system->geom_map[entity_id];
    }
    
    // Get entity and collision component
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity || !(entity->component_mask & COMPONENT_COLLISION)) return 0;
    
    struct Collision* collision = entity->collision;
    if (!collision) return 0;
    
    dGeomID geom = 0;
    
    // Create geometry based on shape type
    switch (collision->shape) {
        case COLLISION_SPHERE:
            geom = dCreateSphere(system->space, collision->radius);
            break;
            
        case COLLISION_BOX:
            geom = dCreateBox(system->space, 
                            collision->box_size.x * 2,  // ODE uses full extents
                            collision->box_size.y * 2,
                            collision->box_size.z * 2);
            break;
            
        case COLLISION_CAPSULE:
            geom = dCreateCapsule(system->space, collision->capsule.radius, collision->capsule.height);
            break;
            
        default:
            printf("⚠️ Unknown collision shape for entity %u\n", entity_id);
            return 0;
    }
    
    if (!geom) {
        printf("❌ Failed to create ODE geometry for entity %u\n", entity_id);
        return 0;
    }
    
    // Attach to body if it exists
    dBodyID body = ode_get_body(system, entity_id);
    if (body) {
        dGeomSetBody(geom, body);
    }
    
    // Set collision layers
    ode_set_collision_layers(geom, collision->layer_mask, collision->layer_mask);
    
    // Store entity ID in geom user data
    dGeomSetData(geom, (void*)(uintptr_t)entity_id);
    
    // Store in mapping
    system->geom_map[entity_id] = geom;
    
    return geom;
}

void ode_destroy_geometry(ODEPhysicsSystem* system, EntityID entity_id) {
    if (!system || entity_id >= system->map_capacity) return;
    
    dGeomID geom = system->geom_map[entity_id];
    if (geom) {
        dGeomDestroy(geom);
        system->geom_map[entity_id] = 0;
    }
}

// ============================================================================
// COMPONENT SYNCHRONIZATION
// ============================================================================

void ode_sync_to_body(dBodyID body, const struct Physics* physics, const struct Transform* transform) {
    if (!body || !physics || !transform) return;
    
    // Set position
    dBodySetPosition(body, transform->position.x, transform->position.y, transform->position.z);
    
    // Set orientation
    dQuaternion q;
    quaternion_to_ode(&transform->rotation, q);
    dBodySetQuaternion(body, q);
    
    // Set velocities
    dBodySetLinearVel(body, physics->velocity.x, physics->velocity.y, physics->velocity.z);
    dBodySetAngularVel(body, physics->angular_velocity.x, physics->angular_velocity.y, physics->angular_velocity.z);
    
    // Clear force accumulators (will be reapplied)
    dBodySetForce(body, 0, 0, 0);
    dBodySetTorque(body, 0, 0, 0);
}

void ode_sync_from_body(dBodyID body, struct Physics* physics, struct Transform* transform) {
    if (!body || !physics || !transform) return;
    
    // Get position
    const dReal* pos = dBodyGetPosition(body);
    transform->position.x = pos[0];
    transform->position.y = pos[1];
    transform->position.z = pos[2];
    
    // Get orientation
    const dReal* q = dBodyGetQuaternion(body);
    transform->rotation = ode_to_quaternion(q);
    
    // Get velocities
    const dReal* vel = dBodyGetLinearVel(body);
    physics->velocity.x = vel[0];
    physics->velocity.y = vel[1];
    physics->velocity.z = vel[2];
    
    const dReal* avel = dBodyGetAngularVel(body);
    physics->angular_velocity.x = avel[0];
    physics->angular_velocity.y = avel[1];
    physics->angular_velocity.z = avel[2];
    
    // Mark transform as dirty for matrix update
    transform->dirty = true;
}

// ============================================================================
// FORCE APPLICATION
// ============================================================================

void ode_apply_thruster_forces(dBodyID body, const struct ThrusterSystem* thrusters, const Quaternion* orientation) {
    if (!body || !thrusters || !orientation) return;
    
    // Calculate forces in world space
    Vector3 local_force = {
        thrusters->current_linear_thrust.x * thrusters->max_linear_force.x,
        thrusters->current_linear_thrust.y * thrusters->max_linear_force.y,
        thrusters->current_linear_thrust.z * thrusters->max_linear_force.z
    };
    
    // Transform to world space
    Vector3 world_force = quaternion_rotate_vector(*orientation, local_force);
    
    // Apply force
    dBodyAddForce(body, world_force.x, world_force.y, world_force.z);
    
    // Calculate and apply torques
    Vector3 torque = {
        thrusters->current_angular_thrust.x * thrusters->max_angular_torque.x,
        thrusters->current_angular_thrust.y * thrusters->max_angular_torque.y,
        thrusters->current_angular_thrust.z * thrusters->max_angular_torque.z
    };
    
    // Transform torque to world space
    Vector3 world_torque = quaternion_rotate_vector(*orientation, torque);
    
    // Apply torque
    dBodyAddTorque(body, world_torque.x, world_torque.y, world_torque.z);
}

void ode_apply_environmental_forces(dBodyID body, const struct Physics* physics) {
    if (!body || !physics) return;
    
    // ODE handles linear and angular damping internally via
    // dBodySetLinearDamping and dBodySetAngularDamping
    // which we set during body creation
    
    // Additional environmental effects can be added here
    // For example: solar wind, magnetic fields, etc.
}

// ============================================================================
// COLLISION HANDLING
// ============================================================================

void ode_near_callback(void* data, dGeomID o1, dGeomID o2) {
    ODEPhysicsSystem* system = (ODEPhysicsSystem*)data;
    if (!system) return;
    
    // Get bodies
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);
    
    // Exit if both bodies are connected by a joint
    if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact)) {
        return;
    }
    
    // Create contact array
    dContact contacts[ODE_MAX_CONTACTS];
    
    // Collide
    int num_contacts = dCollide(o1, o2, ODE_MAX_CONTACTS, &contacts[0].geom, sizeof(dContact));
    
    if (num_contacts > 0) {
        // Get entity IDs from user data
        EntityID id1 = (EntityID)(uintptr_t)dGeomGetData(o1);
        EntityID id2 = (EntityID)(uintptr_t)dGeomGetData(o2);
        
        // Create contact joints
        for (int i = 0; i < num_contacts; i++) {
            // Configure contact parameters
            contacts[i].surface.mode = dContactBounce | dContactSoftCFM;
            contacts[i].surface.mu = 0.5;                    // Friction
            contacts[i].surface.bounce = 0.1;               // Restitution
            contacts[i].surface.bounce_vel = 0.1;           // Minimum velocity for bounce
            contacts[i].surface.soft_cfm = 0.001;           // Softness
            
            // Create contact joint
            dJointID c = dJointCreateContact(system->world, system->contact_group, &contacts[i]);
            dJointAttach(c, b1, b2);
        }
        
        // Debug output
        static uint32_t collision_counter = 0;
        if (++collision_counter % 10 == 0) {  // Log every 10th collision
            printf("💥 Collision: Entity %u ↔ Entity %u (%d contacts)\n", id1, id2, num_contacts);
        }
    }
}

void ode_set_collision_layers(dGeomID geom, uint32_t category_bits, uint32_t collide_bits) {
    if (!geom) return;
    
    // ODE doesn't have built-in collision filtering like this
    // We would need to implement it in the near_callback
    // For now, store in geom user data alongside entity ID
    // This is a simplified implementation
    
    // TODO: Implement proper collision filtering
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void ode_get_statistics(const ODEPhysicsSystem* system, uint32_t* active_bodies, 
                       uint32_t* total_bodies, float* step_time) {
    if (!system) return;
    
    if (active_bodies) *active_bodies = system->active_bodies;
    if (total_bodies) *total_bodies = system->total_bodies;
    if (step_time) *step_time = system->last_step_time;
}

void ode_set_debug_draw(ODEPhysicsSystem* system, bool enable) {
    if (!system) return;
    
    // TODO: Implement debug drawing
    (void)enable;
}