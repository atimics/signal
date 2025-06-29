#include "render.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// 3D PROJECTION
// ============================================================================

Point2D project_3d_to_2d(Vector3 world_pos, const Camera3D* camera, int screen_width, int screen_height) {
    // Transform world position to camera space
    Vector3 relative_pos = {
        world_pos.x - camera->position.x,
        world_pos.y - camera->position.y,
        world_pos.z - camera->position.z
    };
    
    // Calculate camera coordinate system
    Vector3 forward = {
        camera->target.x - camera->position.x,
        camera->target.y - camera->position.y,
        camera->target.z - camera->position.z
    };
    
    // Normalize forward vector
    float forward_len = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (forward_len > 0.001f) {
        forward.x /= forward_len;
        forward.y /= forward_len;
        forward.z /= forward_len;
    } else {
        forward = (Vector3){0, 0, -1};  // Default forward
    }
    
    // Calculate right vector (cross product of forward and up)
    Vector3 right = {
        forward.y * camera->up.z - forward.z * camera->up.y,
        forward.z * camera->up.x - forward.x * camera->up.z,
        forward.x * camera->up.y - forward.y * camera->up.x
    };
    
    // Calculate actual up vector (cross product of right and forward)
    Vector3 up = {
        right.y * forward.z - right.z * forward.y,
        right.z * forward.x - right.x * forward.z,
        right.x * forward.y - right.y * forward.x
    };
    
    // Transform to camera space using dot products
    float cam_x = relative_pos.x * right.x + relative_pos.y * right.y + relative_pos.z * right.z;
    float cam_y = relative_pos.x * up.x + relative_pos.y * up.y + relative_pos.z * up.z;
    float cam_z = relative_pos.x * forward.x + relative_pos.y * forward.y + relative_pos.z * forward.z;
    
    // Perspective projection
    if (cam_z <= 0.1f) cam_z = 0.1f;  // Prevent division by zero and behind camera
    
    // Use proper FOV calculation
    float fov_rad = camera->fov * M_PI / 180.0f;
    float focal_length = (screen_height / 2.0f) / tanf(fov_rad / 2.0f);
    
    float screen_x = (cam_x * focal_length / cam_z) + screen_width / 2.0f;
    float screen_y = screen_height / 2.0f - (cam_y * focal_length / cam_z);  // Flip Y
    
    Point2D result = {
        (int)screen_x,
        (int)screen_y
    };
    
    return result;
}

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

bool render_init(RenderConfig* config, AssetRegistry* assets, float viewport_width, float viewport_height) {
    if (!config || !assets) return false;
    
    printf("🎨 Initializing 3D SDL Render System...\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("❌ SDL could not initialize: %s\n", SDL_GetError());
        return false;
    }
    
    // Create window
    config->screen_width = (int)viewport_width;
    config->screen_height = (int)viewport_height;
    config->assets = assets;
    
    config->window = SDL_CreateWindow("CGGame - Spaceport",
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     config->screen_width,
                                     config->screen_height,
                                     SDL_WINDOW_SHOWN);
    
    if (!config->window) {
        printf("❌ Window could not be created: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    config->renderer = SDL_CreateRenderer(config->window, -1, 
                                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!config->renderer) {
        printf("❌ Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(config->window);
        SDL_Quit();
        return false;
    }
    
    // Initialize render settings
    config->mode = RENDER_MODE_TEXTURED;  // Use textured mode by default
    config->show_debug_info = true;
    config->show_velocities = true;
    config->show_collision_bounds = false;
    config->show_orbits = true;
    config->update_interval = 1.0f / 60.0f;  // 60 FPS
    config->last_update = 0.0f;
    config->frame_count = 0;
    
    // Initialize cockpit UI
    if (!cockpit_ui_init(&config->ui, config->renderer, config->screen_width, config->screen_height)) {
        printf("❌ Failed to initialize cockpit UI\n");
        SDL_DestroyRenderer(config->renderer);
        SDL_DestroyWindow(config->window);
        SDL_Quit();
        return false;
    }
    
    // Initialize 3D camera for first-person view
    config->camera.position = (Vector3){-20, 5, -20};    // Start at player position (elevated)
    config->camera.target = (Vector3){0, 5, 0};          // Looking toward spaceport center
    config->camera.up = (Vector3){0, 1, 0};              // Y is up
    config->camera.fov = 75.0f;                          // Wider FOV for FPS
    config->camera.near_plane = 0.1f;                    // Closer near plane for FPS
    config->camera.far_plane = 500.0f;
    config->camera.aspect_ratio = viewport_width / viewport_height;
    
    // Initialize lighting system
    lighting_init(&config->lighting);
    
    printf("✅ 3D Render system initialized (%dx%d)\n", config->screen_width, config->screen_height);
    printf("   Camera position: (%.1f, %.1f, %.1f)\n", 
           config->camera.position.x, config->camera.position.y, config->camera.position.z);
    printf("   Using data-driven asset system\n");
    
    return true;
}

void render_cleanup(RenderConfig* config) {
    if (!config) return;
    
    // Cleanup UI
    cockpit_ui_cleanup(&config->ui);
    
    // Asset cleanup is handled by the asset system
    
    // Cleanup SDL
    if (config->renderer) {
        SDL_DestroyRenderer(config->renderer);
        config->renderer = NULL;
    }
    
    if (config->window) {
        SDL_DestroyWindow(config->window);
        config->window = NULL;
    }
    
    SDL_Quit();
    printf("🎨 Render system cleaned up\n");
}

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
// RENDERING FUNCTIONS
// ============================================================================

void render_clear_screen(RenderConfig* config) {
    if (!config || !config->renderer) return;
    
    // Set background to space black
    SDL_SetRenderDrawColor(config->renderer, 0, 0, 0, 255);
    SDL_RenderClear(config->renderer);
}



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
        material = &config->assets->materials[0];
    }
    
    Texture* texture = NULL;
    if (material && strlen(material->texture_name) > 0) {
        texture = assets_get_texture(config->assets, material->texture_name);
    }
    
    render_mesh(mesh, transform, material, texture, config);
}

