#include "../scene_script.h"
#include "../core.h"
#include "../system/input.h"
#include "../system/control.h"
#include "../system/thrusters.h"
#include "../system/physics.h"
#include "../system/ode_physics.h"
#include "../hud_system.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// ============================================================================
// ODE PHYSICS TEST SCENE
// ============================================================================

static bool ode_test_initialized = false;
static EntityID test_ship_id = INVALID_ENTITY;
static float test_time = 0.0f;

// ODE Physics System
static ODEPhysicsSystem* ode_system = NULL;

// Debug display options
static bool show_physics_stats = true;
static bool show_collision_debug = false;
static bool use_ode_physics = true;

void ode_test_enter(struct World* world, SceneStateManager* state) {
    (void)state;
    
    printf("🌐 Initializing ODE Physics Test Scene\n");
    
    // Create and initialize ODE physics system
    ode_system = calloc(1, sizeof(ODEPhysicsSystem));
    if (!ode_system) {
        printf("❌ Failed to allocate ODE physics system\n");
        return;
    }
    
    if (!ode_physics_init(ode_system, MAX_ENTITIES)) {
        printf("❌ Failed to initialize ODE physics\n");
        free(ode_system);
        ode_system = NULL;
        return;
    }
    
    // Find the player ship
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            test_ship_id = entity->id;
            printf("🎯 Test ship found: Entity ID %d\n", test_ship_id);
            break;
        }
    }
    
    if (test_ship_id == INVALID_ENTITY) {
        printf("❌ No player ship found in scene!\n");
        // Don't return - continue setup even without player ship
    }
    
    // Configure ship if found
    if (test_ship_id != INVALID_ENTITY) {
        control_configure_ship(world, test_ship_id, SHIP_CONFIG_FIGHTER);
        
        // Enable ODE physics for the ship
        struct Physics* physics = entity_get_physics(world, test_ship_id);
        if (physics) {
            physics->use_ode = true;
            
            // Create ODE body for the ship
            dBodyID body = ode_create_body(ode_system, world, test_ship_id);
            if (body) {
                printf("✅ ODE body created for player ship\n");
                
                // Create collision geometry if the ship has collision component
                if (entity_has_component(world, test_ship_id, COMPONENT_COLLISION)) {
                    dGeomID geom = ode_create_geometry(ode_system, world, test_ship_id);
                    if (geom) {
                        printf("✅ ODE collision geometry created for player ship\n");
                    }
                }
            }
        }
    }
    
    // Enable ODE physics for all existing entities with physics components
    printf("🗿 Enabling ODE physics for scene entities...\n");
    int ode_body_count = 0;
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Skip if no physics component
        if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
        
        struct Physics* physics = entity->physics;
        if (!physics) continue;
        
        // Enable ODE physics
        physics->use_ode = true;
        physics->has_6dof = true;
        
        // Set default mass if not set
        if (physics->mass <= 0.0f) {
            physics->mass = 50.0f;  // Default mass
        }
        
        // Create ODE body
        dBodyID body = ode_create_body(ode_system, world, entity->id);
        if (body) {
            ode_body_count++;
            
            struct Transform* transform = entity->transform;
            if (transform) {
                printf("   🗿 Entity %u: ODE body created at (%.1f, %.1f, %.1f) with mass %.1f\n", 
                       entity->id, transform->position.x, transform->position.y, 
                       transform->position.z, physics->mass);
            }
            
            // Create collision geometry if entity has collision
            if (entity->component_mask & COMPONENT_COLLISION) {
                dGeomID geom = ode_create_geometry(ode_system, world, entity->id);
                if (geom) {
                    printf("   🗿 Entity %u: ODE collision geometry created\n", entity->id);
                }
            }
        }
    }
    
    printf("✅ Created %d ODE bodies\n", ode_body_count);
    
    // Initialize input and control systems
    input_init();
    control_set_player_entity(world, test_ship_id);
    
    // Set HUD mode
    hud_system_set_camera_mode(HUD_CAMERA_MODE_CHASE_NEAR);
    
    // Configure camera
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA) {
            struct Camera* camera = entity_get_camera(world, entity->id);
            if (camera) {
                camera->follow_target = test_ship_id;
                camera->behavior = CAMERA_BEHAVIOR_THIRD_PERSON;
                camera->follow_offset = (Vector3){0, 20, -40};
                printf("📷 Camera configured for ODE test\n");
            }
        }
    }
    
    ode_test_initialized = true;
    test_time = 0.0f;
    
    printf("\n📋 ODE PHYSICS TEST CONTROLS:\n");
    printf("   F1 - Toggle physics stats display\n");
    printf("   F2 - Toggle collision debug info\n");
    printf("   F3 - Toggle ODE physics (compare with custom)\n");
    printf("   F5 - Reset positions\n");
    printf("   F6 - Apply random impulse to obstacles\n");
    printf("\n🌐 ODE Physics Features:\n");
    printf("   - Rigid body dynamics\n");
    printf("   - Collision detection and response\n");
    printf("   - Stable constraint solver\n");
    printf("   - Automatic sleeping for performance\n");
}

