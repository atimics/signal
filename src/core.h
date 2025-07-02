/**
 * @file core.h
 * @brief Defines the core data structures and APIs for the engine's ECS.
 *
 * This file contains the foundational types, component definitions, and the
 * main World struct that drives the engine's simulation. It is the central
 * hub for all gameplay-related data.
 */

#ifndef CORE_H
#define CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// R06 Solution: PIMPL Idiom - Forward declaration for opaque graphics resources
struct GpuResources;

// ============================================================================
// CORE TYPES
// ============================================================================

/** @brief A 3D vector with float components. */
typedef struct
{
    float x, y, z;
} Vector3;

/** @brief A 2D vector with float components, typically for UV coordinates. */
typedef struct
{
    float u, v;
} Vector2;

/** @brief A quaternion for representing 3D rotations. */
typedef struct
{
    float x, y, z, w;
} Quaternion;

/** @brief A unique identifier for an entity in the world. */
typedef uint32_t EntityID;
#define INVALID_ENTITY 0 /**< A reserved ID for an invalid or null entity. */
#define INVALID_ENTITY_ID 0 /**< Alternative name for consistency with tests. */

/** @brief A bitmask representing the components attached to an entity. */
typedef enum
{
    COMPONENT_TRANSFORM = 1 << 0,
    COMPONENT_PHYSICS = 1 << 1,
    COMPONENT_COLLISION = 1 << 2,
    COMPONENT_AI = 1 << 3,
    COMPONENT_RENDERABLE = 1 << 4,
    COMPONENT_PLAYER = 1 << 5,
    COMPONENT_CAMERA = 1 << 6,
    COMPONENT_SCENENODE = 1 << 7,
} ComponentType;

/** @brief Level of Detail enumeration for performance optimization. */
typedef enum
{
    LOD_CULLED = 0,  /**< Entity is too far away to render */
    LOD_LOW = 1,     /**< Low detail rendering */
    LOD_MEDIUM = 2,  /**< Medium detail rendering */
    LOD_HIGH = 3     /**< High detail rendering */
} LODLevel;

// ============================================================================
// COMPONENT DEFINITIONS (Pure Data)
// ============================================================================

/** @brief Defines an entity's position, rotation, and scale in the world. */
struct Transform
{
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    bool dirty;  // Needs matrix update
};

/** @brief Defines an entity's physical properties for simulation. */
struct Physics
{
    // Linear dynamics
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 force_accumulator;    // Sum forces per frame
    float mass;
    
    // Angular dynamics (6DOF)
    Vector3 angular_velocity;     // Rotation rates (rad/s)
    Vector3 angular_acceleration; // Angular acceleration (rad/s²)
    Vector3 torque_accumulator;   // Sum torques per frame
    Vector3 moment_of_inertia;    // Per-axis resistance to rotation
    
    // Configuration
    float drag_linear;            // Linear drag coefficient
    float drag_angular;           // Angular drag coefficient
    bool kinematic;               // Not affected by forces
    bool has_6dof;               // Enable angular dynamics
    
    // Environment
    enum {
        PHYSICS_SPACE,
        PHYSICS_ATMOSPHERE
    } environment;
};

/** @brief Defines an entity's collision shape and properties. */
struct Collision
{
    enum
    {
        COLLISION_SPHERE,
        COLLISION_BOX,
        COLLISION_CAPSULE
    } shape;

    union
    {
        float radius;
        Vector3 box_size;
        struct
        {
            float radius;
            float height;
        } capsule;
    };

    bool is_trigger;            // Ghost vs solid
    uint32_t layer_mask;        // What this collides with
    uint32_t last_check_frame;  // For temporal optimization
};

/** @brief Defines the state and behavior for an AI-controlled entity. */
struct AI
{
    enum
    {
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

/** @brief Defines the data needed to render an entity. */
struct Renderable
{
    struct GpuResources* gpu_resources; /**< Opaque pointer to GPU-specific resources. */
    uint32_t index_count;               /**< Number of indices to draw. */
    uint32_t material_id;               /**< Material ID for shading properties. */
    bool visible;                       /**< Whether the entity should be rendered. */
    float lod_distance;                 /**< Distance at which to switch LOD levels. */
    uint8_t lod_level;                  /**< The current level of detail. */
};

/** @brief Defines data specific to a player-controlled entity. */
struct Player
{
    float throttle;
    float afterburner_energy;
    bool controls_enabled;
};

/** @brief Defines a camera for viewing the world. */
struct Camera
{
    // Position and orientation
    Vector3 position;
    Vector3 target;
    Vector3 up;

    // Projection parameters
    float fov;           // Field of view in degrees
    float aspect_ratio;  // Width/height ratio
    float near_plane;    // Near clipping plane
    float far_plane;     // Far clipping plane

    // Cached matrices (updated when camera changes)
    float view_matrix[16];
    float projection_matrix[16];
    float view_projection_matrix[16];
    bool matrices_dirty;

    // Camera behavior settings
    enum
    {
        CAMERA_BEHAVIOR_THIRD_PERSON,
        CAMERA_BEHAVIOR_FIRST_PERSON,
        CAMERA_BEHAVIOR_STATIC,
        CAMERA_BEHAVIOR_CHASE,
        CAMERA_BEHAVIOR_ORBITAL
    } behavior;

    // Follow target for chase cameras
    EntityID follow_target;
    float follow_distance;
    Vector3 follow_offset;
    float follow_smoothing;

    bool is_active;
};

/** @brief Defines a node in the scene graph hierarchy. */
#define MAX_SCENE_CHILDREN 16
struct SceneNode
{
    EntityID entity_id;                      /**< The entity this node belongs to. */
    EntityID parent;                         /**< Parent entity ID, INVALID_ENTITY if root. */
    EntityID children[MAX_SCENE_CHILDREN];   /**< Child entity IDs. */
    uint32_t num_children;                   /**< Current number of children. */
    
