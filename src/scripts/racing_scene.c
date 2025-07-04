// src/scripts/derelict_navigation_scene.c
// Derelict navigation scene - 3D magnetic racing through broken ship sections

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
#include "../graphics_api.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Derelict navigation state
static bool derelict_nav_initialized = false;
static float navigation_time = 0.0f;
static bool navigation_active = false;

// Gravitational constants for magnetic ship physics
#define GRAVITY_RANGE 100.0f
#define MAGNETIC_ATTRACTION_FORCE 50.0f
#define ORIENTATION_ADJUSTMENT_SPEED 2.0f

void derelict_navigation_init(struct World* world, RenderConfig* render_config) {
    (void)world;
    (void)render_config;
    
    if (derelict_nav_initialized) return;
    
    printf("🧲 Initializing Derelict Navigation Scene\n");
    
    navigation_time = 0.0f;
    navigation_active = true;
    derelict_nav_initialized = true;
    
    printf("🧲 Derelict navigation initialized - Magnetic systems online!\n");
}

// Calculate gravitational influence and orientation
Vector3 calculate_nearest_mass_direction(Vector3 ship_pos, struct World* world) {
    Vector3 strongest_direction = {0, -1, 0}; // Default down
    float strongest_mass = 0.0f;
    float closest_distance = GRAVITY_RANGE;
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_TRANSFORM)) continue;
        struct Transform* transform = entity->transform;
        if (!transform) continue;
        
        // Calculate distance to potential gravity source
        Vector3 diff = {
            transform->position.x - ship_pos.x,
            transform->position.y - ship_pos.y,
            transform->position.z - ship_pos.z
        };
        
        float distance = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        // Determine mass based on entity type and scale
        float mass = 1.0f;
        float scale_factor = transform->scale.x * transform->scale.y * transform->scale.z;
        
        // Larger objects have more gravitational influence
        if (scale_factor > 5.0f) mass = scale_factor * 2.0f;
        if (entity->component_mask & COMPONENT_PHYSICS) mass *= 1.5f;
        
        // Check if this is the dominant gravity source
        float influence = mass / (distance + 1.0f); // +1 to avoid division by zero
        
        if (distance < GRAVITY_RANGE && influence > strongest_mass) {
            strongest_mass = influence;
            closest_distance = distance;
            
            // Direction vector from ship to mass (normalized)
            if (distance > 0.1f) {
                strongest_direction.x = diff.x / distance;
                strongest_direction.y = diff.y / distance;
                strongest_direction.z = diff.z / distance;
            }
        }
    }
    
    return strongest_direction;
}

void derelict_navigation_update(struct World* world, RenderConfig* render_config, float delta_time) {
    if (!derelict_nav_initialized || !navigation_active) return;
    
    navigation_time += delta_time;
    
    // Apply magnetic ship physics to all ships
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Apply magnetic navigation to ships
        if (entity->component_mask & (COMPONENT_PHYSICS | COMPONENT_TRANSFORM)) {
            struct Physics* physics = entity->physics;
            struct Transform* transform = entity->transform;
            
            if (!physics || !transform) continue;
            
            // Calculate nearest mass and gravitational direction
            Vector3 gravity_dir = calculate_nearest_mass_direction(transform->position, world);
            
            // Apply magnetic attraction force toward nearest mass
            float attraction_force = MAGNETIC_ATTRACTION_FORCE * delta_time;
            physics->acceleration.x += gravity_dir.x * attraction_force;
            physics->acceleration.y += gravity_dir.y * attraction_force;
            physics->acceleration.z += gravity_dir.z * attraction_force;
            
            // Add navigation thrust for active ships
            if (entity->id > 0) { // All ships get some movement
                float thrust_multiplier = 1.0f;
                
                // Calculate distance to nearest major mass
                Vector3 pos = transform->position;
                float min_distance = 1000.0f;
                
                for (uint32_t j = 0; j < world->entity_count; j++) {
                    struct Entity* other = &world->entities[j];
                    if (i == j || !(other->component_mask & COMPONENT_TRANSFORM)) continue;
                    
                    struct Transform* other_transform = other->transform;
                    if (!other_transform) continue;
                    
                    float dx = pos.x - other_transform->position.x;
                    float dy = pos.y - other_transform->position.y;
                    float dz = pos.z - other_transform->position.z;
                    float distance = sqrtf(dx*dx + dy*dy + dz*dz);
                    
                    if (distance < min_distance) {
                        min_distance = distance;
                    }
                }
                
                // Ships closer to obstacles get more maneuverability
                if (min_distance < 50.0f) {
                    thrust_multiplier = 1.0f + (50.0f - min_distance) / 50.0f * 2.0f;
                }
                
                // Add forward/sideways navigation thrust
                float nav_x = (float)(rand() % 200 - 100) / 100.0f * 15.0f * thrust_multiplier;
                float nav_z = (float)(rand() % 200 - 100) / 100.0f * 15.0f * thrust_multiplier;
                
                physics->acceleration.x += nav_x * delta_time;
                physics->acceleration.z += nav_z * delta_time;
                
                // Add slight upward thrust to prevent ships from crashing
                physics->acceleration.y += 5.0f * thrust_multiplier * delta_time;
            }
            
            // Apply drag to prevent runaway acceleration
            physics->velocity.x *= 0.98f;
            physics->velocity.y *= 0.98f;
            physics->velocity.z *= 0.98f;
        }
    }
    
    // Debug output every few seconds
    static float last_debug = 0.0f;
    if (navigation_time - last_debug > 4.0f) {
        printf("🧲 Navigation time: %.1fs - Magnetic derelict navigation active\n", navigation_time);
        last_debug = navigation_time;
    }
    
    (void)render_config;
}

static bool racing_scene_input(struct World* world, SceneStateManager* state, const void* event) {
    (void)world; // Unused parameter
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
    {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE)
        {
            printf("🏁 Racing scene: ESC pressed, returning to navigation menu\n");
            scene_state_request_transition(state, "navigation_menu");
            return true; // Event handled
        }
    }
    
    return false; // Event not handled
}

void racing_scene_cleanup(struct World* world, RenderConfig* render_config) {
    (void)world;
    (void)render_config;
    
    if (!racing_initialized) return;
    
    race_active = false;
    racing_initialized = false;
    
    printf("🏁 Racing scene cleanup complete\n");
}

// ============================================================================
// RACING SCRIPT DEFINITION
// ============================================================================

const SceneScript racing_script = {
    .scene_name = "racing",
    .on_enter = NULL, // Will use data-driven initialization
    .on_update = NULL, // Will use systems-based update
    .on_exit = NULL, // Will use data-driven cleanup
    .on_input = racing_scene_input
};
