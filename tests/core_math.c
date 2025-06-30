// tests/core_math.c - Standalone math functions for testing
#include "core_math.h"

#include <math.h>

Vector3 vector3_add(Vector3 a, Vector3 b)
{
    return (Vector3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3 vector3_subtract(Vector3 a, Vector3 b)
{
    return (Vector3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

Vector3 vector3_multiply(Vector3 v, float scalar)
{
    return (Vector3){ v.x * scalar, v.y * scalar, v.z * scalar };
}

Vector3 vector3_normalize(Vector3 v)
{
    float len = vector3_length(v);
    if (len > 0.0f)
    {
        return (Vector3){ v.x / len, v.y / len, v.z / len };
    }
    return (Vector3){ 0.0f, 0.0f, 0.0f };
}

float vector3_length(Vector3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vector3_distance(Vector3 a, Vector3 b)
{
    Vector3 diff = { a.x - b.x, a.y - b.y, a.z - b.z };
    return vector3_length(diff);
}
