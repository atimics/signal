/**
 * @file render_thrust_cones.c
 * @brief Simple cone visualization for thruster exhaust
 */

#include "render_thrust_cones.h"
#include "graphics_api.h"
#include "gpu_resources.h"
#include "assets.h"
#include <math.h>
#include <stdlib.h>

// Generate cone mesh for thrust visualization
Mesh* generate_thrust_cone_mesh(float radius, float length, int segments) {
    int vertex_count = segments + 2; // cone vertices + tip + base center
    int index_count = segments * 6;   // triangles for sides and base
    
    Mesh* mesh = calloc(1, sizeof(Mesh));
    mesh->vertices = calloc(vertex_count, sizeof(Vertex));
    mesh->indices = calloc(index_count, sizeof(uint32_t));
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;
    
    // Tip vertex
    mesh->vertices[0].position = (Vector3){0, 0, 0};
    mesh->vertices[0].normal = (Vector3){0, 0, 1};
    mesh->vertices[0].tex_coord = (Vector2){0.5f, 0};
    
    // Base center
    mesh->vertices[1].position = (Vector3){0, 0, -length};
    mesh->vertices[1].normal = (Vector3){0, 0, -1};
    mesh->vertices[1].tex_coord = (Vector2){0.5f, 1};
    
    // Generate base vertices
    for (int i = 0; i < segments; i++) {
        float angle = (2.0f * M_PI * i) / segments;
        float x = radius * cosf(angle);
        float y = radius * sinf(angle);
        
        Vertex* v = &mesh->vertices[i + 2];
        v->position = (Vector3){x, y, -length};
        
        // Normal points outward from cone axis
        float len = sqrtf(x*x + y*y);
        v->normal = (Vector3){x/len, y/len, 0.5f};
        
        v->tex_coord = (Vector2){(cosf(angle) + 1) * 0.5f, 1};
    }
    
    // Generate indices
    int idx = 0;
    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        
        // Side triangle (tip to base edge)
        mesh->indices[idx++] = 0;
        mesh->indices[idx++] = i + 2;
        mesh->indices[idx++] = next + 2;
        
        // Base triangle (base center to edge)
        mesh->indices[idx++] = 1;
        mesh->indices[idx++] = next + 2;
        mesh->indices[idx++] = i + 2;
    }
    
    return mesh;
}

// Render thrust cones for a thruster system
void render_thrust_cones(const ThrusterPointSystem* system, const struct Transform* transform,
                        RenderContext* ctx, const float* view_matrix, const float* proj_matrix) {
    (void)view_matrix;  // TODO: Use when integrated with rendering
    (void)proj_matrix;  // TODO: Use when integrated with rendering
    if (!system || !transform || !ctx) return;
    
    // Get or create cone mesh
    static Mesh* cone_mesh = NULL;
    if (!cone_mesh) {
        cone_mesh = generate_thrust_cone_mesh(0.2f, 1.0f, 8);
        // TODO: Create GPU resources when integrated with rendering pipeline
    }
    
    // Render each active thruster
    for (int i = 0; i < system->thrusters.count; i++) {
        const ThrusterPoint* thruster = &system->thrusters.points[i];
        
        if (thruster->current_thrust < 0.01f) continue;
        
        // Calculate cone transform
        Vector3 world_pos = quaternion_rotate_vector(transform->rotation, thruster->position);
        world_pos = vector3_add(world_pos, transform->position);
        
        // Create rotation to align cone with thrust direction
        Vector3 world_dir = quaternion_rotate_vector(transform->rotation, thruster->direction);
        
        // Calculate rotation from default cone direction (0,0,-1) to thrust direction
        Vector3 default_dir = {0, 0, -1};
        Vector3 axis = vector3_cross(default_dir, world_dir);
        float dot = vector3_dot(default_dir, world_dir);
        
        Quaternion cone_rotation;
        if (vector3_length(axis) < 0.001f) {
            // Vectors are parallel
            if (dot > 0) {
                cone_rotation = (Quaternion){0, 0, 0, 1}; // Identity
            } else {
                // 180 degree rotation around any perpendicular axis
                cone_rotation = (Quaternion){1, 0, 0, 0};
            }
        } else {
            axis = vector3_normalize(axis);
            float angle = acosf(fmaxf(-1.0f, fminf(1.0f, dot)));
            
            // Create quaternion from axis and angle
            float half_angle = angle * 0.5f;
            float s = sinf(half_angle);
            cone_rotation.x = axis.x * s;
            cone_rotation.y = axis.y * s;
            cone_rotation.z = axis.z * s;
            cone_rotation.w = cosf(half_angle);
        }
        
        // Scale based on thrust level
        float scale = thruster->current_thrust;
        Vector3 cone_scale = {scale * 0.5f, scale * 0.5f, scale * 2.0f};
        
        // Create transform matrix
        float model[16];
        mat4_identity(model);
        mat4_translate(model, world_pos);
        
        // Apply rotation
        float rotation_matrix[16];
        mat4_from_quaternion(rotation_matrix, cone_rotation);
        mat4_multiply(model, model, rotation_matrix);
        
        // Apply scale
        mat4_scale(model, cone_scale);
        
        // TODO: Render cone using actual graphics API
        // For now, we've prepared all the data:
        // - model: transform matrix  
        // - thrust color would be: (1.0 - 0.5*thrust, 0.5 + 0.5*thrust, thrust, 0.8)
        // - cone_mesh: the mesh geometry
        
        // This would be integrated with the actual rendering pipeline
        (void)model;  // Suppress unused warning
    }
}

// Debug visualization showing thrust vectors
void debug_draw_thrust_vectors(const ThrusterPointSystem* system, const struct Transform* transform) {
    if (!system || !transform) return;
    
    for (int i = 0; i < system->thrusters.count; i++) {
        const ThrusterPoint* thruster = &system->thrusters.points[i];
        
        // Transform position to world space
        Vector3 world_pos = quaternion_rotate_vector(transform->rotation, thruster->position);
        world_pos = vector3_add(world_pos, transform->position);
        
        // Transform direction to world space
        Vector3 world_dir = quaternion_rotate_vector(transform->rotation, thruster->direction);
        (void)world_dir;  // TODO: Use for debug visualization
        
        // Scale by thrust amount and max force
        float force_scale = thruster->current_thrust * thruster->max_force * 0.0001f;
        
        // TODO: Debug visualization would draw:
        // - Line from world_pos along world_dir scaled by force_scale
        // - Color: (1.0, 1.0 - thrust, 0.0, 1.0) - yellow to red based on thrust
        // - Sphere at world_pos with radius 0.1
        
        (void)force_scale;  // Suppress unused warning
    }
}