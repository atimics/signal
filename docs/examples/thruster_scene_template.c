/**
 * Template for creating a scene with the simplified thruster system
 */

#include "../scene_script.h"
#include "../component/thruster_points_component.h"
#include "../system/physics.h"
#include "../assets.h"
#include "../systems.h"

// Example scene with thruster-equipped ship
void example_thruster_scene_enter(struct World* world, SceneStateManager* state) {
    (void)state;
    
    // Create ship entity
    EntityID ship = entity_create(world);
    entity_add_components(world, ship, 
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | 
        COMPONENT_RENDERABLE | COMPONENT_THRUSTER_SYSTEM);
    
    // Setup transform
    struct Transform* transform = entity_get_transform(world, ship);
    if (transform) {
        transform->position = (Vector3){0, 0, 0};
        transform->rotation = (Quaternion){0, 0, 0, 1};
        transform->scale = (Vector3){1, 1, 1};
    }
    
    // Setup physics
    struct Physics* physics = entity_get_physics(world, ship);
    if (physics) {
        physics->mass = 1000.0f;  // 1 ton
        physics->has_6dof = true;
        physics->drag_linear = 0.1f;
        physics->drag_angular = 0.2f;
        physics->moment_of_inertia = (Vector3){500, 800, 500};
    }
    
    // Setup rendering
    struct Renderable* renderable = entity_get_renderable(world, ship);
    if (renderable) {
        assets_create_renderable_from_mesh(get_asset_registry(), "wedge_ship_mk2", renderable);
    }
    
    // Setup legacy thruster system (will be converted by thruster_points_system)
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, ship);
    if (thrusters) {
        // Configure thrust capabilities
        thrusters->max_linear_force = (Vector3){5000, 3000, 20000};  // N
        thrusters->max_angular_torque = (Vector3){2000, 2000, 1000}; // Nm
        
        // Set efficiencies
        thrusters->atmosphere_efficiency = 0.8f;
        thrusters->vacuum_efficiency = 1.0f;
        
        // Enable thrusters
        thrusters->thrusters_enabled = true;
        thrusters->ship_type = SHIP_TYPE_FIGHTER;
    }
    
    // Optional: Load thruster point definitions for visual debugging
    // This would enable thrust cone visualization once integrated
    /*
    ThrusterPointsComponent* thruster_points = 
        thruster_points_component_create("data/thrusters/wedge_ship_mk2.thrusters");
    if (thruster_points) {
        const Mesh* mesh = assets_get_mesh(get_asset_registry(), "wedge_ship_mk2");
        thruster_points_component_init(thruster_points, mesh);
    }
    */
}

void example_thruster_scene_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)world;
    (void)state;
    (void)delta_time;
    
    // The thruster system update is handled automatically by the system scheduler
    // No need for manual updates here
}

void example_thruster_scene_exit(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    // Cleanup is handled automatically by the ECS
}