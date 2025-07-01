#include "camera.h"

#include <stdio.h>
#include <stdbool.h>

// Forward declarations
static void camera_initialize_from_transform(struct World* world, EntityID camera_entity);
static void camera_update_behavior(struct World* world, RenderConfig* render_config,
                                   EntityID camera_id, float delta_time);
static void update_legacy_render_config(RenderConfig* render_config, struct Camera* camera);

// Global initialization state
static bool g_cameras_initialized = false;

void camera_system_init(struct World* world, RenderConfig* render_config)
{
    (void)render_config; // Unused for now
    
    if (!world) return;
    
    printf("🎥 Initializing camera system...\n");
    
    // Initialize all camera entities
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA)
        {
            camera_initialize_from_transform(world, entity->id);
        }
    }
    
    // Activate the first available camera if none is active
    if (world_get_active_camera(world) == INVALID_ENTITY)
    {
        camera_switch_to_index(world, 0);
    }
    
    g_cameras_initialized = true;
    printf("🎥 Camera system initialized\n");
}

void camera_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    if (!world) return;

    // Initialize cameras if not done yet (fallback)
    if (!g_cameras_initialized)
    {
        camera_system_init(world, render_config);
    }

    EntityID active_camera_id = world_get_active_camera(world);
    if (active_camera_id == INVALID_ENTITY)
    {
        // Try to activate first available camera
        camera_switch_to_index(world, 0);
        active_camera_id = world_get_active_camera(world);
    }

    if (active_camera_id == INVALID_ENTITY)
    {
        return; // No cameras available
    }

    struct Camera* camera = entity_get_camera(world, active_camera_id);
    if (!camera) return;

    // Update camera behavior
    camera_update_behavior(world, render_config, active_camera_id, delta_time);

    // Update render config with camera data
    if (camera->matrices_dirty)
    {
        camera->matrices_dirty = false;
        update_legacy_render_config(render_config, camera);
    }
}

void camera_system_cleanup(void)
{
    g_cameras_initialized = false;
    printf("🎥 Camera system cleaned up\n");
}

bool camera_switch_to_index(struct World* world, int camera_index)
{
    if (!world) return false;
    
    int found_cameras = 0;
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA)
        {
            if (found_cameras == camera_index)
            {
                return camera_activate_entity(world, entity->id);
            }
            found_cameras++;
        }
    }
    return false;
}

bool camera_activate_entity(struct World* world, EntityID camera_entity)
{
    if (!world) return false;
    
    struct Camera* camera = entity_get_camera(world, camera_entity);
    if (!camera) return false;
    
    world_set_active_camera(world, camera_entity);
    camera->is_active = true;
    camera->matrices_dirty = true;
    
    printf("📹 Activated camera Entity %d: pos:(%.1f,%.1f,%.1f) behavior:%d\n",
           camera_entity, camera->position.x, camera->position.y, 
           camera->position.z, camera->behavior);
    
    return true;
}

// Private helper functions

