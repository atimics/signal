#ifndef RENDER_THRUST_CONES_H
#define RENDER_THRUST_CONES_H

#include "thruster_points.h"
#include "core.h"
#include "render.h"

// Forward declarations
typedef struct RenderContext RenderContext;

// Generate cone mesh for thrust visualization
Mesh* generate_thrust_cone_mesh(float radius, float length, int segments);

// Render thrust cones for a thruster system
void render_thrust_cones(const ThrusterPointSystem* system, const struct Transform* transform,
                        RenderContext* ctx, const float* view_matrix, const float* proj_matrix);

// Debug visualization showing thrust vectors
void debug_draw_thrust_vectors(const ThrusterPointSystem* system, const struct Transform* transform);

#endif // RENDER_THRUST_CONES_H