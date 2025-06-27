#include "cgame.h"

// Math utility functions
float deg_to_rad(float degrees) {
    return degrees * M_PI / 180.0f;
}

Vector3D vector3d_add(Vector3D a, Vector3D b) {
    return (Vector3D){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3D vector3d_multiply(Vector3D v, float scalar) {
    return (Vector3D){v.x * scalar, v.y * scalar, v.z * scalar};
}

// 3D rotation functions
Vector3D rotate_x(Vector3D v, float angle) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    return (Vector3D){
        v.x,
        v.y * cos_a - v.z * sin_a,
        v.y * sin_a + v.z * cos_a
    };
}

Vector3D rotate_y(Vector3D v, float angle) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    return (Vector3D){
        v.x * cos_a + v.z * sin_a,
        v.y,
        -v.x * sin_a + v.z * cos_a
    };
}

Vector3D rotate_z(Vector3D v, float angle) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    return (Vector3D){
        v.x * cos_a - v.y * sin_a,
        v.x * sin_a + v.y * cos_a,
        v.z
    };
}

// Apply all rotations (order: X, Y, Z)
Vector3D apply_rotation(Vector3D v, Vector3D rotation) {
    Vector3D result = v;
    result = rotate_x(result, rotation.x);
    result = rotate_y(result, rotation.y);
    result = rotate_z(result, rotation.z);
    return result;
}
