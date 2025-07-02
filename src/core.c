#include "core.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// WORLD MANAGEMENT
// ============================================================================

bool world_init(struct World* world)
{
    if (!world) return false;

    memset(world, 0, sizeof(struct World));
    
    // Set default max entities
    world->max_entities = MAX_ENTITIES;
    
    // Allocate entity array
    world->entities = malloc(sizeof(struct Entity) * world->max_entities);
    if (!world->entities) {
        printf("❌ Failed to allocate entities array\n");
        return false;
    }
    
    memset(world->entities, 0, sizeof(struct Entity) * world->max_entities);
    world->next_entity_id = 1;  // Start at 1, 0 = INVALID_ENTITY

    printf("🌍 World initialized - ready for %d entities\n", world->max_entities);
    return true;
}

void world_destroy(struct World* world)
{
    if (!world) return;

    printf("🌍 World destroyed - processed %d entities over %d frames\n", world->entity_count,
           world->frame_number);
    
    // Free entity array
    if (world->entities) {
        free(world->entities);
        world->entities = NULL;
    }
    
    world->entity_count = 0;
    world->max_entities = 0;
}

void world_clear(struct World* world)
{
    if (!world) return;
    
    printf("🌍 Clearing world - removing %d entities\n", world->entity_count);
    
    // Clear all entities
    world->entity_count = 0;
    world->next_entity_id = 1; // Reset ID counter
    world->active_camera_entity = INVALID_ENTITY; // Reset active camera
    
    // Clear entity array
    if (world->entities) {
        memset(world->entities, 0, sizeof(struct Entity) * world->max_entities);
    }
    
    printf("🌍 World cleared\n");
}

void world_update(struct World* world, float delta_time)
{
    if (!world) return;

    world->frame_number++;
    world->delta_time = delta_time;
    world->total_time += delta_time;

    // Systems will be called from here in order
    // For now, just update frame info
}

// ============================================================================
// ENTITY MANAGEMENT
// ============================================================================

EntityID entity_create(struct World* world)
{
    if (!world || !world->entities || world->entity_count >= world->max_entities)
    {
        return INVALID_ENTITY;
    }

    EntityID id = world->next_entity_id++;
    struct Entity* entity = &world->entities[world->entity_count++];

    entity->id = id;
    entity->component_mask = 0;

    // Clear component pointers
    entity->transform = NULL;
    entity->physics = NULL;
    entity->collision = NULL;
    entity->ai = NULL;
    entity->renderable = NULL;
    entity->player = NULL;

    return id;
}

bool entity_destroy(struct World* world, EntityID entity_id)
{
    if (!world || entity_id == INVALID_ENTITY) return false;

    // Find entity
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        if (world->entities[i].id == entity_id)
        {
            // Remove all components
            struct Entity* entity = &world->entities[i];

            if (entity->component_mask & COMPONENT_TRANSFORM)
            {
                entity_remove_component(world, entity_id, COMPONENT_TRANSFORM);
            }
            if (entity->component_mask & COMPONENT_PHYSICS)
            {
                entity_remove_component(world, entity_id, COMPONENT_PHYSICS);
            }
            if (entity->component_mask & COMPONENT_COLLISION)
            {
                entity_remove_component(world, entity_id, COMPONENT_COLLISION);
            }
            if (entity->component_mask & COMPONENT_AI)
            {
                entity_remove_component(world, entity_id, COMPONENT_AI);
            }
            if (entity->component_mask & COMPONENT_RENDERABLE)
            {
                entity_remove_component(world, entity_id, COMPONENT_RENDERABLE);
            }
            if (entity->component_mask & COMPONENT_PLAYER)
            {
                entity_remove_component(world, entity_id, COMPONENT_PLAYER);
            }
            if (entity->component_mask & COMPONENT_CAMERA)
            {
                entity_remove_component(world, entity_id, COMPONENT_CAMERA);
            }

            // Swap with last entity to avoid gaps
            if (i < world->entity_count - 1)
            {
                world->entities[i] = world->entities[world->entity_count - 1];
            }
            world->entity_count--;
            return true;
        }
    }
    return false;
}

struct Entity* entity_get(struct World* world, EntityID entity_id)
{
    if (!world || entity_id == INVALID_ENTITY) return NULL;

    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        if (world->entities[i].id == entity_id)
        {
            return &world->entities[i];
        }
    }
    return NULL;
}