    // Transform hierarchy
    float local_transform[16];               /**< Transform relative to parent. */
    float world_transform[16];               /**< Final transform in world space. */
    bool transform_dirty;                    /**< Whether world transform needs recalculation. */
    
    // Scene graph metadata
    bool is_visible;                         /**< Visibility flag set by culling system. */
    uint32_t depth;                          /**< Depth in the scene graph (0 = root). */
};

// ============================================================================
// ENTITY DEFINITION
// ============================================================================

#define MAX_ENTITIES 4096

/** @brief Represents an object in the game world. */
struct Entity
{
    EntityID id;
    uint32_t component_mask;

    // Optional component pointers
    struct Transform* transform;
    struct Physics* physics;
    struct Collision* collision;
    struct AI* ai;
    struct Renderable* renderable;
    struct Player* player;
    struct Camera* camera;
    struct SceneNode* scene_node;
};

// ============================================================================
// COMPONENT POOLS
// ============================================================================

/** @brief Contains pre-allocated pools for all component types. */
struct ComponentPools
{
    struct Transform transforms[MAX_ENTITIES];
    struct Physics physics[MAX_ENTITIES];
    struct Collision collisions[MAX_ENTITIES];
    struct AI ais[MAX_ENTITIES];
    struct Renderable renderables[MAX_ENTITIES];
    struct Player players[MAX_ENTITIES];
    struct Camera cameras[MAX_ENTITIES];
    struct SceneNode scene_nodes[MAX_ENTITIES];

    uint32_t transform_count;
    uint32_t physics_count;
    uint32_t collision_count;
    uint32_t ai_count;
    uint32_t renderable_count;
    uint32_t player_count;
    uint32_t camera_count;
    uint32_t scene_node_count;
};

// ============================================================================
// WORLD STATE
// ============================================================================

/** @brief Represents the entire state of the game world. */
struct World
{
    struct Entity* entities;        // Dynamic allocation for TDD flexibility
    uint32_t entity_count;
    uint32_t max_entities;          // Maximum number of entities (for TDD)
    uint32_t next_entity_id;

    struct ComponentPools components;

    EntityID active_camera_entity;

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
void world_clear(struct World* world);
void world_update(struct World* world, float delta_time);

// Entity management
EntityID entity_create(struct World* world);
bool entity_destroy(struct World* world, EntityID entity_id);
struct Entity* entity_get(struct World* world, EntityID entity_id);

// Component management
bool entity_add_component(struct World* world, EntityID entity_id, ComponentType type);
bool entity_remove_component(struct World* world, EntityID entity_id, ComponentType type);
bool entity_has_component(struct World* world, EntityID entity_id, ComponentType type);

// Component accessors
struct Transform* entity_get_transform(struct World* world, EntityID entity_id);
struct Physics* entity_get_physics(struct World* world, EntityID entity_id);
struct Collision* entity_get_collision(struct World* world, EntityID entity_id);
struct AI* entity_get_ai(struct World* world, EntityID entity_id);
struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id);
struct Player* entity_get_player(struct World* world, EntityID entity_id);
struct Camera* entity_get_camera(struct World* world, EntityID entity_id);
struct SceneNode* entity_get_scene_node(struct World* world, EntityID entity_id);

// Camera management
void world_set_active_camera(struct World* world, EntityID camera_entity);
EntityID world_get_active_camera(struct World* world);
void camera_update_matrices(struct Camera* camera);
bool switch_to_camera(struct World* world, int camera_index);
bool cycle_to_next_camera(struct World* world);
void update_camera_aspect_ratio(struct World* world, float aspect_ratio);

/**
 * @brief Extracts the six planes of the camera's view frustum.
 * @param camera The camera to extract the frustum from.
 * @param frustum_planes A 2D array to store the six plane equations (Ax + By + Cz + D = 0).
 */
void camera_extract_frustum_planes(const struct Camera* camera, float frustum_planes[6][4]);

// Scene graph management
void scene_graph_update(struct World* world);
bool scene_node_add_child(struct World* world, EntityID parent_id, EntityID child_id);
bool scene_node_remove_child(struct World* world, EntityID parent_id, EntityID child_id);
void scene_node_update_world_transform(struct World* world, EntityID entity_id, const float* parent_transform);
EntityID scene_node_find_by_name(struct World* world, const char* name);

// Utility functions
Vector3 vector3_add(Vector3 a, Vector3 b);
Vector3 vector3_subtract(Vector3 a, Vector3 b);
Vector3 vector3_multiply(Vector3 v, float scalar);
Vector3 vector3_normalize(Vector3 v);
float vector3_length(Vector3 v);
float vector3_distance(Vector3 a, Vector3 b);

// Matrix utility functions
void mat4_identity(float* m);
void mat4_perspective(float* m, float fov, float aspect, float near, float far);
void mat4_lookat(float* m, Vector3 eye, Vector3 target, Vector3 up);
void mat4_multiply(float* result, const float* a, const float* b);
void mat4_translate(float* m, Vector3 t);
void mat4_rotation_x(float* m, float angle_radians);
void mat4_rotation_y(float* m, float angle_radians);
void mat4_rotation_z(float* m, float angle_radians);
void mat4_scale(float* m, Vector3 scale);
void mat4_from_quaternion(float* m, Quaternion q);
void mat4_compose_transform(float* result, Vector3 position, Quaternion rotation, Vector3 scale);

// TDD-required functions for Sprint 19
bool entity_add_components(struct World* world, EntityID entity_id, ComponentType components);
bool entity_is_valid(struct World* world, EntityID entity_id);

#endif  // CORE_H
