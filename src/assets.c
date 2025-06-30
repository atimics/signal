#include "sokol_gfx.h"
#include "graphics_api.h"
#include "assets.h"
#include "render_gpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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
    strncpy(registry->asset_root, asset_root, sizeof(registry->asset_root) - 1);
#endif
    
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
    
    printf("🔍 DEBUG parse_obj_file: Starting two-pass parsing of file: %s\n", filepath);
    
    // Initialize mesh data
    memset(mesh, 0, sizeof(Mesh));
    
    // ============================================================================
    // PASS 1: Count vertices, normals, tex coords, and faces
    // ============================================================================
    int pos_count = 0;
    int normal_count = 0;
    int tex_coord_count = 0;
    int face_count = 0;
    
    char line[256];
    int line_number = 0;
    
    printf("🔍 DEBUG: Pass 1 - Counting elements...\n");
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        if (strncmp(line, "usemtl ", 7) == 0) {
            sscanf(line, "usemtl %63s", mesh->material_name);
        } else if (strncmp(line, "v ", 2) == 0) {
            pos_count++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            normal_count++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            tex_coord_count++;
        } else if (strncmp(line, "f ", 2) == 0) {
            face_count++;
        }
    }
    
    printf("🔍 DEBUG: Pass 1 complete - Positions: %d, Normals: %d, TexCoords: %d, Faces: %d\n", 
           pos_count, normal_count, tex_coord_count, face_count);
    
    // Validate counts
    if (pos_count == 0 || face_count == 0) {
        printf("❌ Invalid mesh file: no vertices (%d) or faces (%d)\n", pos_count, face_count);
        fclose(file);
        return false;
    }
    
    // ============================================================================
    // PASS 2: Allocate exact memory needed
    // ============================================================================
    printf("🔍 DEBUG: Pass 2 - Allocating memory...\n");
    
    // Allocate temporary storage for parsing (exact sizes)
    Vector3* temp_positions = malloc(pos_count * sizeof(Vector3));
    Vector3* temp_normals = normal_count > 0 ? malloc(normal_count * sizeof(Vector3)) : NULL;
    Vector2* temp_tex_coords = tex_coord_count > 0 ? malloc(tex_coord_count * sizeof(Vector2)) : NULL;
    
    if (!temp_positions) {
        printf("❌ Failed to allocate memory for positions\n");
        fclose(file);
        return false;
    }
    
    // Each face creates 3 vertices and 3 indices (triangulated)
    int final_vertex_count = face_count * 3;
    int final_index_count = face_count * 3;
    
    // Allocate final mesh data
    mesh->vertices = malloc(final_vertex_count * sizeof(Vertex));
    mesh->indices = malloc(final_index_count * sizeof(int));
    
    if (!mesh->vertices || !mesh->indices) {
        printf("❌ Failed to allocate memory for final mesh data\n");
        free(temp_positions);
        if (temp_normals) free(temp_normals);
        if (temp_tex_coords) free(temp_tex_coords);
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->indices) free(mesh->indices);
        fclose(file);
        return false;
    }
    
    printf("🔍 DEBUG: Pass 2 complete - Allocated %d vertices, %d indices\n", 
           final_vertex_count, final_index_count);
    
    // ============================================================================
    // PASS 3: Rewind file and populate data
    // ============================================================================
    printf("🔍 DEBUG: Pass 3 - Parsing and populating data...\n");
    
    rewind(file);
    
    int current_pos = 0;
    int current_normal = 0;
    int current_tex_coord = 0;
    int current_vertex = 0;
    int current_index = 0;
    
    line_number = 0;
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        if (strncmp(line, "v ", 2) == 0) {
            if (current_pos < pos_count) {
                sscanf(line, "v %f %f %f", 
                       &temp_positions[current_pos].x, 
                       &temp_positions[current_pos].y, 
                       &temp_positions[current_pos].z);
                if (current_pos < 3) { // Only log first few
                    printf("🔍 DEBUG: Vertex %d: %.2f %.2f %.2f\n", current_pos, 
                           temp_positions[current_pos].x, temp_positions[current_pos].y, temp_positions[current_pos].z);
                }
                current_pos++;
            }
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (temp_normals && current_normal < normal_count) {
                sscanf(line, "vn %f %f %f", 
                       &temp_normals[current_normal].x, 
                       &temp_normals[current_normal].y, 
                       &temp_normals[current_normal].z);
                current_normal++;
            }
        } else if (strncmp(line, "vt ", 3) == 0) {
            if (temp_tex_coords && current_tex_coord < tex_coord_count) {
                sscanf(line, "vt %f %f", 
                       &temp_tex_coords[current_tex_coord].u, 
                       &temp_tex_coords[current_tex_coord].v);
                current_tex_coord++;
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            if (current_index < final_index_count - 2) { // Safety check
                if (current_index < 10) { // Only log first few faces
                    printf("🔍 DEBUG: Face line %d: %s", line_number, line);
                }
                
                int v[3], vt[3], vn[3];
                
                // Try parsing full format first: f v/vt/vn v/vt/vn v/vt/vn
                int parsed = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                                   &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]);
                
                if (parsed == 9) {
                    // Full format with normals
                    for (int i = 0; i < 3; i++) {
                        Vertex new_vertex = {
                            .position = temp_positions[v[i] - 1],
                            .normal = temp_normals ? temp_normals[vn[i] - 1] : (Vector3){0.0f, 1.0f, 0.0f},
                            .tex_coord = temp_tex_coords ? temp_tex_coords[vt[i] - 1] : (Vector2){0.0f, 0.0f}
                        };
                        mesh->vertices[current_vertex] = new_vertex;
                        mesh->indices[current_index++] = current_vertex;
                        current_vertex++;
                    }
                } else {
                    // Try parsing without normals: f v/vt v/vt v/vt
                    parsed = sscanf(line, "f %d/%d %d/%d %d/%d", 
                                   &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]);
                    
                    if (parsed == 6) {
                        if (current_index < 3) {
                            printf("🔍 DEBUG: Parsing face without normals - v/vt format\n");
                        }
                        for (int i = 0; i < 3; i++) {
                            Vertex new_vertex = {
                                .position = temp_positions[v[i] - 1],
                                .normal = {0.0f, 1.0f, 0.0f},  // Default normal pointing up
                                .tex_coord = temp_tex_coords ? temp_tex_coords[vt[i] - 1] : (Vector2){0.0f, 0.0f}
                            };
                            mesh->vertices[current_vertex] = new_vertex;
                            mesh->indices[current_index++] = current_vertex;
                            current_vertex++;
                        }
                    } else {
                        // Try simple vertex-only format: f v v v
                        parsed = sscanf(line, "f %d %d %d", &v[0], &v[1], &v[2]);
                        if (parsed == 3) {
                            if (current_index < 3) {
                                printf("🔍 DEBUG: Parsing face with vertices only\n");
                            }
                            for (int i = 0; i < 3; i++) {
                                Vertex new_vertex = {
                                    .position = temp_positions[v[i] - 1],
                                    .normal = {0.0f, 1.0f, 0.0f},  // Default normal
                                    .tex_coord = {0.0f, 0.0f}      // Default UV
                                };
                                mesh->vertices[current_vertex] = new_vertex;
                                mesh->indices[current_index++] = current_vertex;
                                current_vertex++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    // Free temporary storage
    free(temp_positions);
    if (temp_normals) free(temp_normals);
    if (temp_tex_coords) free(temp_tex_coords);
    
    // Set final counts
    mesh->vertex_count = current_vertex;
    mesh->index_count = current_index;
    mesh->loaded = true;
    
    printf("🔍 DEBUG parse_obj_file: Three-pass parsing complete\n");
    printf("   Final vertices: %d, Final indices: %d\n", mesh->vertex_count, mesh->index_count);
    printf("   Memory allocated: vertices=%p (heap), indices=%p (heap)\n", 
           (void*)mesh->vertices, (void*)mesh->indices);
    
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
    
    printf("🔍 DEBUG load_compiled_mesh: filename='%s', mesh_name='%s'\n", filename, mesh_name);
    
    // Build full path
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/meshes/%s", registry->asset_root, filename);
    
    printf("🔍 DEBUG load_compiled_mesh: Full filepath='%s'\n", filepath);
    
    // Check if file exists
    FILE* test_file = fopen(filepath, "r");
    if (!test_file) {
        printf("❌ DEBUG: Cannot open file '%s'\n", filepath);
        return false;
    }
    fclose(test_file);
    printf("✅ DEBUG: File exists and is readable\n");
    
    // Find or create mesh slot
    Mesh* mesh = &registry->meshes[registry->mesh_count];
    strncpy(mesh->name, mesh_name, sizeof(mesh->name) - 1);
    
    if (parse_obj_file(filepath, mesh)) {
        printf("✅ DEBUG: parse_obj_file succeeded - vertices=%d, indices=%d\n", 
               mesh->vertex_count, mesh->index_count);
        
        // Validate mesh data before creating buffers
        if (mesh->vertex_count == 0 || mesh->index_count == 0) {
            printf("❌ DEBUG: Mesh has zero vertices (%d) or indices (%d)\n", 
                   mesh->vertex_count, mesh->index_count);
            return false;
        }
        
        if (!mesh->vertices || !mesh->indices) {
            printf("❌ DEBUG: Mesh has NULL vertex (%p) or index (%p) data\n",
                   (void*)mesh->vertices, (void*)mesh->indices);
            return false;
        }
        
        // Calculate buffer sizes
        size_t vertex_buffer_size = mesh->vertex_count * sizeof(Vertex);
        size_t index_buffer_size = mesh->index_count * sizeof(int);
        
        printf("🔍 DEBUG: Buffer sizes - VB=%zu bytes, IB=%zu bytes\n", 
               vertex_buffer_size, index_buffer_size);
        
        if (vertex_buffer_size == 0 || index_buffer_size == 0) {
            printf("❌ DEBUG: Would create zero-sized buffers\n");
            return false;
        }
        
        // Create vertex buffer
        mesh->sg_vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = {
                .ptr = mesh->vertices,
                .size = mesh->vertex_count * sizeof(Vertex)
            },
            .usage = { .vertex_buffer = true },
            .label = mesh->name
        });

        // Create index buffer
        mesh->sg_index_buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = {
                .ptr = mesh->indices,
                .size = mesh->index_count * sizeof(int)
            },
            .usage = { .index_buffer = true },
            .label = mesh->name
        });

        mesh->loaded = true;  // Mark as successfully loaded
        registry->mesh_count++;
        return true;
    }
    
    return false;
}

