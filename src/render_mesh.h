// TASK 2.1: Restored and modernized mesh renderer header
#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include "render.h"
#include "core.h"

#include "graphics_api.h"

// Mesh renderer structure
typedef struct {
    sg_pipeline pipeline;
    sg_shader shader;
    sg_sampler sampler;
} MeshRenderer;

// Mesh renderer functions
bool mesh_renderer_init(MeshRenderer* renderer);
void mesh_renderer_render_entity(MeshRenderer* renderer, struct Entity* entity, 
                                 struct Transform* transform, struct Renderable* renderable,
                                 float* view_projection_matrix);
void mesh_renderer_cleanup(MeshRenderer* renderer);

#endif // RENDER_MESH_H
