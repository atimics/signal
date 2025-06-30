#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// WORLD MANAGEMENT
// ============================================================================

bool world_init(struct World* world) {
    if (!world) return false;
    
    memset(world, 0, sizeof(struct World));
    world->next_entity_id = 1;  // Start at 1, 0 = INVALID_ENTITY
    
    printf("🌍 World initialized - ready for %d entities\n", MAX_ENTITIES);
    return true;
}

void world_destroy(struct World* world) {
    if (!world) return;
    
    printf("🌍 World destroyed - processed %d entities over %d frames\n", 
           world->entity_count, world->frame_number);
}

void world_update(struct World* world, float delta_time) {
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

EntityID entity_create(struct World* world) {
    if (!world || world->entity_count >= MAX_ENTITIES) {
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

void entity_destroy(struct World* world, EntityID entity_id) {
    if (!world || entity_id == INVALID_ENTITY) return;
    
    // Find entity
    for (uint32_t i = 0; i < world->entity_count; i++) {
        if (world->entities[i].id == entity_id) {
            // Remove all components
            struct Entity* entity = &world->entities[i];
            
            if (entity->component_mask & COMPONENT_TRANSFORM) {
                entity_remove_component(world, entity_id, COMPONENT_TRANSFORM);
            }
            if (entity->component_mask & COMPONENT_PHYSICS) {
                entity_remove_component(world, entity_id, COMPONENT_PHYSICS);
            }
            if (entity->component_mask & COMPONENT_COLLISION) {
                entity_remove_component(world, entity_id, COMPONENT_COLLISION);
            }
            if (entity->component_mask & COMPONENT_AI) {
                entity_remove_component(world, entity_id, COMPONENT_AI);
            }
            if (entity->component_mask & COMPONENT_RENDERABLE) {
                entity_remove_component(world, entity_id, COMPONENT_RENDERABLE);
            }
            if (entity->component_mask & COMPONENT_PLAYER) {
                entity_remove_component(world, entity_id, COMPONENT_PLAYER);
            }
            if (entity->component_mask & COMPONENT_CAMERA) {
                entity_remove_component(world, entity_id, COMPONENT_CAMERA);
            }
            
            // Swap with last entity to avoid gaps
            if (i < world->entity_count - 1) {
                world->entities[i] = world->entities[world->entity_count - 1];
            }
            world->entity_count--;
            return;
        }
    }
}

struct Entity* entity_get(struct World* world, EntityID entity_id) {
    if (!world || entity_id == INVALID_ENTITY) return NULL;
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        if (world->entities[i].id == entity_id) {
            return &world->entities[i];
        }
    }
    return NULL;
}

// ============================================================================
// COMPONENT MANAGEMENT
// ============================================================================

bool entity_add_component(struct World* world, EntityID entity_id, ComponentType type) {
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity || (entity->component_mask & type)) {
        return false;  // Entity not found or component already exists
    }
    
    entity->component_mask |= type;
    
    // Allocate component from appropriate pool
    switch (type) {
        case COMPONENT_TRANSFORM:
            if (world->components.transform_count >= MAX_ENTITIES) return false;
            entity->transform = &world->components.transforms[world->components.transform_count++];
            memset(entity->transform, 0, sizeof(struct Transform));
            entity->transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
            break;
            
        case COMPONENT_PHYSICS:
            if (world->components.physics_count >= MAX_ENTITIES) return false;
            entity->physics = &world->components.physics[world->components.physics_count++];
            memset(entity->physics, 0, sizeof(struct Physics));
            entity->physics->mass = 1.0f;
            entity->physics->drag = 0.99f;
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
            entity->renderable = &world->components.renderables[world->components.renderable_count++];
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
            entity->camera->follow_offset = (Vector3){5.0f, 15.0f, 25.0f};
            entity->camera->follow_smoothing = 0.02f;
            break;
            
        default:
            entity->component_mask &= ~type;  // Remove flag
            return false;
    }
    
    return true;
}

void entity_remove_component(struct World* world, EntityID entity_id, ComponentType type) {
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity || !(entity->component_mask & type)) {
        return;  // Entity not found or component doesn't exist
    }
    
    entity->component_mask &= ~type;
    
    // Note: In a production system, we'd want to properly manage the pool
    // For now, just null the pointer (components stay allocated but unused)
    switch (type) {
        case COMPONENT_TRANSFORM:  entity->transform = NULL; break;
        case COMPONENT_PHYSICS:    entity->physics = NULL; break;
        case COMPONENT_COLLISION:  entity->collision = NULL; break;
        case COMPONENT_AI:         entity->ai = NULL; break;
        case COMPONENT_RENDERABLE: entity->renderable = NULL; break;
        case COMPONENT_PLAYER:     entity->player = NULL; break;
        case COMPONENT_CAMERA:     entity->camera = NULL; break;
    }
}

bool entity_has_component(struct World* world, EntityID entity_id, ComponentType type) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity && (entity->component_mask & type);
}

