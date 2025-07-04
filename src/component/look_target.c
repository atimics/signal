#include "look_target.h"
#include <math.h>

// Define M_PI if not available (WASM builds)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void look_target_init(LookTarget* look_target) {
    if (!look_target) return;
    
    look_target->world_position = (Vector3){0, 0, -LOOK_TARGET_DEFAULT_DISTANCE};
    look_target->distance = LOOK_TARGET_DEFAULT_DISTANCE;
    look_target->azimuth = 0.0f;
    look_target->elevation = 0.0f;
    look_target->smoothing = LOOK_TARGET_SMOOTHING;
    look_target->is_initialized = true;
}

void look_target_update(LookTarget* look_target, const Vector3* entity_position,
                       float delta_azimuth, float delta_elevation, float delta_distance) {
    if (!look_target || !entity_position) return;
    
    if (!look_target->is_initialized) {
        look_target_init(look_target);
    }
    
    // Update spherical coordinates
    look_target->azimuth += delta_azimuth;
    look_target->elevation += delta_elevation;
    look_target->distance += delta_distance;
    
    // Wrap azimuth to [-PI, PI]
    while (look_target->azimuth > M_PI) look_target->azimuth -= 2.0f * M_PI;
    while (look_target->azimuth < -M_PI) look_target->azimuth += 2.0f * M_PI;
    
    // Clamp values
    look_target_clamp_values(look_target);
    
    // Update world position
    look_target_update_world_position(look_target, entity_position);
}

Vector3 look_target_get_direction(const LookTarget* look_target, const Vector3* entity_position) {
    if (!look_target || !entity_position) {
        return (Vector3){0, 0, -1}; // Default forward
    }
    
    Vector3 direction = vector3_subtract(look_target->world_position, *entity_position);
    return vector3_normalize(direction);
}

void look_target_update_world_position(LookTarget* look_target, const Vector3* entity_position) {
    if (!look_target || !entity_position) return;
    
    // Convert spherical to Cartesian coordinates
    float cos_elevation = cosf(look_target->elevation);
    float sin_elevation = sinf(look_target->elevation);
    float cos_azimuth = cosf(look_target->azimuth);
    float sin_azimuth = sinf(look_target->azimuth);
    
    // Calculate offset from entity
    Vector3 offset = {
        look_target->distance * cos_elevation * sin_azimuth,
        look_target->distance * sin_elevation,
        -look_target->distance * cos_elevation * cos_azimuth
    };
    
    // Apply smoothing to world position update
    Vector3 target_position = vector3_add(*entity_position, offset);
    
    if (look_target->smoothing > 0.0f) {
        look_target->world_position = vector3_lerp(
            look_target->world_position,
            target_position,
            1.0f - look_target->smoothing
        );
    } else {
        look_target->world_position = target_position;
    }
}

void look_target_clamp_values(LookTarget* look_target) {
    if (!look_target) return;
    
    // Clamp elevation to prevent gimbal lock
    if (look_target->elevation < LOOK_TARGET_ELEVATION_MIN) {
        look_target->elevation = LOOK_TARGET_ELEVATION_MIN;
    }
    if (look_target->elevation > LOOK_TARGET_ELEVATION_MAX) {
        look_target->elevation = LOOK_TARGET_ELEVATION_MAX;
    }
    
    // Clamp distance
    if (look_target->distance < LOOK_TARGET_MIN_DISTANCE) {
        look_target->distance = LOOK_TARGET_MIN_DISTANCE;
    }
    if (look_target->distance > LOOK_TARGET_MAX_DISTANCE) {
        look_target->distance = LOOK_TARGET_MAX_DISTANCE;
    }
    
    // Clamp smoothing
    if (look_target->smoothing < 0.0f) look_target->smoothing = 0.0f;
    if (look_target->smoothing > 0.99f) look_target->smoothing = 0.99f;
}