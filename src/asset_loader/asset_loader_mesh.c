#include "../assets.h"
#include "../gpu_resources.h"
#include "../sokol_gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Define the actual implementation of our opaque structs
struct MeshGpuResources {
    sg_buffer sg_vertex_buffer;
    sg_buffer sg_index_buffer;
};

// Forward declarations
static void cleanup_mesh_on_error(Mesh* mesh);

/**
 * @brief Creates GPU resources for a loaded mesh
 * @param mesh The mesh to create GPU resources for
 * @return true if GPU resources were created successfully, false otherwise
 */
static bool create_mesh_gpu_resources(Mesh* mesh) {
#ifdef CGAME_TESTING
    // Skip GPU resource creation in test mode
    (void)mesh; // Suppress unused parameter warning in test builds
    return true;
#else
    if (!mesh || !mesh->vertices || !mesh->indices) {
        printf("❌ Invalid mesh data for GPU resource creation\n");
        return false;
    }
    
    // Validate buffer sizes
    size_t vertex_buffer_size = mesh->vertex_count * sizeof(Vertex);
    size_t index_buffer_size = mesh->index_count * sizeof(int);
    
    if (vertex_buffer_size == 0 || index_buffer_size == 0) {
        printf("❌ Mesh %s would create zero-sized buffers: VB=%zu IB=%zu\n",
               mesh->name, vertex_buffer_size, index_buffer_size);
        return false;
    }
    
    printf("🔍 DEBUG: Creating GPU buffers for %s - VB=%zu bytes, IB=%zu bytes\n", 
           mesh->name, vertex_buffer_size, index_buffer_size);
    
    // Allocate memory for our opaque struct
    mesh->gpu_resources = calloc(1, sizeof(struct MeshGpuResources));
    if (!mesh->gpu_resources) {
        printf("❌ Failed to allocate GPU resources for mesh %s\n", mesh->name);
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
    
    // Validate that buffers were created
    if (mesh->gpu_resources->sg_vertex_buffer.id == SG_INVALID_ID ||
        mesh->gpu_resources->sg_index_buffer.id == SG_INVALID_ID) {
        printf("❌ Failed to create GPU buffers for mesh %s\n", mesh->name);
        if (mesh->gpu_resources->sg_vertex_buffer.id != SG_INVALID_ID) {
            sg_destroy_buffer(mesh->gpu_resources->sg_vertex_buffer);
        }
        if (mesh->gpu_resources->sg_index_buffer.id != SG_INVALID_ID) {
            sg_destroy_buffer(mesh->gpu_resources->sg_index_buffer);
        }
        free(mesh->gpu_resources);
        mesh->gpu_resources = NULL;
        return false;
    }
    
    printf("✅ GPU resources created successfully for mesh %s\n", mesh->name);
    return true;
#endif
}

bool parse_obj_file(const char* filepath, Mesh* mesh) {
    if (!filepath || !mesh) return false;
    
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("❌ Failed to open compiled mesh file: %s\n", filepath);
        return false;
    }
    
    printf("🔍 DEBUG parse_obj_file: Starting two-pass parsing of file: %s\n", filepath);
    
    // Initialize mesh data - preserve the name field that was already set
    char preserved_name[64];
    if (mesh->name[0] != '\0') {
        strncpy(preserved_name, mesh->name, sizeof(preserved_name) - 1);
        preserved_name[sizeof(preserved_name) - 1] = '\0';
    } else {
        preserved_name[0] = '\0';
    }
    
    memset(mesh, 0, sizeof(Mesh));
    
    // Restore the preserved name
    if (preserved_name[0] != '\0') {
        strncpy(mesh->name, preserved_name, sizeof(mesh->name) - 1);
        mesh->name[sizeof(mesh->name) - 1] = '\0';
    }
    
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

/**
 * @brief Load a binary .cobj mesh file according to R15 specification
 * @param registry The asset registry to load into
 * @param absolute_filepath The path to the binary .cobj file
 * @param mesh_name The name to assign to the loaded mesh
 * @return true if the binary mesh was loaded successfully, false otherwise
 */
bool load_cobj_binary(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name) {
    if (!registry || !absolute_filepath || !mesh_name) {
        printf("❌ Invalid parameters for binary mesh loading\n");
        return false;
    }
    
    // Find next available slot
    int slot = -1;
    for (int i = 0; i < MAX_MESHES; i++) {
        if (!registry->meshes[i].loaded) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf("❌ No available mesh slots for binary mesh\n");
        return false;
    }
    
    // Clear the mesh and set name
    Mesh* mesh = &registry->meshes[slot];
    memset(mesh, 0, sizeof(Mesh));
    strncpy(mesh->name, mesh_name, sizeof(mesh->name) - 1);
    mesh->name[sizeof(mesh->name) - 1] = '\0';
    
    FILE* file = fopen(absolute_filepath, "rb");
    if (!file) {
        printf("❌ Could not open binary mesh file: %s\n", absolute_filepath);
        return false;
    }
    
    // Read and validate the header
    COBJHeader header;
    if (fread(&header, sizeof(COBJHeader), 1, file) != 1) {
        printf("❌ Failed to read header from binary mesh file: %s\n", absolute_filepath);
        fclose(file);
        return false;
    }
    
    // Validate the magic number
    if (strncmp(header.magic, "CGMF", 4) != 0) {
        printf("❌ Invalid magic number in binary mesh file: %s\n", absolute_filepath);
        fclose(file);
        return false;
    }
    
    // Check version compatibility
    if (header.version != 1) {
        printf("❌ Unsupported binary mesh version %u in file: %s\n", header.version, absolute_filepath);
        fclose(file);
        return false;
    }
    
    printf("🔍 Loading binary mesh: %u vertices, %u indices\n", 
           header.vertex_count, header.index_count);
    
    // Allocate memory for vertices (using VertexEnhanced for binary format)
    mesh->vertex_count = header.vertex_count;
    mesh->vertices = calloc(header.vertex_count, sizeof(Vertex));
    if (!mesh->vertices) {
        printf("❌ Failed to allocate memory for %u vertices\n", header.vertex_count);
        fclose(file);
        return false;
    }
    
    // Allocate memory for indices
    mesh->index_count = header.index_count;
    mesh->indices = calloc(header.index_count, sizeof(int));
    if (!mesh->indices) {
        printf("❌ Failed to allocate memory for %u indices\n", header.index_count);
        cleanup_mesh_on_error(mesh);
        fclose(file);
        return false;
    }
    
    // Read vertex data directly into the Vertex struct (32 bytes each)
    // The binary format now matches exactly with the engine's Vertex struct
    if (fread(mesh->vertices, sizeof(Vertex), header.vertex_count, file) != header.vertex_count) {
        printf("❌ Failed to read vertex data from binary mesh file: %s\n", absolute_filepath);
        cleanup_mesh_on_error(mesh);
        fclose(file);
        return false;
    }
    
    // Read index data directly into memory
    uint32_t* temp_indices = calloc(header.index_count, sizeof(uint32_t));
    if (!temp_indices) {
        printf("❌ Failed to allocate temporary index buffer\n");
        cleanup_mesh_on_error(mesh);
        fclose(file);
        return false;
    }
    
    if (fread(temp_indices, sizeof(uint32_t), header.index_count, file) != header.index_count) {
        printf("❌ Failed to read index data from binary mesh file: %s\n", absolute_filepath);
        free(temp_indices);
        cleanup_mesh_on_error(mesh);
        fclose(file);
        return false;
    }
    
    // Convert uint32_t indices to int (current mesh format)
    for (uint32_t i = 0; i < header.index_count; i++) {
        mesh->indices[i] = (int)temp_indices[i];
    }
    
    free(temp_indices);
    fclose(file);
    
    // Store AABB data from header
    mesh->aabb_min = header.aabb_min;
    mesh->aabb_max = header.aabb_max;
    
    // Store AABB data from header
    mesh->aabb_min = header.aabb_min;
    mesh->aabb_max = header.aabb_max;
    
    // Create GPU resources for the loaded mesh
    if (!create_mesh_gpu_resources(mesh)) {
        printf("❌ Failed to create GPU resources for binary mesh: %s\n", mesh_name);
        // Clean up mesh data
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->indices) free(mesh->indices);
        memset(mesh, 0, sizeof(Mesh));
        return false;
    }
    
    mesh->loaded = true;
    registry->mesh_count++;
    
    printf("✅ Loaded binary mesh: %s (%d vertices, %d indices) with AABB\n", 
           mesh->name, mesh->vertex_count, mesh->index_count);
    
    return true;
}

