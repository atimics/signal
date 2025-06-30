#include "sokol_gfx.h"
#include "graphics_api.h"
#include "assets.h"
#include "gpu_resources.h"
#include "asset_loader/asset_loader_index.h"
#include "asset_loader/asset_loader_mesh.h"
#include "asset_loader/asset_loader_material.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Define the actual implementation of our opaque structs
struct MeshGpuResources {
    sg_buffer sg_vertex_buffer;
    sg_buffer sg_index_buffer;
};

struct TextureGpuResources {
    sg_image sg_image;
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Add fmaxf and fminf for older C standard compatibility
#ifndef fmaxf
#define fmaxf(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef fminf
#define fminf(a, b) ((a) < (b) ? (a) : (b))
#endif

// ============================================================================
// ASSET SYSTEM IMPLEMENTATION
// ============================================================================

bool assets_init(AssetRegistry* registry, const char* asset_root) {
    if (!registry || !asset_root) return false;
    
    // Clear the registry
    memset(registry, 0, sizeof(AssetRegistry));
#ifdef __EMSCRIPTEN__
    // WASM uses preloaded virtual filesystem
    strncpy(registry->asset_root, "/assets", sizeof(registry->asset_root) - 1);
#else
    // Native: use provided path
    strncpy(registry->asset_root, asset_root, sizeof(registry->asset_root) - 1);
#endif
    registry->asset_root[sizeof(registry->asset_root) - 1] = '\0';
    
    printf("📁 Asset root: %s\n", registry->asset_root);
    return true;
}

void assets_cleanup(AssetRegistry* registry) {
    if (!registry) return;
    
    // Clean up meshes
    for (int i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        if (mesh->vertices) {
            free(mesh->vertices);
            mesh->vertices = NULL;
        }
        if (mesh->indices) {
            free(mesh->indices);
            mesh->indices = NULL;
        }
        if (mesh->gpu_resources) {
            // Destroy Sokol buffers
            if (mesh->gpu_resources->sg_vertex_buffer.id != SG_INVALID_ID) {
                sg_destroy_buffer(mesh->gpu_resources->sg_vertex_buffer);
            }
            if (mesh->gpu_resources->sg_index_buffer.id != SG_INVALID_ID) {
                sg_destroy_buffer(mesh->gpu_resources->sg_index_buffer);
            }
            
            free(mesh->gpu_resources);
            mesh->gpu_resources = NULL;
        }
    }
    
    // Clean up textures
    for (int i = 0; i < registry->texture_count; i++) {
        Texture* texture = &registry->textures[i];
        if (texture->gpu_resources) {
            // Destroy Sokol image
            if (texture->gpu_resources->sg_image.id != SG_INVALID_ID) {
                sg_destroy_image(texture->gpu_resources->sg_image);
            }
            
            free(texture->gpu_resources);
            texture->gpu_resources = NULL;
        }
    }
    
    printf("🎨 Asset system cleaned up\n");
}

// ============================================================================
// ASSET LOADING
// ============================================================================

// Load mesh from absolute file path
bool load_mesh_from_file(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name) {
    if (!registry || !absolute_filepath || !mesh_name) return false;
    if (registry->mesh_count >= 32) return false;
    
    printf("🔍 DEBUG load_mesh_from_file: filepath='%s', mesh_name='%s'\n", absolute_filepath, mesh_name);
    
    // Check if file exists
    FILE* test_file = fopen(absolute_filepath, "r");
    if (!test_file) {
        printf("❌ DEBUG: Cannot open file '%s'\n", absolute_filepath);
        return false;
    }
    fclose(test_file);
    printf("✅ DEBUG: File exists and is readable\n");
    
    // Find or create mesh slot
    Mesh* mesh = &registry->meshes[registry->mesh_count];
    strncpy(mesh->name, mesh_name, sizeof(mesh->name) - 1);
    mesh->name[sizeof(mesh->name) - 1] = '\0';  // Ensure null termination
    
    if (parse_obj_file(absolute_filepath, mesh)) {
        printf("✅ DEBUG: parse_obj_file succeeded - vertices=%d, indices=%d\n", 
               mesh->vertex_count, mesh->index_count);
        
        // Comprehensive validation
        if (mesh->vertex_count == 0 || mesh->index_count == 0) {
            printf("❌ Mesh %s has zero vertices (%d) or indices (%d)\n", 
                   mesh_name, mesh->vertex_count, mesh->index_count);
            return false;
        }
        
        if (!mesh->vertices || !mesh->indices) {
            printf("❌ Mesh %s has NULL vertex (%p) or index (%p) data\n",
                   mesh_name, (void*)mesh->vertices, (void*)mesh->indices);
            return false;
        }
        
        // Validate buffer creation before calling sg_make_buffer
        size_t vertex_buffer_size = mesh->vertex_count * sizeof(Vertex);
        size_t index_buffer_size = mesh->index_count * sizeof(int);
        
        if (vertex_buffer_size == 0 || index_buffer_size == 0) {
            printf("❌ Mesh %s would create zero-sized buffers: VB=%zu IB=%zu\n",
                   mesh_name, vertex_buffer_size, index_buffer_size);
            return false;
        }
        
        printf("🔍 DEBUG: Creating GPU buffers - VB=%zu bytes, IB=%zu bytes\n", 
               vertex_buffer_size, index_buffer_size);
        
        // Allocate memory for our opaque struct
        mesh->gpu_resources = calloc(1, sizeof(struct MeshGpuResources));
        if (!mesh->gpu_resources) {
            printf("❌ DEBUG: Failed to allocate GPU resources\n");
            return false;
        }
        
        // Create vertex buffer
        mesh->gpu_resources->sg_vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = {
                .ptr = mesh->vertices,
                .size = vertex_buffer_size
            },
            .usage = { .vertex_buffer = true },
            .label = mesh->name
        });

