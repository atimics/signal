#ifndef UNIVERSE_H
#define UNIVERSE_H

#include <stdbool.h>
#include "math3d.h"
#include "entity.h"

// Forward declarations to avoid circular dependencies
struct AIEngine;

// Maximum entities (your multi-thousand agents goal)
#define MAX_ENTITIES 8888
#define MAX_ACTIVE_ENTITIES 1000  // LOD limit for full processing

// Universe sectors for spatial partitioning
#define SECTOR_SIZE 1000.0f
#define SECTORS_PER_AXIS 100

struct UniverseSector {
    struct Entity* entities[MAX_ENTITIES / 10];  // Entities in this sector
    int entity_count;
    bool needs_ai_update;
};

// Main Universe Structure
struct Universe {
    // Entity management
    struct Entity* entities;           // Linked list of all entities
    int total_entity_count;
    int active_entity_count;    // Entities currently being processed
    
    // AI system
    struct AIEngine* ai_engine;
    float ai_update_frequency;  // How often to process AI tasks
    float last_ai_update;
    
    // Spatial partitioning (for efficient neighbor finding)
    struct UniverseSector sectors[SECTORS_PER_AXIS][SECTORS_PER_AXIS];
    
    // Player reference for LOD calculations
    struct Entity* player_entity;
    Vector3D player_position;
    
    // Universe state
    float universe_time;
    bool paused;
    
    // Performance metrics
    int ai_tasks_processed_this_frame;
    float average_ai_response_time;
    int entities_with_active_ai;
    
    // Communication system
    struct Message* message_queue;
    int pending_messages;
    
    // Event system
    void (*on_entity_created)(struct Entity* entity);
    void (*on_entity_destroyed)(struct Entity* entity);
    void (*on_entities_communicate)(struct Entity* sender, struct Entity* receiver, const char* message);
};

// Message system for entity communication
struct Message {
    struct Entity* sender;
    struct Entity* receiver;
    char* content;
    float timestamp;
    bool broadcast;
    float range;
    struct Message* next;
};

// Universe Management API
struct Universe* universe_create(const char* ai_model_path);
void universe_destroy(struct Universe* universe);
void universe_update(struct Universe* universe, float deltaTime);

// Entity management
void universe_add_entity(struct Universe* universe, struct Entity* entity);
void universe_remove_entity(struct Universe* universe, struct Entity* entity);
struct Entity* universe_find_entity_by_id(struct Universe* universe, uint32_t id);

// Spatial queries (for AI context building)
int universe_find_entities_near(struct Universe* universe, Vector3D position, float radius, struct Entity** results, int max_results);
struct Entity* universe_find_closest_entity(struct Universe* universe, Vector3D position, EntityType type);

// AI processing management
void universe_process_ai_tasks(struct Universe* universe, float deltaTime);
void universe_update_lod_system(struct Universe* universe);

// Communication system
void universe_send_message(struct Universe* universe, struct Entity* sender, struct Entity* receiver, const char* message);
void universe_broadcast_message(struct Universe* universe, struct Entity* sender, const char* message, float range);
void universe_process_messages(struct Universe* universe);

// Spawning system (AI can request new entities)
struct Entity* universe_spawn_entity(struct Universe* universe, EntityType type, Vector3D position, const char* ai_personality);

// Debug and monitoring
void universe_print_stats(struct Universe* universe);
void universe_debug_ai_system(struct Universe* universe);

#endif
