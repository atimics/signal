#include "sokol_gfx.h"
#include "systems.h"
#include "render.h"
#include "assets.h"
#include "data.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// Forward declarations for camera system helpers
static void camera_update_behavior(struct World* world, RenderConfig* render_config, EntityID camera_id, float delta_time);
static void update_legacy_render_config(RenderConfig* render_config, struct Camera* camera);

// Global asset and data registries
static AssetRegistry g_asset_registry;
static DataRegistry g_data_registry;

// ============================================================================
// SYSTEM SCHEDULER IMPLEMENTATION
// ============================================================================

bool scheduler_init(struct SystemScheduler* scheduler, RenderConfig* render_config) {
    if (!scheduler) return false;
    
    memset(scheduler, 0, sizeof(struct SystemScheduler));
    
    // Initialize asset system first
    if (!assets_init(&g_asset_registry, "/Users/ratimics/develop/cgame/build/assets")) {
        printf("❌ Failed to initialize asset system\n");
        return false;
    }
    
    // Initialize data system
    if (!data_registry_init(&g_data_registry, "/Users/ratimics/develop/cgame/data")) {
        printf("❌ Failed to initialize data system\n");
        return false;
    }
    
    // Load entity and scene templates
    load_entity_templates(&g_data_registry, "templates/entities.txt");
    load_scene_templates(&g_data_registry, "scenes/mesh_test.txt");
    load_scene_templates(&g_data_registry, "scenes/spaceport.txt");
    load_scene_templates(&g_data_registry, "scenes/camera_test.txt");
    
    // Initialize render system with asset registry FIRST
    if (!render_init(render_config, &g_asset_registry, 1200.0f, 800.0f)) {
        printf("❌ Failed to initialize render system\n");
        return false;
    }
    
    // Set camera for zoomed-out solar system view
    camera_set_position(&render_config->camera, (Vector3){0, 100, 300});  // Position camera above and back

    
    // Load assets from files (now that we have a renderer for textures)
    printf("🔍 Loading assets...\n");
    assets_load_all_in_directory(&g_asset_registry);
    
    // Print loaded assets and templates
    assets_list_loaded(&g_asset_registry);
    list_entity_templates(&g_data_registry);
    list_scene_templates(&g_data_registry);
    
    // Initialize system configurations
    scheduler->systems[SYSTEM_PHYSICS] = (SystemInfo){
        .name = "Physics",
        .frequency = 60.0f,      // Every frame
        .enabled = true,
        .update_func = physics_system_update
    };
    
    scheduler->systems[SYSTEM_COLLISION] = (SystemInfo){
        .name = "Collision",
        .frequency = 20.0f,      // Every 3 frames at 60 FPS
        .enabled = true,
        .update_func = collision_system_update
    };
    
    scheduler->systems[SYSTEM_AI] = (SystemInfo){
        .name = "AI",
        .frequency = 5.0f,       // Base frequency, LOD will adjust
        .enabled = true,
        .update_func = ai_system_update
    };
    
    scheduler->systems[SYSTEM_CAMERA] = (SystemInfo){
        .name = "Camera",
        .frequency = 60.0f,      // Every frame
        .enabled = true,
        .update_func = camera_system_update
    };
    
    printf("🎯 System scheduler initialized\n");
    printf("   Physics: %.1f Hz\n", scheduler->systems[SYSTEM_PHYSICS].frequency);
    printf("   Collision: %.1f Hz\n", scheduler->systems[SYSTEM_COLLISION].frequency);
    printf("   AI: %.1f Hz (base)\n", scheduler->systems[SYSTEM_AI].frequency);
    printf("   Camera: %.1f Hz\n", scheduler->systems[SYSTEM_CAMERA].frequency);
    
    return true;
}

void scheduler_destroy(struct SystemScheduler* scheduler) {
    if (!scheduler) return;
    
    render_cleanup(&g_render_config);
    assets_cleanup(&g_asset_registry);
    data_registry_cleanup(&g_data_registry);
    printf("🎯 System scheduler destroyed after %d frames\n", scheduler->frame_count);
    scheduler_print_stats(scheduler);
}

