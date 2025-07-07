#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "assets.h"
#include "core.h"

// ============================================================================
// DATA-DRIVEN ENTITY SYSTEM
// ============================================================================

// Entity template definition (loaded from data files)
typedef struct
{
    char name[64];
    char description[256];
    char tags[10][32];
    int tag_count;

    // Component configuration
    bool has_transform;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;

    bool has_physics;
    float mass;
    float drag;
    Vector3 velocity;
    Vector3 acceleration;
    bool kinematic;

    bool has_collision;
    int collision_shape;  // 0=sphere, 1=box, 2=capsule
    float collision_radius;
    uint32_t layer_mask;
    bool is_trigger;

    bool has_renderable;
    char mesh_name[64];
    char material_name[64];
    bool visible;

    bool has_ai;
    int initial_ai_state;  // 0=idle, 1=patrolling, etc.
    float ai_update_frequency;

    bool has_player;
    
    bool has_thrusters;
    bool has_control_authority;

    bool has_camera;
    int camera_behavior;  // 0=third_person, 1=first_person, 2=static, etc.
    float fov;
    float near_plane;
    float far_plane;
    float aspect_ratio;
    float follow_distance;
    Vector3 follow_offset;
    float follow_smoothing;
} EntityTemplate;

// Scene definition (collection of entity spawns)
typedef struct
{
    char entity_type[64];
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    char custom_params[256];  // JSON-like custom parameters
} EntitySpawn;

typedef struct
{
    char name[64];
    char description[256];
    EntitySpawn spawns[256];
    uint32_t spawn_count;
} SceneTemplate;

// Data registry for templates
typedef struct
{
    EntityTemplate entity_templates[128];
    uint32_t entity_template_count;

    SceneTemplate scene_templates[32];
    uint32_t scene_template_count;

    char data_root[512];
} DataRegistry;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Data registry management
bool data_registry_init(DataRegistry* registry, const char* data_root);
void data_registry_cleanup(DataRegistry* registry);

// Template loading
bool load_entity_templates_with_fallback(DataRegistry* registry, const char* base_name);
bool load_entity_templates(DataRegistry* registry, const char* templates_path);
bool load_scene_templates(DataRegistry* registry, const char* scenes_path);
bool load_all_scene_templates(DataRegistry* registry, const char* scenes_dir);

// Entity creation from templates
EntityID create_entity_from_template(struct World* world, DataRegistry* registry,
                                     AssetRegistry* assets, const char* template_name,
                                     Vector3 position);

// Scene loading
bool load_scene(struct World* world, DataRegistry* registry, AssetRegistry* assets,
                const char* scene_name);

// Template queries
EntityTemplate* find_entity_template(DataRegistry* registry, const char* name);
SceneTemplate* find_scene_template(DataRegistry* registry, const char* name);

// Data serialization (binary format for performance)
bool save_world_to_binary(struct World* world, const char* filename);
bool load_world_from_binary(struct World* world, const char* filename);

// Template listing
void list_entity_templates(DataRegistry* registry);
void list_scene_templates(DataRegistry* registry);
