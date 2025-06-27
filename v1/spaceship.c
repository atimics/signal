#include "cgame.h"

// Initialize spaceship
void init_spaceship(struct Spaceship* ship) {
    ship->position = (Vector3D){0, 0, 0};
    ship->velocity = (Vector3D){0, 0, 0};
    ship->rotation = (Vector3D){0, 0, 0};
    ship->angular_velocity = (Vector3D){0, 0, 0};
    ship->speed = 3.0f;
    ship->rotSpeed = 3.0f;
    ship->throttle = 0.0f;
    ship->shields = 100.0f;
    ship->energy = 100.0f;
    ship->afterburner = false;
    ship->firing = false;
    ship->mesh = &starship_mesh;
    
    // Initialize collision properties
    ship->collision_radius = 4.0f;
    ship->collision_enabled = true;
}

// Update spaceship physics
void update_spaceship(struct Spaceship* ship, float deltaTime) {
    // Apply throttle to forward velocity
    float thrust = ship->throttle * ship->speed;
    if (ship->afterburner && ship->energy > 0) {
        thrust *= 2.0f; // Afterburner doubles thrust
        ship->energy -= 30.0f * deltaTime; // Drain energy
        if (ship->energy < 0) ship->energy = 0;
    } else {
        // Regenerate energy when not using afterburner
        ship->energy += 20.0f * deltaTime;
        if (ship->energy > 100.0f) ship->energy = 100.0f;
    }
    
    // Apply thrust in forward direction (Z-axis)
    ship->velocity.z += thrust * deltaTime;
    
    // Apply angular velocity to rotation
    ship->rotation = vector3d_add(ship->rotation, 
                                vector3d_multiply(ship->angular_velocity, deltaTime));
    
    // Apply velocity to position
    ship->position = vector3d_add(ship->position, 
                                vector3d_multiply(ship->velocity, deltaTime));
    
    // Apply damping to linear velocity (space friction)
    ship->velocity = vector3d_multiply(ship->velocity, 0.985f);
    
    // Apply damping to angular velocity (rotational friction)
    ship->angular_velocity = vector3d_multiply(ship->angular_velocity, 0.95f);
}

// Update spaceship with collision detection against universe entities
void update_spaceship_with_universe(struct Spaceship* ship, struct Universe* universe, float deltaTime) {
    if (!ship || !universe) return;
    
    // Store previous position for collision recovery
    Vector3D prev_position = ship->position;
    
    // Update spaceship physics normally
    update_spaceship(ship, deltaTime);
    
    // Check collisions with universe entities if collision is enabled
    if (ship->collision_enabled) {
        struct Entity* entity = universe->entities;
        static float last_collision_message_time = 0.0f;
        float current_time = universe->universe_time;
        
        while (entity) {
            if (entity->collision && entity->collision->enabled && entity->collision->is_solid) {
                // Calculate distance to entity
                float dx = ship->position.x - entity->position.x;
                float dy = ship->position.y - entity->position.y;
                float dz = ship->position.z - entity->position.z;
                float distance = sqrtf(dx*dx + dy*dy + dz*dz);
                
                // Check collision
                float combined_radius = ship->collision_radius + entity->collision->radius;
                if (distance < combined_radius) {
                    // Only print collision message once per second to avoid spam
                    if (current_time - last_collision_message_time > 1.0f) {
                        printf("💥 Player ship collided with %s!\n", 
                               entity->name ? entity->name : "Unknown Entity");
                        last_collision_message_time = current_time;
                    }
                    
                    // Calculate proper separation vector
                    if (distance > 0.001f) {
                        // Normalize separation direction
                        float inv_distance = 1.0f / distance;
                        dx *= inv_distance;
                        dy *= inv_distance;
                        dz *= inv_distance;
                        
                        // Calculate overlap
                        float overlap = combined_radius - distance;
                        
                        // Move ship away from the entity with some buffer
                        float separation_distance = overlap + 2.0f; // 2 unit buffer
                        ship->position.x = entity->position.x + dx * (entity->collision->radius + ship->collision_radius + 2.0f);
                        ship->position.y = entity->position.y + dy * (entity->collision->radius + ship->collision_radius + 2.0f);
                        ship->position.z = entity->position.z + dz * (entity->collision->radius + ship->collision_radius + 2.0f);
                    } else {
                        // If positions are identical, push ship away from origin
                        ship->position.x = entity->position.x + combined_radius + 2.0f;
                        ship->position.y = entity->position.y;
                        ship->position.z = entity->position.z;
                    }
                    
                    // Dampen velocity in collision direction to prevent bouncing
                    float dot_product = ship->velocity.x * dx + ship->velocity.y * dy + ship->velocity.z * dz;
                    if (dot_product < 0) { // Moving towards the object
                        ship->velocity.x -= dot_product * dx;
                        ship->velocity.y -= dot_product * dy;
                        ship->velocity.z -= dot_product * dz;
                    }
                    
                    // Apply additional damping to prevent jittering
                    ship->velocity.x *= 0.8f;
                    ship->velocity.y *= 0.8f;
                    ship->velocity.z *= 0.8f;
                    ship->angular_velocity.x *= 0.8f;
                    ship->angular_velocity.y *= 0.8f;
                    ship->angular_velocity.z *= 0.8f;
                    
                    // Take damage to shields (but not every frame)
                    if (current_time - last_collision_message_time > 0.9f) {
                        ship->shields -= 5.0f;
                        if (ship->shields < 0) ship->shields = 0;
                    }
                    
                    break; // Only handle one collision per frame
                }
            }
            entity = entity->next;
        }
    }
}
