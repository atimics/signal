#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>

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
    strncpy(registry->asset_root, asset_root, sizeof(registry->asset_root) - 1);
    
    printf("🎨 Asset system initialized\n");
    printf("   Asset root: %s\n", registry->asset_root);
    
    return true;
}

void assets_cleanup(AssetRegistry* registry) {
    if (!registry) return;
    
    // Free all loaded meshes
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->indices) free(mesh->indices);
    }
    
    // Free all loaded textures
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        Texture* texture = &registry->textures[i];
        if (texture->sdl_texture) {
            SDL_DestroyTexture(texture->sdl_texture);
        }
    }
    
    printf("🎨 Asset system cleaned up\n");
}

// ============================================================================
// COMPILED OBJ FILE PARSING (.cobj files from asset compiler)
// ============================================================================

bool parse_obj_file(const char* filepath, Mesh* mesh) {
    if (!filepath || !mesh) return false;
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("❌ Failed to open compiled mesh file: %s\n", filepath);
        return false;
    }
    
    // Temporary storage for parsing
    Vector3 temp_positions[4096];
    Vector3 temp_normals[4096];
    Vector2 temp_tex_coords[4096];
    
    Vertex final_vertices[8192];
    int final_indices[16384];
    
    int pos_count = 0;
    int normal_count = 0;
    int tex_coord_count = 0;
    int final_vertex_count = 0;
    int final_index_count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        if (strncmp(line, "usemtl ", 7) == 0) {
            sscanf(line, "usemtl %63s", mesh->material_name);
        } else if (strncmp(line, "v ", 2) == 0) {
            sscanf(line, "v %f %f %f", &temp_positions[pos_count].x, &temp_positions[pos_count].y, &temp_positions[pos_count].z);
            pos_count++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            sscanf(line, "vn %f %f %f", &temp_normals[normal_count].x, &temp_normals[normal_count].y, &temp_normals[normal_count].z);
            normal_count++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            sscanf(line, "vt %f %f", &temp_tex_coords[tex_coord_count].u, &temp_tex_coords[tex_coord_count].v);
            tex_coord_count++;
        } else if (strncmp(line, "f ", 2) == 0) {
            int v[3], vt[3], vn[3];
            if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]) == 9) {
                for (int i = 0; i < 3; i++) {
                    // Create a final vertex from the indices
                    Vertex new_vertex = {
                        .position = temp_positions[v[i] - 1],
                        .normal = temp_normals[vn[i] - 1],
                        .tex_coord = temp_tex_coords[vt[i] - 1]
                    };
                    
                    // For simplicity, we are not de-duplicating vertices here.
                    // A more advanced implementation would use a hash map to find existing
                    // vertices and reuse indices.
                    final_vertices[final_vertex_count] = new_vertex;
                    final_indices[final_index_count++] = final_vertex_count;
                    final_vertex_count++;
                }
            }
        }
    }
    
    fclose(file);
    
    // Allocate and copy data to mesh
    mesh->vertices = malloc(final_vertex_count * sizeof(Vertex));
    memcpy(mesh->vertices, final_vertices, final_vertex_count * sizeof(Vertex));
    mesh->vertex_count = final_vertex_count;
    
    mesh->indices = malloc(final_index_count * sizeof(int));
    memcpy(mesh->indices, final_indices, final_index_count * sizeof(int));
    mesh->index_count = final_index_count;
    
    mesh->loaded = true;
    
    return true;
}


