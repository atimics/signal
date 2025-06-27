#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include <stdint.h>
#include "math3d.h"
#include "mesh.h"
#include "ai_engine.h"

// Forward declarations to avoid circular dependencies
struct Universe;

// Entity types
typedef enum {
    ENTITY_PLAYER_SHIP,
    ENTITY_AI_FIGHTER,
    ENTITY_AI_TRADER,
    ENTITY_AI_EXPLORER,
    ENTITY_AI_STATION,
    ENTITY_AI_CIVILIAN,
    ENTITY_AI_COMMANDER,
    ENTITY_ENVIRONMENT
} EntityType;

// Entity states
typedef enum {
    ENTITY_STATE_IDLE,
    ENTITY_STATE_PATROLLING,
    ENTITY_STATE_ATTACKING,
    ENTITY_STATE_FLEEING,
    ENTITY_STATE_TRADING,
    ENTITY_STATE_EXPLORING,
    ENTITY_STATE_DIALOG,
    ENTITY_STATE_DEAD,
    ENTITY_STATE_PURSUING,
    ENTITY_STATE_COMMUNICATING
} EntityState;

// AI Personality (like your CosyWorld prompts)
struct AIPersonality {
    char* base_prompt;           // Core personality description
    char* dialog_style;          // How they speak
    char* goals_and_motivations; // What drives them
    char* relationship_context;  // How they relate to others
    float aggression_level;      // 0.0 = peaceful, 1.0 = hostile
    float intelligence_level;    // Affects response complexity
    float social_tendency;       // How much they interact with others
};

// Entity Sensors (your location + sensors concept)
struct EntitySensors {
    float visual_range;
    float communication_range;
    bool can_detect_stealth;
    bool has_long_range_sensors;
    int sensor_resolution;      // Affects AI context detail
};

// Collision Component for physics
struct CollisionComponent {
    float radius;               // Sphere collision radius
    bool is_solid;              // Can be collided with
    bool is_sensor;             // Triggers events but doesn't block movement
    float mass;                 // For physics calculations
    bool enabled;               // Can disable collision temporarily
};

// Main Entity Structure (your multi-thousand agents)
struct Entity {
    // Core identity
    uint32_t id;
    EntityType type;
    char* name;
    
    // 3D representation
    struct Mesh* mesh;
    Vector3D position;
    Vector3D rotation;
    Vector3D velocity;
    Vector3D angular_velocity;
    
    // Physics
    struct CollisionComponent* collision;
    
    // AI system
    struct AIPersonality* personality;
    struct EntitySensors* sensors;
    EntityState current_state;
    char* current_script;        // AI-generated behavior script
    float script_update_timer;   // When to request new script
    
    // Game properties
    float health;
    float energy;
    float fuel;
    float cargo_capacity;
    
    // AI context cache (performance optimization)
    struct AIContext* cached_context;
    float context_cache_age;
    
    // Communication system
    char* last_message_received;
    char* last_message_sent;
    float communication_cooldown;
    
    // LOD system
    float distance_to_player;
    float ai_update_frequency;   // How often AI processes this entity
    float last_ai_update;
    
    // Behavior callbacks
    void (*update)(struct Entity* self, struct Universe* universe, float deltaTime);
    void (*on_message)(struct Entity* self, const char* message, struct Entity* sender);
    void (*on_collision)(struct Entity* self, struct Entity* other);
    void (*on_ai_response)(struct Entity* self, const char* ai_response);
    
    // Linked list for efficient iteration
    struct Entity* next;
};

// Entity Management API
struct Entity* entity_create(EntityType type, Vector3D position);
void entity_destroy(struct Entity* entity);
void entity_update(struct Entity* entity, struct Universe* universe, float deltaTime);

// AI Integration
void entity_request_ai_task(struct Entity* entity, struct AIEngine* engine, AITaskType task_type);
void entity_process_ai_response(struct Entity* entity, const char* response, AITaskType task_type);

// Communication
void entity_send_message(struct Entity* sender, struct Entity* receiver, const char* message);
void entity_broadcast_message(struct Entity* sender, const char* message, float range);

// Collision System
bool entity_check_collision(struct Entity* a, struct Entity* b);
void entity_handle_collision(struct Entity* a, struct Entity* b);
void entity_update_physics(struct Entity* entity, struct Universe* universe, float deltaTime);

// LOD system
void entity_update_lod(struct Entity* entity, Vector3D player_position);
bool entity_should_update_ai(struct Entity* entity, float current_time);

#endif
