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
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
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
    for (uint32_t i = 0; i < registry->texture_count; i++) {
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

bool load_texture(AssetRegistry* registry, const char* texture_path, const char* texture_name) {
    if (!registry || !texture_path || !texture_name) return false;
    if (registry->texture_count >= MAX_TEXTURES) return false;
    
    // Construct full path to texture with improved path handling
    char full_path[512];
    
    // Check if texture_path is already an absolute path
    if (texture_path[0] == '/' || (strlen(texture_path) > 1 && texture_path[1] == ':')) {
        // Absolute path - use as is
        strncpy(full_path, texture_path, sizeof(full_path) - 1);
        full_path[sizeof(full_path) - 1] = '\0';
    } else {
        // Relative path - first try as-is from asset root
        snprintf(full_path, sizeof(full_path), "%s/%s", registry->asset_root, texture_path);
        
        // Check if file exists at this path
        FILE* test_file = fopen(full_path, "rb");
        if (!test_file) {
            // If not found, try with textures subdirectory
            if (strstr(texture_path, "textures/") == texture_path || strstr(texture_path, "textures\\") == texture_path) {
                // Path already includes textures directory
                snprintf(full_path, sizeof(full_path), "%s/%s", registry->asset_root, texture_path);
            } else {
                // Traditional relative path - add textures directory
                snprintf(full_path, sizeof(full_path), "%s/textures/%s", registry->asset_root, texture_path);
            }
        } else {
            fclose(test_file);
        }
    }
    
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
    
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        if (strcmp(registry->meshes[i].name, name) == 0) {
            return &registry->meshes[i];
        }
    }
    return NULL;
}

Texture* assets_get_texture(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        if (strcmp(registry->textures[i].name, name) == 0) {
            return &registry->textures[i];
        }
    }
    return NULL;
}

Material* assets_get_material(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (uint32_t i = 0; i < registry->material_count; i++) {
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
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        printf("     %s: %d vertices, %d indices%s\n", 
               mesh->name, mesh->vertex_count, mesh->index_count,
               mesh->loaded ? " ✅" : " ❌");
    }
    
    printf("   Textures (%d):\n", registry->texture_count);
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        Texture* texture = &registry->textures[i];
        printf("     %s: %dx%d%s\n", 
               texture->name, texture->width, texture->height,
               texture->loaded ? " ✅" : " ❌");
    }
    
    printf("   Materials (%d):\n", registry->material_count);
    for (uint32_t i = 0; i < registry->material_count; i++) {
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
        
        // Use PIMPL pattern: Create GPU resources struct for renderable
        renderable->gpu_resources = gpu_resources_create();
        if (!renderable->gpu_resources) {
            printf("❌ Failed to allocate GPU resources for mesh '%s'\n", mesh_name);
            return false;
        }
        
        // Set buffers through PIMPL interface
        gpu_resources_set_vertex_buffer(renderable->gpu_resources, (gpu_buffer_t){mesh->gpu_resources->sg_vertex_buffer.id});
        gpu_resources_set_index_buffer(renderable->gpu_resources, (gpu_buffer_t){mesh->gpu_resources->sg_index_buffer.id});
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

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* get_shader_path(const char* base_name, const char* stage) {
    static char path_buffers[2][512];  // Two buffers to handle consecutive calls
    static int buffer_index = 0;
    
    char* path = path_buffers[buffer_index];
    buffer_index = (buffer_index + 1) % 2;  // Alternate between buffers
    
#ifdef SOKOL_METAL
    const char* extension = "metal";
#else
    const char* extension = "glsl";
#endif
    
    snprintf(path, 512, "assets/shaders/%s.%s.%s", base_name, stage, extension);
    return path;
}

char* load_shader_source(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("❌ Could not open shader file: %s\n", filepath);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size <= 0) {
        printf("❌ Invalid shader file size: %ld\n", size);
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer and read
    char* source = malloc(size + 1);
    if (!source) {
        printf("❌ Failed to allocate shader source buffer\n");
        fclose(file);
        return NULL;
    }
    
    size_t read_bytes = fread(source, 1, size, file);
    source[read_bytes] = '\0';
    fclose(file);
    
    return source;
}

void free_shader_source(char* source) {
    if (source) {
        free(source);
    }
}

bool assets_initialize_gpu_resources(AssetRegistry* registry) {
    if (!registry) {
        printf("❌ Invalid registry for GPU resource initialization\n");
        return false;
    }
    
    printf("🎨 Initializing GPU resources...\n");
    
    // Note: Meshes are already loaded to GPU during mesh loading
    // Check that all loaded meshes have valid GPU resources
    bool meshes_ok = true;
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        if (mesh->loaded && (!mesh->gpu_resources || 
            sg_query_buffer_state(mesh->gpu_resources->sg_vertex_buffer) != SG_RESOURCESTATE_VALID ||
            sg_query_buffer_state(mesh->gpu_resources->sg_index_buffer) != SG_RESOURCESTATE_VALID)) {
            printf("❌ Mesh '%s' has invalid GPU resources\n", mesh->name);
            meshes_ok = false;
        }
    }
    
    if (meshes_ok) {
        printf("✅ All GPU resources initialized successfully\n");
    } else {
        printf("❌ Some GPU resources failed to initialize\n");
    }
    
    return meshes_ok;
}

void mesh_get_gpu_buffers(const Mesh* mesh, void* out_vbuf, void* out_ibuf) {
    if (!mesh || !mesh->gpu_resources || !out_vbuf || !out_ibuf) {
        if (out_vbuf) *(sg_buffer*)out_vbuf = (sg_buffer){0};
        if (out_ibuf) *(sg_buffer*)out_ibuf = (sg_buffer){0};
        return;
    }
    
    *(sg_buffer*)out_vbuf = mesh->gpu_resources->sg_vertex_buffer;
    *(sg_buffer*)out_ibuf = mesh->gpu_resources->sg_index_buffer;
}
