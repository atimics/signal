#include "cgame.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Simple implementation to demonstrate the architecture
// This would be expanded significantly in the full system

struct Universe* universe_create(const char* ai_model_path) {
    struct Universe* universe = malloc(sizeof(struct Universe));
    memset(universe, 0, sizeof(struct Universe));
    
    // Initialize AI engine
    universe->ai_engine = malloc(sizeof(struct AIEngine));
    if (!ai_engine_init(universe->ai_engine, ai_model_path)) {
        printf("❌ Failed to initialize AI engine\n");
        free(universe->ai_engine);
        free(universe);
        return NULL;
    }
    
    universe->ai_update_frequency = 0.1f; // 10 times per second
    universe->universe_time = 0.0f;
    
    printf("🌌 struct Universe created with AI engine\n");
    return universe;
}

void universe_destroy(struct Universe* universe) {
    if (!universe) return;
    
    // Destroy all entities
    struct Entity* entity = universe->entities;
    while (entity) {
        struct Entity* next = entity->next;
        entity_destroy(entity);
        entity = next;
    }
    
    // Shutdown AI engine
    if (universe->ai_engine) {
        ai_engine_shutdown(universe->ai_engine);
        free(universe->ai_engine);
    }
    
    free(universe);
    printf("🌌 struct Universe destroyed\n");
}

void universe_update(struct Universe* universe, float deltaTime) {
    if (!universe) return;
    
    universe->universe_time += deltaTime;
    
    // Update all entities
    struct Entity* entity = universe->entities;
    while (entity) {
        entity_update(entity, universe, deltaTime);
        entity = entity->next;
    }
    
    // Process AI tasks at specified frequency
    universe->last_ai_update += deltaTime;
    if (universe->last_ai_update >= universe->ai_update_frequency) {
        universe_process_ai_tasks(universe, deltaTime);
        universe->last_ai_update = 0.0f;
    }
    
    // Update LOD system
    universe_update_lod_system(universe);
}

void universe_add_entity(struct Universe* universe, struct Entity* entity) {
    if (!universe || !entity) return;
    
    // Add to linked list
    entity->next = universe->entities;
    universe->entities = entity;
    universe->total_entity_count++;
    
    // Set unique ID
    entity->id = universe->total_entity_count;
    
    printf("➕ struct Entity added: %s (ID: %d)\n", 
           entity->name ? entity->name : "Unnamed", entity->id);
    
    // Trigger event
    if (universe->on_entity_created) {
        universe->on_entity_created(entity);
    }
}

void universe_remove_entity(struct Universe* universe, struct Entity* entity) {
    if (!universe || !entity) return;
    
    // Remove from linked list
    if (universe->entities == entity) {
        universe->entities = entity->next;
    } else {
        struct Entity* current = universe->entities;
        while (current && current->next != entity) {
            current = current->next;
        }
        if (current) {
            current->next = entity->next;
        }
    }
    
    universe->total_entity_count--;
    
    printf("➖ struct Entity removed: %s (ID: %d)\n", 
           entity->name ? entity->name : "Unnamed", entity->id);
    
    // Trigger event
    if (universe->on_entity_destroyed) {
        universe->on_entity_destroyed(entity);
    }
}

int universe_find_entities_near(struct Universe* universe, Vector3D position, float radius, 
                               struct Entity** results, int max_results) {
    if (!universe || !results) return 0;
    
    int count = 0;
    struct Entity* entity = universe->entities;
    
    while (entity && count < max_results) {
        // Calculate distance
        Vector3D diff = {
            entity->position.x - position.x,
            entity->position.y - position.y,
            entity->position.z - position.z
        };
        float distance = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        if (distance <= radius) {
            results[count++] = entity;
        }
        
        entity = entity->next;
    }
    
    return count;
}