void render_debug_info(struct World* world, RenderConfig* config) {
    if (!config || !config->show_debug_info) return;
    
    // For now, just count entities - we'd need a font system for proper text
    // This is where you'd render FPS, entity count, camera position, etc.
    
    static uint32_t debug_counter = 0;
    if (++debug_counter % 60 == 0) {  // Print debug info every second
        printf("🎮 Frame %d | Entities: %d | Camera: (%.1f,%.1f,%.1f)\n", 
               config->frame_count, world->entity_count,
               config->camera.position.x, config->camera.position.y, config->camera.position.z);
    }
}

void render_present(RenderConfig* config) {
    if (!config || !config->renderer) return;
    
    SDL_RenderPresent(config->renderer);
    config->frame_count++;
}

// ============================================================================
// CAMERA CONTROLS
// ============================================================================

void camera_set_position(Camera3D* camera, Vector3 position) {
    if (!camera) return;
    camera->position = position;
}

void camera_look_at(Camera3D* camera, Vector3 eye, Vector3 target, Vector3 up) {
    if (!camera) return;
    camera->position = eye;
    camera->target = target;
    camera->up = up;
}

void camera_follow_entity(Camera3D* camera, struct World* world, EntityID entity_id, float distance) {
    if (!camera || !world || entity_id == INVALID_ENTITY) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    if (!transform) return;
    
    // THIRD-PERSON CHASE CAMERA: Position camera behind and above the target
    Vector3 target_pos = transform->position;
    
    // Camera offset: behind (positive Z), above (positive Y), and slightly to the side
    Vector3 camera_offset = {5.0f, 15.0f, 25.0f};
    
    Vector3 desired_camera_pos = {
        target_pos.x + camera_offset.x,
        target_pos.y + camera_offset.y,
        target_pos.z + camera_offset.z
    };
    
    // Smooth camera movement (less aggressive than before)
    float lerp = 0.02f;  // Much slower, more stable
    camera->position.x += (desired_camera_pos.x - camera->position.x) * lerp;
    camera->position.y += (desired_camera_pos.y - camera->position.y) * lerp;
    camera->position.z += (desired_camera_pos.z - camera->position.z) * lerp;
    
    // ALWAYS LOOK AT THE TARGET ENTITY (this is key for stable camera)
    camera->target = target_pos;
    
    // Keep up vector stable
    camera->up = (Vector3){0, 1, 0};
}

// ============================================================================
// UI MESSAGE API
// ============================================================================

void render_add_comm_message(RenderConfig* config, const char* sender, const char* message, bool is_player) {
    if (!config) return;
    cockpit_ui_add_message(&config->ui, sender, message, is_player);
}

// ============================================================================
// SCREENSHOT FUNCTIONALITY
// ============================================================================