void scheduler_update(struct SystemScheduler* scheduler, struct World* world, RenderConfig* render_config, float delta_time) {
    if (!scheduler || !world) return;
    
    scheduler->total_time += delta_time;
    scheduler->frame_count++;
    
    // Update each system based on its frequency
    for (int i = 0; i < SYSTEM_COUNT; i++) {
        SystemInfo* system = &scheduler->systems[i];
        
        if (!system->enabled || !system->update_func) continue;
        
        float time_since_update = scheduler->total_time - system->last_update;
        float update_interval = 1.0f / system->frequency;
        
        if (time_since_update >= update_interval) {
            clock_t start = clock();
            
            system->update_func(world, render_config, delta_time);
            
            clock_t end = clock();
            float execution_time = ((float)(end - start)) / CLOCKS_PER_SEC;
            
            system->last_update = scheduler->total_time;
            scheduler->system_times[i] += execution_time;
            scheduler->system_calls[i]++;
            
            // Debug output every 300 frames (5 seconds at 60 FPS)
            if (scheduler->frame_count % 300 == 0 && i == SYSTEM_COLLISION) {
                printf("⚡ Frame %d: %s took %.2fms\n", 
                       scheduler->frame_count, system->name, execution_time * 1000);
            }
        }
    }
}

void scheduler_print_stats(struct SystemScheduler* scheduler) {
    if (!scheduler) return;
    
    printf("\n📊 SYSTEM PERFORMANCE STATS\n");
    printf("Total frames: %d, Total time: %.2fs\n", 
           scheduler->frame_count, scheduler->total_time);
    
    for (int i = 0; i < SYSTEM_COUNT; i++) {
        if (scheduler->system_calls[i] > 0) {
            float avg_time = scheduler->system_times[i] / scheduler->system_calls[i];
            float actual_frequency = scheduler->system_calls[i] / scheduler->total_time;
            
            printf("  %s: %d calls, %.2fms avg, %.1f Hz actual\n",
                   scheduler->systems[i].name,
                   scheduler->system_calls[i],
                   avg_time * 1000,
                   actual_frequency);
        }
    }
    printf("═══════════════════════════════════\n\n");
}

// ============================================================================
// SYSTEM CONTROL
// ============================================================================

void scheduler_enable_system(struct SystemScheduler* scheduler, SystemType type) {
    if (scheduler && type < SYSTEM_COUNT) {
        scheduler->systems[type].enabled = true;
    }
}

void scheduler_disable_system(struct SystemScheduler* scheduler, SystemType type) {
    if (scheduler && type < SYSTEM_COUNT) {
        scheduler->systems[type].enabled = false;
    }
}

void scheduler_set_frequency(struct SystemScheduler* scheduler, SystemType type, float frequency) {
    if (scheduler && type < SYSTEM_COUNT && frequency > 0) {
        scheduler->systems[type].frequency = frequency;
    }
}

// ============================================================================
// PHYSICS SYSTEM
// ============================================================================

void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config; // Unused
    if (!world) return;
    
    uint32_t updates = 0;
    
    // Update all entities with physics + transform components
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_PHYSICS) || 
            !(entity->component_mask & COMPONENT_TRANSFORM)) {
            continue;
        }
        
        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;
        
        if (physics->kinematic) continue;  // Skip kinematic objects
        
        // Apply acceleration to velocity
        physics->velocity = vector3_add(physics->velocity, 
                                       vector3_multiply(physics->acceleration, delta_time));
        
        // Apply drag
        physics->velocity = vector3_multiply(physics->velocity, physics->drag);
        
        // Apply velocity to position
        transform->position = vector3_add(transform->position,
                                         vector3_multiply(physics->velocity, delta_time));
        
        transform->dirty = true;
        updates++;
    }
    
    // Only log occasionally to avoid spam
    static uint32_t log_counter = 0;
    if (++log_counter % 600 == 0) {  // Every 10 seconds
        printf("🔧 Physics: Updated %d entities\n", updates);
    }
}

// ============================================================================
// COLLISION SYSTEM
// ============================================================================