void universe_process_ai_tasks(struct Universe* universe, float deltaTime) {
    if (!universe || !universe->ai_engine) return;
    
    universe->ai_tasks_processed_this_frame = 0;
    
    // Process pending AI tasks
    ai_engine_process_tasks(universe->ai_engine, deltaTime);
    
    // Request AI updates for entities that need them
    struct Entity* entity = universe->entities;
    while (entity) {
        if (entity_should_update_ai(entity, universe->universe_time)) {
            // Request appropriate AI task based on entity state
            switch (entity->current_state) {
                case ENTITY_STATE_COMMUNICATING:
                    // Dialog requests are handled separately by player interaction
                    break;
                case ENTITY_STATE_IDLE:
                    ai_request_behavior_update(universe->ai_engine, entity);
                    break;
                case ENTITY_STATE_PATROLLING:
                case ENTITY_STATE_PURSUING:
                    // Request navigation decisions
                    Vector3D target = {0, 0, 0}; // Simplified target
                    ai_request_navigation_decision(universe->ai_engine, entity, target);
                    break;
                default:
                    break;
            }
            
            entity->last_ai_update = universe->universe_time;
        }
        entity = entity->next;
    }
}

void universe_update_lod_system(struct Universe* universe) {
    if (!universe) return;
    
    universe->active_entity_count = 0;
    universe->entities_with_active_ai = 0;
    
    struct Entity* entity = universe->entities;
    while (entity) {
        entity_update_lod(entity, universe->player_position);
        
        if (entity->distance_to_player < 1000.0f) {
            universe->active_entity_count++;
        }
        
        if (entity->ai_update_frequency > 0.0f) {
            universe->entities_with_active_ai++;
        }
        
        entity = entity->next;
    }
}

void universe_send_message(struct Universe* universe, struct Entity* sender, struct Entity* receiver, const char* message) {
    if (!universe || !sender || !receiver || !message) return;
    
    printf("📡 Message: %s → %s: '%s'\n", 
           sender->name ? sender->name : "Unknown",
           receiver->name ? receiver->name : "Unknown",
           message);
    
    // Update entity message state
    if (receiver->last_message_received) free(receiver->last_message_received);
    receiver->last_message_received = strdup(message);
    
    if (sender->last_message_sent) free(sender->last_message_sent);
    sender->last_message_sent = strdup(message);
    
    // Trigger AI response if this is a dialog
    if (receiver->type != ENTITY_PLAYER_SHIP) {
        // AI entity received a message, request dialog response
        ai_request_dialog(universe->ai_engine, receiver, message, 
                         (void (*)(struct Entity*, char*))receiver->on_ai_response);
    }
    
    // Trigger event
    if (universe->on_entities_communicate) {
        universe->on_entities_communicate(sender, receiver, message);
    }
}

struct Entity* universe_spawn_entity(struct Universe* universe, EntityType type, Vector3D position, const char* ai_personality) {
    if (!universe) return NULL;
    
    struct Entity* entity = entity_create(type, position);
    if (!entity) return NULL;
    
    // Set AI personality if provided
    if (ai_personality && strlen(ai_personality) > 0) {
        entity->personality = malloc(sizeof(struct AIPersonality));
        memset(entity->personality, 0, sizeof(struct AIPersonality));
        entity->personality->base_prompt = strdup(ai_personality);
        entity->personality->dialog_style = strdup("friendly and curious");
        entity->personality->goals_and_motivations = strdup("explore and interact");
    }
    
    universe_add_entity(universe, entity);
    return entity;
}

void universe_print_stats(struct Universe* universe) {
    if (!universe) return;
    
    printf("\n🌌 UNIVERSE STATISTICS 🌌\n");
    printf("Total Entities: %d\n", universe->total_entity_count);
    printf("Active Entities: %d\n", universe->active_entity_count);
    printf("Entities with AI: %d\n", universe->entities_with_active_ai);
    printf("AI Tasks/Second: %d\n", universe->ai_engine ? universe->ai_engine->tasks_processed_per_second : 0);
    printf("Avg AI Response Time: %.3fms\n", universe->ai_engine ? universe->ai_engine->average_inference_time * 1000 : 0);
    printf("struct Universe Time: %.1f seconds\n", universe->universe_time);
    printf("═══════════════════════════════════\n\n");
}
