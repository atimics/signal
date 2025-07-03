#include "core.h"
#include <stdio.h>

int main() {
    struct World world;
    world_init(&world);
    
    EntityID entity = entity_create(&world);
    printf("Created entity ID: %u\n", entity);
    
    bool success = entity_add_component(&world, entity, COMPONENT_THRUSTER_SYSTEM);
    printf("Add component success: %s\n", success ? "true" : "false");
    
    struct ThrusterSystem* thrusters = entity_get_thruster_system(&world, entity);
    printf("Thruster pointer: %p\n", (void*)thrusters);
    
    if (thrusters) {
        printf("max_linear_force: [%f, %f, %f]\n", 
               thrusters->max_linear_force.x, 
               thrusters->max_linear_force.y, 
               thrusters->max_linear_force.z);
        printf("max_angular_torque: [%f, %f, %f]\n", 
               thrusters->max_angular_torque.x, 
               thrusters->max_angular_torque.y, 
               thrusters->max_angular_torque.z);
        printf("atmosphere_efficiency: %f\n", thrusters->atmosphere_efficiency);
        printf("vacuum_efficiency: %f\n", thrusters->vacuum_efficiency);
        printf("thrusters_enabled: %s\n", thrusters->thrusters_enabled ? "true" : "false");
    } else {
        printf("ERROR: Thrusters pointer is NULL!\n");
    }
    
    world_destroy(&world);
    return 0;
}
