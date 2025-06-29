#include "render_mesh.h"
#include "render_camera.h"
#include "render_lighting.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#ifndef fminf
#define fminf(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef fmaxf
#define fmaxf(a, b) ((a) > (b) ? (a) : (b))
#endif

// ============================================================================
// ENTITY CLASSIFICATION
// ============================================================================

VisualType get_entity_visual_type(struct World* world, EntityID entity_id) {
    if (!world || entity_id == INVALID_ENTITY) return VISUAL_TYPE_AI_SHIP;
    
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return VISUAL_TYPE_AI_SHIP;
    
    // Check component combinations to determine type
    if (entity->component_mask & COMPONENT_PLAYER) {
        return VISUAL_TYPE_PLAYER;
    }
    
    if (entity->component_mask & COMPONENT_AI) {
        return VISUAL_TYPE_AI_SHIP;
    }
    
    // Check if it's a large, stationary object (sun/planet)
    if ((entity->component_mask & COMPONENT_PHYSICS) && 
        (entity->component_mask & COMPONENT_COLLISION)) {
        
        struct Physics* physics = entity_get_physics(world, entity_id);
        struct Collision* collision = entity_get_collision(world, entity_id);
        
        if (physics && collision) {
            // Large radius suggests sun or planet
            if (collision->radius > 15.0f) {
                return VISUAL_TYPE_SUN;
            } else if (collision->radius > 5.0f) {
                return VISUAL_TYPE_PLANET;
            } else {
                return VISUAL_TYPE_ASTEROID;
            }
        }
    }
    
    return VISUAL_TYPE_ASTEROID;  // Default
}

// ============================================================================
// TRANSFORM UTILITIES
// ============================================================================

Vector3 transform_vertex(Vector3 vertex, struct Transform* transform) {
    // Apply scale
    Vector3 result = {
        vertex.x * transform->scale.x,
        vertex.y * transform->scale.y,
        vertex.z * transform->scale.z
    };
    
    // TODO: Apply rotation (quaternion math)
    // For now, just apply translation
    result.x += transform->position.x;
    result.y += transform->position.y;
    result.z += transform->position.z;
    
    return result;
}

// ============================================================================
// MESH RENDERING
// ============================================================================

void render_mesh(Mesh* mesh, struct Transform* transform, Material* material, Texture* texture, RenderConfig* config) {
    if (!mesh || !transform || !config || !material) return;

    // Allocate memory for SDL_Vertex array
    SDL_Vertex* sdl_vertices = malloc(mesh->vertex_count * sizeof(SDL_Vertex));
    if (!sdl_vertices) {
        printf("❌ Failed to allocate memory for SDL vertices\n");
        return;
    }

    // Transform and light each vertex
    for (int i = 0; i < mesh->vertex_count; i++) {
        Vertex* v = &mesh->vertices[i];
        
        // Apply world transform to position and normal
        Vector3 world_pos = transform_vertex(v->position, transform);
        // TODO: Transform normal correctly (inverse transpose of model matrix)
        Vector3 world_normal = v->normal;

        // Project to screen space
        Point2D screen_pos = project_3d_to_2d(world_pos, &config->camera, config->screen_width, config->screen_height);
        sdl_vertices[i].position = (SDL_FPoint){(float)screen_pos.x, (float)screen_pos.y};
        
        // Calculate lighting for this vertex
        Vector3 lit_color = calculate_lighting(world_pos, world_normal, material->diffuse_color, &config->lighting);
        sdl_vertices[i].color = (SDL_Color){
            (Uint8)(lit_color.x * 255.0f),
            (Uint8)(lit_color.y * 255.0f),
            (Uint8)(lit_color.z * 255.0f),
            255
        };
        
        // Set texture coordinates
        if (texture) {
            sdl_vertices[i].tex_coord = (SDL_FPoint){v->tex_coord.u, 1.0f - v->tex_coord.v}; // Flip V for SDL
        }
    }

    // Render the geometry
    SDL_RenderGeometry(
        config->renderer, 
        texture ? texture->sdl_texture : NULL, 
        sdl_vertices, 
        mesh->vertex_count, 
        mesh->indices, 
        mesh->index_count
    );

    // Free the allocated memory
    free(sdl_vertices);
}