// ============================================================================
// COMPONENT ACCESSORS
// ============================================================================

struct Transform* entity_get_transform(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->transform : NULL;
}

struct Physics* entity_get_physics(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->physics : NULL;
}

struct Collision* entity_get_collision(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->collision : NULL;
}

struct AI* entity_get_ai(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->ai : NULL;
}

struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->renderable : NULL;
}

struct Player* entity_get_player(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->player : NULL;
}

struct Camera* entity_get_camera(struct World* world, EntityID entity_id) {
    struct Entity* entity = entity_get(world, entity_id);
    return entity ? entity->camera : NULL;
}

// ============================================================================
// CAMERA MANAGEMENT
// ============================================================================

void world_set_active_camera(struct World* world, EntityID camera_entity) {
    if (!world) return;
    
    // Deactivate current camera if any
    if (world->active_camera_entity != INVALID_ENTITY) {
        struct Camera* old_camera = entity_get_camera(world, world->active_camera_entity);
        if (old_camera) {
            old_camera->is_active = false;
        }
    }
    
    // Set new active camera
    world->active_camera_entity = camera_entity;
    
    // Activate new camera if valid
    if (camera_entity != INVALID_ENTITY) {
        struct Camera* new_camera = entity_get_camera(world, camera_entity);
        if (new_camera) {
            new_camera->is_active = true;
        }
    }
}

EntityID world_get_active_camera(struct World* world) {
    return world ? world->active_camera_entity : INVALID_ENTITY;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

Vector3 vector3_add(Vector3 a, Vector3 b) {
    return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 vector3_subtract(Vector3 a, Vector3 b) {
    return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 vector3_multiply(Vector3 v, float scalar) {
    return (Vector3){v.x * scalar, v.y * scalar, v.z * scalar};
}

Vector3 vector3_normalize(Vector3 v) {
    float len = vector3_length(v);
    if (len > 0.0f) {
        return (Vector3){v.x / len, v.y / len, v.z / len};
    }
    return (Vector3){0.0f, 0.0f, 0.0f};
}

float vector3_length(Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vector3_distance(Vector3 a, Vector3 b) {
    Vector3 diff = {a.x - b.x, a.y - b.y, a.z - b.z};
    return vector3_length(diff);
}

// ============================================================================
// MATRIX UTILITY FUNCTIONS
// ============================================================================

void mat4_identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov * 0.5f * M_PI / 180.0f);
    memset(m, 0, 16 * sizeof(float));
    
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}

void mat4_lookat(float* m, Vector3 eye, Vector3 target, Vector3 up) {
    // Calculate camera basis vectors
    Vector3 f = vector3_normalize(vector3_subtract(target, eye));
    Vector3 s = vector3_normalize((Vector3){
        f.y * up.z - f.z * up.y,
        f.z * up.x - f.x * up.z,
        f.x * up.y - f.y * up.x
    });
    Vector3 u = (Vector3){
        s.y * f.z - s.z * f.y,
        s.z * f.x - s.x * f.z,
        s.x * f.y - s.y * f.x
    };
    
    // Build view matrix
    mat4_identity(m);
    m[0] = s.x;  m[4] = s.y;  m[8] = s.z;   m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
    m[1] = u.x;  m[5] = u.y;  m[9] = u.z;   m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
    m[2] = -f.x; m[6] = -f.y; m[10] = -f.z; m[14] = (f.x*eye.x + f.y*eye.y + f.z*eye.z);
}

void mat4_multiply(float* result, const float* a, const float* b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i*4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result[i*4 + j] += a[i*4 + k] * b[k*4 + j];
            }
        }
    }
}

void mat4_translate(float* m, Vector3 t) {
    mat4_identity(m);
    m[12] = t.x;
    m[13] = t.y;
    m[14] = t.z;
}

// ============================================================================
// CAMERA UTILITY FUNCTIONS
// ============================================================================

void camera_update_matrices(struct Camera* camera) {
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

bool switch_to_camera(struct World* world, int camera_index) {
    if (!world || camera_index < 0 || camera_index >= 9) return false;
    
    // Find camera by index
    int current_index = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA) {
            if (current_index == camera_index) {
                // Found the camera, switch to it
                world_set_active_camera(world, entity->id);
                
                struct Camera* camera = entity_get_camera(world, entity->id);
                if (camera) {
                    camera->matrices_dirty = true;
                }
                return true;
            }
            current_index++;
        }
    }
    
    return false;
}

void update_camera_aspect_ratio(struct World* world, float aspect_ratio) {
    if (!world) return;
    
    EntityID active_camera = world_get_active_camera(world);
    if (active_camera != INVALID_ENTITY) {
        struct Camera* camera = entity_get_camera(world, active_camera);
        if (camera) {
            camera->aspect_ratio = aspect_ratio;
            camera->matrices_dirty = true;
        }
    }
}