static void camera_initialize_from_transform(struct World* world, EntityID camera_entity)
{
    struct Camera* camera = entity_get_camera(world, camera_entity);
    struct Transform* transform = entity_get_transform(world, camera_entity);
    
    if (!camera) return;
    
    // Set default camera properties if not already set
    if (camera->fov == 0.0f) camera->fov = 90.0f;  // Wide FOV for good view
    if (camera->near_plane == 0.0f) camera->near_plane = 0.1f;
    if (camera->far_plane == 0.0f) camera->far_plane = 1000.0f;
    if (camera->aspect_ratio == 0.0f) camera->aspect_ratio = 16.0f / 9.0f;

    // Initialize position from transform if available and camera pos is zero
    if (transform && (camera->position.x == 0.0f && camera->position.y == 0.0f && camera->position.z == 0.0f))
    {
        // Use transform position as camera position - this fixes the spawn position issue!
        camera->position = transform->position;
        camera->target = (Vector3){ 0.0f, 0.0f, 0.0f }; // Look at origin by default
        
        printf("🎥 Camera Entity %d: Using transform position (%.1f, %.1f, %.1f)\n",
               camera_entity, camera->position.x, camera->position.y, camera->position.z);
    }
    else if (camera->position.x == 0.0f && camera->position.y == 0.0f && camera->position.z == 0.0f)
    {
        // Fallback to default positions based on behavior
        switch (camera->behavior)
        {
            case CAMERA_BEHAVIOR_STATIC:
                camera->position = (Vector3){ 0.0f, 5.0f, 10.0f };  // Default static position
                camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
                break;
            case CAMERA_BEHAVIOR_THIRD_PERSON:
            case CAMERA_BEHAVIOR_CHASE:
                camera->position = (Vector3){ 10.0f, 20.0f, 30.0f };
                camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
                break;
            default:
                camera->position = (Vector3){ 0.0f, 15.0f, 25.0f };
                camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
                break;
        }
        
        printf("🎥 Camera Entity %d: Using default position (%.1f, %.1f, %.1f)\n",
               camera_entity, camera->position.x, camera->position.y, camera->position.z);
    }

    camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera->matrices_dirty = true;
    
    printf("🎥 Initialized camera Entity %d: pos:(%.1f,%.1f,%.1f) target:(%.1f,%.1f,%.1f) fov:%.1f\n",
           camera_entity, camera->position.x, camera->position.y, camera->position.z,
           camera->target.x, camera->target.y, camera->target.z, camera->fov);
}

static void camera_update_behavior(struct World* world, RenderConfig* render_config, 
                                   EntityID camera_id, float delta_time)
{
    (void)render_config; // Unused for now
    
    struct Camera* camera = entity_get_camera(world, camera_id);
    if (!camera) return;

    Vector3 old_pos = camera->position;

    switch (camera->behavior)
    {
        case CAMERA_BEHAVIOR_CHASE:
        case CAMERA_BEHAVIOR_THIRD_PERSON:
            if (camera->follow_target != INVALID_ENTITY)
            {
                struct Transform* target_transform = entity_get_transform(world, camera->follow_target);
                if (target_transform)
                {
                    Vector3 target_pos = target_transform->position;
                    Vector3 desired_pos = {
                        target_pos.x + camera->follow_offset.x,
                        target_pos.y + camera->follow_offset.y,
                        target_pos.z + camera->follow_offset.z
                    };

                    // Smooth camera movement
                    float lerp = camera->follow_smoothing * delta_time * 60.0f;
                    if (lerp > 1.0f) lerp = 1.0f;

                    // Make camera more responsive for better gameplay
                    lerp *= 3.0f;
                    if (lerp > 0.3f) lerp = 0.3f;

                    // Apply smooth interpolation
                    camera->position.x += (desired_pos.x - camera->position.x) * lerp;
                    camera->position.y += (desired_pos.y - camera->position.y) * lerp;
                    camera->position.z += (desired_pos.z - camera->position.z) * lerp;

                    // Update target
                    camera->target = target_pos;

                    // Mark as dirty if position changed significantly
                    if (vector3_distance(old_pos, camera->position) > 0.001f)
                    {
                        camera->matrices_dirty = true;
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
                struct Transform* target_transform = entity_get_transform(world, camera->follow_target);
                if (target_transform)
                {
                    camera->position = target_transform->position;
                    camera->matrices_dirty = true;
                }
            }
            break;

        default:
            break;
    }
}

static void update_legacy_render_config(RenderConfig* render_config, struct Camera* camera)
{
    if (!render_config || !camera) return;

    // Update the legacy render config camera
    render_config->camera.position = camera->position;
    render_config->camera.target = camera->target;
    render_config->camera.up = camera->up;
    render_config->camera.fov = camera->fov;
    render_config->camera.near_plane = camera->near_plane;
    render_config->camera.far_plane = camera->far_plane;
    render_config->camera.aspect_ratio = camera->aspect_ratio;
}