void render_mesh_enhanced(Mesh* mesh, struct Transform* transform, Material* material, Texture* texture, RenderConfig* config) {
    if (!mesh || !transform || !config || !material) return;

    // Allocate memory for SDL_Vertex array
    SDL_Vertex* sdl_vertices = malloc(mesh->vertex_count * sizeof(SDL_Vertex));
    if (!sdl_vertices) {
        printf("❌ Failed to allocate memory for SDL vertices\n");
        return;
    }

    // Enhanced lighting calculation using material properties
    for (int i = 0; i < mesh->vertex_count; i++) {
        Vertex* v = &mesh->vertices[i];
        
        // Apply world transform to position and normal
        Vector3 world_pos = transform_vertex(v->position, transform);
        Vector3 world_normal = v->normal;

        // Project to screen space
        Point2D screen_pos = project_3d_to_2d(world_pos, &config->camera, config->screen_width, config->screen_height);
        sdl_vertices[i].position = (SDL_FPoint){(float)screen_pos.x, (float)screen_pos.y};
        
        // Enhanced lighting calculation
        Vector3 base_color = material->diffuse_color;
        
        // Add emission for glowing materials
        if (material->emission_color.x > 0.01f || material->emission_color.y > 0.01f || material->emission_color.z > 0.01f) {
            base_color.x += material->emission_color.x * 0.5f;
            base_color.y += material->emission_color.y * 0.5f;
            base_color.z += material->emission_color.z * 0.5f;
        }
        
        Vector3 lit_color = calculate_lighting(world_pos, world_normal, base_color, &config->lighting);
        
        // Apply metallic properties (simple approximation)
        if (material->metallic > 0.5f) {
            // Make metallic surfaces more reflective
            float metallic_boost = material->metallic * 0.3f;
            lit_color.x += metallic_boost;
            lit_color.y += metallic_boost;
            lit_color.z += metallic_boost;
        }
        
        // Clamp colors to valid range
        lit_color.x = fminf(1.0f, fmaxf(0.0f, lit_color.x));
        lit_color.y = fminf(1.0f, fmaxf(0.0f, lit_color.y));
        lit_color.z = fminf(1.0f, fmaxf(0.0f, lit_color.z));
        
        sdl_vertices[i].color = (SDL_Color){
            (Uint8)(lit_color.x * 255.0f),
            (Uint8)(lit_color.y * 255.0f),
            (Uint8)(lit_color.z * 255.0f),
            255
        };
        
        // Set texture coordinates
        if (texture) {
            sdl_vertices[i].tex_coord = (SDL_FPoint){v->tex_coord.u, 1.0f - v->tex_coord.v}; // Flip V for SDL
        }
    }

    // Apply texture color modulation based on material properties
    if (texture) {
        // Adjust texture brightness based on material emission
        float emission_factor = (material->emission_color.x + material->emission_color.y + material->emission_color.z) / 3.0f;
        if (emission_factor > 0.1f) {
            Uint8 mod_factor = (Uint8)(255.0f * (1.0f + emission_factor));
            SDL_SetTextureColorMod(texture->sdl_texture, mod_factor, mod_factor, mod_factor);
        } else {
            SDL_SetTextureColorMod(texture->sdl_texture, 255, 255, 255);
        }
    }

    // Render the geometry
    SDL_RenderGeometry(
        config->renderer, 
        texture ? texture->sdl_texture : NULL, 
        sdl_vertices, 
        mesh->vertex_count, 
        mesh->indices, 
        mesh->index_count
    );
    
    // Reset texture modulation
    if (texture) {
        SDL_SetTextureColorMod(texture->sdl_texture, 255, 255, 255);
    }

    // Free the allocated memory
    free(sdl_vertices);
}

void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config) {
    if (!world || entity_id == INVALID_ENTITY || !config || !config->assets) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Renderable* renderable = entity_get_renderable(world, entity_id);
    
    if (!transform || !renderable || !renderable->visible) return;
    
    Mesh* mesh = &config->assets->meshes[renderable->mesh_id];
    
    // REPOSITORY-BASED LOOKUP: Get material from mesh metadata
    Material* material = assets_get_material(config->assets, mesh->material_name);
    if (!material) {
        // Fallback to default material if not found
        if (config->assets->material_count > 0) {
            material = &config->assets->materials[0];
        }
    }
    
    // Multi-texture support: Get primary texture from material
    Texture* texture = NULL;
    if (material) {
        // Try diffuse texture first (new multi-texture system)
        if (strlen(material->diffuse_texture) > 0) {
            char texture_name[128];
            snprintf(texture_name, sizeof(texture_name), "%s_diffuse", material->name);
            texture = assets_get_texture(config->assets, texture_name);
        }
        
        // Fallback to legacy texture_name for compatibility
        if (!texture && strlen(material->texture_name) > 0) {
            texture = assets_get_texture(config->assets, material->texture_name);
        }
    }
    
    render_mesh_enhanced(mesh, transform, material, texture, config);
}

// ============================================================================
// TRIANGLE RASTERIZATION
// ============================================================================

