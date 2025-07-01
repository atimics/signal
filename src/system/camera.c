#include "camera.h"

#include <stdio.h>

#include "core.h"
#include "render.h"

// Forward declarations for camera system helpers
static void camera_update_behavior(struct World* world, RenderConfig* render_config,
                                   EntityID camera_id, float delta_time);
static void update_legacy_render_config(RenderConfig* render_config, struct Camera* camera);

void camera_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    if (!world) return;

    // Set up camera follow targets if not already set
    static bool cameras_initialized = false;
    if (!cameras_initialized)
    {n        EntityID player_id = INVALID_ENTITY;

        // Find player entity
        for (uint32_t i = 0; i < world->entity_count; i++)
        {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_PLAYER)
            {
                player_id = entity->id;
                break;
            }
        }

        // Initialize cameras with default values
        for (uint32_t i = 0; i < world->entity_count; i++)
        {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA)
            {
                struct Camera* camera = entity->camera;
                if (camera)
                {
                    // Initialize cameras with default values
                    for (uint32_t i = 0; i < world->entity_count; i++)
                    {
                        struct Entity* entity = &world->entities[i];
                        if (entity->component_mask & COMPONENT_CAMERA)
                        {
                            struct Camera* camera = entity->camera;
                            if (camera)
                            {
                                // Set default camera properties if not already set
                                if (camera->fov == 0.0f)
                                    camera->fov = 60.0f;  // Wider FOV for better view
                                if (camera->near_plane == 0.0f) camera->near_plane = 0.1f;
                                if (camera->far_plane == 0.0f) camera->far_plane = 1000.0f;
                                if (camera->aspect_ratio == 0.0f)
                                    camera->aspect_ratio = 16.0f / 9.0f;

                                // Initialize position and target if not set - better default
                                // positions
                                if (camera->position.x == 0.0f && camera->position.y == 0.0f &&
                                    camera->position.z == 0.0f)
                                {
                                    // Different positions for different camera types
                                    switch (camera->behavior)
                                    {
                                        case CAMERA_BEHAVIOR_THIRD_PERSON:
                                        case CAMERA_BEHAVIOR_CHASE:
                                            camera->position =
                                                (Vector3){ 10.0f, 20.0f,
                                                           30.0f };  // Behind and above
                                            camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
                                            break;
                                        case CAMERA_BEHAVIOR_STATIC:
                                            camera->position =
                                                (Vector3){ -30.0f, 25.0f, -30.0f };  // Corner view
                                            camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
                                            break;
                                        default:
                                            camera->position =
                                                (Vector3){ 0.0f, 15.0f,
                                                           25.0f };  // General overhead
                                            camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
                                            break;
                                    }
                                }
                                camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };

                                // Set follow target for chase cameras with better default offsets
                                if (camera->follow_target == INVALID_ENTITY)
                                {
                                    if (camera->behavior == CAMERA_BEHAVIOR_THIRD_PERSON ||
                                        camera->behavior == CAMERA_BEHAVIOR_CHASE)
                                    {
                                        camera->follow_target = player_id;
                                        // Better chase camera positioning
                                        camera->follow_offset =
                                            (Vector3){ 8.0f, 20.0f,
                                                       30.0f };  // Further back and higher
                                        camera->follow_smoothing =
                                            0.05f;  // Slightly more responsive
                                    }
                                }

                                // Mark matrices as dirty for initial calculation
                                camera->matrices_dirty = true;

                                printf(
                                    "🎥 Initialized camera Entity %d: pos:(%.1f,%.1f,%.1f) "
                                    "behavior:%d\n",
                                    entity->id, camera->position.x, camera->position.y,
                                    camera->position.z, camera->behavior);
                            }
                        }
                    }
                }
            }
        }

        // Set the first available camera as active
        if (world->active_camera_entity == INVALID_ENTITY)
        {
            for (uint32_t i = 0; i < world->entity_count; i++)
            {
                struct Entity* entity = &world->entities[i];
                if (entity->component_mask & COMPONENT_CAMERA)
                {
                    world_set_active_camera(world, entity->id);
                    printf("🎯 Set initial active camera: Entity %d\n", entity->id);
                    break;
                }
            }
        }

        cameras_initialized = true;
    }

    // Find active camera entity
    EntityID active_camera_id = world_get_active_camera(world);
    if (active_camera_id == INVALID_ENTITY)
    {
        // No active camera, try to find any camera
        for (uint32_t i = 0; i < world->entity_count; i++)
        {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA)
            {
                world_set_active_camera(world, entity->id);
                active_camera_id = entity->id;
                break;
            }
        }
    }

    if (active_camera_id == INVALID_ENTITY)
    {
        return;  // No camera found
    }

    struct Camera* camera = entity_get_camera(world, active_camera_id);
    if (!camera) return;

    // Update camera position and target based on behavior
    camera_update_behavior(world, render_config, active_camera_id, delta_time);

    // Update matrices if camera changed
    if (camera->matrices_dirty)
    {
        camera_update_matrices(camera);
    }

    // Update legacy render config for backward compatibility
    update_legacy_render_config(render_config, camera);
}

