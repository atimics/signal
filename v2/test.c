#include "core.h"
#include "systems.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void populate_solar_system(struct World* world, EntityID* player_id);
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
    
    // Simulate player behavior: orbit around the sun with some exploration
    float orbit_radius = 50.0f;
    float orbit_speed = 8.0f;
    
    // Calculate desired orbital position
    float desired_x = orbit_radius * cosf(time * orbit_speed / orbit_radius);
    float desired_z = orbit_radius * sinf(time * orbit_speed / orbit_radius);
    
    // Get current position
    struct Transform* transform = entity_get_transform(world, player_id);
    if (!transform) return;
    
    // Simple steering toward orbital path
    float dx = desired_x - transform->position.x;
    float dz = desired_z - transform->position.z;
    
    // Apply thrust toward desired position (simplified)
    physics->acceleration.x = dx * 0.5f;
    physics->acceleration.z = dz * 0.5f;
    
    // Add some vertical movement for interest
    physics->acceleration.y = sinf(time * 2.0f) * 2.0f;
}

void populate_solar_system(struct World* world, EntityID* player_id) {
    printf("🌌 Creating solar system...\n");
    
    // Create the sun at the center
    EntityID sun = create_sun(world, (Vector3){0, 0, 0});
    
    // Create planets in orbits around the sun
    create_planet(world, (Vector3){60, 0, 0}, 8.0f, 15.0f);      // Inner planet
    create_planet(world, (Vector3){100, 0, 0}, 12.0f, 12.0f);    // Earth-like
    create_planet(world, (Vector3){0, 0, 140}, 10.0f, 8.0f);     // Outer planet
    create_planet(world, (Vector3){-180, 0, 0}, 6.0f, 5.0f);     // Far planet
    
    // Create asteroid belt between inner and outer orbits
    for (int i = 0; i < 20; i++) {
        float angle = (float)i / 20.0f * 2.0f * M_PI;
        float distance = 75.0f + (rand() % 30);  // Between planets
        float x = distance * cosf(angle) + (rand() % 10 - 5);
        float z = distance * sinf(angle) + (rand() % 10 - 5);
        float y = (rand() % 10 - 5);
        
        create_asteroid(world, (Vector3){x, y, z}, 1.0f + (rand() % 3));
    }
    
    // Create scattered asteroids in outer system
    for (int i = 0; i < 15; i++) {
        float distance = 200.0f + (rand() % 100);
        float angle = (float)rand() / RAND_MAX * 2.0f * M_PI;
        float x = distance * cosf(angle);
        float z = distance * sinf(angle);
        float y = (rand() % 20 - 10);
        
        create_asteroid(world, (Vector3){x, y, z}, 0.5f + (rand() % 2));
    }
    
    // Create AI ships scattered around the system
    create_ai_ship(world, (Vector3){45, 5, 45}, "Explorer");
    create_ai_ship(world, (Vector3){-80, 10, 30}, "Trader");
    create_ai_ship(world, (Vector3){20, -15, -160}, "Fighter");
    create_ai_ship(world, (Vector3){150, 8, -50}, "Patrol");
    create_ai_ship(world, (Vector3){-200, 0, -200}, "Scout");
    
    // Create player ship starting near the inner system
    *player_id = create_player(world, (Vector3){-40, 0, -40});
    
    printf("🌍 Solar system created with %d entities\n", world->entity_count);
}

int main(void) {
    printf("🎮 V2 Component-Based Game Engine Test\n");
    printf("=====================================\n\n");
    
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
    populate_solar_system(&world, &player);
    
    printf("\n� Starting simulation...\n");
    printf("Press Ctrl+C to exit\n\n");
    
    // Simulation loop
    const float dt = 1.0f / 60.0f;  // 60 FPS
    const float sim_duration = 60.0f;  // Run for 60 seconds
    const int max_frames = (int)(sim_duration / dt);
    
    for (int frame = 0; frame < max_frames; frame++) {
        float time = frame * dt;
        
        // Simulate player input
        simulate_player_input(&world, player, time);
        
        // Update world
        world_update(&world, dt);
        
        // Update systems (this will render the frame)
        scheduler_update(&scheduler, &world, dt);
        
        // Brief pause for real-time effect (simplified)
        // Note: In a real implementation, you'd use proper timing
        for (volatile int i = 0; i < 1000000; i++);  // Simple delay
        
        // Print status occasionally
        if (frame % 300 == 0 && frame > 0) {  // Every 5 seconds
            printf("\n⏱️  Time: %.1fs, Frame: %d, Entities: %d\n\n", 
                   time, frame, world.entity_count);
        }
    }
    
    printf("\n🏁 Simulation complete!\n");
    
    // Print final stats
    scheduler_destroy(&scheduler);
    world_destroy(&world);
    
    return 0;
}
