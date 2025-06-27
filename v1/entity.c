#include "cgame.h"
#include "entity.h"  // Ensure we have full entity definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Entity counter for unique IDs
static uint32_t g_next_entity_id = 1;

struct Entity* entity_create(EntityType type, Vector3D position) {
    struct Entity* entity = malloc(sizeof(struct Entity));
    memset(entity, 0, sizeof(struct Entity));
    
    entity->id = g_next_entity_id++;
    entity->type = type;
    entity->position = position;
    entity->velocity = (Vector3D){0, 0, 0};
    entity->rotation = (Vector3D){0, 0, 0};
    
    // Initialize default stats
    entity->health = 100.0f;
    entity->energy = 100.0f;
    entity->fuel = 100.0f;
    entity->cargo_capacity = 50.0f;
    
    // Set default state
    entity->current_state = ENTITY_STATE_IDLE;
    entity->ai_update_frequency = 1.0f; // Update AI once per second by default
    
    // Initialize sensors based on type
    entity->sensors = malloc(sizeof(struct EntitySensors));
    
    // Initialize collision component
    entity->collision = malloc(sizeof(struct CollisionComponent));
    entity->collision->enabled = true;
    entity->collision->is_sensor = false;
    
    switch (type) {
        case ENTITY_AI_FIGHTER:
            entity->sensors->visual_range = 300.0f;
            entity->sensors->communication_range = 500.0f;
            entity->sensors->can_detect_stealth = true;
            entity->ai_update_frequency = 0.5f; // Fighters need faster AI
            // Fighter collision properties
            entity->collision->radius = 5.0f;
            entity->collision->is_solid = true;
            entity->collision->mass = 100.0f;
            break;
        case ENTITY_AI_TRADER:
            entity->sensors->visual_range = 200.0f;
            entity->sensors->communication_range = 400.0f;
            entity->sensors->can_detect_stealth = false;
            entity->ai_update_frequency = 2.0f; // Traders think slower
            // Trader collision properties
            entity->collision->radius = 8.0f;
            entity->collision->is_solid = true;
            entity->collision->mass = 200.0f;
            break;
        case ENTITY_AI_EXPLORER:
            entity->sensors->visual_range = 400.0f;
            entity->sensors->communication_range = 600.0f;
            entity->sensors->has_long_range_sensors = true;
            entity->ai_update_frequency = 1.5f;
            // Explorer collision properties
            entity->collision->radius = 6.0f;
            entity->collision->is_solid = true;
            entity->collision->mass = 120.0f;
            break;
        case ENTITY_PLAYER_SHIP:
            entity->sensors->visual_range = 250.0f;
            entity->sensors->communication_range = 350.0f;
            entity->sensors->can_detect_stealth = false;
            // Player ship collision properties
            entity->collision->radius = 4.0f;
            entity->collision->is_solid = true;
            entity->collision->mass = 80.0f;
            break;
        case ENTITY_ENVIRONMENT:
            // For celestial bodies like the sun
            entity->sensors->visual_range = 1000.0f;
            entity->sensors->communication_range = 0.0f; // No communication
            entity->sensors->can_detect_stealth = false;
            // Environment objects (like sun) have large collision spheres
            entity->collision->radius = 50.0f; // Large collision sphere for sun
            entity->collision->is_solid = true;
            entity->collision->mass = 10000.0f; // Immovable
            break;
        default:
            entity->sensors->visual_range = 150.0f;
            entity->sensors->communication_range = 250.0f;
            entity->sensors->can_detect_stealth = false;
            // Default collision properties
            entity->collision->radius = 5.0f;
            entity->collision->is_solid = true;
            entity->collision->mass = 100.0f;
            break;
    }
    
    printf("✨ Created entity #%d of type %d at (%.1f, %.1f, %.1f)\n", 
           entity->id, type, position.x, position.y, position.z);
    
    return entity;
}

void entity_destroy(struct Entity* entity) {
    if (!entity) return;
    
    printf("💀 Destroying entity #%d\n", entity->id);
    
    if (entity->name) free(entity->name);
    if (entity->personality) {
        if (entity->personality->base_prompt) free(entity->personality->base_prompt);
        if (entity->personality->dialog_style) free(entity->personality->dialog_style);
        if (entity->personality->goals_and_motivations) free(entity->personality->goals_and_motivations);
        if (entity->personality->relationship_context) free(entity->personality->relationship_context);
        free(entity->personality);
    }
    if (entity->sensors) free(entity->sensors);
    if (entity->collision) free(entity->collision);
    if (entity->cached_context) free(entity->cached_context);
    if (entity->last_message_received) free(entity->last_message_received);
    if (entity->last_message_sent) free(entity->last_message_sent);
    
    free(entity);
}