static void camera_update_behavior(struct World* world, RenderConfig* render_config,
                                   EntityID camera_id, float delta_time)
{
    (void)render_config;  // Unused parameter
    struct Camera* camera = entity_get_camera(world, camera_id);
    if (!camera) return;

    bool position_changed = false;

    // Update camera based on its behavior
    switch (camera->behavior)
    {
        case CAMERA_BEHAVIOR_THIRD_PERSON:
        case CAMERA_BEHAVIOR_CHASE:
            if (camera->follow_target != INVALID_ENTITY)
            {
                struct Transform* target_transform =
                    entity_get_transform(world, camera->follow_target);
                if (target_transform)
                {
                    // Calculate desired camera position
                    Vector3 target_pos = target_transform->position;
                    Vector3 desired_pos = { target_pos.x + camera->follow_offset.x,
                                            target_pos.y + camera->follow_offset.y,
                                            target_pos.z + camera->follow_offset.z };

                    // Smooth camera movement - more responsive
                    float lerp =
                        camera->follow_smoothing * delta_time * 60.0f;  // Frame rate independent
                    if (lerp > 1.0f) lerp = 1.0f;  // Clamp to prevent overshooting

                    // Apply smoothing factor for stability but make it more responsive
                    lerp *= 3.0f;                  // Make camera more responsive
                    if (lerp > 0.3f) lerp = 0.3f;  // But not too fast

                    Vector3 old_pos = camera->position;
                    camera->position.x += (desired_pos.x - camera->position.x) * lerp;
                    camera->position.y += (desired_pos.y - camera->position.y) * lerp;
                    camera->position.z += (desired_pos.z - camera->position.z) * lerp;

                    // Always look at the target entity for proper framing
                    camera->target = target_pos;

                    // Check if position changed
                    if (vector3_distance(old_pos, camera->position) > 0.001f)
                    {
                        position_changed = true;
                    }
                }
            }
            break;

        case CAMERA_BEHAVIOR_STATIC:
            // Static cameras don't move
            break;

        case CAMERA_BEHAVIOR_FIRST_PERSON:
            if (camera->follow_target != INVALID_ENTITY)
            {
                struct Transform* target_transform =
                    entity_get_transform(world, camera->follow_target);
                if (target_transform)
                {
                    // First person camera follows exactly
                    Vector3 old_pos = camera->position;
                    camera->position = target_transform->position;

                    // Calculate forward direction from rotation (simplified)
                    Vector3 forward = { 0.0f, 0.0f, -1.0f };  // Default forward
                    camera->target = vector3_add(camera->position, forward);

                    if (vector3_distance(old_pos, camera->position) > 0.001f)
                    {
                        position_changed = true;
                    }
                }
            }
            break;

        case CAMERA_BEHAVIOR_ORBITAL:
            // TODO: Implement orbital camera behavior
            break;
    }

    if (position_changed)
    {
        camera->matrices_dirty = true;
    }
}

static void update_legacy_render_config(RenderConfig* render_config, struct Camera* camera)
{
    // Update legacy render config for backward compatibility
    if (render_config && camera)
    {
        render_config->camera.position = camera->position;
        render_config->camera.target = camera->target;
        render_config->camera.up = camera->up;
        render_config->camera.fov = camera->fov;
        render_config->camera.near_plane = camera->near_plane;
        render_config->camera.far_plane = camera->far_plane;
        render_config->camera.aspect_ratio = camera->aspect_ratio;
    }
}