// ============================================================================
// COMPONENT MANAGEMENT
// ============================================================================

bool entity_add_component(struct World* world, EntityID entity_id, ComponentType type)
{
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity || (entity->component_mask & type))
    {
        return false;  // Entity not found or component already exists
    }

    entity->component_mask |= type;

    // Allocate component from appropriate pool
    switch (type)
    {
        case COMPONENT_TRANSFORM:
            if (world->components.transform_count >= MAX_ENTITIES) return false;
            entity->transform = &world->components.transforms[world->components.transform_count++];
            memset(entity->transform, 0, sizeof(struct Transform));
            entity->transform->scale = (Vector3){ 1.0f, 1.0f, 1.0f };
            break;

        case COMPONENT_PHYSICS:
            if (world->components.physics_count >= MAX_ENTITIES) return false;
            entity->physics = &world->components.physics[world->components.physics_count++];
            memset(entity->physics, 0, sizeof(struct Physics));
            
            // Initialize linear dynamics
            entity->physics->mass = 1.0f;
            entity->physics->drag_linear = 0.99f;
            
            // Initialize angular dynamics
            entity->physics->drag_angular = 0.95f;
            entity->physics->moment_of_inertia = (Vector3){ 1.0f, 1.0f, 1.0f };
            entity->physics->has_6dof = false;  // Disabled by default
            entity->physics->environment = PHYSICS_SPACE;
            break;

        case COMPONENT_COLLISION:
            if (world->components.collision_count >= MAX_ENTITIES) return false;
            entity->collision = &world->components.collisions[world->components.collision_count++];
            memset(entity->collision, 0, sizeof(struct Collision));
            entity->collision->shape = COLLISION_SPHERE;
            entity->collision->radius = 1.0f;
            entity->collision->layer_mask = 0xFFFFFFFF;  // Collide with everything by default
            break;

        case COMPONENT_AI:
            if (world->components.ai_count >= MAX_ENTITIES) return false;
            entity->ai = &world->components.ais[world->components.ai_count++];
            memset(entity->ai, 0, sizeof(struct AI));
            entity->ai->state = AI_STATE_IDLE;
            entity->ai->update_frequency = 2.0f;  // 2 Hz by default
            break;

        case COMPONENT_RENDERABLE:
            if (world->components.renderable_count >= MAX_ENTITIES) return false;
            entity->renderable =
                &world->components.renderables[world->components.renderable_count++];
            memset(entity->renderable, 0, sizeof(struct Renderable));
            entity->renderable->visible = true;
            break;

        case COMPONENT_PLAYER:
            if (world->components.player_count >= MAX_ENTITIES) return false;
            entity->player = &world->components.players[world->components.player_count++];
            memset(entity->player, 0, sizeof(struct Player));
            entity->player->afterburner_energy = 100.0f;
            entity->player->controls_enabled = true;
            break;

        case COMPONENT_CAMERA:
            if (world->components.camera_count >= MAX_ENTITIES) return false;
            entity->camera = &world->components.cameras[world->components.camera_count++];
            memset(entity->camera, 0, sizeof(struct Camera));
            entity->camera->fov = 60.0f;
            entity->camera->near_plane = 0.1f;
            entity->camera->far_plane = 1000.0f;
            entity->camera->aspect_ratio = 16.0f / 9.0f;
            entity->camera->is_active = false;
            entity->camera->behavior = CAMERA_BEHAVIOR_THIRD_PERSON;
            entity->camera->follow_target = INVALID_ENTITY;
            entity->camera->follow_distance = 10.0f;
            entity->camera->follow_offset = (Vector3){ 5.0f, 15.0f, 25.0f };
            entity->camera->follow_smoothing = 0.02f;
            break;

        case COMPONENT_SCENENODE:
            if (world->components.scene_node_count >= MAX_ENTITIES) return false;
            entity->scene_node = &world->components.scene_nodes[world->components.scene_node_count++];
            memset(entity->scene_node, 0, sizeof(struct SceneNode));
            entity->scene_node->entity_id = entity_id;
            entity->scene_node->parent = INVALID_ENTITY;
            entity->scene_node->num_children = 0;
            entity->scene_node->transform_dirty = true;
            entity->scene_node->is_visible = true;
            entity->scene_node->depth = 0;
            // Initialize transforms to identity
            mat4_identity(entity->scene_node->local_transform);
            mat4_identity(entity->scene_node->world_transform);
            break;

        case COMPONENT_THRUSTER_SYSTEM:
            if (world->components.thruster_system_count >= MAX_ENTITIES) return false;
            entity->thruster_system = &world->components.thruster_systems[world->components.thruster_system_count++];
            memset(entity->thruster_system, 0, sizeof(struct ThrusterSystem));
            // Initialize thruster capabilities
            entity->thruster_system->max_linear_force = (Vector3){ 100.0f, 100.0f, 100.0f };
            entity->thruster_system->max_angular_torque = (Vector3){ 50.0f, 50.0f, 50.0f };
            entity->thruster_system->thrust_response_time = 0.1f;
            entity->thruster_system->atmosphere_efficiency = 0.8f;
            entity->thruster_system->vacuum_efficiency = 1.0f;
            entity->thruster_system->thrusters_enabled = true;
            break;

        case COMPONENT_CONTROL_AUTHORITY:
            if (world->components.control_authority_count >= MAX_ENTITIES) return false;
            entity->control_authority = &world->components.control_authorities[world->components.control_authority_count++];
            memset(entity->control_authority, 0, sizeof(struct ControlAuthority));
            // Initialize control settings
            entity->control_authority->controlled_by = INVALID_ENTITY;
            entity->control_authority->control_sensitivity = 1.0f;
            entity->control_authority->stability_assist = 0.5f;
            entity->control_authority->flight_assist_enabled = true;
            entity->control_authority->control_mode = CONTROL_ASSISTED;
            break;

        default:
            entity->component_mask &= ~type;  // Remove flag
            return false;
    }

    return true;
}