void entity_update(struct Entity* entity, struct Universe* universe, float deltaTime) {
    if (!entity || !universe) return;
    
    // Update physics and handle collisions
    entity_update_physics(entity, universe, deltaTime);
    
    // Update energy/fuel consumption
    float speed = sqrt(entity->velocity.x * entity->velocity.x + 
                      entity->velocity.y * entity->velocity.y + 
                      entity->velocity.z * entity->velocity.z);
    
    if (speed > 1.0f) {
        entity->fuel -= speed * deltaTime * 0.1f;
        if (entity->fuel < 0) entity->fuel = 0;
    }
    
    // Regenerate energy slowly
    if (entity->energy < 100.0f) {
        entity->energy += deltaTime * 5.0f;
        if (entity->energy > 100.0f) entity->energy = 100.0f;
    }
    
    // Call custom update function if available
    if (entity->update) {
        entity->update(entity, universe, deltaTime);
    }
}

void entity_update_lod(struct Entity* entity, Vector3D player_position) {
    if (!entity) return;
    
    // Calculate distance to player
    Vector3D diff = {
        entity->position.x - player_position.x,
        entity->position.y - player_position.y,
        entity->position.z - player_position.z
    };
    
    entity->distance_to_player = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    
    // Adjust AI update frequency based on distance (LOD system)
    if (entity->distance_to_player < 100.0f) {
        // Close: High frequency updates, but not too aggressive
        entity->ai_update_frequency = entity->type == ENTITY_AI_FIGHTER ? 1.0f : 2.0f;
    } else if (entity->distance_to_player < 500.0f) {
        // Medium: Normal frequency
        entity->ai_update_frequency = entity->type == ENTITY_AI_FIGHTER ? 2.0f : 4.0f;
    } else if (entity->distance_to_player < 2000.0f) {
        // Far: Low frequency
        entity->ai_update_frequency = 8.0f;
    } else {
        // Very far: Very low frequency
        entity->ai_update_frequency = 30.0f;
    }
}

bool entity_should_update_ai(struct Entity* entity, float current_time) {
    if (!entity) return false;
    return (current_time - entity->last_ai_update) >= entity->ai_update_frequency;
}

void entity_request_ai_task(struct Entity* entity, struct AIEngine* engine, AITaskType task_type) {
    if (!entity || !engine) return;
    
    switch (task_type) {
        case AI_TASK_DIALOG:
            // Dialog requests are handled by the dialog system
            break;
        case AI_TASK_BEHAVIOR_SCRIPT:
            ai_request_behavior_update(engine, entity);
            break;
        case AI_TASK_NAVIGATION:
            // Generate random target for demo
            {
                Vector3D target = {
                    entity->position.x + (rand() % 200 - 100),
                    entity->position.y + (rand() % 200 - 100),
                    entity->position.z + (rand() % 200 - 100)
                };
                ai_request_navigation_decision(engine, entity, target);
            }
            break;
        default:
            break;
    }
}

void entity_process_ai_response(struct Entity* entity, const char* response, AITaskType task_type) {
    if (!entity || !response) return;
    
    printf("🤖 Entity %s received AI response for task %d: \"%.80s%s\"\n",
           entity->name ? entity->name : "Unknown",
           task_type,
           response,
           strlen(response) > 80 ? "..." : "");
    
    // Process response based on task type
    switch (task_type) {
        case AI_TASK_BEHAVIOR_SCRIPT:
            // Parse behavior commands (simplified)
            if (strstr(response, "patrol")) {
                entity->current_state = ENTITY_STATE_PATROLLING;
            } else if (strstr(response, "idle")) {
                entity->current_state = ENTITY_STATE_IDLE;
            }
            break;
        case AI_TASK_NAVIGATION:
            // Parse navigation decisions
            if (strstr(response, "approach") || strstr(response, "move")) {
                // Add some movement
                entity->velocity.x += (rand() % 10 - 5) * 0.1f;
                entity->velocity.z += (rand() % 10 - 5) * 0.1f;
            }
            break;
        case AI_TASK_DIALOG:
            // Dialog responses are handled by the dialog system
            break;
        case AI_TASK_REACTION:
            // Parse and execute reaction commands
            if (strstr(response, "turn_to_face_player")) {
                printf("👀 %s turns to face the player\n", entity->name ? entity->name : "Ship");
                // Simple face player logic - rotate toward player
                // This would need player position, but for now just indicate the action
                entity->current_state = ENTITY_STATE_COMMUNICATING;
            } else if (strstr(response, "send_greeting")) {
                printf("👋 %s sends a greeting to the player\n", entity->name ? entity->name : "Ship");
                // This would trigger a radio message in a full implementation
                entity->current_state = ENTITY_STATE_COMMUNICATING;
            } else if (strstr(response, "move_away_cautiously")) {
                printf("🚀 %s moves away cautiously from the player\n", entity->name ? entity->name : "Ship");
                // Add evasive movement
                entity->velocity.x += (rand() % 20 - 10) * 0.2f;
                entity->velocity.z += (rand() % 20 - 10) * 0.2f;
                entity->current_state = ENTITY_STATE_FLEEING;
            } else if (strstr(response, "ignore_player")) {
                printf("😐 %s ignores the player and continues current activity\n", entity->name ? entity->name : "Ship");
                // No action, maintain current state
            }
            break;
        default:
            break;
    }
    
    // Call custom AI response handler if available
    if (entity->on_ai_response) {
        entity->on_ai_response(entity, response);
    }
}