void render_filled_triangle(SDL_Renderer* renderer, Point2D p1, Point2D p2, Point2D p3, Uint8 r, Uint8 g, Uint8 b) {
    if (!renderer) return;
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    // Simple triangle filling using scanline algorithm
    // Sort vertices by Y coordinate (p1.y <= p2.y <= p3.y)
    if (p1.y > p2.y) {
        Point2D temp = p1; p1 = p2; p2 = temp;
    }
    if (p2.y > p3.y) {
        Point2D temp = p2; p2 = p3; p3 = temp;
        if (p1.y > p2.y) {
            temp = p1; p1 = p2; p2 = temp;
        }
    }
    
    // Handle degenerate cases
    if (p1.y == p3.y) {
        // Horizontal line
        int x_min = (p1.x < p2.x) ? ((p1.x < p3.x) ? p1.x : p3.x) : ((p2.x < p3.x) ? p2.x : p3.x);
        int x_max = (p1.x > p2.x) ? ((p1.x > p3.x) ? p1.x : p3.x) : ((p2.x > p3.x) ? p2.x : p3.x);
        SDL_RenderDrawLine(renderer, x_min, p1.y, x_max, p1.y);
        return;
    }
    
    // Scanline fill
    for (int y = p1.y; y <= p3.y; y++) {
        int x_left = INT_MAX;
        int x_right = INT_MIN;
        
        // Calculate intersections with triangle edges
        if (y >= p1.y && y <= p2.y && p2.y != p1.y) {
            int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
            x_left = (x < x_left) ? x : x_left;
            x_right = (x > x_right) ? x : x_right;
        }
        
        if (y >= p2.y && y <= p3.y && p3.y != p2.y) {
            int x = p2.x + (y - p2.y) * (p3.x - p2.x) / (p3.y - p2.y);
            x_left = (x < x_left) ? x : x_left;
            x_right = (x > x_right) ? x : x_right;
        }
        
        if (y >= p1.y && y <= p3.y && p3.y != p1.y) {
            int x = p1.x + (y - p1.y) * (p3.x - p1.x) / (p3.y - p1.y);
            x_left = (x < x_left) ? x : x_left;
            x_right = (x > x_right) ? x : x_right;
        }
        
        // Draw horizontal line
        if (x_left <= x_right) {
            SDL_RenderDrawLine(renderer, x_left, y, x_right, y);
        }
    }
}

void render_textured_triangle(SDL_Renderer* renderer, Vector3 v1, Vector3 v2, Vector3 v3, 
                             Vector3 n1, Vector3 n2, Vector3 n3, SDL_Texture* texture,
                             Camera3D* camera, LightingSystem* lighting, int screen_width, int screen_height) {
    
    // Suppress unused parameter warnings for normals (not used in this simple rasterizer)
    (void)n1;
    (void)n2;
    (void)n3;
    
    if (!renderer || !texture || !camera || !lighting) return;
    
    // Project 3D vertices to 2D screen coordinates
    Point2D p1 = project_3d_to_2d(v1, camera, screen_width, screen_height);
    Point2D p2 = project_3d_to_2d(v2, camera, screen_width, screen_height);
    Point2D p3 = project_3d_to_2d(v3, camera, screen_width, screen_height);
    
    // Calculate face normal for basic lighting
    Vector3 edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
    Vector3 edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};
    
    Vector3 face_normal = {
        edge1.y * edge2.z - edge1.z * edge2.y,
        edge1.z * edge2.x - edge1.x * edge2.z,
        edge1.x * edge2.y - edge1.y * edge2.x
    };
    
    // Calculate triangle center for lighting
    Vector3 center = {
        (v1.x + v2.x + v3.x) / 3.0f,
        (v1.y + v2.y + v3.y) / 3.0f,
        (v1.z + v2.z + v3.z) / 3.0f
    };
    
    // Calculate lighting for this triangle
    Vector3 base_color = {1.0f, 1.0f, 1.0f};  // White base color for textures
    Vector3 lit_color = calculate_lighting(center, face_normal, base_color, lighting);
    
    // Convert lighting to SDL color (0-255 range)
    Uint8 r = (Uint8)(lit_color.x * 255.0f);
    Uint8 g = (Uint8)(lit_color.y * 255.0f);
    Uint8 b = (Uint8)(lit_color.z * 255.0f);
    
    // Set the texture color modulation based on lighting
    SDL_SetTextureColorMod(texture, r, g, b);
    
    // Draw filled triangle with texture (simplified using SDL filled triangles)
    render_filled_triangle(renderer, p1, p2, p3, r, g, b);
}