void collision_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config; // Unused
    if (!world) return;
    
    // Suppress unused parameter warning
    (void)delta_time;
    
    uint32_t collision_checks = 0;
    uint32_t collisions_found = 0;
    
    // Simple O(n²) collision detection for now
    // TODO: Implement spatial partitioning (octree/grid)
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity_a = &world->entities[i];
        
        if (!(entity_a->component_mask & COMPONENT_COLLISION) ||
            !(entity_a->component_mask & COMPONENT_TRANSFORM)) {
            continue;
        }
        
        for (uint32_t j = i + 1; j < world->entity_count; j++) {
            struct Entity* entity_b = &world->entities[j];
            
            if (!(entity_b->component_mask & COMPONENT_COLLISION) ||
                !(entity_b->component_mask & COMPONENT_TRANSFORM)) {
                continue;
            }
            
            collision_checks++;
            
            // Check if collision layers match
            struct Collision* col_a = entity_a->collision;
            struct Collision* col_b = entity_b->collision;
            
            if (!(col_a->layer_mask & col_b->layer_mask)) continue;
            
            // Only do sphere collision for now
            if (col_a->shape != COLLISION_SPHERE || col_b->shape != COLLISION_SPHERE) {
                continue;
            }
            
            float distance = vector3_distance(entity_a->transform->position,
                                             entity_b->transform->position);
            float combined_radius = col_a->radius + col_b->radius;
            
            if (distance < combined_radius) {
                collisions_found++;
                
                // Fire collision event (for now, just log but limit spam)
                static int collision_count = 0;
                if (collision_count < 10) {
                    printf("💥 Collision: Entity %d <-> Entity %d (dist: %.2f)\n",
                           entity_a->id, entity_b->id, distance);
                    collision_count++;
                } else if (collision_count == 10) {
                    printf("💥 ... (collision logging suppressed after first 10)\n");
                    collision_count++;
                }
                
                // Simple collision response - separate objects
                if (!col_a->is_trigger && !col_b->is_trigger) {
                    Vector3 separation = {
                        entity_a->transform->position.x - entity_b->transform->position.x,
                        entity_a->transform->position.y - entity_b->transform->position.y,
                        entity_a->transform->position.z - entity_b->transform->position.z
                    };
                    
                    float sep_length = vector3_length(separation);
                    if (sep_length > 0) {
                        separation = vector3_multiply(separation, 1.0f / sep_length);
                        float overlap = combined_radius - distance;
                        
                        // Move objects apart
                        entity_a->transform->position = vector3_add(
                            entity_a->transform->position,
                            vector3_multiply(separation, overlap * 0.5f)
                        );
                        entity_b->transform->position = vector3_add(
                            entity_b->transform->position,
                            vector3_multiply(separation, -overlap * 0.5f)
                        );
                        
                        // Stop velocities if they have physics
                        if (entity_a->physics) {
                            entity_a->physics->velocity = (Vector3){0, 0, 0};
                        }
                        if (entity_b->physics) {
                            entity_b->physics->velocity = (Vector3){0, 0, 0};
                        }
                    }
                }
            }
        }
    }
    
    // Log collision stats every 5 seconds
    static uint32_t log_counter = 0;
    if (++log_counter % 100 == 0) {  // Every 5 seconds at 20Hz
        printf("🔍 Collision: %d checks, %d hits\n", collision_checks, collisions_found);
    }
}

// ============================================================================
// AI SYSTEM
// ============================================================================

