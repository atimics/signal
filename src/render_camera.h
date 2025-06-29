#ifndef RENDER_CAMERA_H
#define RENDER_CAMERA_H

#include "core.h"
#include "render.h"

// ============================================================================
// CAMERA SYSTEM API
// ============================================================================

// 3D projection
Point2D project_3d_to_2d(Vector3 world_pos, const Camera3D* camera, int screen_width, int screen_height);

// Camera controls
void camera_set_position(Camera3D* camera, Vector3 position);
void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up);
void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance);

#endif // RENDER_CAMERA_H
