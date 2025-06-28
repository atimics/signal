#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
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
        if (mesh->vertices) {
            free(mesh->vertices);
            mesh->vertices = NULL;
        }
        if (mesh->normals) {
            free(mesh->normals);
            mesh->normals = NULL;
        }
        if (mesh->indices) {
            free(mesh->indices);
            mesh->indices = NULL;
        }
    }
    
    printf("🎨 Asset system cleaned up\n");
}

// ============================================================================
// OBJ FILE PARSING
// ============================================================================

bool parse_obj_file(const char* filepath, Mesh* mesh) {
    if (!filepath || !mesh) return false;
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("❌ Failed to open OBJ file: %s\n", filepath);
        return false;
    }
    
    printf("📦 Loading OBJ: %s\n", filepath);
    
    // Temporary storage for parsing
    Vector3 temp_vertices[1024];
    Vector3 temp_normals[1024];
    uint32_t temp_indices[2048];
    
    uint32_t vertex_count = 0;
    uint32_t normal_count = 0;
    uint32_t index_count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        // Parse vertices
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                if (vertex_count < 1024) {
                    temp_vertices[vertex_count++] = (Vector3){x, y, z};
                }
            }
        }
        // Parse normals
        else if (strncmp(line, "vn ", 3) == 0) {
            float x, y, z;
            if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) {
                if (normal_count < 1024) {
                    temp_normals[normal_count++] = (Vector3){x, y, z};
                }
            }
        }
        // Parse faces (triangles only for now)
        else if (strncmp(line, "f ", 2) == 0) {
            int v1, v2, v3;
            // Simple format: f v1 v2 v3
            if (sscanf(line, "f %d %d %d", &v1, &v2, &v3) == 3) {
                if (index_count + 2 < 2048) {
                    // OBJ indices are 1-based, convert to 0-based
                    temp_indices[index_count++] = v1 - 1;
                    temp_indices[index_count++] = v2 - 1;
                    temp_indices[index_count++] = v3 - 1;
                }
            }
            // Format with texture coords: f v1/vt1 v2/vt2 v3/vt3
            else if (sscanf(line, "f %d/%*d %d/%*d %d/%*d", &v1, &v2, &v3) == 3) {
                if (index_count + 2 < 2048) {
                    temp_indices[index_count++] = v1 - 1;
                    temp_indices[index_count++] = v2 - 1;
                    temp_indices[index_count++] = v3 - 1;
                }
            }
            // Format with normals: f v1//vn1 v2//vn2 v3//vn3
            else if (sscanf(line, "f %d//%*d %d//%*d %d//%*d", &v1, &v2, &v3) == 3) {
                if (index_count + 2 < 2048) {
                    temp_indices[index_count++] = v1 - 1;
                    temp_indices[index_count++] = v2 - 1;
                    temp_indices[index_count++] = v3 - 1;
                }
            }
            // Full format: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            else if (sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &v1, &v2, &v3) == 3) {
                if (index_count + 2 < 2048) {
                    temp_indices[index_count++] = v1 - 1;
                    temp_indices[index_count++] = v2 - 1;
                    temp_indices[index_count++] = v3 - 1;
                }
            }
        }
    }
    
    fclose(file);
    
    // Allocate and copy data to mesh
    if (vertex_count > 0) {
        mesh->vertices = malloc(vertex_count * sizeof(Vector3));
        memcpy(mesh->vertices, temp_vertices, vertex_count * sizeof(Vector3));
        mesh->vertex_count = vertex_count;
    }
    
    if (normal_count > 0) {
        mesh->normals = malloc(normal_count * sizeof(Vector3));
        memcpy(mesh->normals, temp_normals, normal_count * sizeof(Vector3));
    }
    
    if (index_count > 0) {
        mesh->indices = malloc(index_count * sizeof(uint32_t));
        memcpy(mesh->indices, temp_indices, index_count * sizeof(uint32_t));
        mesh->index_count = index_count;
    }
    
    mesh->loaded = true;
    
    printf("   ✅ Loaded %d vertices, %d normals, %d indices\n", 
           vertex_count, normal_count, index_count);
    
    return true;
}

bool parse_mtl_file(const char* filepath, AssetRegistry* registry) {
    if (!filepath || !registry) return false;
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("⚠️  Could not open MTL file: %s\n", filepath);
        return false;
    }
    
    printf("🎨 Loading MTL: %s\n", filepath);
    
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
        // Shininess
        else if (strncmp(line, "Ns ", 3) == 0 && current_material) {
            sscanf(line, "Ns %f", &current_material->shininess);
        }
        // Texture map
        else if (strncmp(line, "map_Kd ", 7) == 0 && current_material) {
            sscanf(line, "map_Kd %63s", current_material->texture_name);
        }
    }
    
    fclose(file);
    printf("   ✅ Loaded %d materials\n", registry->material_count);
    return true;
}