void ai_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config; // Unused
    if (!world) return;
    
    // Suppress unused parameter warning
    (void)delta_time;
    
    uint32_t ai_updates = 0;
    
    // Find player entity for LOD calculations
    EntityID player_id = INVALID_ENTITY;
    Vector3 player_pos = {0, 0, 0};
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        if (world->entities[i].component_mask & COMPONENT_PLAYER) {
            player_id = world->entities[i].id;
            if (world->entities[i].transform) {
                player_pos = world->entities[i].transform->position;
            }
            break;
        }
    }
    
    // Update AI entities
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_AI) ||
            !(entity->component_mask & COMPONENT_TRANSFORM)) {
            continue;
        }
        
        struct AI* ai = entity->ai;
        
        // LOD: Adjust update frequency based on distance to player
        if (player_id != INVALID_ENTITY) {
            float distance = vector3_distance(entity->transform->position, player_pos);
            
            if (distance < 50.0f) {
                ai->update_frequency = 10.0f;  // Close: 10 Hz
            } else if (distance < 200.0f) {
                ai->update_frequency = 5.0f;   // Medium: 5 Hz
            } else {
                ai->update_frequency = 2.0f;   // Far: 2 Hz
            }
        }
        
        // Check if it's time to update this AI
        float time_since_update = world->total_time - ai->last_update;
        float update_interval = 1.0f / ai->update_frequency;
        
        if (time_since_update >= update_interval) {
            ai->last_update = world->total_time;
            ai_updates++;
            
            // Simple AI logic for now
            switch (ai->state) {
                case AI_STATE_IDLE:
                    // Maybe switch to patrolling
                    if (world->total_time - ai->decision_timer > 5.0f) {
                        ai->state = AI_STATE_PATROLLING;
                        ai->decision_timer = world->total_time;
                        printf("🤖 Entity %d: Idle -> Patrolling\n", entity->id);
                    }
                    break;
                    
                case AI_STATE_PATROLLING:
                    // Simple patrol movement
                    if (entity->physics) {
                        entity->physics->velocity.x += (rand() % 20 - 10) * 0.1f;
                        entity->physics->velocity.z += (rand() % 20 - 10) * 0.1f;
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    // Log AI stats every 10 seconds
    static uint32_t log_counter = 0;
    if (++log_counter % 50 == 0) {  // Every 10 seconds at 5Hz
        printf("🧠 AI: Updated %d entities\n", ai_updates);
    }
}

// ============================================================================
// CAMERA SYSTEM
// ============================================================================

void camera_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    if (!world) return;
    
    // Set up camera follow targets if not already set
    static bool cameras_initialized = false;
    if (!cameras_initialized) {
        EntityID player_id = INVALID_ENTITY;
        
        // Find player entity
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_PLAYER) {
                player_id = entity->id;
                break;
            }
        }
        
        // Initialize cameras with default values
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA) {
                struct Camera* camera = entity->camera;
                if (camera) {
                    // Initialize cameras with default values
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA) {
                struct Camera* camera = entity->camera;
                if (camera) {
                    // Set default camera properties if not already set
                    if (camera->fov == 0.0f) camera->fov = 60.0f;  // Wider FOV for better view
                    if (camera->near_plane == 0.0f) camera->near_plane = 0.1f;
                    if (camera->far_plane == 0.0f) camera->far_plane = 1000.0f;
                    if (camera->aspect_ratio == 0.0f) camera->aspect_ratio = 16.0f / 9.0f;
                    
                    // Initialize position and target if not set - better default positions
                    if (camera->position.x == 0.0f && camera->position.y == 0.0f && camera->position.z == 0.0f) {
                        // Different positions for different camera types
                        switch (camera->behavior) {
                            case CAMERA_BEHAVIOR_THIRD_PERSON:
                            case CAMERA_BEHAVIOR_CHASE:
                                camera->position = (Vector3){10.0f, 20.0f, 30.0f};  // Behind and above
                                camera->target = (Vector3){0.0f, 0.0f, 0.0f};
                                break;
                            case CAMERA_BEHAVIOR_STATIC:
                                camera->position = (Vector3){-30.0f, 25.0f, -30.0f};  // Corner view
                                camera->target = (Vector3){0.0f, 0.0f, 0.0f};
                                break;
                            default:
                                camera->position = (Vector3){0.0f, 15.0f, 25.0f};  // General overhead
                                camera->target = (Vector3){0.0f, 0.0f, 0.0f};
                                break;
                        }
                    }
                    camera->up = (Vector3){0.0f, 1.0f, 0.0f};
                    
                    // Set follow target for chase cameras with better default offsets
                    if (camera->follow_target == INVALID_ENTITY) {
                        if ((entity->component_mask & COMPONENT_PLAYER) || 
                            (camera->behavior == CAMERA_BEHAVIOR_THIRD_PERSON || 
                             camera->behavior == CAMERA_BEHAVIOR_CHASE)) {
                            camera->follow_target = player_id;
                            // Better chase camera positioning
                            camera->follow_offset = (Vector3){8.0f, 20.0f, 30.0f};  // Further back and higher
                            camera->follow_smoothing = 0.05f;  // Slightly more responsive
                        }
                    }
                    
                    // Mark matrices as dirty for initial calculation
                    camera->matrices_dirty = true;
                    
                    printf("🎥 Initialized camera Entity %d: pos:(%.1f,%.1f,%.1f) behavior:%d\n",
                           entity->id, camera->position.x, camera->position.y, camera->position.z, camera->behavior);
                }
            }
        }
                }
            }
        }
        
        // Set the first available camera as active
        if (world->active_camera_entity == INVALID_ENTITY) {
            for (uint32_t i = 0; i < world->entity_count; i++) {
                struct Entity* entity = &world->entities[i];
                if (entity->component_mask & COMPONENT_CAMERA) {
                    world_set_active_camera(world, entity->id);
                    printf("🎯 Set initial active camera: Entity %d\n", entity->id);
                    break;
                }
            }
        }
        
        cameras_initialized = true;
    }
    
    // Find active camera entity
    EntityID active_camera_id = world_get_active_camera(world);
    if (active_camera_id == INVALID_ENTITY) {
        // No active camera, try to find any camera
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA) {
                world_set_active_camera(world, entity->id);
                active_camera_id = entity->id;
                break;
            }
        }
    }
    
    if (active_camera_id == INVALID_ENTITY) {
        return; // No camera found
    }
    
    struct Camera* camera = entity_get_camera(world, active_camera_id);
    if (!camera) return;
    
    // Update camera position and target based on behavior
    camera_update_behavior(world, render_config, active_camera_id, delta_time);
    
    // Update matrices if camera changed
    if (camera->matrices_dirty) {
        camera_update_matrices(camera);
    }
    
    // Update legacy render config for backward compatibility
    update_legacy_render_config(render_config, camera);
}

