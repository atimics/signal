#include "sokol_gfx.h"
#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// PIMPL Implementation: Define the opaque GpuResources struct here
// This is only visible in this compilation unit
struct GpuResources {
    sg_buffer vbuf;    // Vertex buffer
    sg_buffer ibuf;    // Index buffer  
    sg_image tex;      // Texture
};

// PIMPL Helper Functions
static struct GpuResources* gpu_resources_create(void);
static void gpu_resources_destroy(struct GpuResources* resources) __attribute__((unused));

static struct GpuResources* gpu_resources_create(void) {
    struct GpuResources* resources = malloc(sizeof(struct GpuResources));
    if (resources) {
        // Initialize with invalid handles
        resources->vbuf.id = SG_INVALID_ID;
        resources->ibuf.id = SG_INVALID_ID;
        resources->tex.id = SG_INVALID_ID;
    }
    return resources;
}

static void gpu_resources_destroy(struct GpuResources* resources) {
    if (resources) {
        // Destroy Sokol resources if they're valid
        if (resources->vbuf.id != SG_INVALID_ID) {
            sg_destroy_buffer(resources->vbuf);
        }
        if (resources->ibuf.id != SG_INVALID_ID) {
            sg_destroy_buffer(resources->ibuf);
        }
        if (resources->tex.id != SG_INVALID_ID) {
            sg_destroy_image(resources->tex);
        }
        free(resources);
    }
}

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
    
    printf("🔍 DEBUG parse_obj_file: Starting to parse file: %s\n", filepath);
    
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
    int line_number = 0;
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        if (strncmp(line, "usemtl ", 7) == 0) {
            sscanf(line, "usemtl %63s", mesh->material_name);
            printf("🔍 DEBUG: Found material: %s\n", mesh->material_name);
        } else if (strncmp(line, "v ", 2) == 0) {
            sscanf(line, "v %f %f %f", &temp_positions[pos_count].x, &temp_positions[pos_count].y, &temp_positions[pos_count].z);
            if (pos_count < 3) { // Only log first few
                printf("🔍 DEBUG: Vertex %d: %.2f %.2f %.2f\n", pos_count, 
                       temp_positions[pos_count].x, temp_positions[pos_count].y, temp_positions[pos_count].z);
            }
            pos_count++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            sscanf(line, "vn %f %f %f", &temp_normals[normal_count].x, &temp_normals[normal_count].y, &temp_normals[normal_count].z);
            normal_count++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            sscanf(line, "vt %f %f", &temp_tex_coords[tex_coord_count].u, &temp_tex_coords[tex_coord_count].v);
            tex_coord_count++;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (final_index_count < 10) { // Only log first few faces
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
                        .normal = temp_normals[vn[i] - 1],
                        .tex_coord = temp_tex_coords[vt[i] - 1]
                    };
                    final_vertices[final_vertex_count] = new_vertex;
                    final_indices[final_index_count++] = final_vertex_count;
                    final_vertex_count++;
                }
            } else {
                // Try parsing without normals: f v/vt v/vt v/vt
                parsed = sscanf(line, "f %d/%d %d/%d %d/%d", 
                               &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]);
                
                if (parsed == 6) {
                    if (final_index_count < 3) {
                        printf("🔍 DEBUG: Parsing face without normals - v/vt format\n");
                    }
                    for (int i = 0; i < 3; i++) {
                        Vertex new_vertex = {
                            .position = temp_positions[v[i] - 1],
                            .normal = {0.0f, 1.0f, 0.0f},  // Default normal pointing up
                            .tex_coord = temp_tex_coords[vt[i] - 1]
                        };
                        final_vertices[final_vertex_count] = new_vertex;
                        final_indices[final_index_count++] = final_vertex_count;
                        final_vertex_count++;
                    }
                } else {
                    // Try simple vertex-only format: f v v v
                    parsed = sscanf(line, "f %d %d %d", &v[0], &v[1], &v[2]);
                    if (parsed == 3) {
                        if (final_index_count < 3) {
                            printf("🔍 DEBUG: Parsing face with vertices only\n");
                        }
                        for (int i = 0; i < 3; i++) {
                            Vertex new_vertex = {
                                .position = temp_positions[v[i] - 1],
                                .normal = {0.0f, 1.0f, 0.0f},  // Default normal
                                .tex_coord = {0.0f, 0.0f}      // Default UV
                            };
                            final_vertices[final_vertex_count] = new_vertex;
                            final_indices[final_index_count++] = final_vertex_count;
                            final_vertex_count++;
                        }
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    printf("🔍 DEBUG parse_obj_file: Finished parsing\n");
    printf("   Positions: %d, Normals: %d, TexCoords: %d\n", pos_count, normal_count, tex_coord_count);
    printf("   Final vertices: %d, Final indices: %d\n", final_vertex_count, final_index_count);
    
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
        
        // Use existing GPU resources - direct assignment since we're in the same file
        renderable->gpu_resources->vbuf = mesh->sg_vertex_buffer;
        renderable->gpu_resources->ibuf = mesh->sg_index_buffer;
        renderable->index_count = mesh->index_count;
        renderable->visible = true;
        renderable->lod_distance = 100.0f;  // Default LOD distance
        renderable->lod_level = 0;
        
        // Try to find associated texture
        char texture_name[128];
        snprintf(texture_name, sizeof(texture_name), "%s_texture", mesh_name);
        Texture* texture = assets_get_texture(registry, texture_name);
        if (texture && texture->loaded) {
            renderable->gpu_resources->tex = texture->sg_image;
        } else {
            // Use invalid texture handle - shader should handle this
            renderable->gpu_resources->tex.id = SG_INVALID_ID;
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
