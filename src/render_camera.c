#include "render_camera.h"

#include <math.h>

#include "graphics_api.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// CAMERA CONTROLS
// ============================================================================

void camera_set_position(Camera3D* camera, Vector3 position)
{
    if (!camera) return;
    camera->position = position;
}

void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up)
{
    if (!camera) return;
    camera->position = eye;
    camera->target = target;
    camera->up = up;
}

void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance)
{
    if (!camera || !world || entity_id == INVALID_ENTITY) return;

    // Suppress unused parameter warning (distance not used in current implementation)
    (void)distance;

    struct Transform* transform = entity_get_transform(world, entity_id);
    if (!transform) return;

    // THIRD-PERSON CHASE CAMERA: Position camera behind and above the target
    Vector3 target_pos = transform->position;

    // Camera offset: behind (positive Z), above (positive Y), and slightly to the side
    Vector3 camera_offset = { 5.0f, 15.0f, 25.0f };

    Vector3 desired_camera_pos = { target_pos.x + camera_offset.x, target_pos.y + camera_offset.y,
                                   target_pos.z + camera_offset.z };

    // Smooth camera movement (less aggressive than before)
    float lerp = 0.02f;  // Much slower, more stable
    camera->position.x += (desired_camera_pos.x - camera->position.x) * lerp;
    camera->position.y += (desired_camera_pos.y - camera->position.y) * lerp;
    camera->position.z += (desired_camera_pos.z - camera->position.z) * lerp;

    // ALWAYS LOOK AT THE TARGET ENTITY (this is key for stable camera)
    camera->target = target_pos;

    // Keep up vector stable
    camera->up = (Vector3){ 0, 1, 0 };
}