        // Create index buffer
        mesh->gpu_resources->sg_index_buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = {
                .ptr = mesh->indices,
                .size = index_buffer_size
            },
            .usage = { .index_buffer = true },
            .label = mesh->name
        });

        mesh->loaded = true;  // Mark as successfully loaded
        registry->mesh_count++;
        printf("✅ Mesh '%s' loaded successfully with %d vertices, %d indices\n", 
               mesh_name, mesh->vertex_count, mesh->index_count);
        return true;
    }
    
    printf("❌ parse_obj_file failed for %s\n", absolute_filepath);
    return false;
}

bool load_texture(AssetRegistry* registry, const char* texture_path, const char* texture_name) {
    if (!registry || !texture_path || !texture_name) return false;
    if (registry->texture_count >= 32) return false;
    
    // Construct full path to texture
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/textures/%s", registry->asset_root, texture_path);
    
    // Load image data
    int width, height, channels;
    unsigned char* image_data = stbi_load(full_path, &width, &height, &channels, 4); // Force RGBA
    
    if (!image_data) {
        printf("⚠️  Could not load texture: %s\n", full_path);
        return false;
    }
    
    // Find texture slot
    Texture* texture = &registry->textures[registry->texture_count];
    strncpy(texture->name, texture_name, sizeof(texture->name) - 1);
    texture->name[sizeof(texture->name) - 1] = '\0';
    strncpy(texture->filepath, full_path, sizeof(texture->filepath) - 1);
    texture->filepath[sizeof(texture->filepath) - 1] = '\0';
    texture->width = width;
    texture->height = height;
    
    // Allocate GPU resources
    texture->gpu_resources = calloc(1, sizeof(struct TextureGpuResources));
    if (!texture->gpu_resources) {
        printf("❌ Failed to allocate texture GPU resources\n");
        stbi_image_free(image_data);
        return false;
    }
    
    // Create Sokol image
    texture->gpu_resources->sg_image = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = image_data,
            .size = width * height * 4
        },
        .label = texture_name
    });
    
    stbi_image_free(image_data);
    
    texture->loaded = true;
    registry->texture_count++;
    
    printf("✅ Loaded texture: %s (%dx%d)\n", texture_name, width, height);
    return true;
}