static void camera_update_behavior(struct World* world, RenderConfig* render_config, EntityID camera_id, float delta_time) {
    struct Camera* camera = entity_get_camera(world, camera_id);
    if (!camera) return;
    
    bool position_changed = false;
    
    // Update camera based on its behavior
    switch (camera->behavior) {
        case CAMERA_BEHAVIOR_THIRD_PERSON:
        case CAMERA_BEHAVIOR_CHASE:
            if (camera->follow_target != INVALID_ENTITY) {
                struct Transform* target_transform = entity_get_transform(world, camera->follow_target);
                if (target_transform) {
                    // Calculate desired camera position
                    Vector3 target_pos = target_transform->position;
                    Vector3 desired_pos = {
                        target_pos.x + camera->follow_offset.x,
                        target_pos.y + camera->follow_offset.y,
                        target_pos.z + camera->follow_offset.z
                    };
                    
                    // Smooth camera movement - more responsive
                    float lerp = camera->follow_smoothing * delta_time * 60.0f; // Frame rate independent
                    if (lerp > 1.0f) lerp = 1.0f; // Clamp to prevent overshooting
                    
                    // Apply smoothing factor for stability but make it more responsive
                    lerp *= 3.0f;  // Make camera more responsive
                    if (lerp > 0.3f) lerp = 0.3f;  // But not too fast
                    
                    Vector3 old_pos = camera->position;
                    camera->position.x += (desired_pos.x - camera->position.x) * lerp;
                    camera->position.y += (desired_pos.y - camera->position.y) * lerp;
                    camera->position.z += (desired_pos.z - camera->position.z) * lerp;
                    
                    // Always look at the target entity for proper framing
                    camera->target = target_pos;
                    
                    // Check if position changed
                    if (vector3_distance(old_pos, camera->position) > 0.001f) {
                        position_changed = true;
                    }
                }
            }
            break;
            
        case CAMERA_BEHAVIOR_STATIC:
            // Static cameras don't move
            break;
            
        case CAMERA_BEHAVIOR_FIRST_PERSON:
            if (camera->follow_target != INVALID_ENTITY) {
                struct Transform* target_transform = entity_get_transform(world, camera->follow_target);
                if (target_transform) {
                    // First person camera follows exactly
                    Vector3 old_pos = camera->position;
                    camera->position = target_transform->position;
                    
                    // Calculate forward direction from rotation (simplified)
                    Vector3 forward = {0.0f, 0.0f, -1.0f}; // Default forward
                    camera->target = vector3_add(camera->position, forward);
                    
                    if (vector3_distance(old_pos, camera->position) > 0.001f) {
                        position_changed = true;
                    }
                }
            }
            break;
            
        case CAMERA_BEHAVIOR_ORBITAL:
            // TODO: Implement orbital camera behavior
            break;
    }
    
    if (position_changed) {
        camera->matrices_dirty = true;
    }
}

static void update_legacy_render_config(RenderConfig* render_config, struct Camera* camera) {
    // Update legacy render config for backward compatibility
    if (render_config && camera) {
        render_config->camera.position = camera->position;
        render_config->camera.target = camera->target;
        render_config->camera.up = camera->up;
        render_config->camera.fov = camera->fov;
        render_config->camera.near_plane = camera->near_plane;
        render_config->camera.far_plane = camera->far_plane;
        render_config->camera.aspect_ratio = camera->aspect_ratio;
    }
}



// ============================================================================
// DATA ACCESS
// ============================================================================

DataRegistry* get_data_registry(void) {
    return &g_data_registry;
}

// ============================================================================
// GLOBAL SYSTEM ACCESSORS
// ============================================================================



AssetRegistry* get_asset_registry(void) {
    return &g_asset_registry;
}
