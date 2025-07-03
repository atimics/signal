#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/system/thrusters.h"
#include "../../src/system/physics.h"
#include <stdio.h>

static struct World test_world;

void setUp(void) {
    world_init(&test_world);
}

void tearDown(void) {
    world_destroy(&test_world);
}

void test_thruster_linear_force_generation_debug(void)
{
    printf("DEBUG: Starting test\n");
    
    EntityID entity = entity_create(&test_world);
    printf("DEBUG: Created entity %d\n", entity);
    
    bool success1 = entity_add_component(&test_world, entity, COMPONENT_THRUSTER_SYSTEM);
    printf("DEBUG: Add thruster component result: %s\n", success1 ? "true" : "false");
    
    bool success2 = entity_add_component(&test_world, entity, COMPONENT_PHYSICS);
    printf("DEBUG: Add physics component result: %s\n", success2 ? "true" : "false");
    
    bool success3 = entity_add_component(&test_world, entity, COMPONENT_TRANSFORM);
    printf("DEBUG: Add transform component result: %s\n", success3 ? "true" : "false");
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&test_world, entity);
    printf("DEBUG: Got thrusters: %p\n", (void*)thrusters);
    
    struct Physics* physics = entity_get_physics(&test_world, entity);
    printf("DEBUG: Got physics: %p\n", (void*)physics);
    
    if (!thrusters || !physics) {
        printf("ERROR: Missing components!\n");
        return;
    }
    
    // Set thruster capabilities
    thrusters->max_linear_force = (Vector3){ 100.0f, 80.0f, 120.0f };
    printf("DEBUG: Set max_linear_force\n");
    
    // Set thrust command
    thruster_set_linear_command(thrusters, (Vector3){ 0.5f, -0.25f, 1.0f });
    printf("DEBUG: Set thrust command\n");
    
    // Clear any existing forces
    physics->force_accumulator = (Vector3){ 0.0f, 0.0f, 0.0f };
    printf("DEBUG: Cleared forces\n");
    
    // Update thruster system
    printf("DEBUG: About to call thruster_system_update\n");
    thruster_system_update(&test_world, NULL, 0.016f);
    printf("DEBUG: thruster_system_update completed\n");
    
    printf("DEBUG: Test completed successfully\n");
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_thruster_linear_force_generation_debug);
    return UNITY_END();
}