void ode_test_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)state;
    
    if (!ode_test_initialized || !ode_system) return;
    
    test_time += delta_time;
    
    // Update input
    input_update();
    
    // Update physics using ODE or custom physics
    if (use_ode_physics) {
        physics_system_update_with_ode(world, ode_system, NULL, delta_time);
    } else {
        physics_system_update(world, NULL, delta_time);
    }
    
    // Display physics stats
    if (show_physics_stats) {
        static float stats_timer = 0.0f;
        stats_timer += delta_time;
        
        if (stats_timer >= 1.0f) {
            stats_timer = 0.0f;
            
            // Get ODE statistics
            uint32_t active_bodies = 0;
            uint32_t total_bodies = 0;
            float step_time = 0.0f;
            ode_get_statistics(ode_system, &active_bodies, &total_bodies, &step_time);
            
            printf("\n🌐 ODE PHYSICS STATUS (T=%.1fs):\n", test_time);
            printf("   Physics: %s\n", use_ode_physics ? "ODE" : "Custom");
            printf("   Active Bodies: %u / %u\n", active_bodies, total_bodies);
            printf("   Step Time: %.2f ms\n", step_time);
            
            // Get player ship stats
            struct Physics* ship_physics = entity_get_physics(world, test_ship_id);
            struct Transform* ship_transform = entity_get_transform(world, test_ship_id);
            
            if (ship_physics && ship_transform) {
                float speed = vector3_length(ship_physics->velocity);
                float angular_speed = vector3_length(ship_physics->angular_velocity);
                
                printf("   Ship Speed: %.1f m/s | Angular: %.2f rad/s\n", speed, angular_speed);
                printf("   Ship Pos: [%.1f, %.1f, %.1f]\n",
                       ship_transform->position.x, ship_transform->position.y, ship_transform->position.z);
            }
        }
    }
}

