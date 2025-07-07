#include "render_thrusters.h"
#include "render.h"
#include "render_mesh.h"
#include "core.h"
#include <math.h>
#include <stdio.h>

// Thruster attachment points for wedge_ship_mk2
static const ThrusterMount default_thruster_mounts[] = {
    // Main engines (rear)
    { .position = {-2.0f, 0.0f, -3.0f}, .direction = {0.0f, 0.0f, -1.0f}, .type = THRUSTER_MAIN, .size = 1.0f },
    { .position = { 2.0f, 0.0f, -3.0f}, .direction = {0.0f, 0.0f, -1.0f}, .type = THRUSTER_MAIN, .size = 1.0f },
    
    // RCS thrusters (maneuvering)
    // Front
    { .position = {-1.0f, 0.0f,  3.0f}, .direction = {0.0f, 0.0f,  1.0f}, .type = THRUSTER_RCS, .size = 0.3f },
    { .position = { 1.0f, 0.0f,  3.0f}, .direction = {0.0f, 0.0f,  1.0f}, .type = THRUSTER_RCS, .size = 0.3f },
    
    // Top/Bottom
    { .position = {0.0f,  1.5f, 0.0f}, .direction = {0.0f,  1.0f, 0.0f}, .type = THRUSTER_RCS, .size = 0.3f },
    { .position = {0.0f, -1.5f, 0.0f}, .direction = {0.0f, -1.0f, 0.0f}, .type = THRUSTER_RCS, .size = 0.3f },
    
    // Left/Right
    { .position = {-2.5f, 0.0f, 0.0f}, .direction = {-1.0f, 0.0f, 0.0f}, .type = THRUSTER_RCS, .size = 0.3f },
    { .position = { 2.5f, 0.0f, 0.0f}, .direction = { 1.0f, 0.0f, 0.0f}, .type = THRUSTER_RCS, .size = 0.3f },
};

static const int num_default_mounts = sizeof(default_thruster_mounts) / sizeof(default_thruster_mounts[0]);

// Simple cone geometry for thruster visualization
// TODO: Enable when implementing cone mesh rendering
/*
static void create_cone_geometry(Vector3* vertices, int* indices, int segments, float radius, float length) {
    // Tip of cone
    vertices[0] = (Vector3){0, 0, 0};
    
    // Base vertices
    for (int i = 0; i < segments; i++) {
        float angle = (2.0f * M_PI * i) / segments;
        vertices[i + 1] = (Vector3){
            radius * cosf(angle),
            radius * sinf(angle),
            -length
        };
    }
    
    // Triangles for cone surface
    int idx = 0;
    for (int i = 0; i < segments; i++) {
        indices[idx++] = 0;  // Tip
        indices[idx++] = i + 1;
        indices[idx++] = ((i + 1) % segments) + 1;
    }
}
*/

void render_thrusters(struct World* world, RenderConfig* render_config, EntityID entity_id) {
    if (!world || !render_config) return;
    
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return;
    
    // Check if entity has thruster system
    if (!(entity->component_mask & COMPONENT_THRUSTER_SYSTEM)) return;
    
    struct ThrusterSystem* thrusters = entity->thruster_system;
    struct Transform* transform = entity->transform;
    struct Physics* physics = entity->physics;
    
    if (!thrusters || !transform || !physics) return;
    
    // Only render if thrusters are enabled
    if (!thrusters->thrusters_enabled) return;
    
    // Get current thrust values
    Vector3 linear_thrust = thrusters->current_linear_thrust;
    Vector3 angular_thrust = thrusters->current_angular_thrust;
    
    // Render each thruster mount
    for (int i = 0; i < num_default_mounts; i++) {
        const ThrusterMount* mount = &default_thruster_mounts[i];
        
        // Calculate thrust intensity based on direction
        float intensity = 0.0f;
        Vector3 thrust_dir = mount->direction;
        
        // Check linear thrust contribution
        float linear_dot = vector3_dot(linear_thrust, thrust_dir);
        if (linear_dot > 0.0f) {
            intensity += linear_dot;
        }
        
        // Check angular thrust contribution (simplified)
        if (mount->type == THRUSTER_RCS) {
            // RCS thrusters fire for rotation
            float angular_contribution = fabsf(angular_thrust.x) + fabsf(angular_thrust.y) + fabsf(angular_thrust.z);
            intensity += angular_contribution * 0.3f;
        }
        
        // Skip if thruster not firing
        if (intensity < 0.01f) continue;
        
        // Calculate world position of thruster
        Vector3 world_pos = vector3_add(
            transform->position,
            quaternion_rotate_vector(transform->rotation, mount->position)
        );
        
        // Calculate world direction of thrust
        Vector3 world_dir = quaternion_rotate_vector(transform->rotation, thrust_dir);
        
        // Render thrust plume (simple line for now)
        float plume_length = mount->size * 3.0f * intensity;
        Vector3 plume_end = vector3_add(world_pos, vector3_multiply_scalar(world_dir, plume_length));
        (void)plume_end;  // TODO: Use for cone mesh rendering
        
        // TODO: Replace with actual cone mesh rendering
        // For now, we'll use debug visualization
        printf("🔥 Thruster %d: pos(%.1f,%.1f,%.1f) intensity=%.2f\n", 
               i, world_pos.x, world_pos.y, world_pos.z, intensity);
    }
}

// Get thruster mount configuration for a ship
const ThrusterMount* get_thruster_mounts(const char* ship_type, int* count) {
    (void)ship_type;  // TODO: Use ship_type for different configurations
    // For now, return default configuration
    // TODO: Load from ship-specific configuration
    *count = num_default_mounts;
    return default_thruster_mounts;
}

// Check if a specific thruster should be firing
bool is_thruster_firing(const ThrusterMount* mount, 
                       const Vector3* linear_thrust, 
                       const Vector3* angular_thrust) {
    if (!mount || !linear_thrust || !angular_thrust) return false;
    
    // Check linear thrust contribution
    float linear_dot = vector3_dot(*linear_thrust, mount->direction);
    if (linear_dot > 0.01f) return true;
    
    // Check angular thrust contribution
    if (mount->type == THRUSTER_RCS) {
        float angular_mag = vector3_length(*angular_thrust);
        if (angular_mag > 0.01f) return true;
    }
    
    return false;
}