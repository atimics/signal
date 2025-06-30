// tests/core_math.h - Minimal math definitions for testing
#ifndef CORE_MATH_H
#define CORE_MATH_H

// Vector3 structure - just the math part
typedef struct {
    float x, y, z;
} Vector3;

// Quaternion structure
typedef struct {
    float x, y, z, w;
} Quaternion;

// Function declarations - only the math functions we're testing
Vector3 vector3_add(Vector3 a, Vector3 b);
Vector3 vector3_subtract(Vector3 a, Vector3 b);
Vector3 vector3_multiply(Vector3 v, float scalar);
Vector3 vector3_normalize(Vector3 v);
float vector3_length(Vector3 v);
float vector3_distance(Vector3 a, Vector3 b);

#endif // CORE_MATH_H