// ============================================================================
// ASSET LOADING
// ============================================================================

bool load_obj_mesh(AssetRegistry* registry, const char* filename, const char* mesh_name) {
    if (!registry || !filename || !mesh_name) return false;
    if (registry->mesh_count >= 32) return false;
    
    // Build full path
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/meshes/%s", registry->asset_root, filename);
    
    // Find or create mesh slot
    Mesh* mesh = &registry->meshes[registry->mesh_count];
    strncpy(mesh->name, mesh_name, sizeof(mesh->name) - 1);
    
    if (parse_obj_file(filepath, mesh)) {
        registry->mesh_count++;
        
        // Try to load corresponding MTL file
        char mtl_path[512];
        char* dot = strrchr(filename, '.');
        if (dot) {
            size_t base_len = dot - filename;
            snprintf(mtl_path, sizeof(mtl_path), "%s/meshes/%.*s.mtl", 
                    registry->asset_root, (int)base_len, filename);
            parse_mtl_file(mtl_path, registry);
        }
        
        return true;
    }
    
    return false;
}

bool load_texture(AssetRegistry* registry, const char* filename, const char* texture_name) {
    if (!registry || !filename || !texture_name) return false;
    if (registry->texture_count >= 32) return false;
    
    // For now, just record the texture info
    Texture* texture = &registry->textures[registry->texture_count++];
    strncpy(texture->name, texture_name, sizeof(texture->name) - 1);
    snprintf(texture->filepath, sizeof(texture->filepath), "%s/meshes/%s", 
             registry->asset_root, filename);
    texture->loaded = true;  // Mark as loaded (even though we're not loading pixels yet)
    
    printf("📷 Registered texture: %s -> %s\n", texture_name, texture->filepath);
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

bool assets_load_all_in_directory(AssetRegistry* registry) {
    if (!registry) return false;
    
    printf("🔍 Auto-loading assets from %s/meshes/\n", registry->asset_root);
    
    // Load assets based on metadata.json
    return load_assets_from_metadata(registry);
}

// ============================================================================
// FALLBACK MESH GENERATION
// ============================================================================

void generate_fallback_meshes(AssetRegistry* registry) {
    if (!registry) return;
    
    printf("🔧 Generating fallback meshes...\n");
    
    // Generate fallback sun if not loaded
    if (!assets_get_mesh(registry, "sun")) {
        if (registry->mesh_count < 32) {
            Mesh* mesh = &registry->meshes[registry->mesh_count++];
            strncpy(mesh->name, "sun", sizeof(mesh->name) - 1);
            
            // Generate a simple icosphere for the sun
            int segments = 12;
            int rings = 6;
            mesh->vertex_count = (rings + 1) * (segments + 1);
            mesh->vertices = malloc(mesh->vertex_count * sizeof(Vector3));
            
            int vertex_index = 0;
            for (int ring = 0; ring <= rings; ring++) {
                float theta = (float)ring * M_PI / rings;
                float sin_theta = sinf(theta);
                float cos_theta = cosf(theta);
                
                for (int segment = 0; segment <= segments; segment++) {
                    float phi = (float)segment * 2.0f * M_PI / segments;
                    float sin_phi = sinf(phi);
                    float cos_phi = cosf(phi);
                    
                    mesh->vertices[vertex_index++] = (Vector3){
                        25.0f * sin_theta * cos_phi,
                        25.0f * cos_theta,
                        25.0f * sin_theta * sin_phi
                    };
                }
            }
            
            // Generate wireframe indices
            mesh->index_count = rings * segments * 4;
            mesh->indices = malloc(mesh->index_count * sizeof(uint32_t));
            
            int index = 0;
            for (int ring = 0; ring < rings; ring++) {
                for (int segment = 0; segment < segments; segment++) {
                    int current = ring * (segments + 1) + segment;
                    int next_ring = (ring + 1) * (segments + 1) + segment;
                    int next_segment = ring * (segments + 1) + ((segment + 1) % (segments + 1));
                    
                    mesh->indices[index++] = current;
                    mesh->indices[index++] = next_ring;
                    mesh->indices[index++] = current;
                    mesh->indices[index++] = next_segment;
                }
            }
            
            mesh->loaded = true;
            printf("   ✅ Generated fallback sun mesh\n");
        }
    }
    
    // Generate fallback player ship if not loaded
    if (!assets_get_mesh(registry, "player_ship")) {
        if (registry->mesh_count < 32) {
            Mesh* mesh = &registry->meshes[registry->mesh_count++];
            strncpy(mesh->name, "player_ship", sizeof(mesh->name) - 1);
            
            // Simple X-wing style ship
            mesh->vertex_count = 12;
            mesh->vertices = malloc(12 * sizeof(Vector3));
            
            mesh->vertices[0] = (Vector3){ 0.0f,  0.0f,  3.0f};  // Nose
            mesh->vertices[1] = (Vector3){-1.0f,  0.0f, -2.0f};  // Left rear
            mesh->vertices[2] = (Vector3){ 1.0f,  0.0f, -2.0f};  // Right rear
            mesh->vertices[3] = (Vector3){ 0.0f,  0.5f, -1.0f};  // Top mid
            mesh->vertices[4] = (Vector3){ 0.0f, -0.5f, -1.0f};  // Bottom mid
            mesh->vertices[5] = (Vector3){-3.0f,  0.0f,  0.0f};  // Left wing tip
            mesh->vertices[6] = (Vector3){ 3.0f,  0.0f,  0.0f};  // Right wing tip
            mesh->vertices[7] = (Vector3){-2.0f,  1.0f, -1.0f};  // Left wing top
            mesh->vertices[8] = (Vector3){ 2.0f,  1.0f, -1.0f};  // Right wing top
            mesh->vertices[9] = (Vector3){-2.0f, -1.0f, -1.0f};  // Left wing bottom
            mesh->vertices[10] = (Vector3){ 2.0f, -1.0f, -1.0f}; // Right wing bottom
            mesh->vertices[11] = (Vector3){ 0.0f,  0.0f, -3.0f}; // Tail
            
            mesh->index_count = 32;
            mesh->indices = malloc(32 * sizeof(uint32_t));
            
            uint32_t ship_edges[] = {
                0,1, 0,2, 0,3, 0,4,      // Nose to body
                1,2, 1,3, 1,4,           // Left connections
                2,3, 2,4,                // Right connections
                3,4, 1,11, 2,11,         // Body to tail
                1,5, 2,6,                // Body to wing tips
                5,7, 5,9,                // Left wing
                6,8, 6,10,               // Right wing
                7,9, 8,10                // Wing connections
            };
            
            memcpy(mesh->indices, ship_edges, 32 * sizeof(uint32_t));
            mesh->loaded = true;
            printf("   ✅ Generated fallback player ship mesh\n");
        }
    }
}

// ============================================================================
// METADATA-DRIVEN ASSET LOADING
// ============================================================================

bool load_assets_from_metadata(AssetRegistry* registry) {
    if (!registry) return false;
    
    char metadata_path[1024];
    snprintf(metadata_path, sizeof(metadata_path), "%s/meshes/metadata.json", registry->asset_root);
    
    FILE* file = fopen(metadata_path, "r");
    if (!file) {
        printf("⚠️  Could not open metadata.json: %s\n", metadata_path);
        return false;
    }
    
    printf("📋 Loading assets from metadata: %s\n", metadata_path);
    
    // Simple JSON parser for our specific metadata format
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
                        if (len < sizeof(current_folder)) {
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
        
        // Parse OBJ file names in meshes section
        if (in_meshes_section && strstr(trimmed, ".obj\":")) {
            char obj_name[128];
            char* start = strchr(trimmed, '"');
            if (start) {
                start++;
                char* end = strchr(start, '"');
                if (end) {
                    int len = end - start;
                    if (len < sizeof(obj_name)) {
                        strncpy(obj_name, start, len);
                        obj_name[len] = 0;
                        
                        // Construct full path and load mesh
                        char mesh_path[256];
                        snprintf(mesh_path, sizeof(mesh_path), "%s/%s", current_folder, obj_name);
                        
                        // Extract base name for registration (remove .obj extension)
                        char mesh_name[128];
                        strncpy(mesh_name, obj_name, sizeof(mesh_name) - 1);
                        char* dot = strrchr(mesh_name, '.');
                        if (dot) *dot = 0;
                        
                        printf("   Loading mesh: %s -> %s\n", mesh_path, mesh_name);
                        
                        if (!load_obj_mesh(registry, mesh_path, mesh_name)) {
                            printf("   ❌ Failed to load %s\n", mesh_path);
                            success = false;
                        } else {
                            printf("   ✅ Loaded %s\n", mesh_name);
                            
                            // Try to load associated texture
                            char texture_path[256];
                            char texture_name[128];
                            snprintf(texture_path, sizeof(texture_path), "%s/%s.png", current_folder, mesh_name);
                            snprintf(texture_name, sizeof(texture_name), "%s_texture", mesh_name);
                            
                            if (load_texture(registry, texture_path, texture_name)) {
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