bool render_take_screenshot(RenderConfig* config, const char* filename) {
    if (!config || !config->renderer || !filename) return false;
    
    // Get the current render target surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, config->screen_width, config->screen_height, 32,
                                               0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        printf("❌ Failed to create surface for screenshot: %s\n", SDL_GetError());
        return false;
    }
    
    // Read pixels from renderer
    if (SDL_RenderReadPixels(config->renderer, NULL, surface->format->format,
                           surface->pixels, surface->pitch) != 0) {
        printf("❌ Failed to read pixels for screenshot: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    
    // Save as BMP file
    if (SDL_SaveBMP(surface, filename) != 0) {
        printf("❌ Failed to save screenshot: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    
    SDL_FreeSurface(surface);
    printf("📸 Screenshot saved: %s (%dx%d)\n", filename, config->screen_width, config->screen_height);
    return true;
}

bool render_take_screenshot_from_position(struct World* world, RenderConfig* config, 
                                         Vector3 camera_pos, Vector3 look_at_pos, 
                                         const char* filename) {
    if (!world || !config || !filename) return false;
    
    // Save current camera state
    Camera3D original_camera = config->camera;
    
    // Set up camera at specified position
    camera_look_at(&config->camera, camera_pos, look_at_pos, (Vector3){0, 1, 0});
    
    printf("📸 Taking screenshot from (%.1f, %.1f, %.1f) looking at (%.1f, %.1f, %.1f)\n",
           camera_pos.x, camera_pos.y, camera_pos.z,
           look_at_pos.x, look_at_pos.y, look_at_pos.z);
    
    // Clear screen with space background
    SDL_SetRenderDrawColor(config->renderer, 8, 8, 20, 255);  // Dark space blue
    SDL_RenderClear(config->renderer);
    
    // Render all entities from this viewpoint
    uint32_t rendered_count = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if ((entity->component_mask & COMPONENT_RENDERABLE) &&
            (entity->component_mask & COMPONENT_TRANSFORM)) {
            
            EntityID entity_id = i + 1;  // Entity IDs are 1-based
            render_entity_3d(world, entity_id, config);
            rendered_count++;
        }
    }
    
    printf("📸 Rendered %d entities for screenshot\n", rendered_count);
    
    // Present the frame (this updates the renderer's back buffer)
    SDL_RenderPresent(config->renderer);
    
    // Small delay to ensure rendering is complete
    SDL_Delay(100);
    
    // Take the screenshot
    bool success = render_take_screenshot(config, filename);
    
    // Restore original camera
    config->camera = original_camera;
    
    return success;
}

// ============================================================================
// LIGHTING SYSTEM IMPLEMENTATION
// ============================================================================

void lighting_init(LightingSystem* lighting) {
    if (!lighting) return;
    
    memset(lighting, 0, sizeof(LightingSystem));
    
    // Set default ambient lighting
    lighting->ambient_color = (Vector3){0.2f, 0.2f, 0.3f};  // Slight blue tint
    lighting->ambient_intensity = 0.3f;
    
    // Add a default sun light
    lighting_add_directional_light(lighting, 
                                  (Vector3){-0.5f, -1.0f, -0.3f},  // From upper left
                                  (Vector3){1.0f, 0.95f, 0.8f},    // Warm white
                                  0.8f);                           // Strong intensity
    
    printf("💡 Lighting system initialized with %d lights\n", lighting->light_count);
}

void lighting_add_directional_light(LightingSystem* lighting, Vector3 direction, Vector3 color, float intensity) {
    if (!lighting || lighting->light_count >= 8) return;
    
    Light* light = &lighting->lights[lighting->light_count++];
    light->type = LIGHT_TYPE_DIRECTIONAL;
    light->direction = direction;
    light->color = color;
    light->intensity = intensity;
    light->enabled = true;
    
    // Normalize direction
    float len = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len > 0.001f) {
        light->direction.x /= len;
        light->direction.y /= len;
        light->direction.z /= len;
    }
    
    printf("💡 Added directional light: dir(%.2f,%.2f,%.2f) color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           direction.x, direction.y, direction.z, color.x, color.y, color.z, intensity);
}

void lighting_add_point_light(LightingSystem* lighting, Vector3 position, Vector3 color, float intensity) {
    if (!lighting || lighting->light_count >= 8) return;
    
    Light* light = &lighting->lights[lighting->light_count++];
    light->type = LIGHT_TYPE_POINT;
    light->position = position;
    light->color = color;
    light->intensity = intensity;
    light->enabled = true;
    
    printf("💡 Added point light: pos(%.1f,%.1f,%.1f) color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           position.x, position.y, position.z, color.x, color.y, color.z, intensity);
}

void lighting_set_ambient(LightingSystem* lighting, Vector3 color, float intensity) {
    if (!lighting) return;
    
    lighting->ambient_color = color;
    lighting->ambient_intensity = intensity;
    
    printf("💡 Set ambient light: color(%.2f,%.2f,%.2f) intensity=%.2f\n",
           color.x, color.y, color.z, intensity);
}

Vector3 calculate_lighting(Vector3 surface_pos, Vector3 surface_normal, Vector3 material_color, LightingSystem* lighting) {
    if (!lighting) return material_color;
    
    // Start with ambient lighting
    Vector3 final_color = {
        material_color.x * lighting->ambient_color.x * lighting->ambient_intensity,
        material_color.y * lighting->ambient_color.y * lighting->ambient_intensity,
        material_color.z * lighting->ambient_color.z * lighting->ambient_intensity
    };
    
    // Normalize surface normal
    float normal_len = sqrtf(surface_normal.x * surface_normal.x + 
                            surface_normal.y * surface_normal.y + 
                            surface_normal.z * surface_normal.z);
    if (normal_len > 0.001f) {
        surface_normal.x /= normal_len;
        surface_normal.y /= normal_len;
        surface_normal.z /= normal_len;
    }
    
    // Process each light
    for (uint32_t i = 0; i < lighting->light_count; i++) {
        Light* light = &lighting->lights[i];
        if (!light->enabled) continue;
        
        Vector3 light_dir = {0, 0, 0};
        float attenuation = 1.0f;
        
        if (light->type == LIGHT_TYPE_DIRECTIONAL) {
            // Directional light - direction is constant
            light_dir = light->direction;
        } else if (light->type == LIGHT_TYPE_POINT) {
            // Point light - calculate direction from surface to light
            light_dir.x = light->position.x - surface_pos.x;
            light_dir.y = light->position.y - surface_pos.y;
            light_dir.z = light->position.z - surface_pos.z;
            
            // Calculate distance for attenuation
            float distance = sqrtf(light_dir.x * light_dir.x + 
                                  light_dir.y * light_dir.y + 
                                  light_dir.z * light_dir.z);
            
            if (distance > 0.001f) {
                light_dir.x /= distance;
                light_dir.y /= distance;
                light_dir.z /= distance;
                
                // Simple distance attenuation
                attenuation = 1.0f / (1.0f + 0.01f * distance + 0.001f * distance * distance);
            }
        }
        
        // Calculate dot product for diffuse lighting (Lambert's cosine law)
        float dot_product = -(surface_normal.x * light_dir.x + 
                             surface_normal.y * light_dir.y + 
                             surface_normal.z * light_dir.z);
        
        // Clamp to positive values (no negative lighting)
        if (dot_product > 0.0f) {
            float light_contribution = dot_product * light->intensity * attenuation;
            
            final_color.x += material_color.x * light->color.x * light_contribution;
            final_color.y += material_color.y * light->color.y * light_contribution;
            final_color.z += material_color.z * light->color.z * light_contribution;
        }
    }
    
    // Clamp final color to [0.0, 1.0] range
    if (final_color.x > 1.0f) final_color.x = 1.0f;
    if (final_color.y > 1.0f) final_color.y = 1.0f;
    if (final_color.z > 1.0f) final_color.z = 1.0f;
    
    return final_color;
}

// Simple textured triangle rasterization with lighting
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

// ============================================================================
// MESH RENDERING WITH LIGHTING
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

// ============================================================================
// MAIN RENDER FRAME
// ============================================================================

void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time) {
    if (!world || !config) return;
    
    // Suppress unused parameter warning
    (void)delta_time;
    (void)player_id;  // Camera system now handles camera positioning
    
    // Clear the screen
    render_clear_screen(config);
    
    // Camera position is now managed by camera system
    
    // Render all entities
    for (uint32_t i = 0; i < world->entity_count; i++) {
        EntityID entity_id = i + 1; // Entity IDs are 1-based
        render_entity_3d(world, entity_id, config);
    }
    
    // Render debug information
    render_debug_info(world, config);
    
    // Render UI
    cockpit_ui_render(&config->ui);
    
    // Present the frame
    render_present(config);
}