void entity_send_message(struct Entity* sender, struct Entity* receiver, const char* message) {
    if (!sender || !receiver || !message) return;
    
    printf("📡 %s → %s: \"%s\"\n",
           sender->name ? sender->name : "Unknown",
           receiver->name ? receiver->name : "Unknown",
           message);
    
    // Store message in receiver
    if (receiver->last_message_received) free(receiver->last_message_received);
    receiver->last_message_received = strdup(message);
    
    // Store message in sender
    if (sender->last_message_sent) free(sender->last_message_sent);
    sender->last_message_sent = strdup(message);
    
    // Call message handler if available
    if (receiver->on_message) {
        receiver->on_message(receiver, message, sender);
    }
}

void entity_broadcast_message(struct Entity* sender, const char* message, float range) {
    if (!sender || !message) return;
    
    printf("📢 %s broadcasts: \"%s\" (range: %.1f)\n",
           sender->name ? sender->name : "Unknown",
           message, range);
    
    // In a full implementation, this would find all entities within range
    // and send the message to each one
}

// Collision Detection System
bool entity_check_collision(struct Entity* a, struct Entity* b) {
    if (!a || !b || !a->collision || !b->collision) return false;
    if (!a->collision->enabled || !b->collision->enabled) return false;
    if (a == b) return false; // Don't collide with self
    
    // Calculate distance between entities
    float dx = a->position.x - b->position.x;
    float dy = a->position.y - b->position.y;
    float dz = a->position.z - b->position.z;
    float distance = sqrtf(dx*dx + dy*dy + dz*dz);
    
    // Check if collision spheres overlap
    float combined_radius = a->collision->radius + b->collision->radius;
    return distance < combined_radius;
}

void entity_handle_collision(struct Entity* a, struct Entity* b) {
    if (!a || !b) return;
    
    printf("💥 Collision detected between entity #%d and #%d\n", a->id, b->id);
    
    // Basic collision response - stop both entities
    if (a->collision->is_solid && b->collision->is_solid) {
        // For now, just stop movement on collision
        a->velocity = (Vector3D){0, 0, 0};
        b->velocity = (Vector3D){0, 0, 0};
        
        // Separate entities to prevent overlap
        float dx = a->position.x - b->position.x;
        float dy = a->position.y - b->position.y;
        float dz = a->position.z - b->position.z;
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);
        
        if (distance > 0) {
            float combined_radius = a->collision->radius + b->collision->radius;
            float overlap = combined_radius - distance;
            
            // Normalize separation vector
            dx /= distance;
            dy /= distance;
            dz /= distance;
            
            // Move entities apart based on mass ratio
            float total_mass = a->collision->mass + b->collision->mass;
            float a_ratio = b->collision->mass / total_mass;
            float b_ratio = a->collision->mass / total_mass;
            
            a->position.x += dx * overlap * a_ratio;
            a->position.y += dy * overlap * a_ratio;
            a->position.z += dz * overlap * a_ratio;
            
            b->position.x -= dx * overlap * b_ratio;
            b->position.y -= dy * overlap * b_ratio;
            b->position.z -= dz * overlap * b_ratio;
        }
    }
    
    // Call collision handlers
    if (a->on_collision) a->on_collision(a, b);
    if (b->on_collision) b->on_collision(b, a);
}

void entity_update_physics(struct Entity* entity, struct Universe* universe, float deltaTime) {
    if (!entity || !universe) return;
    
    // Update basic physics
    entity->position.x += entity->velocity.x * deltaTime;
    entity->position.y += entity->velocity.y * deltaTime;
    entity->position.z += entity->velocity.z * deltaTime;
    
    // Apply basic damping
    entity->velocity.x *= 0.99f;
    entity->velocity.y *= 0.99f;
    entity->velocity.z *= 0.99f;
    
    // Only check collisions for entities that are moving or nearby
    float speed = sqrtf(entity->velocity.x * entity->velocity.x + 
                       entity->velocity.y * entity->velocity.y + 
                       entity->velocity.z * entity->velocity.z);
    
    if (speed > 0.1f || entity->distance_to_player < 200.0f) {
        // Check collisions with other entities (optimized)
        struct Entity* other = universe->entities;
        int collision_checks = 0;
        while (other && collision_checks < 10) { // Limit collision checks per frame
            if (other != entity && entity_check_collision(entity, other)) {
                entity_handle_collision(entity, other);
                break; // Only handle one collision per frame
            }
            other = other->next;
            collision_checks++;
        }
    }
}
