#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include "core.h"
#include "render.h"
#include "assets.h"

// ============================================================================
// MESH RENDERING API
// ============================================================================

// Transform utilities
Vector3 transform_vertex(Vector3 vertex, struct Transform* transform);

// Core mesh rendering
void render_mesh(Mesh* mesh, struct Transform* transform, Material* material, Texture* texture, RenderConfig* config);
void render_mesh_enhanced(Mesh* mesh, struct Transform* transform, Material* material, Texture* texture, RenderConfig* config);

// Entity rendering
void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config);
VisualType get_entity_visual_type(struct World* world, EntityID entity_id);

// Triangle rasterization
void render_filled_triangle(SDL_Renderer* renderer, Point2D p1, Point2D p2, Point2D p3, Uint8 r, Uint8 g, Uint8 b);
void render_textured_triangle(SDL_Renderer* renderer, Vector3 v1, Vector3 v2, Vector3 v3, 
                             Vector3 n1, Vector3 n2, Vector3 n3, SDL_Texture* texture,
                             Camera3D* camera, LightingSystem* lighting, int screen_width, int screen_height);

#endif // RENDER_MESH_H
