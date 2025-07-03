// Debug test to understand physics issues
#include "src/core.h"
#include "src/system/physics.h"
#include "src/system/thrusters.h"
#include <stdio.h>

// Stub functions to avoid graphics dependencies
bool assets_init(AssetRegistry* reg, const char* path) { return true; }
void assets_cleanup(AssetRegistry* reg) {}

int main() {
    struct World world;
    world_init(&world);
    
    // Create test entity
    EntityID entity = entity_create(&world);
    printf("Created entity: %d\n", entity);
    
    // Add components
    bool result = entity_add_components(&world, entity, 
                    COMPONENT_PHYSICS | COMPONENT_TRANSFORM | COMPONENT_THRUSTER_SYSTEM);
    printf("Add components result: %s\n", result ? "SUCCESS" : "FAILED");
    
    // Get components
    struct Physics* physics = entity_get_physics(&world, entity);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&world, entity);
    struct Transform* transform = entity_get_transform(&world, entity);
    
    printf("Physics pointer: %p\n", (void*)physics);
    printf("Thrusters pointer: %p\n", (void*)thrusters);
    printf("Transform pointer: %p\n", (void*)transform);
    
    if (physics && thrusters && transform) {
        // Initialize physics
        physics->mass = 100.0f;
        physics->force_accumulator = (Vector3){0, 0, 0};
        
        // Initialize thrusters
        thrusters->max_linear_force = (Vector3){500, 400, 600};
        thrusters->thrusters_enabled = true;
        
        // Set thrust command
        thruster_set_linear_command(thrusters, (Vector3){1.0f, 0.0f, 0.0f});
        
        printf("Before thruster update:\n");
        printf("  Force accumulator: [%.3f, %.3f, %.3f]\n", 
               physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z);
        
        // Update thruster system
        thruster_system_update(&world, NULL, 0.016f);
        
        printf("After thruster update:\n");
        printf("  Force accumulator: [%.3f, %.3f, %.3f]\n", 
               physics->force_accumulator.x, physics->force_accumulator.y, physics->force_accumulator.z);
        
        // Update physics system
        physics_system_update(&world, NULL, 0.016f);
        
        printf("After physics update:\n");
        printf("  Velocity: [%.3f, %.3f, %.3f]\n", 
               physics->velocity.x, physics->velocity.y, physics->velocity.z);
        printf("  Position: [%.3f, %.3f, %.3f]\n", 
               transform->position.x, transform->position.y, transform->position.z);
    } else {
        printf("CRITICAL: Failed to get component pointers!\n");
    }
    
    world_destroy(&world);
    return 0;
}