bool parse_mtl_file(const char* filepath, AssetRegistry* registry) {
    if (!filepath || !registry) return false;
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        // Don't log missing MTL files as errors - they're optional
        return false;
    }
    
    Material* current_material = NULL;
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        // New material
        if (strncmp(line, "newmtl ", 7) == 0) {
            if (registry->material_count < 32) {
                current_material = &registry->materials[registry->material_count++];
                memset(current_material, 0, sizeof(Material));
                sscanf(line, "newmtl %63s", current_material->name);
                current_material->diffuse_color = (Vector3){0.8f, 0.8f, 0.8f};  // Default gray
                current_material->loaded = true;
            }
        }
        // Diffuse color
        else if (strncmp(line, "Kd ", 3) == 0 && current_material) {
            sscanf(line, "Kd %f %f %f", 
                   &current_material->diffuse_color.x,
                   &current_material->diffuse_color.y,
                   &current_material->diffuse_color.z);
        }
        // Ambient color
        else if (strncmp(line, "Ka ", 3) == 0 && current_material) {
            sscanf(line, "Ka %f %f %f",
                   &current_material->ambient_color.x,
                   &current_material->ambient_color.y,
                   &current_material->ambient_color.z);
        }
        // Specular color
        else if (strncmp(line, "Ks ", 3) == 0 && current_material) {
            sscanf(line, "Ks %f %f %f",
                   &current_material->specular_color.x,
                   &current_material->specular_color.y,
                   &current_material->specular_color.z);
        }
        // Emission color
        else if (strncmp(line, "Ke ", 3) == 0 && current_material) {
            sscanf(line, "Ke %f %f %f",
                   &current_material->emission_color.x,
                   &current_material->emission_color.y,
                   &current_material->emission_color.z);
        }
        // Shininess
        else if (strncmp(line, "Ns ", 3) == 0 && current_material) {
            sscanf(line, "Ns %f", &current_material->shininess);
        }
        // Diffuse texture map (primary texture)
        else if (strncmp(line, "map_Kd ", 7) == 0 && current_material) {
            sscanf(line, "map_Kd %63s", current_material->diffuse_texture);
            // Also set legacy texture_name for compatibility
            strncpy(current_material->texture_name, current_material->diffuse_texture, 
                   sizeof(current_material->texture_name) - 1);
        }
        // Normal/bump map
        else if (strncmp(line, "map_Bump ", 9) == 0 && current_material) {
            sscanf(line, "map_Bump %63s", current_material->normal_texture);
        }
        else if (strncmp(line, "bump ", 5) == 0 && current_material) {
            sscanf(line, "bump %63s", current_material->normal_texture);
        }
        // Specular map
        else if (strncmp(line, "map_Ks ", 7) == 0 && current_material) {
            sscanf(line, "map_Ks %63s", current_material->specular_texture);
        }
        // Emission map
        else if (strncmp(line, "map_Ke ", 7) == 0 && current_material) {
            sscanf(line, "map_Ke %63s", current_material->emission_texture);
        }
    }
    
    fclose(file);
    return true;
}

// ============================================================================
// ASSET LOADING
// ============================================================================

bool load_compiled_mesh(AssetRegistry* registry, const char* filename, const char* mesh_name) {
    if (!registry || !filename || !mesh_name) return false;
    if (registry->mesh_count >= 32) return false;
    
    // Build full path - handle both .mesh and .cobj extensions
    char filepath[512];
    char actual_filename[256];
    
    // Convert .mesh extension to .cobj for compiled files
    if (strstr(filename, ".mesh")) {
        strncpy(actual_filename, filename, sizeof(actual_filename) - 1);
        char* dot = strrchr(actual_filename, '.');
        if (dot) {
            strcpy(dot, ".cobj");  // Replace .mesh with .cobj
        }
    } else if (!strstr(filename, ".cobj")) {
        // If no extension, assume it's a .cobj file
        snprintf(actual_filename, sizeof(actual_filename), "%s", filename);
        if (!strstr(actual_filename, ".cobj")) {
            strncat(actual_filename, ".cobj", sizeof(actual_filename) - strlen(actual_filename) - 1);
        }
    } else {
        // Already has .cobj extension
        strncpy(actual_filename, filename, sizeof(actual_filename) - 1);
    }
    
    snprintf(filepath, sizeof(filepath), "%s/meshes/%s", registry->asset_root, actual_filename);
    
    // Find or create mesh slot
    Mesh* mesh = &registry->meshes[registry->mesh_count];
    strncpy(mesh->name, mesh_name, sizeof(mesh->name) - 1);
    
    if (parse_obj_file(filepath, mesh)) {
        registry->mesh_count++;
        return true;
    }
    
    return false;
}

bool load_texture(AssetRegistry* registry, const char* filename, const char* texture_name, SDL_Renderer* renderer) {
    if (!registry || !filename || !texture_name || !renderer) return false;
    if (registry->texture_count >= 32) return false;
    
    Texture* texture = &registry->textures[registry->texture_count];
    strncpy(texture->name, texture_name, sizeof(texture->name) - 1);
    strncpy(texture->filepath, filename, sizeof(texture->filepath) - 1);
    
    // Load the actual texture using SDL_image
    SDL_Surface* surface = IMG_Load(texture->filepath);
    if (!surface) {
        printf("❌ Failed to load texture surface: %s (%s)\n", texture->filepath, IMG_GetError());
        return false;
    }
    
    // Create SDL texture from surface
    texture->sdl_texture = SDL_CreateTextureFromSurface(renderer, surface);
    texture->width = surface->w;
    texture->height = surface->h;
    
    // Free the surface
    SDL_FreeSurface(surface);
    
    if (!texture->sdl_texture) {
        printf("❌ Failed to create texture from surface: %s (%s)\n", texture->filepath, SDL_GetError());
        return false;
    }
    
    texture->loaded = true;
    registry->texture_count++;
    
    return true;
}

