#include "render_camera.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// 3D PROJECTION
// ============================================================================

Point2D project_3d_to_2d(Vector3 world_pos, const Camera3D* camera, int screen_width, int screen_height) {
    // Transform world position to camera space
    Vector3 relative_pos = {
        world_pos.x - camera->position.x,
        world_pos.y - camera->position.y,
        world_pos.z - camera->position.z
    };
    
    // Calculate camera coordinate system
    Vector3 forward = {
        camera->target.x - camera->position.x,
        camera->target.y - camera->position.y,
        camera->target.z - camera->position.z
    };
    
    // Normalize forward vector
    float forward_len = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (forward_len > 0.001f) {
        forward.x /= forward_len;
        forward.y /= forward_len;
        forward.z /= forward_len;
    } else {
        forward = (Vector3){0, 0, -1};  // Default forward
    }
    
    // Calculate right vector (cross product of forward and up)
    Vector3 right = {
        forward.y * camera->up.z - forward.z * camera->up.y,
        forward.z * camera->up.x - forward.x * camera->up.z,
        forward.x * camera->up.y - forward.y * camera->up.x
    };
    
    // Calculate actual up vector (cross product of right and forward)
    Vector3 up = {
        right.y * forward.z - right.z * forward.y,
        right.z * forward.x - right.x * forward.z,
        right.x * forward.y - right.y * forward.x
    };
    
    // Transform to camera space using dot products
    float cam_x = relative_pos.x * right.x + relative_pos.y * right.y + relative_pos.z * right.z;
    float cam_y = relative_pos.x * up.x + relative_pos.y * up.y + relative_pos.z * up.z;
    float cam_z = relative_pos.x * forward.x + relative_pos.y * forward.y + relative_pos.z * forward.z;
    
    // Perspective projection
    if (cam_z <= 0.1f) cam_z = 0.1f;  // Prevent division by zero and behind camera
    
    // Use proper FOV calculation
    float fov_rad = camera->fov * M_PI / 180.0f;
    float focal_length = (screen_height / 2.0f) / tanf(fov_rad / 2.0f);
    
    float screen_x = (cam_x * focal_length / cam_z) + screen_width / 2.0f;
    float screen_y = screen_height / 2.0f - (cam_y * focal_length / cam_z);  // Flip Y
    
    Point2D result = {
        (int)screen_x,
        (int)screen_y
    };
    
    return result;
}

// ============================================================================
// CAMERA CONTROLS
// ============================================================================

void camera_set_position(Camera3D* camera, Vector3 position) {
    if (!camera) return;
    camera->position = position;
}

void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up) {
    if (!camera) return;
    camera->position = eye;
    camera->target = target;
    camera->up = up;
}

void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance) {
    if (!camera || !world || entity_id == INVALID_ENTITY) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    if (!transform) return;
    
    // THIRD-PERSON CHASE CAMERA: Position camera behind and above the target
    Vector3 target_pos = transform->position;
    
    // Camera offset: behind (positive Z), above (positive Y), and slightly to the side
    Vector3 camera_offset = {5.0f, 15.0f, 25.0f};
    
    Vector3 desired_camera_pos = {
        target_pos.x + camera_offset.x,
        target_pos.y + camera_offset.y,
        target_pos.z + camera_offset.z
    };
    
    // Smooth camera movement (less aggressive than before)
    float lerp = 0.02f;  // Much slower, more stable
    camera->position.x += (desired_camera_pos.x - camera->position.x) * lerp;
    camera->position.y += (desired_camera_pos.y - camera->position.y) * lerp;
    camera->position.z += (desired_camera_pos.z - camera->position.z) * lerp;
    
    // ALWAYS LOOK AT THE TARGET ENTITY (this is key for stable camera)
    camera->target = target_pos;
    
    // Keep up vector stable
    camera->up = (Vector3){0, 1, 0};
}
