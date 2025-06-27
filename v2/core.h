#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// CORE TYPES
// ============================================================================

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float x, y, z, w;
} Quaternion;

// Entity ID type
typedef uint32_t EntityID;
#define INVALID_ENTITY 0

// Component type flags (bitfield)
typedef enum {
    COMPONENT_TRANSFORM  = 1 << 0,
    COMPONENT_PHYSICS    = 1 << 1, 
    COMPONENT_COLLISION  = 1 << 2,
    COMPONENT_AI         = 1 << 3,
    COMPONENT_RENDERABLE = 1 << 4,
    COMPONENT_PLAYER     = 1 << 5,
} ComponentType;

// ============================================================================
// COMPONENT DEFINITIONS (Pure Data)
// ============================================================================

struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    bool dirty;  // Needs matrix update
};

struct Physics {
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float drag;
    bool kinematic;  // Not affected by forces
};

struct Collision {
    enum {
        COLLISION_SPHERE,
        COLLISION_BOX,
        COLLISION_CAPSULE
    } shape;
    
    union {
        float radius;
        Vector3 box_size;
        struct { float radius; float height; } capsule;
    };
    
    bool is_trigger;      // Ghost vs solid
    uint32_t layer_mask;  // What this collides with
    uint32_t last_check_frame;  // For temporal optimization
};

struct AI {
    enum {
        AI_STATE_IDLE,
        AI_STATE_PATROLLING,
        AI_STATE_REACTING,
        AI_STATE_COMMUNICATING,
        AI_STATE_FLEEING
    } state;
    
    float decision_timer;
    float reaction_cooldown;
    Vector3 target_position;
    EntityID target_entity;
    
    // AI scheduling
    float update_frequency;  // Hz (2-10 based on distance)
    float last_update;
};

struct Renderable {
    uint32_t mesh_id;
    uint32_t material_id;
    bool visible;
    float lod_distance;
    uint8_t lod_level;
};

struct Player {
    float throttle;
    float afterburner_energy;
    bool controls_enabled;
};

// ============================================================================
// ENTITY DEFINITION
// ============================================================================

#define MAX_ENTITIES 4096

struct Entity {
    EntityID id;
    uint32_t component_mask;
    
    // Optional component pointers
    struct Transform* transform;
    struct Physics* physics;
    struct Collision* collision;
    struct AI* ai;
    struct Renderable* renderable; 
    struct Player* player;
};

// ============================================================================
// COMPONENT POOLS
// ============================================================================

struct ComponentPools {
    // Preallocated component arrays
    struct Transform transforms[MAX_ENTITIES];
    struct Physics physics[MAX_ENTITIES];
    struct Collision collisions[MAX_ENTITIES];
    struct AI ais[MAX_ENTITIES];
    struct Renderable renderables[MAX_ENTITIES];
    struct Player players[MAX_ENTITIES];
    
    // Free list management
    uint32_t transform_count;
    uint32_t physics_count;
    uint32_t collision_count;
    uint32_t ai_count;
    uint32_t renderable_count;
    uint32_t player_count;
};

// ============================================================================
// WORLD STATE
// ============================================================================

struct World {
    struct Entity entities[MAX_ENTITIES];
    uint32_t entity_count;
    uint32_t next_entity_id;
    
    struct ComponentPools components;
    
    // Frame timing
    uint32_t frame_number;
    float delta_time;
    float total_time;
};

// ============================================================================
// CORE API
// ============================================================================

// World management
bool world_init(struct World* world);
void world_destroy(struct World* world);
void world_update(struct World* world, float delta_time);

// Entity management  
EntityID entity_create(struct World* world);
void entity_destroy(struct World* world, EntityID entity_id);
struct Entity* entity_get(struct World* world, EntityID entity_id);

// Component management
bool entity_add_component(struct World* world, EntityID entity_id, ComponentType type);
void entity_remove_component(struct World* world, EntityID entity_id, ComponentType type);
bool entity_has_component(struct World* world, EntityID entity_id, ComponentType type);

// Component accessors
struct Transform* entity_get_transform(struct World* world, EntityID entity_id);
struct Physics* entity_get_physics(struct World* world, EntityID entity_id);
struct Collision* entity_get_collision(struct World* world, EntityID entity_id);
struct AI* entity_get_ai(struct World* world, EntityID entity_id);
struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id);
struct Player* entity_get_player(struct World* world, EntityID entity_id);

// Utility functions
Vector3 vector3_add(Vector3 a, Vector3 b);
Vector3 vector3_multiply(Vector3 v, float scalar);
float vector3_length(Vector3 v);
float vector3_distance(Vector3 a, Vector3 b);

#endif // CORE_H