// ============================================================================
// ASSET LOOKUP
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

Material* assets_get_material(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (uint32_t i = 0; i < registry->material_count; i++) {
        if (strcmp(registry->materials[i].name, name) == 0) {
            return &registry->materials[i];
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

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void assets_list_loaded(AssetRegistry* registry) {
    if (!registry) return;
    
    printf("📦 Loaded Assets:\n");
    printf("   Meshes (%d):\n", registry->mesh_count);
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        printf("     - %s (%d verts, %d indices)\n", 
               mesh->name, mesh->vertex_count, mesh->index_count);
    }
    
    printf("   Materials (%d):\n", registry->material_count);
    for (uint32_t i = 0; i < registry->material_count; i++) {
        Material* mat = &registry->materials[i];
        printf("     - %s (%.2f, %.2f, %.2f)\n", 
               mat->name, mat->diffuse_color.x, mat->diffuse_color.y, mat->diffuse_color.z);
    }
    
    printf("   Textures (%d):\n", registry->texture_count);
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        Texture* tex = &registry->textures[i];
        printf("     - %s -> %s\n", tex->name, tex->filepath);
    }
}

bool assets_load_all_in_directory(AssetRegistry* registry, SDL_Renderer* renderer) {
    if (!registry || !renderer) return false;
    
    printf("🔍 Auto-loading assets from %s/meshes/\n", registry->asset_root);
    
    // Load assets based on metadata.json
    return load_assets_from_metadata(registry, renderer);
}

// ============================================================================
// METADATA-DRIVEN ASSET LOADING
// ============================================================================

bool load_single_mesh_metadata(AssetRegistry* registry, SDL_Renderer* renderer, const char* metadata_path) {
    FILE* file = fopen(metadata_path, "r");
    if (!file) {
        printf("⚠️  Could not open mesh metadata: %s\n", metadata_path);
        return false;
    }
    
    // Extract directory from metadata path for relative file loading
    char mesh_dir[512];
    strncpy(mesh_dir, metadata_path, sizeof(mesh_dir) - 1);
    char* last_slash = strrchr(mesh_dir, '/');
    if (last_slash) {
        *last_slash = '\0';  // Remove filename, keep directory
    }
    
    printf("📋 Loading mesh from metadata: %s\n", metadata_path);
    
    // Parse metadata.json for mesh information
    char line[512];
    char mesh_name[128] = "";
    char mesh_filename[128] = "";
    char texture_filename[128] = "";
    char mtl_filename[128] = "";
    
    while (fgets(line, sizeof(line), file)) {
        // Remove whitespace and newlines
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        trimmed[strcspn(trimmed, "\n\r")] = 0;
        
        // Skip empty lines and comments
        if (strlen(trimmed) == 0 || trimmed[0] == '/' || trimmed[0] == '#') continue;
        
        // Parse name field
        if (strstr(trimmed, "\"name\":")) {
            char* value_start = strstr(trimmed, ":");
            if (value_start) {
                value_start++;
                while (*value_start == ' ' || *value_start == '\t') value_start++;
                if (*value_start == '"') {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        int len = value_end - value_start;
                        if (len > 0 && (size_t)len < sizeof(mesh_name)) {
                            strncpy(mesh_name, value_start, len);
                            mesh_name[len] = 0;
                        }
                    }
                }
            }
        }
        
        // Parse filename field
        if (strstr(trimmed, "\"filename\":")) {
            char* value_start = strstr(trimmed, ":");
            if (value_start) {
                value_start++;
                while (*value_start == ' ' || *value_start == '\t') value_start++;
                if (*value_start == '"') {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        int len = value_end - value_start;
                        if (len > 0 && (size_t)len < sizeof(mesh_filename)) {
                            strncpy(mesh_filename, value_start, len);
                            mesh_filename[len] = 0;
                        }
                    }
                }
            }
        }
        
        // Parse texture field
        if (strstr(trimmed, "\"texture\":")) {
            char* value_start = strstr(trimmed, ":");
            if (value_start) {
                value_start++;
                while (*value_start == ' ' || *value_start == '\t') value_start++;
                if (*value_start == '"') {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        int len = value_end - value_start;
                        if (len > 0 && (size_t)len < sizeof(texture_filename)) {
                            strncpy(texture_filename, value_start, len);
                            texture_filename[len] = 0;
                        }
                    }
                }
            }
        }
        
        // Parse mtl field
        if (strstr(trimmed, "\"mtl\":")) {
            char* value_start = strstr(trimmed, ":");
            if (value_start) {
                value_start++;
                while (*value_start == ' ' || *value_start == '\t') value_start++;
                if (*value_start == '"') {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        int len = value_end - value_start;
                        if (len > 0 && (size_t)len < sizeof(mtl_filename)) {
                            strncpy(mtl_filename, value_start, len);
                            mtl_filename[len] = 0;
                        }
                    }
                }
            }
        }
    }
    fclose(file);
    
    // Validate required fields
    if (strlen(mesh_name) == 0 || strlen(mesh_filename) == 0) {
        printf("❌ Invalid metadata: missing name or filename\n");
        return false;
    }
    
    // Load the mesh file
    char mesh_path[512];
    snprintf(mesh_path, sizeof(mesh_path), "%s/%s", mesh_dir, mesh_filename);
    
    // Convert absolute path to relative path from meshes directory
    char relative_mesh_path[512];
    char meshes_prefix[512];
    snprintf(meshes_prefix, sizeof(meshes_prefix), "%s/meshes/", registry->asset_root);
    
    if (strncmp(mesh_path, meshes_prefix, strlen(meshes_prefix)) == 0) {
        // Path starts with asset_root/meshes/, extract the relative part
        strcpy(relative_mesh_path, mesh_path + strlen(meshes_prefix));
    } else {
        // Path doesn't start with expected prefix, use as-is
        strcpy(relative_mesh_path, mesh_path);
    }
    
    // Load compiled mesh files (.cobj format from asset compiler)
    bool loaded = load_compiled_mesh(registry, relative_mesh_path, mesh_name);
    
    if (!loaded) {
        printf("❌ Failed to load mesh: %s\n", mesh_path);
        return false;
    }
    
    // Load associated MTL file if specified in metadata
    if (strlen(mtl_filename) > 0) {
        char mtl_path[512];
        snprintf(mtl_path, sizeof(mtl_path), "%s/%s", mesh_dir, mtl_filename);
        parse_mtl_file(mtl_path, registry);
    }
    
    // Load associated texture if specified
    if (strlen(texture_filename) > 0) {
        char texture_path[512];
        char texture_name[128];
        snprintf(texture_path, sizeof(texture_path), "%s/%s", mesh_dir, texture_filename);
        snprintf(texture_name, sizeof(texture_name), "%s_texture", mesh_name);
        
        if (load_texture(registry, texture_path, texture_name, renderer)) {
            // Texture loaded successfully (no verbose logging)
        }
    }
    
    printf("✅ %s\n", mesh_name);
    
    return true;
}

