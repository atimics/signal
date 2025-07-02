#pragma once

#include "../core.h"

// Look target component for canyon racing control scheme
typedef struct LookTarget {
    Vector3 world_position;      // Where we're looking in world space
    float distance;              // Distance from entity (default: 50.0f)
    float azimuth;              // Horizontal angle in radians
    float elevation;            // Vertical angle in radians (-PI/3 to PI/3)
    float smoothing;            // Camera smoothing factor (0.0 = instant, 1.0 = very smooth)
    bool is_initialized;        // Whether look target has been set up
} LookTarget;

// Look target defaults
#define LOOK_TARGET_DEFAULT_DISTANCE 50.0f
#define LOOK_TARGET_MIN_DISTANCE 10.0f
#define LOOK_TARGET_MAX_DISTANCE 200.0f
#define LOOK_TARGET_SMOOTHING 0.85f
#define LOOK_TARGET_ELEVATION_MIN -1.047f  // -60 degrees
#define LOOK_TARGET_ELEVATION_MAX 1.047f   // +60 degrees

// Initialize a look target component
void look_target_init(LookTarget* look_target);

// Update look target based on input (mouse or gamepad)
void look_target_update(LookTarget* look_target, const Vector3* entity_position, 
                       float delta_azimuth, float delta_elevation, float delta_distance);

// Get the direction vector from entity to look target
Vector3 look_target_get_direction(const LookTarget* look_target, const Vector3* entity_position);

// Convert spherical coordinates to world position
void look_target_update_world_position(LookTarget* look_target, const Vector3* entity_position);

// Clamp look target values to valid ranges
void look_target_clamp_values(LookTarget* look_target);