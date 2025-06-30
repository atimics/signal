// tests/sprint_10_5/test_task_2_standalone.c
// Standalone test for dynamic memory allocation in parse_obj_file
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Minimal definitions needed for testing
typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float u, v;
} Vector2;

typedef struct {
    Vector3 position;
    Vector3 normal;
    Vector2 tex_coord;
} Vertex;

typedef struct {
    char name[64];
    char material_name[64];
    Vertex* vertices;
    int vertex_count;
    int* indices;
    int index_count;
    bool loaded;
    // Omit GPU-related fields for this test
} Mesh;

// Forward-declare the function we are testing
bool parse_obj_file(const char* filepath, Mesh* mesh);

// Standalone implementation of parse_obj_file with dynamic memory allocation
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

// Helper to check if a pointer is to the heap (not foolproof, but good enough for this test)
bool is_heap_pointer(void* p) {
    // A simple check: stack pointers are usually very high in the address space.
    // This is not guaranteed but is a reasonable heuristic for a test.
    char stack_var;
    return p < (void*)&stack_var;
}

void test_mesh_parser_stability() {
    printf("Running Test: test_mesh_parser_stability\n");

    Mesh small_mesh = {0};
    Mesh large_mesh = {0};

    // --- Test Case 1: Parse small mesh ---
    printf("  Case 1: Parse small_mesh.cobj...\n");
    bool small_parsed = parse_obj_file("tests/sprint_10_5/small_mesh.cobj", &small_mesh);
    assert(small_parsed == true);
    assert(small_mesh.vertex_count == 3);
    assert(small_mesh.index_count == 3);
    assert(small_mesh.vertices != NULL);
    assert(small_mesh.indices != NULL);
    // Verify memory is from the heap, not the stack
    assert(is_heap_pointer(small_mesh.vertices));
    assert(is_heap_pointer(small_mesh.indices));
    printf("    ... PASSED\n");

    // --- Test Case 2: Parse large mesh ---
    printf("  Case 2: Parse large_mesh.cobj...\n");
    bool large_parsed = parse_obj_file("tests/sprint_10_5/large_mesh.cobj", &large_mesh);
    assert(large_parsed == true);
    assert(large_mesh.vertex_count == (20000 - 2) * 3); // Each face adds 3 vertices
    assert(large_mesh.index_count == (20000 - 2) * 3);
    assert(large_mesh.vertices != NULL);
    assert(large_mesh.indices != NULL);
    assert(is_heap_pointer(large_mesh.vertices));
    assert(is_heap_pointer(large_mesh.indices));
    printf("    ... PASSED\n");

    // --- Test Case 3: Handle non-existent file ---
    printf("  Case 3: Handle non-existent file...\n");
    Mesh non_existent_mesh = {0};
    bool non_existent_parsed = parse_obj_file("tests/sprint_10_5/no_such_file.cobj", &non_existent_mesh);
    assert(non_existent_parsed == false);
    assert(non_existent_mesh.vertex_count == 0);
    assert(non_existent_mesh.vertices == NULL);
    printf("    ... PASSED\n");

    // Cleanup
    free(small_mesh.vertices);
    free(small_mesh.indices);
    free(large_mesh.vertices);
    free(large_mesh.indices);

    printf("Test Finished: test_mesh_parser_stability\n\n");
}

int main() {
    test_mesh_parser_stability();
    printf("All Task 2 tests passed!\n");
    return 0;
}