bool entity_remove_component(struct World* world, EntityID entity_id, ComponentType type)
{
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity || !(entity->component_mask & type))
    {
        return false;  // Entity not found or component doesn't exist
    }

    entity->component_mask &= ~type;

    // Note: In a production system, we'd want to properly manage the pool
    // For now, just null the pointer (components stay allocated but unused)
    switch (type)
    {
        case COMPONENT_TRANSFORM:
            entity->transform = NULL;
            break;
        case COMPONENT_PHYSICS:
            entity->physics = NULL;
            break;
        case COMPONENT_COLLISION:
            entity->collision = NULL;
            break;
        case COMPONENT_AI:
            entity->ai = NULL;
            break;
        case COMPONENT_RENDERABLE:
            entity->renderable = NULL;
            break;
        case COMPONENT_PLAYER:
            entity->player = NULL;
            break;
        case COMPONENT_CAMERA:
            entity->camera = NULL;
            break;
        case COMPONENT_SCENENODE:
            // Remove from parent's children list if it has a parent
            if (entity->scene_node && entity->scene_node->parent != INVALID_ENTITY) {
                scene_node_remove_child(world, entity->scene_node->parent, entity_id);
            }
            entity->scene_node = NULL;
            break;
        case COMPONENT_THRUSTER_SYSTEM:
            entity->thruster_system = NULL;
            break;
        case COMPONENT_CONTROL_AUTHORITY:
            entity->control_authority = NULL;
            break;
    }
    return true;
}

bool entity_has_component(struct World* world, EntityID entity_id, ComponentType type)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity && (entity->component_mask & type);
}

// ============================================================================
// COMPONENT ACCESSORS
// ============================================================================

struct Transform* entity_get_transform(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->transform : NULL;
}

struct Physics* entity_get_physics(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->physics : NULL;
}

struct Collision* entity_get_collision(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->collision : NULL;
}

struct AI* entity_get_ai(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->ai : NULL;
}

struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->renderable : NULL;
}

struct Player* entity_get_player(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->player : NULL;
}

struct Camera* entity_get_camera(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->camera : NULL;
}

struct SceneNode* entity_get_scene_node(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->scene_node : NULL;
}

struct ThrusterSystem* entity_get_thruster_system(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->thruster_system : NULL;
}

struct ControlAuthority* entity_get_control_authority(struct World* world, EntityID entity_id)
{
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->control_authority : NULL;
}

// ============================================================================
// CAMERA MANAGEMENT
// ============================================================================