bool ode_test_handle_event(struct World* world, SceneStateManager* state, const void* event) {
    (void)state;
    
    if (!ode_test_initialized) return false;
    
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch (ev->key_code) {
            case SAPP_KEYCODE_F1:
                show_physics_stats = !show_physics_stats;
                printf("📊 Physics stats: %s\n", show_physics_stats ? "ON" : "OFF");
                return true;
                
            case SAPP_KEYCODE_F2:
                show_collision_debug = !show_collision_debug;
                printf("💥 Collision debug: %s\n", show_collision_debug ? "ON" : "OFF");
                return true;
                
            case SAPP_KEYCODE_F3:
                use_ode_physics = !use_ode_physics;
                printf("🌐 Physics engine: %s\n", use_ode_physics ? "ODE" : "Custom");
                
                // Toggle use_ode flag on all entities
                for (uint32_t i = 0; i < world->entity_count; i++) {
                    struct Physics* physics = entity_get_physics(world, world->entities[i].id);
                    if (physics) {
                        physics->use_ode = use_ode_physics;
                    }
                }
                return true;
                
            case SAPP_KEYCODE_F5: {
                // Reset all positions
                printf("🔄 Resetting positions...\n");
                
                // Reset ship
                struct Transform* ship_transform = entity_get_transform(world, test_ship_id);
                struct Physics* ship_physics = entity_get_physics(world, test_ship_id);
                if (ship_transform && ship_physics) {
                    ship_transform->position = (Vector3){0, 10, 0};
                    ship_transform->rotation = (Quaternion){0, 0, 0, 1};
                    ship_physics->velocity = (Vector3){0, 0, 0};
                    ship_physics->angular_velocity = (Vector3){0, 0, 0};
                    
                    // Sync with ODE if using it
                    if (use_ode_physics) {
                        dBodyID body = ode_get_body(ode_system, test_ship_id);
                        if (body) {
                            ode_sync_to_body(body, ship_physics, ship_transform);
                        }
                    }
                }
                
                // Reset all entities with physics
                for (uint32_t i = 0; i < world->entity_count; i++) {
                    struct Entity* entity = &world->entities[i];
                    
                    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
                    if (entity->id == test_ship_id) continue;  // Skip player ship
                    
                    struct Transform* transform = entity_get_transform(world, entity->id);
                    struct Physics* physics = entity_get_physics(world, entity->id);
                    
                    if (transform && physics) {
                        // Reset velocities
                        physics->velocity = (Vector3){0, 0, 0};
                        physics->angular_velocity = (Vector3){0, 0, 0};
                        
                        if (use_ode_physics) {
                            dBodyID body = ode_get_body(ode_system, entity->id);
                            if (body) {
                                ode_sync_to_body(body, physics, transform);
                            }
                        }
                    }
                }
                return true;
            }
            
            case SAPP_KEYCODE_F6: {
                // Apply random impulse to all physics entities
                printf("💥 Applying random impulses to physics entities...\n");
                
                for (uint32_t i = 0; i < world->entity_count; i++) {
                    struct Entity* entity = &world->entities[i];
                    
                    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
                    if (entity->id == test_ship_id) continue;  // Skip player ship
                    
                    struct Physics* physics = entity_get_physics(world, entity->id);
                    if (!physics) continue;
                    
                    // Random impulse
                    Vector3 impulse = {
                        ((float)rand() / RAND_MAX - 0.5f) * 500.0f,
                        ((float)rand() / RAND_MAX) * 500.0f,
                        ((float)rand() / RAND_MAX - 0.5f) * 500.0f
                    };
                    
                    if (use_ode_physics) {
                        dBodyID body = ode_get_body(ode_system, entity->id);
                        if (body) {
                            dBodyAddForce(body, impulse.x, impulse.y, impulse.z);
                            
                            // Add some random torque too
                            Vector3 torque = {
                                ((float)rand() / RAND_MAX - 0.5f) * 100.0f,
                                ((float)rand() / RAND_MAX - 0.5f) * 100.0f,
                                ((float)rand() / RAND_MAX - 0.5f) * 100.0f
                            };
                            dBodyAddTorque(body, torque.x, torque.y, torque.z);
                        }
                    } else {
                        // Use custom physics
                        physics_add_force(physics, impulse);
                        
                        Vector3 torque = {
                            ((float)rand() / RAND_MAX - 0.5f) * 100.0f,
                            ((float)rand() / RAND_MAX - 0.5f) * 100.0f,
                            ((float)rand() / RAND_MAX - 0.5f) * 100.0f
                        };
                        physics_add_torque(physics, torque);
                    }
                }
                return true;
            }
                
            default:
                break;
        }
    }
    
    // Pass other input to the input system
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN || ev->type == SAPP_EVENTTYPE_KEY_UP) {
        return input_handle_keyboard(ev->key_code, ev->type == SAPP_EVENTTYPE_KEY_DOWN);
    }
    
    return false;
}

void ode_test_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (!ode_test_initialized) return;
    
    // Shutdown ODE physics system
    if (ode_system) {
        ode_physics_shutdown(ode_system);
        free(ode_system);
        ode_system = NULL;
    }
    
    ode_test_initialized = false;
    input_shutdown();
    
    printf("🌐 ODE test cleanup complete\n");
}

// Scene script definition
const SceneScript ode_test_script = {
    .scene_name = "ode_test",
    .on_enter = ode_test_enter,
    .on_update = ode_test_update,
    .on_exit = ode_test_cleanup,
    .on_input = ode_test_handle_event
};