bool load_assets_from_metadata(AssetRegistry* registry, SDL_Renderer* renderer) {
    if (!registry || !renderer) return false;
    
    // Load from index.json to get list of metadata files
    char index_path[512];
    snprintf(index_path, sizeof(index_path), "%s/meshes/index.json", registry->asset_root);
    
    FILE* file = fopen(index_path, "r");
    if (!file) {
        printf("⚠️  Could not open index.json: %s\n", index_path);
        printf("⚠️  Falling back to legacy metadata.json format\n");
        return load_legacy_metadata(registry, renderer);
    }
    
    printf("📋 Loading asset index: %s\n", index_path);
    
    char line[512];
    bool success = true;
    int loaded_count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove whitespace and newlines
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        trimmed[strcspn(trimmed, "\n\r")] = 0;
        
        // Skip empty lines, comments, and JSON syntax
        if (strlen(trimmed) == 0 || trimmed[0] == '/' || trimmed[0] == '#' ||
            trimmed[0] == '[' || trimmed[0] == ']' || strcmp(trimmed, "{") == 0 || strcmp(trimmed, "}") == 0) {
            continue;
        }
        
        // Extract metadata path from JSON array entry
        char* start = strchr(trimmed, '"');
        if (start) {
            start++;
            char* end = strchr(start, '"');
            if (end) {
                // Extract the metadata path
                char metadata_relative[256];
                int len = end - start;
                if (len > 0 && (size_t)len < sizeof(metadata_relative)) {
                    strncpy(metadata_relative, start, len);
                    metadata_relative[len] = 0;
                    
                    // Build full path
                    char metadata_full_path[512];
                    snprintf(metadata_full_path, sizeof(metadata_full_path), "%s/meshes/%s", 
                             registry->asset_root, metadata_relative);
                    
                    // Load this mesh
                    if (load_single_mesh_metadata(registry, renderer, metadata_full_path)) {
                        loaded_count++;
                    } else {
                        success = false;
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    printf("📋 Loaded %d meshes from asset index\n", loaded_count);
    return success;
}

// Legacy metadata loading for backward compatibility
bool load_legacy_metadata(AssetRegistry* registry, SDL_Renderer* renderer) {
    char metadata_path[1024];
    snprintf(metadata_path, sizeof(metadata_path), "%s/meshes/metadata.json", registry->asset_root);
    
    FILE* file = fopen(metadata_path, "r");
    if (!file) {
        printf("⚠️  Could not open legacy metadata.json: %s\n", metadata_path);
        return false;
    }
    
    printf("📋 Loading assets from legacy metadata: %s\n", metadata_path);
    
    // Simple JSON parser for legacy metadata format
    char line[512];
    char current_folder[128] = "";
    bool in_meshes_section = false;
    bool success = true;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove whitespace and newlines
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        trimmed[strcspn(trimmed, "\n\r")] = 0;
        
        // Skip empty lines and comments
        if (strlen(trimmed) == 0 || trimmed[0] == '/' || trimmed[0] == '#') continue;
        
        // Parse folder name
        if (strstr(trimmed, "\"folder\":")) {
            // Find the value after "folder":
            char* folder_start = strstr(trimmed, "\"folder\":");
            if (folder_start) {
                folder_start += 9;  // Skip "folder":
                // Skip whitespace and find opening quote
                while (*folder_start == ' ' || *folder_start == '\t') folder_start++;
                if (*folder_start == '"') {
                    folder_start++;  // Skip opening quote
                    char* folder_end = strchr(folder_start, '"');
                    if (folder_end) {
                        int len = folder_end - folder_start;
                        if (len > 0 && (size_t)len < sizeof(current_folder)) {
                            strncpy(current_folder, folder_start, len);
                            current_folder[len] = 0;
                        }
                    }
                }
            }
            continue;
        }
        
        // Detect meshes section
        if (strstr(trimmed, "\"meshes\":")) {
            in_meshes_section = true;
            continue;
        }
        
        // Parse compiled mesh file names in meshes section
        if (in_meshes_section && strstr(trimmed, ".cobj\":")) {
            char obj_name[128];
            char* start = strchr(trimmed, '"');
            if (start) {
                start++;
                char* end = strchr(start, '"');
                if (end) {
                    int len = end - start;
                    if (len > 0 && (size_t)len < sizeof(obj_name)) {
                        strncpy(obj_name, start, len);
                        obj_name[len] = 0;
                        
                        // Construct full path and load mesh
                        char mesh_path[256];
                        snprintf(mesh_path, sizeof(mesh_path), "%s/%s", current_folder, obj_name);
                        
                        // Extract base name for registration (remove .cobj extension)
                        char mesh_name[128];
                        strncpy(mesh_name, obj_name, sizeof(mesh_name) - 1);
                        char* dot = strrchr(mesh_name, '.');
                        if (dot) *dot = 0;
                        
                        printf("   Loading mesh: %s -> %s\n", mesh_path, mesh_name);
                        
                        if (!load_compiled_mesh(registry, mesh_path, mesh_name)) {
                            printf("   ❌ Failed to load %s\n", mesh_path);
                            success = false;
                        } else {
                            printf("   ✅ Loaded %s\n", mesh_name);
                            
                            // Try to load associated texture
                            char texture_path[256];
                            char texture_name[128];
                            snprintf(texture_path, sizeof(texture_path), "%s/%s.png", current_folder, mesh_name);
                            snprintf(texture_name, sizeof(texture_name), "%s_texture", mesh_name);
                            
                            if (load_texture(registry, texture_path, texture_name, renderer)) {
                                printf("   ✅ Loaded texture: %s\n", texture_name);
                            }
                        }
                    }
                }
            }
        }
        
        // End of meshes section
        if (in_meshes_section && trimmed[0] == '}') {
            in_meshes_section = false;
        }
    }
    
    fclose(file);
    return success;
}

// ============================================================================
// MATERIAL REPOSITORY IMPLEMENTATION
// ============================================================================

bool materials_load_library(AssetRegistry* registry, const char* materials_dir, SDL_Renderer* renderer) {
    if (!registry || !materials_dir || !renderer) return false;
    
    // TODO: Implement loading of shared material library
    // This would scan materials_dir for .mtl files and load them
    // as reusable materials independent of specific meshes
    printf("📚 Material library loading not yet implemented\n");
    return true;
}

Material* materials_get_by_name(AssetRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (uint32_t i = 0; i < registry->material_count; i++) {
        if (strcmp(registry->materials[i].name, name) == 0) {
            return &registry->materials[i];
        }
    }
    return NULL;
}

bool materials_create_variant(AssetRegistry* registry, const char* base_name, const char* variant_name,
                             Vector3 color_tint, float roughness_modifier) {
    if (!registry || !base_name || !variant_name) return false;
    if (registry->material_count >= 32) return false;
    
    // Find base material
    Material* base_material = materials_get_by_name(registry, base_name);
    if (!base_material) {
        printf("⚠️  Base material '%s' not found for variant creation\n", base_name);
        return false;
    }
    
    // Create variant by copying base and modifying properties
    Material* variant = &registry->materials[registry->material_count];
    *variant = *base_material;  // Copy all properties
    
    // Update variant-specific properties
    strncpy(variant->name, variant_name, sizeof(variant->name) - 1);
    variant->diffuse_color.x *= color_tint.x;
    variant->diffuse_color.y *= color_tint.y;
    variant->diffuse_color.z *= color_tint.z;
    variant->roughness = fmaxf(0.0f, fminf(1.0f, variant->roughness + roughness_modifier));
    
    registry->material_count++;
    printf("🎨 Created material variant: %s (from %s)\n", variant_name, base_name);
    return true;
}

void materials_list_loaded(AssetRegistry* registry) {
    if (!registry) return;
    
    printf("🎨 Material Repository (%d materials):\n", registry->material_count);
    for (uint32_t i = 0; i < registry->material_count; i++) {
        Material* mat = &registry->materials[i];
        printf("   - %s: diffuse(%.2f,%.2f,%.2f) roughness=%.2f metallic=%.2f\n", 
               mat->name, 
               mat->diffuse_color.x, mat->diffuse_color.y, mat->diffuse_color.z,
               mat->roughness, mat->metallic);
        
        // List associated textures
        if (strlen(mat->diffuse_texture) > 0) {
            printf("     └ Diffuse: %s", mat->diffuse_texture);
        }
        if (strlen(mat->normal_texture) > 0) {
            printf(" | Normal: %s", mat->normal_texture);
        }
        if (strlen(mat->specular_texture) > 0) {
            printf(" | Specular: %s", mat->specular_texture);
        }
        if (strlen(mat->emission_texture) > 0) {
            printf(" | Emission: %s", mat->emission_texture);
        }
        if (strlen(mat->diffuse_texture) > 0 || strlen(mat->normal_texture) > 0 || 
            strlen(mat->specular_texture) > 0 || strlen(mat->emission_texture) > 0) {
            printf("\n");
        }
    }
}

bool materials_load_texture_set(AssetRegistry* registry, Material* material, 
                               const char* texture_dir, SDL_Renderer* renderer) {
    if (!registry || !material || !texture_dir || !renderer) return false;
    
    char texture_path[512];
    bool any_loaded = false;
    
    // Try to load diffuse texture
    if (strlen(material->diffuse_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->diffuse_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_diffuse", material->name);
        if (load_texture(registry, texture_path, texture_name, renderer)) {
            any_loaded = true;
        }
    }
    
    // Try to load normal texture
    if (strlen(material->normal_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->normal_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_normal", material->name);
        if (load_texture(registry, texture_path, texture_name, renderer)) {
            any_loaded = true;
        }
    }
    
    // Try to load specular texture
    if (strlen(material->specular_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->specular_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_specular", material->name);
        if (load_texture(registry, texture_path, texture_name, renderer)) {
            any_loaded = true;
        }
    }
    
    // Try to load emission texture
    if (strlen(material->emission_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->emission_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_emission", material->name);
        if (load_texture(registry, texture_path, texture_name, renderer)) {
            any_loaded = true;
        }
    }
    
    return any_loaded;
}

bool materials_bind_textures(Material* material, SDL_Renderer* renderer) {
    if (!material || !renderer) return false;
    
    // This function would bind multiple textures for multi-texture rendering
    // For now, it's a placeholder for future multi-texture support
    // In a real implementation, this would set up texture units for shaders
    
    return true;
}