// TASK 1.1: New simplified function that accepts absolute paths directly
bool load_compiled_mesh_absolute(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name) {
    if (!registry || !absolute_filepath || !mesh_name) return false;
    if (registry->mesh_count >= 32) return false;
    
    printf("🔍 DEBUG load_compiled_mesh_absolute: filepath='%s', mesh_name='%s'\n", absolute_filepath, mesh_name);
    
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
    
    if (parse_obj_file(absolute_filepath, mesh)) {
        printf("✅ DEBUG: parse_obj_file succeeded - vertices=%d, indices=%d\n", 
               mesh->vertex_count, mesh->index_count);
        
        // TASK 1.3: Add comprehensive validation
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
        
        // Create vertex buffer
        mesh->sg_vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = {
                .ptr = mesh->vertices,
                .size = vertex_buffer_size
            },
            .usage = { .vertex_buffer = true },
            .label = mesh->name
        });

        // Create index buffer
        mesh->sg_index_buffer = sg_make_buffer(&(sg_buffer_desc){
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
    
    printf("❌ Failed to parse mesh file: %s\n", absolute_filepath);
    return false;
}

bool load_texture(AssetRegistry* registry, const char* texture_path, const char* texture_name) {
    if (!registry || !texture_path || !texture_name) return false;
    if (registry->texture_count >= 32) return false;

    Texture* texture = &registry->textures[registry->texture_count];
    strncpy(texture->name, texture_name, sizeof(texture->name) - 1);
    strncpy(texture->filepath, texture_path, sizeof(texture->filepath) - 1);

    int width, height, channels;
    unsigned char* data = stbi_load(texture_path, &width, &height, &channels, 4);

    if (data) {
        texture->width = width;
        texture->height = height;
        texture->sg_image = sg_make_image(&(sg_image_desc){
            .width = width,
            .height = height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .data.subimage[0][0] = { .ptr = data, .size = (size_t)(width * height * 4) },
            .label = texture->name
        });
        stbi_image_free(data);
        texture->loaded = true;
        registry->texture_count++;
        return true;
    }

    return false;
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
// METADATA-DRIVEN ASSET LOADING
// ============================================================================

bool load_single_mesh_metadata(AssetRegistry* registry, const char* metadata_path) {
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
    char geometry_filename[128] = "";
    char texture_filename[128] = "";
    char material_filename[128] = "";
    
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
        
        // Parse geometry field
        if (strstr(trimmed, "\"geometry\":")) {
            char* value_start = strstr(trimmed, ":");
            if (value_start) {
                value_start++;
                while (*value_start == ' ' || *value_start == '\t') value_start++;
                if (*value_start == '"') {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        int len = value_end - value_start;
                        if (len > 0 && (size_t)len < sizeof(geometry_filename)) {
                            strncpy(geometry_filename, value_start, len);
                            geometry_filename[len] = 0;
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
        
        // Parse material field
        if (strstr(trimmed, "\"material\":")) {
            char* value_start = strstr(trimmed, ":");
            if (value_start) {
                value_start++;
                while (*value_start == ' ' || *value_start == '\t') value_start++;
                if (*value_start == '"') {
                    value_start++;
                    char* value_end = strchr(value_start, '"');
                    if (value_end) {
                        int len = value_end - value_start;
                        if (len > 0 && (size_t)len < sizeof(material_filename)) {
                            strncpy(material_filename, value_start, len);
                            material_filename[len] = 0;
                        }
                    }
                }
            }
        }
    }
    fclose(file);
    
    // Validate required fields
    if (strlen(mesh_name) == 0 || strlen(geometry_filename) == 0) {
        printf("❌ Invalid metadata: missing name or geometry\n");
        return false;
    }
    
    printf("🔍 DEBUG: Parsed metadata - name='%s', geometry='%s'\n", mesh_name, geometry_filename);
    
    // TASK 1.1: Simplified path resolution - construct full absolute path to .cobj file
    char mesh_path[512];
    snprintf(mesh_path, sizeof(mesh_path), "%s/%s", mesh_dir, geometry_filename);
    
    printf("🔍 DEBUG: Full mesh path: '%s'\n", mesh_path);
    
    // Load compiled mesh files (.cobj format from asset compiler)
    printf("🔍 DEBUG: Calling load_compiled_mesh_absolute with: registry=%p, path='%s', name='%s'\n", 
           (void*)registry, mesh_path, mesh_name);
    bool loaded = load_compiled_mesh_absolute(registry, mesh_path, mesh_name);
    
    if (!loaded) {
        printf("❌ Failed to load mesh: %s\n", mesh_path);
        return false;
    }
    
    // Load associated MTL file if specified in metadata
    if (strlen(material_filename) > 0) {
        char mtl_path[512];
        snprintf(mtl_path, sizeof(mtl_path), "%s/%s", mesh_dir, material_filename);
        parse_mtl_file(mtl_path, registry);
    }
    
    // Load associated texture if specified
    if (strlen(texture_filename) > 0) {
        char texture_path[512];
        char texture_name[128];
        snprintf(texture_path, sizeof(texture_path), "%s/%s", mesh_dir, texture_filename);
        snprintf(texture_name, sizeof(texture_name), "%s_texture", mesh_name);
        
        if (load_texture(registry, texture_path, texture_name)) {
            // Texture loaded successfully (no verbose logging)
        }
    }
    
    printf("✅ %s\n", mesh_name);
    
    return true;
}

bool load_assets_from_metadata(AssetRegistry* registry) {
    if (!registry) return false;
    
    // Load from index.json to get list of metadata files
    char index_path[512];
    snprintf(index_path, sizeof(index_path), "%s/meshes/index.json", registry->asset_root);
    
    FILE* file = fopen(index_path, "r");
    if (!file) {
        printf("⚠️  Could not open index.json: %s\n", index_path);
        printf("⚠️  Falling back to legacy metadata.json format\n");
        return load_legacy_metadata(registry);
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
                    if (load_single_mesh_metadata(registry, metadata_full_path)) {
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
bool load_legacy_metadata(AssetRegistry* registry) {
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
                               const char* texture_dir) {
    if (!registry || !material || !texture_dir) return false;
    
    char texture_path[512];
    bool any_loaded = false;
    
    // Try to load diffuse texture
    if (strlen(material->diffuse_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->diffuse_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_diffuse", material->name);
        if (load_texture(registry, texture_path, texture_name)) {
            any_loaded = true;
        }
    }
    
    // Try to load normal texture
    if (strlen(material->normal_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->normal_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_normal", material->name);
        if (load_texture(registry, texture_path, texture_name)) {
            any_loaded = true;
        }
    }
    
    // Try to load specular texture
    if (strlen(material->specular_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->specular_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_specular", material->name);
        if (load_texture(registry, texture_path, texture_name)) {
            any_loaded = true;
        }
    }
    
    // Try to load emission texture
    if (strlen(material->emission_texture) > 0) {
        snprintf(texture_path, sizeof(texture_path), "%s/%s", texture_dir, material->emission_texture);
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_emission", material->name);
        if (load_texture(registry, texture_path, texture_name)) {
            any_loaded = true;
        }
    }
    
    return any_loaded;
}

bool materials_bind_textures(Material* material) {
    if (!material) return false;
    
    // This function would bind multiple textures for multi-texture rendering
    // For now, it's a placeholder for future multi-texture support
    // In a real implementation, this would set up texture units for shaders
    
    return true;
}

// ============================================================================
// SHADER LOADING FUNCTIONS
// ============================================================================

char* load_shader_source(const char* shader_path) {
    if (!shader_path) {
        printf("❌ Shader path is null\n");
        return NULL;
    }
    
    FILE* file = fopen(shader_path, "r");
    if (!file) {
        printf("❌ Failed to open shader file: %s\n", shader_path);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size <= 0) {
        printf("❌ Shader file is empty or invalid: %s\n", shader_path);
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer and read file
    char* source = malloc(size + 1);
    if (!source) {
        printf("❌ Failed to allocate memory for shader source\n");
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(source, 1, size, file);
    source[bytes_read] = '\0';
    fclose(file);
    
    printf("✅ Loaded shader: %s (%ld bytes)\n", shader_path, bytes_read);
    return source;
}

void free_shader_source(char* source) {
    if (source) {
        free(source);
    }
}

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

// ============================================================================
// GPU RESOURCE CREATION FUNCTIONS
// ============================================================================

bool assets_create_renderable_from_mesh(AssetRegistry* registry, const char* mesh_name, struct Renderable* renderable) {
    if (!registry || !mesh_name || !renderable) return false;
    
    // Find the mesh
    Mesh* mesh = assets_get_mesh(registry, mesh_name);
    if (!mesh || !mesh->loaded) {
        printf("❌ Mesh '%s' not found or not loaded\n", mesh_name);
        return false;
    }
    
    // Check if GPU resources were already created during mesh loading
    if (sg_query_buffer_state(mesh->sg_vertex_buffer) == SG_RESOURCESTATE_VALID &&
        sg_query_buffer_state(mesh->sg_index_buffer) == SG_RESOURCESTATE_VALID) {
        
        // PIMPL: Allocate GPU resources struct
        renderable->gpu_resources = gpu_resources_create();
        if (!renderable->gpu_resources) {
            printf("❌ Failed to allocate GPU resources for mesh '%s'\n", mesh_name);
            return false;
        }
        
        // Use existing GPU resources through PIMPL interface
        gpu_resources_set_vertex_buffer(renderable->gpu_resources, mesh->sg_vertex_buffer);
        gpu_resources_set_index_buffer(renderable->gpu_resources, mesh->sg_index_buffer);
        renderable->index_count = mesh->index_count;
        renderable->visible = true;
        renderable->lod_distance = 100.0f;  // Default LOD distance
        renderable->lod_level = 0;
        
        // Try to find associated texture
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_texture", mesh_name);
        Texture* texture = assets_get_texture(registry, texture_name);
        if (texture && texture->loaded) {
            gpu_resources_set_texture(renderable->gpu_resources, texture->sg_image);
        } else {
            // Use invalid texture handle - shader should handle this
            sg_image invalid_tex = {SG_INVALID_ID};
            gpu_resources_set_texture(renderable->gpu_resources, invalid_tex);
        }
        
        printf("✅ Created Renderable from mesh '%s' (%d indices)\n", mesh_name, mesh->index_count);
        return true;
    }
    
    printf("❌ GPU resources not available for mesh '%s'\n", mesh_name);
    return false;
}

sg_image assets_create_gpu_texture(AssetRegistry* registry, const char* texture_name) {
    if (!registry || !texture_name) {
        return (sg_image){.id = SG_INVALID_ID};
    }
    
    Texture* texture = assets_get_texture(registry, texture_name);
    if (!texture || !texture->loaded) {
        printf("❌ Texture '%s' not found or not loaded\n", texture_name);
        return (sg_image){.id = SG_INVALID_ID};
    }
    
    return texture->sg_image;
}

// Helper function to create a default white texture for entities without textures
sg_image assets_create_default_texture(void) {
    // Create a 1x1 white pixel texture
    uint32_t white_pixel = 0xFFFFFFFF;  // RGBA white
    
    sg_image default_tex = sg_make_image(&(sg_image_desc){
        .width = 1,
        .height = 1,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = &white_pixel,
            .size = 4
        },
        .label = "default_white_texture"
    });
    
    return default_tex;
}

// ============================================================================
// GPU RESOURCE INITIALIZATION FUNCTIONS
// ============================================================================

// Batch load all textures to GPU
bool assets_load_all_textures_to_gpu(AssetRegistry* registry) {
    if (!registry) {
        printf("❌ Invalid registry for GPU texture loading\n");
        return false;
    }
    
    int success_count = 0;
    int total_count = registry->texture_count;
    
    printf("🎨 Loading %d textures to GPU...\n", total_count);
    
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        Texture* texture = &registry->textures[i];
        
        // Skip if already loaded to GPU
        if (texture->sg_image.id != SG_INVALID_ID) {
            success_count++;
            printf("✅ Texture '%s' already loaded to GPU\n", texture->name);
            continue;
        }
        
        // Skip if texture data not loaded
        if (!texture->loaded) {
            printf("⚠️  Texture '%s' data not loaded from file, skipping GPU upload\n", texture->name);
            continue;
        }
        
        // Load texture data to GPU directly (without reloading from file)
        int width, height, channels;
        unsigned char* data = stbi_load(texture->filepath, &width, &height, &channels, 4);
        
        if (data) {
            texture->sg_image = sg_make_image(&(sg_image_desc){
                .width = width,
                .height = height,
                .pixel_format = SG_PIXELFORMAT_RGBA8,
                .data.subimage[0][0] = { .ptr = data, .size = (size_t)(width * height * 4) },
                .label = texture->name
            });
            stbi_image_free(data);
            success_count++;
            printf("✅ Texture '%s' loaded to GPU\n", texture->name);
        } else {
            printf("❌ Failed to reload texture '%s' from file for GPU upload\n", texture->name);
        }
    }
    
    printf("🎨 GPU texture loading complete: %d/%d successful\n", success_count, total_count);
    return success_count == total_count;
}

// Initialize all GPU resources from loaded asset data
bool assets_initialize_gpu_resources(AssetRegistry* registry) {
    if (!registry) {
        printf("❌ Invalid registry for GPU resource initialization\n");
        return false;
    }
    
    printf("🎨 Initializing GPU resources...\n");
    
    // Load all textures to GPU
    bool textures_ok = assets_load_all_textures_to_gpu(registry);
    
    // Note: Meshes are already loaded to GPU during mesh loading
    // as they use sg_make_buffer calls in the mesh loading process
    
    bool meshes_ok = true; // Assume meshes are already loaded
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        if (mesh->loaded && (mesh->sg_vertex_buffer.id == SG_INVALID_ID || mesh->sg_index_buffer.id == SG_INVALID_ID)) {
            printf("⚠️  Mesh '%s' missing GPU buffers\n", mesh->name);
            meshes_ok = false;
        }
    }
    
    if (textures_ok && meshes_ok) {
        printf("✅ All GPU resources initialized successfully\n");
        return true;
    } else {
        printf("❌ Some GPU resources failed to initialize (textures: %s, meshes: %s)\n", 
               textures_ok ? "OK" : "FAILED", meshes_ok ? "OK" : "FAILED");
        return false;
    }
}

// ============================================================================
// PATH RESOLUTION FROM INDEX.JSON
// ============================================================================

bool assets_get_mesh_path_from_index(const char* index_path, const char* asset_name, char* out_path, size_t out_size) {
    if (!index_path || !asset_name || !out_path || out_size == 0) {
        return false;
    }
    
    FILE* file = fopen(index_path, "r");
    if (!file) {
        printf("❌ Failed to open index file: %s\n", index_path);
        return false;
    }
    
    // Read the entire file into memory for simple parsing
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        return false;
    }
    
    char* file_content = malloc(file_size + 1);
    if (!file_content) {
        fclose(file);
        return false;
    }
    
    size_t bytes_read = fread(file_content, 1, file_size, file);
    file_content[bytes_read] = '\0';
    fclose(file);
    
    // Check if this is the new format (object with assets) or old format (array)
    if (strstr(file_content, "\"assets\"")) {
        // New format: { "assets": { "asset_name": { "path": "..." } } }
        char search_pattern[128];
        snprintf(search_pattern, sizeof(search_pattern), "\"%s\"", asset_name);
        
        char* asset_location = strstr(file_content, search_pattern);
        if (!asset_location) {
            free(file_content);
            return false;
        }
        
        // Find the "path" field after the asset name
        char* path_start = strstr(asset_location, "\"path\"");
        if (!path_start) {
            free(file_content);
            return false;
        }
        
        // Find the colon after "path"
        char* colon = strchr(path_start, ':');
        if (!colon) {
            free(file_content);
            return false;
        }
        
        // Skip whitespace and find the opening quote
        char* quote1 = strchr(colon, '"');
        if (!quote1) {
            free(file_content);
            return false;
        }
        quote1++; // Move past the opening quote
        
        // Find the closing quote
        char* quote2 = strchr(quote1, '"');
        if (!quote2) {
            free(file_content);
            return false;
        }
        
        // Calculate the path length
        size_t path_length = quote2 - quote1;
        
        // Check if the output buffer is large enough
        if (path_length >= out_size) {
            free(file_content);
            return false;
        }
        
        // Copy the path to the output buffer
        strncpy(out_path, quote1, path_length);
        out_path[path_length] = '\0';
        
        free(file_content);
        return true;
    } else {
        // Old format: array of metadata paths - search for asset_name in the path
        char search_pattern[128];
        snprintf(search_pattern, sizeof(search_pattern), "props/%s/", asset_name);
        
        char* asset_location = strstr(file_content, search_pattern);
        if (!asset_location) {
            free(file_content);
            return false;
        }
        
        // For old format, construct the path as props/asset_name/geometry.cobj
        snprintf(out_path, out_size, "props/%s/geometry.cobj", asset_name);
        
        free(file_content);
        return true;
    }
}

// Upload mesh data to GPU with pre-emptive validation
bool assets_upload_mesh_to_gpu(Mesh* mesh) {
    if (!mesh) {
        printf("❌ assets_upload_mesh_to_gpu: NULL mesh pointer\n");
        return false;
    }
    
    // Validate mesh data before attempting GPU upload
    if (mesh->vertex_count == 0) {
        printf("❌ Mesh '%s' has zero vertices\n", mesh->name);
        return false;
    }
    
    if (mesh->vertices == NULL) {
        printf("❌ Mesh '%s' has NULL vertex data\n", mesh->name);
        return false;
    }
    
    if (mesh->index_count == 0) {
        printf("❌ Mesh '%s' has zero indices\n", mesh->name);
        return false;
    }
    
    if (mesh->indices == NULL) {
        printf("❌ Mesh '%s' has NULL index data\n", mesh->name);
        return false;
    }
    
    // Calculate buffer sizes for additional validation
    size_t vertex_buffer_size = mesh->vertex_count * sizeof(Vertex);
    size_t index_buffer_size = mesh->index_count * sizeof(int);
    
    if (vertex_buffer_size == 0 || index_buffer_size == 0) {
        printf("❌ Mesh '%s' would create zero-sized buffers: VB=%zu IB=%zu\n",
               mesh->name, vertex_buffer_size, index_buffer_size);
        return false;
    }
    
    printf("🔍 DEBUG: Uploading mesh '%s' to GPU - VB=%zu bytes, IB=%zu bytes\n", 
           mesh->name, vertex_buffer_size, index_buffer_size);
    
    // All validation passed - now create GPU buffers
    mesh->sg_vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = {
            .ptr = mesh->vertices,
            .size = vertex_buffer_size
        },
        .usage = { .vertex_buffer = true },
        .label = mesh->name
    });

    mesh->sg_index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = {
            .ptr = mesh->indices,
            .size = index_buffer_size
        },
        .usage = { .index_buffer = true },
        .label = mesh->name
    });

    // Post-creation validation: Ensure GPU buffers were created successfully
    if (sg_query_buffer_state(mesh->sg_vertex_buffer) != SG_RESOURCESTATE_VALID ||
        sg_query_buffer_state(mesh->sg_index_buffer) != SG_RESOURCESTATE_VALID) {
        printf("❌ Sokol failed to create buffers for mesh '%s'\n", mesh->name);
        return false;
    }

    // Free CPU-side memory after successful upload (Task 4 requirement)
    // This is a critical optimization to reduce memory footprint
    free(mesh->vertices);
    mesh->vertices = NULL;
    free(mesh->indices);
    mesh->indices = NULL;

    printf("✅ Mesh '%s' uploaded to GPU successfully\n", mesh->name);
    return true;
}