void world_set_active_camera(struct World* world, EntityID camera_entity)
{
    if (!world) return;

    // Deactivate current camera if any
    if (world->active_camera_entity != INVALID_ENTITY)
    {
        struct Camera* old_camera = entity_get_camera(world, world->active_camera_entity);
        if (old_camera)
        {
            old_camera->is_active = false;
        }
    }

    // Set new active camera
    world->active_camera_entity = camera_entity;

    // Activate new camera if valid
    if (camera_entity != INVALID_ENTITY)
    {
        struct Camera* new_camera = entity_get_camera(world, camera_entity);
        if (new_camera)
        {
            new_camera->is_active = true;
        }
    }
}

EntityID world_get_active_camera(struct World* world)
{
    return world ? world->active_camera_entity : INVALID_ENTITY;
}

// ============================================================================
// SCENE GRAPH MANAGEMENT
// ============================================================================

void scene_graph_update(struct World* world)
{
    if (!world) return;

    // First pass: Update all root nodes (nodes with no parent)
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (!(entity->component_mask & COMPONENT_SCENENODE)) continue;
        
        struct SceneNode* node = entity->scene_node;
        if (node->parent == INVALID_ENTITY)
        {
            // This is a root node - update its world transform and children
            float identity[16];
            mat4_identity(identity);
            scene_node_update_world_transform(world, entity->id, identity);
        }
    }
}

static void scene_node_update_world_transform_recursive(struct World* world, EntityID entity_id, const float* parent_transform)
{
    struct SceneNode* node = entity_get_scene_node(world, entity_id);
    if (!node) return;

    // Calculate world transform = parent_transform * local_transform
    mat4_multiply(node->world_transform, parent_transform, node->local_transform);
    node->transform_dirty = false;

    // Update all children
    for (uint32_t i = 0; i < node->num_children; i++)
    {
        scene_node_update_world_transform_recursive(world, node->children[i], node->world_transform);
    }
}

void scene_node_update_world_transform(struct World* world, EntityID entity_id, const float* parent_transform)
{
    scene_node_update_world_transform_recursive(world, entity_id, parent_transform);
}

bool scene_node_add_child(struct World* world, EntityID parent_id, EntityID child_id)
{
    struct SceneNode* parent = entity_get_scene_node(world, parent_id);
    struct SceneNode* child = entity_get_scene_node(world, child_id);
    
    if (!parent || !child) return false;
    if (parent->num_children >= MAX_SCENE_CHILDREN) return false;
    if (child->parent != INVALID_ENTITY) return false; // Child already has a parent
    
    // Add child to parent's children list
    parent->children[parent->num_children++] = child_id;
    
    // Set parent reference in child
    child->parent = parent_id;
    child->depth = parent->depth + 1;
    child->transform_dirty = true;
    
    return true;
}

bool scene_node_remove_child(struct World* world, EntityID parent_id, EntityID child_id)
{
    struct SceneNode* parent = entity_get_scene_node(world, parent_id);
    struct SceneNode* child = entity_get_scene_node(world, child_id);
    
    if (!parent || !child) return false;
    if (child->parent != parent_id) return false;
    
    // Find and remove child from parent's children list
    for (uint32_t i = 0; i < parent->num_children; i++)
    {
        if (parent->children[i] == child_id)
        {
            // Shift remaining children down
            for (uint32_t j = i; j < parent->num_children - 1; j++)
            {
                parent->children[j] = parent->children[j + 1];
            }
            parent->num_children--;
            break;
        }
    }
    
    // Clear parent reference in child
    child->parent = INVALID_ENTITY;
    child->depth = 0;
    child->transform_dirty = true;
    
    return true;
}

