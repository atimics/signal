#ifndef MATH3D_H
#define MATH3D_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 3D Vector
typedef struct {
    float x, y, z;
} Vector3D;

// 2D Point for screen projection
typedef struct {
    int x, y;
} Point2D;

// Math utility functions
float deg_to_rad(float degrees);
Vector3D vector3d_add(Vector3D a, Vector3D b);
Vector3D vector3d_multiply(Vector3D v, float scalar);

// 3D rotation functions
Vector3D rotate_x(Vector3D v, float angle);
Vector3D rotate_y(Vector3D v, float angle);
Vector3D rotate_z(Vector3D v, float angle);
Vector3D apply_rotation(Vector3D v, Vector3D rotation);

#endif
