// src/scripts/derelict_navigation_scene.c
// Derelict navigation scene - 3D magnetic racing through broken ship sections
// Demonstrates "sticky ship" physics where gravity/orientation relative to nearest mass

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Derelict navigation state
static bool derelict_nav_initialized = false;
static float navigation_time = 0.0f;
static bool navigation_active = false;

// Physics constants for magnetic ship behavior
#define GRAVITY_RANGE 120.0f
#define MAGNETIC_ATTRACTION_FORCE 25.0f
#define ORIENTATION_ADJUSTMENT_SPEED 1.5f
#define DERELICT_SECTION_COUNT 8

// Derelict navigation zones
typedef struct {
    Vector3 position;
    float mass;
    float magnetic_field;
    bool is_hazardous;
} DerelictSection;

static DerelictSection derelict_sections[DERELICT_SECTION_COUNT];

void derelict_navigation_init(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (derelict_nav_initialized) return;
    
    printf("🧲 Initializing Derelict Navigation Scene - Aethelian Command Ship\n");
    
    // Initialize derelict sections as gravity/magnetic sources
    derelict_sections[0] = (DerelictSection){{0, 0, 0}, 50.0f, 25.0f, false};      // Central command
    derelict_sections[1] = (DerelictSection){{-80, 0, 0}, 30.0f, 15.0f, false};   // Port section
    derelict_sections[2] = (DerelictSection){{80, 0, 0}, 30.0f, 15.0f, false};    // Starboard section
    derelict_sections[3] = (DerelictSection){{0, 0, -100}, 25.0f, 20.0f, true};   // Forward (damaged)
    derelict_sections[4] = (DerelictSection){{0, 0, 100}, 25.0f, 20.0f, false};   // Aft section
    derelict_sections[5] = (DerelictSection){{0, 40, 0}, 20.0f, 10.0f, false};    // Upper section
    derelict_sections[6] = (DerelictSection){{0, -40, 0}, 20.0f, 10.0f, true};    // Lower (breached)
    derelict_sections[7] = (DerelictSection){{-40, 20, -60}, 15.0f, 30.0f, true}; // Engine core (unstable)
    
    navigation_time = 0.0f;
    navigation_active = true;
    derelict_nav_initialized = true;
    
    printf("🧲 Derelict navigation initialized - %d sections detected\n", DERELICT_SECTION_COUNT);
    printf("📡 Magnetic field mapping complete - Sticky ship physics active\n");
}

// Calculate the dominant gravity/magnetic source for ship orientation
Vector3 calculate_dominant_mass_direction(Vector3 ship_pos, float* out_field_strength) {
    Vector3 net_direction = {0, 0, 0};
    float total_influence = 0.0f;
    float strongest_field = 0.0f;
    
    // Check all derelict sections for gravitational influence
    for (int i = 0; i < DERELICT_SECTION_COUNT; i++) {
        DerelictSection* section = &derelict_sections[i];
        
        Vector3 diff = {
            section->position.x - ship_pos.x,
            section->position.y - ship_pos.y,
            section->position.z - ship_pos.z
        };
        
        float distance = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        if (distance < GRAVITY_RANGE && distance > 0.1f) {
            // Calculate gravitational influence (inverse square law with magnetic field)
            float influence = (section->mass * section->magnetic_field) / (distance * distance + 1.0f);
            
            // Normalize direction vector
            Vector3 unit_dir = {diff.x / distance, diff.y / distance, diff.z / distance};
            
            // Add weighted direction to net direction
            net_direction.x += unit_dir.x * influence;
            net_direction.y += unit_dir.y * influence;
            net_direction.z += unit_dir.z * influence;
            
            total_influence += influence;
            
            if (influence > strongest_field) {
                strongest_field = influence;
            }
            
            // Add hazard effects for damaged sections
            if (section->is_hazardous && distance < 30.0f) {
                // Magnetic instability causes wobble
                float wobble = sinf(navigation_time * 3.0f + (float)i) * 0.3f;
                net_direction.x += wobble;
                net_direction.y += wobble * 0.5f;
            }
        }
    }
    
    // Normalize the net direction if there's any influence
    if (total_influence > 0.01f) {
        float magnitude = sqrtf(net_direction.x * net_direction.x + 
                               net_direction.y * net_direction.y + 
                               net_direction.z * net_direction.z);
        if (magnitude > 0.01f) {
            net_direction.x /= magnitude;
            net_direction.y /= magnitude;
            net_direction.z /= magnitude;
        }
    } else {
        // Default orientation if no mass influence
        net_direction = (Vector3){0, -1, 0};
    }
    
    if (out_field_strength) {
        *out_field_strength = strongest_field;
    }
    
    return net_direction;
}

