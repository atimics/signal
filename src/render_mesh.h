// TASK 2.1: Restored and modernized mesh renderer header
#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include "render.h"
#include "core.h"

// Forward declaration for Sokol types
struct sg_pipeline;
struct sg_shader;
struct sg_sampler;

// Mesh renderer structure
typedef struct {
    struct sg_pipeline pipeline;
    struct sg_shader shader;
    struct sg_sampler sampler;
} MeshRenderer;

// Mesh renderer functions
bool mesh_renderer_init(MeshRenderer* renderer);
void mesh_renderer_render_entity(MeshRenderer* renderer, struct Entity* entity, 
                                 struct Transform* transform, struct Renderable* renderable,
                                 float* view_projection_matrix);
void mesh_renderer_cleanup(MeshRenderer* renderer);

#endif // RENDER_MESH_H