EntityID scene_node_find_by_name(struct World* world, const char* name)
{
    // TODO: Implement entity name system
    // For now, this is a placeholder that returns INVALID_ENTITY
    // This will be implemented when we add entity names/tags
    (void)world; // Suppress unused parameter warning
    (void)name;
    return INVALID_ENTITY;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

Vector3 vector3_add(Vector3 a, Vector3 b)
{
    return (Vector3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3 vector3_subtract(Vector3 a, Vector3 b)
{
    return (Vector3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

Vector3 vector3_multiply(Vector3 v, float scalar)
{
    return (Vector3){ v.x * scalar, v.y * scalar, v.z * scalar };
}

Vector3 vector3_normalize(Vector3 v)
{
    float len = vector3_length(v);
    if (len > 0.0f)
    {
        return (Vector3){ v.x / len, v.y / len, v.z / len };
    }
    return (Vector3){ 0.0f, 0.0f, 0.0f };
}

float vector3_length(Vector3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vector3_distance(Vector3 a, Vector3 b)
{
    Vector3 diff = { a.x - b.x, a.y - b.y, a.z - b.z };
    return vector3_length(diff);
}

// ============================================================================
// QUATERNION UTILITY FUNCTIONS
// ============================================================================

Vector3 quaternion_rotate_vector(Quaternion q, Vector3 v)
{
    // Using the formula: v' = q * v * q^-1
    // Optimized version: v' = v + 2 * q.xyz × (q.xyz × v + q.w * v)
    
    // First cross product: q.xyz × v
    Vector3 qv_cross = {
        q.y * v.z - q.z * v.y,
        q.z * v.x - q.x * v.z,
        q.x * v.y - q.y * v.x
    };
    
    // Scale v by q.w and add to cross product
    Vector3 temp = {
        qv_cross.x + q.w * v.x,
        qv_cross.y + q.w * v.y,
        qv_cross.z + q.w * v.z
    };
    
    // Second cross product: q.xyz × temp
    Vector3 qtemp_cross = {
        q.y * temp.z - q.z * temp.y,
        q.z * temp.x - q.x * temp.z,
        q.x * temp.y - q.y * temp.x
    };
    
    // Final result: v + 2 * qtemp_cross
    Vector3 result = {
        v.x + 2.0f * qtemp_cross.x,
        v.y + 2.0f * qtemp_cross.y,
        v.z + 2.0f * qtemp_cross.z
    };
    
    return result;
}

// ============================================================================
// MATRIX UTILITY FUNCTIONS
// ============================================================================

void mat4_identity(float* m)
{
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void mat4_perspective(float* m, float fov, float aspect, float near, float far)
{
    float f = 1.0f / tanf(fov * 0.5f * M_PI / 180.0f);
    memset(m, 0, 16 * sizeof(float));

    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}

void mat4_lookat(float* m, Vector3 eye, Vector3 target, Vector3 up)
{
    // Calculate camera basis vectors
    Vector3 f = vector3_normalize(vector3_subtract(target, eye));
    Vector3 s = vector3_normalize(
        (Vector3){ f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x });
    Vector3 u = (Vector3){ s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x };

    // Build view matrix
    mat4_identity(m);
    m[0] = s.x;
    m[4] = s.y;
    m[8] = s.z;
    m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    m[1] = u.x;
    m[5] = u.y;
    m[9] = u.z;
    m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    m[2] = -f.x;
    m[6] = -f.y;
    m[10] = -f.z;
    m[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
}

void mat4_multiply(float* result, const float* a, const float* b)
{
    float temp[16];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
            {
                temp[i * 4 + j] += a[k * 4 + j] * b[i * 4 + k];
            }
        }
    }
    memcpy(result, temp, sizeof(temp));
}

void mat4_translate(float* m, Vector3 t)
{
    mat4_identity(m);
    m[12] = t.x;
    m[13] = t.y;
    m[14] = t.z;
}

void mat4_rotation_x(float* m, float angle_radians)
{
    mat4_identity(m);
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    m[5] = c;
    m[6] = -s;
    m[9] = s;
    m[10] = c;
}

void mat4_rotation_y(float* m, float angle_radians)
{
    mat4_identity(m);
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    m[0] = c;
    m[2] = s;
    m[8] = -s;
    m[10] = c;
}

void mat4_rotation_z(float* m, float angle_radians)
{
    mat4_identity(m);
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    m[0] = c;
    m[1] = -s;
    m[4] = s;
    m[5] = c;
}

void mat4_scale(float* m, Vector3 scale)
{
    mat4_identity(m);
    m[0] = scale.x;
    m[5] = scale.y;
    m[10] = scale.z;
}

void mat4_compose_transform(float* result, Vector3 position, Quaternion rotation, Vector3 scale)
{
    // Proper quaternion to matrix conversion
    float translation[16], rot_matrix[16], scaling[16];
    float temp[16];

    // Create individual matrices
    mat4_translate(translation, position);
    mat4_from_quaternion(rot_matrix, rotation);  // Use proper quaternion conversion
    mat4_scale(scaling, scale);

    // Combine: T * R * S
    mat4_multiply(temp, rot_matrix, scaling);
    mat4_multiply(result, translation, temp);
}

void mat4_from_quaternion(float* m, Quaternion q)
{
    // Normalize quaternion to avoid scaling issues
    float length = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (length == 0.0f)
    {
        // Handle invalid quaternion - use identity
        mat4_identity(m);
        return;
    }

    float nx = q.x / length;
    float ny = q.y / length;
    float nz = q.z / length;
    float nw = q.w / length;

    // Calculate matrix components (column-major order)
    float xx = nx * nx;
    float yy = ny * ny;
    float zz = nz * nz;
    float xy = nx * ny;
    float xz = nx * nz;
    float yz = ny * nz;
    float wx = nw * nx;
    float wy = nw * ny;
    float wz = nw * nz;

    // Build 4x4 rotation matrix in column-major order
    m[0] = 1.0f - 2.0f * (yy + zz);
    m[1] = 2.0f * (xy + wz);
    m[2] = 2.0f * (xz - wy);
    m[3] = 0.0f;

    m[4] = 2.0f * (xy - wz);
    m[5] = 1.0f - 2.0f * (xx + zz);
    m[6] = 2.0f * (yz + wx);
    m[7] = 0.0f;

    m[8] = 2.0f * (xz + wy);
    m[9] = 2.0f * (yz - wx);
    m[10] = 1.0f - 2.0f * (xx + yy);
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

// ============================================================================
// CAMERA UTILITY FUNCTIONS
// ============================================================================

void camera_update_matrices(struct Camera* camera)
{
    if (!camera) return;

    // Calculate view matrix
    mat4_lookat(camera->view_matrix, camera->position, camera->target, camera->up);

    // Calculate projection matrix
    mat4_perspective(camera->projection_matrix, camera->fov, camera->aspect_ratio,
                     camera->near_plane, camera->far_plane);

    // Combine view and projection
    mat4_multiply(camera->view_projection_matrix, camera->projection_matrix, camera->view_matrix);

    // Mark matrices as clean
    camera->matrices_dirty = false;
}

bool switch_to_camera(struct World* world, int camera_index)
{
    if (!world || camera_index < 0 || camera_index >= 9) return false;

    // Find camera by index
    int current_index = 0;
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA)
        {
            if (current_index == camera_index)
            {
                // Found the camera, switch to it
                world_set_active_camera(world, entity->id);

                struct Camera* camera = entity_get_camera(world, entity->id);
                if (camera)
                {
                    camera->matrices_dirty = true;
                }
                return true;
            }
            current_index++;
        }
    }

    return false;
}

bool cycle_to_next_camera(struct World* world)
{
    if (!world) return false;
    
    // Count total cameras
    int camera_count = 0;
    int current_camera_index = -1;
    EntityID active_camera = world_get_active_camera(world);
    
    // First pass: count cameras and find current active camera index
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA)
        {
            if (entity->id == active_camera)
            {
                current_camera_index = camera_count;
            }
            camera_count++;
        }
    }
    
    if (camera_count == 0) return false;
    
    // Calculate next camera index (wrap around)
    int next_camera_index = (current_camera_index + 1) % camera_count;
    
    // Switch to next camera
    return switch_to_camera(world, next_camera_index);
}

void update_camera_aspect_ratio(struct World* world, float aspect_ratio)
{
    if (!world) return;

    EntityID active_camera = world_get_active_camera(world);
    if (active_camera != INVALID_ENTITY)
    {
        struct Camera* camera = entity_get_camera(world, active_camera);
        if (camera)
        {
            camera->aspect_ratio = aspect_ratio;
            camera->matrices_dirty = true;
        }
    }
}

void camera_extract_frustum_planes(const struct Camera* camera, float frustum_planes[6][4])
{
    if (!camera) return;

    const float* vp = camera->view_projection_matrix;

    // Left plane
    frustum_planes[0][0] = vp[3] + vp[0];
    frustum_planes[0][1] = vp[7] + vp[4];
    frustum_planes[0][2] = vp[11] + vp[8];
    frustum_planes[0][3] = vp[15] + vp[12];

    // Right plane
    frustum_planes[1][0] = vp[3] - vp[0];
    frustum_planes[1][1] = vp[7] - vp[4];
    frustum_planes[1][2] = vp[11] - vp[8];
    frustum_planes[1][3] = vp[15] - vp[12];

    // Bottom plane
    frustum_planes[2][0] = vp[3] + vp[1];
    frustum_planes[2][1] = vp[7] + vp[5];
    frustum_planes[2][2] = vp[11] + vp[9];
    frustum_planes[2][3] = vp[15] + vp[13];

    // Top plane
    frustum_planes[3][0] = vp[3] - vp[1];
    frustum_planes[3][1] = vp[7] - vp[5];
    frustum_planes[3][2] = vp[11] - vp[9];
    frustum_planes[3][3] = vp[15] - vp[13];

    // Near plane
    frustum_planes[4][0] = vp[3] + vp[2];
    frustum_planes[4][1] = vp[7] + vp[6];
    frustum_planes[4][2] = vp[11] + vp[10];
    frustum_planes[4][3] = vp[15] + vp[14];

    // Far plane
    frustum_planes[5][0] = vp[3] - vp[2];
    frustum_planes[5][1] = vp[7] - vp[6];
    frustum_planes[5][2] = vp[11] - vp[10];
    frustum_planes[5][3] = vp[15] - vp[14];

    // Normalize the planes
    for (int i = 0; i < 6; i++)
    {
        float mag = sqrtf(frustum_planes[i][0] * frustum_planes[i][0] +
                          frustum_planes[i][1] * frustum_planes[i][1] +
                          frustum_planes[i][2] * frustum_planes[i][2]);
        if (mag > 0.0f)
        {
            frustum_planes[i][0] /= mag;
            frustum_planes[i][1] /= mag;
            frustum_planes[i][2] /= mag;
            frustum_planes[i][3] /= mag;
        }
    }
}

// ============================================================================
// TDD FUNCTIONS - Sprint 19 Test-Driven Development
// ============================================================================

bool entity_add_components(struct World* world, EntityID entity_id, ComponentType components)
{
    if (!world || entity_id == INVALID_ENTITY_ID) return false;
    
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return false;
    
    // Define mask of all valid components
    const ComponentType VALID_COMPONENTS = 
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_COLLISION | 
        COMPONENT_AI | COMPONENT_RENDERABLE | COMPONENT_PLAYER | 
        COMPONENT_CAMERA | COMPONENT_SCENENODE;
    
    // Check if components contains any invalid bits
    if (components & ~VALID_COMPONENTS) {
        return false; // Invalid component type detected
    }
    
    // Add each component individually
    bool all_success = true;
    
    if (components & COMPONENT_TRANSFORM && !(entity->component_mask & COMPONENT_TRANSFORM)) {
        if (!entity_add_component(world, entity_id, COMPONENT_TRANSFORM)) {
            all_success = false;
        }
    }
    
    if (components & COMPONENT_PHYSICS && !(entity->component_mask & COMPONENT_PHYSICS)) {
        if (!entity_add_component(world, entity_id, COMPONENT_PHYSICS)) {
            all_success = false;
        }
    }
    
    if (components & COMPONENT_COLLISION && !(entity->component_mask & COMPONENT_COLLISION)) {
        if (!entity_add_component(world, entity_id, COMPONENT_COLLISION)) {
            all_success = false;
        }
    }
    
    if (components & COMPONENT_AI && !(entity->component_mask & COMPONENT_AI)) {
        if (!entity_add_component(world, entity_id, COMPONENT_AI)) {
            all_success = false;
        }
    }
    
    if (components & COMPONENT_RENDERABLE && !(entity->component_mask & COMPONENT_RENDERABLE)) {
        if (!entity_add_component(world, entity_id, COMPONENT_RENDERABLE)) {
            all_success = false;
        }
    }
    
    if (components & COMPONENT_PLAYER && !(entity->component_mask & COMPONENT_PLAYER)) {
        if (!entity_add_component(world, entity_id, COMPONENT_PLAYER)) {
            all_success = false;
        }
    }
    
    if (components & COMPONENT_CAMERA && !(entity->component_mask & COMPONENT_CAMERA)) {
        if (!entity_add_component(world, entity_id, COMPONENT_CAMERA)) {
            all_success = false;
        }
    }
    
    return all_success;
}

bool entity_is_valid(struct World* world, EntityID entity_id)
{
    if (!world || entity_id == INVALID_ENTITY_ID) return false;
    
    // Check if entity exists in the world
    for (uint32_t i = 0; i < world->entity_count; i++) {
        if (world->entities[i].id == entity_id) {
            return true;
        }
    }
    
    return false;
}