void derelict_navigation_update(struct World* world, SceneStateManager* state, float delta_time) {
    if (!derelict_nav_initialized || !navigation_active) return;
    
    navigation_time += delta_time;
    
    // Apply magnetic ship physics to all ships with physics components
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & (COMPONENT_PHYSICS | COMPONENT_TRANSFORM))) continue;
        
        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;
        
        if (!physics || !transform) continue;
        
        // Calculate dominant mass direction and field strength
        float field_strength = 0.0f;
        Vector3 gravity_dir = calculate_dominant_mass_direction(transform->position, &field_strength);
        
        // Apply magnetic attraction force toward dominant mass
        float attraction_force = MAGNETIC_ATTRACTION_FORCE * field_strength * delta_time;
        physics->acceleration.x += gravity_dir.x * attraction_force;
        physics->acceleration.y += gravity_dir.y * attraction_force;
        physics->acceleration.z += gravity_dir.z * attraction_force;
        
        // "Sticky ship" orientation: gradually align ship's up vector with away from mass
        // This creates the feeling that the ship's "down" is toward the nearest mass
        Vector3 desired_up = {-gravity_dir.x, -gravity_dir.y, -gravity_dir.z};
        
        // Apply gradual orientation adjustment
        float orientation_speed = ORIENTATION_ADJUSTMENT_SPEED * delta_time;
        
        // Simulate ship rotation by adjusting velocity based on orientation
        physics->velocity.x += desired_up.x * orientation_speed * 2.0f;
        physics->velocity.y += desired_up.y * orientation_speed * 2.0f;
        physics->velocity.z += desired_up.z * orientation_speed * 2.0f;
        
        // Add autonomous navigation for AI ships
        if (entity->id > 0) {
            float thrust_base = 8.0f;
            
            // Calculate distance to nearest obstacle for evasive maneuvers
            float min_obstacle_distance = 1000.0f;
            Vector3 avoidance_vector = {0, 0, 0};
            
            for (uint32_t j = 0; j < world->entity_count; j++) {
                if (i == j) continue;
                struct Entity* other = &world->entities[j];
                if (!(other->component_mask & COMPONENT_TRANSFORM)) continue;
                
                struct Transform* other_transform = other->transform;
                if (!other_transform) continue;
                
                float dx = transform->position.x - other_transform->position.x;
                float dy = transform->position.y - other_transform->position.y;
                float dz = transform->position.z - other_transform->position.z;
                float distance = sqrtf(dx*dx + dy*dy + dz*dz);
                
                if (distance < min_obstacle_distance && distance > 0.1f) {
                    min_obstacle_distance = distance;
                    // Calculate avoidance direction
                    avoidance_vector.x = dx / distance;
                    avoidance_vector.y = dy / distance;
                    avoidance_vector.z = dz / distance;
                }
            }
            
            // Enhanced navigation based on proximity to obstacles
            float navigation_multiplier = 1.0f;
            if (min_obstacle_distance < 40.0f) {
                navigation_multiplier = 1.0f + (40.0f - min_obstacle_distance) / 40.0f * 3.0f;
                
                // Add avoidance thrust
                physics->acceleration.x += avoidance_vector.x * thrust_base * navigation_multiplier * delta_time;
                physics->acceleration.y += avoidance_vector.y * thrust_base * navigation_multiplier * delta_time;
                physics->acceleration.z += avoidance_vector.z * thrust_base * navigation_multiplier * delta_time;
            }
            
            // Add patrol navigation - ships circle around derelict sections
            float patrol_radius = 60.0f + (float)(entity->id % 3) * 20.0f;
            float patrol_speed = 0.5f + (float)(entity->id % 2) * 0.3f;
            float patrol_angle = navigation_time * patrol_speed + (float)entity->id;
            
            Vector3 patrol_target = {
                cosf(patrol_angle) * patrol_radius,
                sinf(patrol_angle * 0.7f) * 15.0f,
                sinf(patrol_angle) * patrol_radius
            };
            
            Vector3 to_patrol = {
                patrol_target.x - transform->position.x,
                patrol_target.y - transform->position.y,
                patrol_target.z - transform->position.z
            };
            
            float patrol_distance = sqrtf(to_patrol.x*to_patrol.x + to_patrol.y*to_patrol.y + to_patrol.z*to_patrol.z);
            if (patrol_distance > 0.1f) {
                physics->acceleration.x += (to_patrol.x / patrol_distance) * thrust_base * 0.5f * delta_time;
                physics->acceleration.y += (to_patrol.y / patrol_distance) * thrust_base * 0.5f * delta_time;
                physics->acceleration.z += (to_patrol.z / patrol_distance) * thrust_base * 0.5f * delta_time;
            }
        }
        
        // Apply atmospheric drag to prevent runaway acceleration
        float drag_factor = 0.96f;
        physics->velocity.x *= drag_factor;
        physics->velocity.y *= drag_factor;
        physics->velocity.z *= drag_factor;
        
        // Velocity limiting for stability
        float max_velocity = 50.0f;
        float velocity_magnitude = sqrtf(physics->velocity.x * physics->velocity.x + 
                                       physics->velocity.y * physics->velocity.y + 
                                       physics->velocity.z * physics->velocity.z);
        if (velocity_magnitude > max_velocity) {
            float scale = max_velocity / velocity_magnitude;
            physics->velocity.x *= scale;
            physics->velocity.y *= scale;
            physics->velocity.z *= scale;
        }
    }
    
    // Periodic status updates
    static float last_debug = 0.0f;
    if (navigation_time - last_debug > 5.0f) {
        printf("🧲 Derelict Navigation: %.1fs - Magnetic field strength varies across %d sections\n", 
               navigation_time, DERELICT_SECTION_COUNT);
        last_debug = navigation_time;
    }
    
    (void)state;
}