bool load_mesh_from_file(AssetRegistry* registry, const char* absolute_filepath, const char* mesh_name) {
    if (!registry || !absolute_filepath || !mesh_name) return false;
    
    printf("🔍 DEBUG load_mesh_from_file: filepath='%s', mesh_name='%s'\n", absolute_filepath, mesh_name);
    
    // Find next available slot
    int slot = -1;
    for (int i = 0; i < MAX_MESHES; i++) {
        if (!registry->meshes[i].loaded) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf("❌ No available mesh slots\n");
        return false;
    }
    
    // Clear the mesh and set name
    Mesh* mesh = &registry->meshes[slot];
    memset(mesh, 0, sizeof(Mesh));
    strncpy(mesh->name, mesh_name, sizeof(mesh->name) - 1);
    mesh->name[sizeof(mesh->name) - 1] = '\0';
    
    // Detect file format and parse accordingly
    // Binary .cobj files start with "CGMF" magic number
    // Text .cobj files (legacy) start with comments or vertex definitions
    FILE* format_check_file = fopen(absolute_filepath, "rb");
    bool is_binary = false;
    
    if (format_check_file) {
        char magic[4];
        if (fread(magic, 1, 4, format_check_file) == 4) {
            is_binary = (strncmp(magic, "CGMF", 4) == 0);
        }
        fclose(format_check_file);
    }
    
    bool parse_success = false;
    if (is_binary) {
        printf("🔍 Detected binary .cobj format for %s\n", absolute_filepath);
        parse_success = load_cobj_binary(registry, absolute_filepath, mesh_name);
        return parse_success;  // Binary loader handles everything including registry management
    } else {
        printf("🔍 Detected text .cobj format for %s (legacy)\n", absolute_filepath);
        parse_success = parse_obj_file(absolute_filepath, mesh);
    }
    
    if (!parse_success) {
        printf("❌ Failed to parse mesh file: %s\n", absolute_filepath);
        memset(mesh, 0, sizeof(Mesh));  // Clear on failure
        return false;
    }
    
    // Create GPU resources for the loaded mesh
    if (!create_mesh_gpu_resources(mesh)) {
        printf("❌ Failed to create GPU resources for mesh: %s\n", mesh_name);
        // Clean up mesh data
        if (mesh->vertices) free(mesh->vertices);
        if (mesh->indices) free(mesh->indices);
        memset(mesh, 0, sizeof(Mesh));
        return false;
    }
    
    registry->mesh_count++;
    
    printf("✅ Loaded mesh: %s (%d vertices, %d indices) with GPU resources\n", 
           mesh_name, mesh->vertex_count, mesh->index_count);
    
    return true;
}

/**
 * @brief Clean up a partially loaded mesh on error
 * @param mesh The mesh to clean up
 */
static void cleanup_mesh_on_error(Mesh* mesh) {
    if (!mesh) return;
    
    if (mesh->vertices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    
    if (mesh->indices) {
        free(mesh->indices);
        mesh->indices = NULL;
    }
    
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    mesh->loaded = false;
}
