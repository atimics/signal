// src/scripts/racing_scene.c
// Racing scene script - High-speed ground-effect racing demonstration

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Racing scene state
static bool racing_initialized = false;
static float race_time = 0.0f;
static bool race_active = false;

void racing_scene_init(struct World* world, RenderConfig* render_config) {
    (void)world;
    (void)render_config;
    
    if (racing_initialized) return;
    
    printf("🏁 Initializing Racing Scene\n");
    
    // Setup racing-specific lighting can be done in render system
    // TODO: Enhance render system lighting for racing
    
    race_time = 0.0f;
    race_active = true;
    racing_initialized = true;
    
    printf("🏁 Racing scene initialized - Engines ready!\n");
}

void racing_scene_update(struct World* world, RenderConfig* render_config, float delta_time) {
    if (!racing_initialized || !race_active) return;
    
    race_time += delta_time;
    
    // Apply racing physics and effects
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Apply special effects to racing ships
        if (entity->component_mask & (COMPONENT_PHYSICS | COMPONENT_TRANSFORM)) {
            struct Physics* physics = entity->physics;
            struct Transform* transform = entity->transform;
            
            if (!physics || !transform) continue;
            
            // Add forward thrust for racing ships (AI and player)
            float altitude = transform->position.y;
            
            // Ships closer to ground get more thrust (ground effect racing)
            if (altitude < 30.0f) {
                float thrust_multiplier = 1.0f + (30.0f - altitude) / 30.0f * 0.5f;
                
                // Add forward acceleration
                physics->acceleration.x += 10.0f * thrust_multiplier * delta_time;
                physics->acceleration.z += 5.0f * thrust_multiplier * delta_time;
                
                // Add some random variations for AI ships
                if (entity->id > 1) { // Not the player ship
                    physics->acceleration.x += (float)(rand() % 100 - 50) / 100.0f;
                    physics->acceleration.z += (float)(rand() % 100 - 50) / 100.0f;
                }
            }
        }
    }
    
    // Debug output every few seconds
    static float last_debug = 0.0f;
    if (race_time - last_debug > 3.0f) {
        printf("🏁 Race time: %.1fs - Ground-effect racing active\n", race_time);
        last_debug = race_time;
    }
    
    // Keep race running indefinitely
    (void)render_config;
}

void racing_scene_cleanup(struct World* world, RenderConfig* render_config) {
    (void)world;
    (void)render_config;
    
    if (!racing_initialized) return;
    
    race_active = false;
    racing_initialized = false;
    
    printf("🏁 Racing scene cleanup complete\n");
}