// ============================================================================
// QUERY FUNCTIONS
// ============================================================================

Mesh* assets_get_mesh(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (int i = 0; i < registry->mesh_count; i++) {
        if (strcmp(registry->meshes[i].name, name) == 0) {
            return &registry->meshes[i];
        }
    }
    return NULL;
}

Texture* assets_get_texture(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (int i = 0; i < registry->texture_count; i++) {
        if (strcmp(registry->textures[i].name, name) == 0) {
            return &registry->textures[i];
        }
    }
    return NULL;
}

Material* assets_get_material(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (int i = 0; i < registry->material_count; i++) {
        if (strcmp(registry->materials[i].name, name) == 0) {
            return &registry->materials[i];
        }
    }
    return NULL;
}

void assets_list_loaded(AssetRegistry* registry) {
    if (!registry) return;
    
    printf("📋 Loaded Assets:\n");
    printf("   Meshes (%d):\n", registry->mesh_count);
    for (int i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        printf("     %s: %d vertices, %d indices%s\n", 
               mesh->name, mesh->vertex_count, mesh->index_count,
               mesh->loaded ? " ✅" : " ❌");
    }
    
    printf("   Textures (%d):\n", registry->texture_count);
    for (int i = 0; i < registry->texture_count; i++) {
        Texture* texture = &registry->textures[i];
        printf("     %s: %dx%d%s\n", 
               texture->name, texture->width, texture->height,
               texture->loaded ? " ✅" : " ❌");
    }
    
    printf("   Materials (%d):\n", registry->material_count);
    for (int i = 0; i < registry->material_count; i++) {
        Material* material = &registry->materials[i];
        printf("     %s%s\n", material->name, material->loaded ? " ✅" : " ❌");
    }
}

bool assets_load_all_in_directory(AssetRegistry* registry) {
    return load_assets_from_metadata(registry);
}

// ============================================================================
// MATERIAL REPOSITORY IMPLEMENTATION
// ============================================================================

bool materials_load_library(AssetRegistry* registry, const char* materials_dir) {
    if (!registry || !materials_dir) return false;
    
    // TODO: Implement loading of shared material library
    // This would scan materials_dir for .mtl files and load them
    // as reusable materials independent of specific meshes
    printf("📚 Material library loading not yet implemented\n");
    return true;
}

// ============================================================================
// MESH/MATERIAL INTEGRATION
// ============================================================================

bool assets_create_renderable_from_mesh(AssetRegistry* registry, const char* mesh_name, struct Renderable* renderable) {
    if (!registry || !mesh_name || !renderable) {
        printf("❌ Invalid parameters to assets_create_renderable_from_mesh\n");
        return false;
    }
    
    Mesh* mesh = assets_get_mesh(registry, mesh_name);
    if (!mesh) {
        printf("❌ Mesh '%s' not found in registry\n", mesh_name);
        return false;
    }
    
    if (!mesh->loaded) {
        printf("❌ Mesh '%s' exists but is not loaded\n", mesh_name);
        return false;
    }
    
    // Validate GPU resources exist and are valid
    if (mesh->gpu_resources &&
        sg_query_buffer_state(mesh->gpu_resources->sg_vertex_buffer) == SG_RESOURCESTATE_VALID &&
        sg_query_buffer_state(mesh->gpu_resources->sg_index_buffer) == SG_RESOURCESTATE_VALID) {
        
        // Transfer GPU resource ownership to the renderable
        renderable->gpu_resources = mesh->gpu_resources;
        renderable->index_count = mesh->index_count;
        renderable->visible = true;
        
        printf("✅ Created renderable from mesh '%s': %d indices\n", 
               mesh_name, renderable->index_count);
        
        return true;
    } else {
        printf("❌ Mesh '%s' has invalid GPU resources\n", mesh_name);
        return false;
    }
}