static bool derelict_navigation_input(struct World* world, SceneStateManager* state, const void* event) {
    (void)world;
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            printf("🧲 Derelict Navigation: ESC pressed, returning to navigation menu\n");
            scene_state_request_transition(state, "navigation_menu");
            return true;
        }
        if (ev->key_code == SAPP_KEYCODE_TAB) {
            printf("🧲 Switching to system overview\n");
            scene_state_request_transition(state, "system_overview");
            return true;
        }
        if (ev->key_code == SAPP_KEYCODE_SPACE) {
            // Toggle navigation active/passive for testing
            navigation_active = !navigation_active;
            printf("🧲 Magnetic navigation %s\n", navigation_active ? "ACTIVE" : "PASSIVE");
            return true;
        }
    }
    
    return false;
}

void derelict_navigation_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (!derelict_nav_initialized) return;
    
    navigation_active = false;
    derelict_nav_initialized = false;
    
    printf("🧲 Derelict navigation cleanup complete\n");
}

// ============================================================================
// SCENE SCRIPT DEFINITION
// ============================================================================

const SceneScript derelict_navigation_script = {
    .scene_name = "derelict_alpha",
    .on_enter = derelict_navigation_init,
    .on_update = derelict_navigation_update,
    .on_exit = derelict_navigation_cleanup,
    .on_input = derelict_navigation_input
};
