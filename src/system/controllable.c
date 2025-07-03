// src/system/controllable.c
// Implementation of controllable system

#include "controllable.h"
#include "thrusters.h"
#include "camera.h"
#include "../component/controllable.h"
#include "../core.h"
#include <stdio.h>

static EntityID g_player_entity = INVALID_ENTITY;

// Initialize controllable system
void controllable_system_init(void) {
    g_player_entity = INVALID_ENTITY;
    printf("✅ Controllable system initialized\n");
}

void controllable_system_shutdown(void) {
    printf("✅ Controllable system shut down\n");
}

// Process spaceship-type controllable
static void process_spaceship_controllable(struct World* world, struct Entity* entity, 
                                         Controllable* controllable, float delta_time) {
    (void)delta_time; // TODO: Use for interpolation
    // Get required components
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, entity->id);
    if (!thrusters) return;
    
    // Get input forces
    InputForceMapping forces = controllable_get_input_forces(controllable);
    
    // Convert to thruster commands
    Vector3 linear_command = {
        forces.left_right,
        forces.up_down,
        forces.forward_backward
    };
    
    Vector3 angular_command = {
        forces.pitch,
        forces.yaw,
        forces.roll
    };
    
    // Handle stabilization
    if (forces.stabilize && entity->physics) {
        // Counter current angular velocity for stabilization
        angular_command.x -= entity->physics->angular_velocity.x * 2.0f;
        angular_command.y -= entity->physics->angular_velocity.y * 2.0f;
        angular_command.z -= entity->physics->angular_velocity.z * 2.0f;
    }
    
    // Set thruster commands
    thruster_set_linear_command(thrusters, linear_command);
    thruster_set_angular_command(thrusters, angular_command);
}

// Process camera-type controllable
static void process_camera_controllable(struct World* world, struct Entity* entity,
                                      Controllable* controllable, float delta_time) {
    // Get camera component
    struct Camera* camera = entity_get_camera(world, entity->id);
    if (!camera) return;
    
    // Get input forces
    InputForceMapping forces = controllable_get_input_forces(controllable);
    
    // Camera control simplified for now
    // TODO: Implement proper camera control once camera struct is updated
    if (entity->transform) {
        entity->transform->position.x += forces.left_right * delta_time * 10.0f;
        entity->transform->position.y += forces.up_down * delta_time * 10.0f;
        entity->transform->position.z += forces.forward_backward * delta_time * 10.0f;
    }
    
    // Mark camera as dirty
    camera->matrices_dirty = true;
}

// Update controllable entities
void controllable_system_update(struct World* world, struct RenderConfig* render_config, float delta_time) {
    (void)render_config; // Not used currently
    if (!world) return;
    
    // Process all entities with controllable components
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Skip if no controllable component
        if (!(entity->component_mask & COMPONENT_CONTROLLABLE)) continue;
        
        Controllable* controllable = entity_get_controllable(world, entity->id);
        if (!controllable || !controllable->enabled) continue;
        
        // Update input for this controllable
        controllable_update_input(controllable, delta_time);
        
        // Process based on scheme type
        switch (controllable->scheme_type) {
            case INPUT_SCHEME_SPACESHIP_6DOF:
            case INPUT_SCHEME_SPACESHIP_ARCADE:
            case INPUT_SCHEME_VEHICLE_CAR:
                process_spaceship_controllable(world, entity, controllable, delta_time);
                break;
                
            case INPUT_SCHEME_FPS_CAMERA:
            case INPUT_SCHEME_ORBIT_CAMERA:
            case INPUT_SCHEME_RTS_CAMERA:
                process_camera_controllable(world, entity, controllable, delta_time);
                break;
                
            default:
                // Custom processing
                if (controllable->process_input) {
                    controllable->process_input(controllable, delta_time);
                }
                break;
        }
    }
}

// Set player entity
void controllable_system_set_player_entity(EntityID entity_id) {
    g_player_entity = entity_id;
    printf("🎮 Controllable system: Player entity set to %d\n", entity_id);
}

EntityID controllable_system_get_player_entity(void) {
    return g_player_entity;
}

// Enable/disable entity input
void controllable_system_enable_entity(struct World* world, EntityID entity_id, bool enabled) {
    if (!world) return;
    
    Controllable* controllable = entity_get_controllable(world, entity_id);
    if (controllable) {
        controllable->enabled = enabled;
        printf("🎮 Entity %d input %s\n", entity_id, enabled ? "enabled" : "disabled");
    }
}

// Debug print
void controllable_system_debug_print(struct World* world) {
    if (!world) return;
    
    printf("\n=== Controllable System Debug ===\n");
    printf("Player entity: %d\n", g_player_entity);
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_CONTROLLABLE)) continue;
        
        Controllable* controllable = entity_get_controllable(world, entity->id);
        if (!controllable) continue;
        
        printf("\nEntity %d:\n", entity->id);
        printf("  Scheme: %d, Enabled: %s\n", controllable->scheme_type, 
               controllable->enabled ? "Yes" : "No");
        printf("  Linear: [%.2f, %.2f, %.2f]\n", 
               controllable->current_input.left_right,
               controllable->current_input.up_down,
               controllable->current_input.forward_backward);
        printf("  Angular: [%.2f, %.2f, %.2f]\n",
               controllable->current_input.pitch,
               controllable->current_input.yaw,
               controllable->current_input.roll);
        printf("  Boost: %.2f, Brake: %.2f\n",
               controllable->current_input.boost,
               controllable->current_input.brake);
    }
    printf("=================================\n");
}