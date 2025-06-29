#include "core.h"
#include "systems.h"
#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <SDL.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void load_spaceport_scene(struct World* world, EntityID* player_id);
void simulate_player_input(struct World* world, EntityID player_id, float time);

// ============================================================================
// ENTITY FACTORY FUNCTIONS
// ============================================================================

EntityID create_player(struct World* world, Vector3 position) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components
    entity_add_component(world, id, COMPONENT_TRANSFORM);
    entity_add_component(world, id, COMPONENT_PHYSICS);
    entity_add_component(world, id, COMPONENT_COLLISION);
    entity_add_component(world, id, COMPONENT_PLAYER);
    entity_add_component(world, id, COMPONENT_RENDERABLE);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    transform->position = position;
    
    struct Physics* physics = entity_get_physics(world, id);
    physics->mass = 80.0f;
    physics->drag = 0.98f;
    
    struct Collision* collision = entity_get_collision(world, id);
    collision->radius = 4.0f;
    collision->layer_mask = 0xFFFFFFFF;
    
    printf("🚀 Created player ship at (%.1f, %.1f, %.1f)\n", 
           position.x, position.y, position.z);
    
    return id;
}

EntityID create_ai_ship(struct World* world, Vector3 position, const char* type) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components
    entity_add_component(world, id, COMPONENT_TRANSFORM);
    entity_add_component(world, id, COMPONENT_PHYSICS);
    entity_add_component(world, id, COMPONENT_COLLISION);
    entity_add_component(world, id, COMPONENT_AI);
    entity_add_component(world, id, COMPONENT_RENDERABLE);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    transform->position = position;
    
    struct Physics* physics = entity_get_physics(world, id);
    physics->mass = 100.0f;
    physics->drag = 0.99f;
    
    struct Collision* collision = entity_get_collision(world, id);
    collision->radius = 5.0f;
    
    struct AI* ai = entity_get_ai(world, id);
    ai->state = AI_STATE_IDLE;
    ai->update_frequency = 2.0f;
    ai->decision_timer = world->total_time;
    
    printf("🤖 Created %s AI ship at (%.1f, %.1f, %.1f)\n", 
           type, position.x, position.y, position.z);
    
    return id;
}

EntityID create_sun(struct World* world, Vector3 position) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components
    entity_add_component(world, id, COMPONENT_TRANSFORM);
    entity_add_component(world, id, COMPONENT_PHYSICS);
    entity_add_component(world, id, COMPONENT_COLLISION);
    entity_add_component(world, id, COMPONENT_RENDERABLE);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    transform->position = position;
    transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
    
    struct Physics* physics = entity_get_physics(world, id);
    physics->mass = 1000.0f;
    physics->kinematic = true;  // Sun doesn't move
    
    struct Collision* collision = entity_get_collision(world, id);
    collision->shape = COLLISION_SPHERE;
    collision->radius = 25.0f;
    collision->layer_mask = 0xFFFFFFFF;
    
    struct Renderable* renderable = entity_get_renderable(world, id);
    renderable->visible = true;
    renderable->mesh_id = 1;
    renderable->material_id = 1;
    
    printf("☀️ Created sun at (%.1f, %.1f, %.1f)\n", 
           position.x, position.y, position.z);
    
    return id;
}

EntityID create_planet(struct World* world, Vector3 position, float radius, float orbit_speed) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components
    entity_add_component(world, id, COMPONENT_TRANSFORM);
    entity_add_component(world, id, COMPONENT_PHYSICS);
    entity_add_component(world, id, COMPONENT_COLLISION);
    entity_add_component(world, id, COMPONENT_RENDERABLE);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    transform->position = position;
    transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
    
    struct Physics* physics = entity_get_physics(world, id);
    physics->mass = 50.0f + radius * 2.0f;
    physics->drag = 0.99f;
    
    // Set initial orbital velocity (perpendicular to position from sun)
    float distance = sqrtf(position.x * position.x + position.z * position.z);
    if (distance > 0.0f) {
        // Circular orbit velocity
        physics->velocity.x = -position.z * orbit_speed / distance;
        physics->velocity.z = position.x * orbit_speed / distance;
    }
    
    struct Collision* collision = entity_get_collision(world, id);
    collision->shape = COLLISION_SPHERE;
    collision->radius = radius;
    collision->layer_mask = 0xFFFFFFFF;
    
    struct Renderable* renderable = entity_get_renderable(world, id);
    renderable->visible = true;
    renderable->mesh_id = 2;
    renderable->material_id = 2;
    
    printf("🪐 Created planet at (%.1f, %.1f, %.1f) with radius %.1f\n", 
           position.x, position.y, position.z, radius);
    
    return id;
}

EntityID create_asteroid(struct World* world, Vector3 position, float radius) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components
    entity_add_component(world, id, COMPONENT_TRANSFORM);
    entity_add_component(world, id, COMPONENT_PHYSICS);
    entity_add_component(world, id, COMPONENT_COLLISION);
    entity_add_component(world, id, COMPONENT_RENDERABLE);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    transform->position = position;
    transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
    
    struct Physics* physics = entity_get_physics(world, id);
    physics->mass = 5.0f + radius;
    physics->drag = 0.999f;
    
    // Random drift velocity
    physics->velocity.x = (rand() % 200 - 100) / 100.0f;
    physics->velocity.z = (rand() % 200 - 100) / 100.0f;
    
    struct Collision* collision = entity_get_collision(world, id);
    collision->shape = COLLISION_SPHERE;
    collision->radius = radius;
    collision->layer_mask = 0xFFFFFFFF;
    
    struct Renderable* renderable = entity_get_renderable(world, id);
    renderable->visible = true;
    renderable->mesh_id = 3;
    renderable->material_id = 3;
    
    return id;
}

