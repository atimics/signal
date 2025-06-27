#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"
#include "spaceship.h"

// Camera structure
struct Camera {
    Vector3D position;
    Vector3D rotation;
    float fov;
    float near;
    float far;
};

// Camera functions
void init_camera(struct Camera* camera);
void update_camera(struct Camera* camera, struct Spaceship* player);

// 3D to 2D projection with rotation
Point2D project_3d_to_2d(Vector3D point, struct Camera* camera);

#endif