// ============================================================================
// SIMULATION
// ============================================================================

void simulate_player_input(struct World* world, EntityID player_id, float time) {
    struct Physics* physics = entity_get_physics(world, player_id);
    if (!physics) return;
    
    // Gentle hovering movement for better camera demonstration
    float hover_radius = 5.0f;        // Small radius
    float hover_speed = 0.5f;         // Much slower
    
    // Calculate gentle hovering position around the starting point (30, 0, 0)
    float center_x = 30.0f;
    float center_z = 0.0f;
    
    float desired_x = center_x + hover_radius * cosf(time * hover_speed);
    float desired_z = center_z + hover_radius * sinf(time * hover_speed);
    
    // Get current position
    struct Transform* transform = entity_get_transform(world, player_id);
    if (!transform) return;
    
    // Gentle steering toward hovering position
    float dx = desired_x - transform->position.x;
    float dz = desired_z - transform->position.z;
    
    // Much gentler acceleration
    physics->acceleration.x = dx * 0.1f;
    physics->acceleration.z = dz * 0.1f;
    
    // Gentle vertical bobbing
    physics->acceleration.y = sinf(time * 1.0f) * 0.5f;
}

void load_scene_by_name(struct World* world, const char* scene_name, EntityID* player_id) {
    printf("🏗️  Loading scene '%s' from data...\n", scene_name);
    
    DataRegistry* data_registry = get_data_registry();
    
    // Load the scene from template
    if (!load_scene(world, data_registry, scene_name)) {
        printf("❌ Failed to load scene: %s\n", scene_name);
        return;
    }
    
    // Find the player entity that was spawned
    *player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            *player_id = entity->id;
            break;
        }
    }
    
    if (*player_id != INVALID_ENTITY) {
        printf("🎯 Player found: Entity ID %d\n", *player_id);
    } else {
        printf("⚠️ No player entity found in scene.\n");
    }
    
    // Set up scene lighting
    RenderConfig* render_config = get_render_config();
    if (render_config) {
        lighting_set_ambient(&render_config->lighting, (Vector3){0.1f, 0.15f, 0.2f}, 0.3f);
        lighting_add_directional_light(&render_config->lighting, 
                                      (Vector3){0.3f, -0.7f, 0.2f}, 
                                      (Vector3){0.4f, 0.5f, 0.6f}, 
                                      0.4f);
        printf("💡 Scene lighting configured\n");
    }
    
    printf("🌍 Scene loaded with %d entities\n", world->entity_count);
}

int main(int argc, char* argv[]) {
    printf("🎮 CGGame - Component-Based Engine\n");
    printf("===================================\n\n");
    
    const char* scene_to_load = "mesh_test"; // Default scene
    if (argc > 1) {
        scene_to_load = argv[1];
        printf("✅ Command-line argument detected, attempting to load scene: %s\n", scene_to_load);
    } else {
        printf("ℹ️ No scene specified, loading default: %s\n", scene_to_load);
    }

    srand((unsigned int)time(NULL));
    
    // Initialize world
    struct World world;
    if (!world_init(&world)) {
        printf("❌ Failed to initialize world\n");
        return 1;
    }
    
    // Initialize system scheduler
    struct SystemScheduler scheduler;
    if (!scheduler_init(&scheduler)) {
        printf("❌ Failed to initialize scheduler\n");
        return 1;
    }
    
    // Create entities
    EntityID player;
    load_scene_by_name(&world, scene_to_load, &player);
    
    printf("\n🎮 Starting simulation...\n");
    printf("Press Ctrl+C or close window to exit\n\n");
    
    // Game loop variables
    bool running = true;
    const float dt = 1.0f / 60.0f;  // 60 FPS
    int frame = 0;
    
    while (running) {
        float time = frame * dt;
        
        // Check for quit events (handled in render system)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                printf("🚪 Window close requested - exiting gracefully\n");
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                printf("⎋ Escape key pressed - exiting\n");
                running = false;
                break;
            }
        }
        
        if (!running) break;
        
        // Simulate player input
        if (player != INVALID_ENTITY) {
            simulate_player_input(&world, player, time);
        }
        
        // Update world
        world_update(&world, dt);
        
        // Update systems (this will render the frame)
        scheduler_update(&scheduler, &world, dt);
        
        frame++;
        
        // Print status occasionally
        if (frame % 300 == 0 && frame > 0) {  // Every 5 seconds
            printf("⏱️  Time: %.1fs, Frame: %d, Entities: %d\n", 
                   time, frame, world.entity_count);
        }
        
        // Cap framerate (simple delay - in production you'd use proper timing)
        SDL_Delay(16);  // ~60 FPS
    }
    
    printf("\n🏁 Simulation complete!\n");
    
    // Print final stats
    scheduler_destroy(&scheduler);
    world_destroy(&world);
    
    return 0;
}
