/**
 * @file validate_thrusters.c
 * @brief Compile-time validation tool for thruster attachment points
 *
 * This tool validates that thruster positions defined in .thrusters files
 * correspond to actual vertices in the referenced mesh files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>

#define ATTACHMENT_TOLERANCE 0.1f  // 10cm tolerance for vertex matching

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    char name[64];
    Vector3 position;
    Vector3 direction;
    float max_force;
} ThrusterDef;

typedef struct {
    Vector3* vertices;
    int vertex_count;
    int capacity;
} MeshData;

// Calculate distance between two vectors
float vector3_distance(const Vector3* a, const Vector3* b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

// Parse OBJ file and extract vertices
bool load_obj_vertices(const char* filename, MeshData* mesh) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("❌ Failed to open mesh file: %s\n", filename);
        return false;
    }
    
    mesh->vertices = NULL;
    mesh->vertex_count = 0;
    mesh->capacity = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            // Vertex line
            if (mesh->vertex_count >= mesh->capacity) {
                mesh->capacity = mesh->capacity ? mesh->capacity * 2 : 128;
                mesh->vertices = realloc(mesh->vertices, mesh->capacity * sizeof(Vector3));
            }
            
            Vector3* v = &mesh->vertices[mesh->vertex_count];
            if (sscanf(line, "v %f %f %f", &v->x, &v->y, &v->z) == 3) {
                mesh->vertex_count++;
            }
        }
    }
    
    fclose(file);
    
    printf("📋 Loaded %d vertices from %s\n", mesh->vertex_count, filename);
    return mesh->vertex_count > 0;
}

// Load thruster definitions from file
int load_thruster_defs(const char* filename, ThrusterDef** thrusters) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("❌ Failed to open thruster file: %s\n", filename);
        return 0;
    }
    
    int count = 0;
    int capacity = 32;
    *thrusters = malloc(capacity * sizeof(ThrusterDef));
    
    char line[256];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        if (count >= capacity) {
            capacity *= 2;
            *thrusters = realloc(*thrusters, capacity * sizeof(ThrusterDef));
        }
        
        ThrusterDef* t = &(*thrusters)[count];
        
        int items = sscanf(line, "%63s %f %f %f %f %f %f %f",
                          t->name,
                          &t->position.x, &t->position.y, &t->position.z,
                          &t->direction.x, &t->direction.y, &t->direction.z,
                          &t->max_force);
        
        if (items == 8) {
            count++;
        } else if (items > 0) {
            printf("⚠️  Line %d: Invalid format (expected 8 items, got %d)\n", line_num, items);
        }
    }
    
    fclose(file);
    
    printf("📋 Loaded %d thruster definitions from %s\n", count, filename);
    return count;
}

// Validate thruster positions against mesh vertices
bool validate_thrusters(const ThrusterDef* thrusters, int thruster_count, 
                       const MeshData* mesh, const char* mesh_name) {
    bool all_valid = true;
    
    printf("\n🔍 Validating thruster positions against mesh: %s\n", mesh_name);
    printf("   Tolerance: %.3f units\n\n", ATTACHMENT_TOLERANCE);
    
    for (int i = 0; i < thruster_count; i++) {
        const ThrusterDef* t = &thrusters[i];
        float min_distance = INFINITY;
        int closest_vertex = -1;
        
        // Find closest vertex
        for (int v = 0; v < mesh->vertex_count; v++) {
            float dist = vector3_distance(&t->position, &mesh->vertices[v]);
            if (dist < min_distance) {
                min_distance = dist;
                closest_vertex = v;
            }
        }
        
        if (min_distance <= ATTACHMENT_TOLERANCE) {
            printf("✅ %-20s at (%6.2f,%6.2f,%6.2f) -> vertex %d (dist: %.3f)\n",
                   t->name, t->position.x, t->position.y, t->position.z,
                   closest_vertex, min_distance);
        } else {
            printf("❌ %-20s at (%6.2f,%6.2f,%6.2f) -> NO MATCH! Closest: vertex %d (dist: %.3f)\n",
                   t->name, t->position.x, t->position.y, t->position.z,
                   closest_vertex, min_distance);
            if (closest_vertex >= 0) {
                Vector3* v = &mesh->vertices[closest_vertex];
                printf("   Suggestion: Try position (%6.2f,%6.2f,%6.2f)\n", v->x, v->y, v->z);
            }
            all_valid = false;
        }
    }
    
    return all_valid;
}

// Process a single thruster/mesh pair
bool process_thruster_file(const char* thruster_file, const char* mesh_dir) {
    // Extract mesh name from thruster filename
    char mesh_name[256];
    const char* basename = strrchr(thruster_file, '/');
    if (!basename) basename = thruster_file;
    else basename++;
    
    strncpy(mesh_name, basename, sizeof(mesh_name) - 1);
    char* ext = strstr(mesh_name, ".thrusters");
    if (ext) *ext = '\0';
    
    // Try to find corresponding mesh file
    char mesh_paths[3][512];
    snprintf(mesh_paths[0], sizeof(mesh_paths[0]), "%s/%s/geometry.obj", mesh_dir, mesh_name);
    snprintf(mesh_paths[1], sizeof(mesh_paths[1]), "%s/props/%s/geometry.obj", mesh_dir, mesh_name);
    snprintf(mesh_paths[2], sizeof(mesh_paths[2]), "%s/%s.obj", mesh_dir, mesh_name);
    
    char* mesh_file = NULL;
    for (int i = 0; i < 3; i++) {
        struct stat st;
        if (stat(mesh_paths[i], &st) == 0) {
            mesh_file = mesh_paths[i];
            break;
        }
    }
    
    if (!mesh_file) {
        printf("❌ Cannot find mesh file for %s\n", mesh_name);
        printf("   Tried:\n");
        for (int i = 0; i < 3; i++) {
            printf("   - %s\n", mesh_paths[i]);
        }
        return false;
    }
    
    printf("\n============================================\n");
    printf("Validating: %s\n", thruster_file);
    printf("Against mesh: %s\n", mesh_file);
    printf("============================================\n");
    
    // Load thruster definitions
    ThrusterDef* thrusters = NULL;
    int thruster_count = load_thruster_defs(thruster_file, &thrusters);
    if (thruster_count == 0) {
        free(thrusters);
        return false;
    }
    
    // Load mesh vertices
    MeshData mesh;
    if (!load_obj_vertices(mesh_file, &mesh)) {
        free(thrusters);
        return false;
    }
    
    // Validate
    bool valid = validate_thrusters(thrusters, thruster_count, &mesh, mesh_name);
    
    // Cleanup
    free(thrusters);
    free(mesh.vertices);
    
    return valid;
}

// Process all thruster files in a directory
int process_directory(const char* thruster_dir, const char* mesh_dir) {
    DIR* dir = opendir(thruster_dir);
    if (!dir) {
        printf("❌ Cannot open thruster directory: %s\n", thruster_dir);
        return 1;
    }
    
    int total_files = 0;
    int valid_files = 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".thrusters")) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", thruster_dir, entry->d_name);
            
            total_files++;
            if (process_thruster_file(full_path, mesh_dir)) {
                valid_files++;
            }
        }
    }
    
    closedir(dir);
    
    printf("\n============================================\n");
    printf("VALIDATION SUMMARY\n");
    printf("============================================\n");
    printf("Total files processed: %d\n", total_files);
    printf("Valid files: %d\n", valid_files);
    printf("Failed files: %d\n", total_files - valid_files);
    
    return (valid_files == total_files) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    printf("🚀 Thruster Validation Tool v1.0\n");
    printf("================================\n\n");
    
    if (argc < 3) {
        printf("Usage: %s <thruster_dir> <mesh_dir>\n", argv[0]);
        printf("   or: %s <thruster_file> <mesh_dir>\n", argv[0]);
        printf("\nExample:\n");
        printf("   %s data/thrusters/ assets/meshes/\n", argv[0]);
        printf("   %s data/thrusters/wedge_ship.thrusters assets/meshes/\n", argv[0]);
        return 1;
    }
    
    const char* thruster_path = argv[1];
    const char* mesh_dir = argv[2];
    
    // Check if it's a file or directory
    struct stat st;
    if (stat(thruster_path, &st) != 0) {
        printf("❌ Cannot access: %s\n", thruster_path);
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        // Process directory
        return process_directory(thruster_path, mesh_dir);
    } else {
        // Process single file
        return process_thruster_file(thruster_path, mesh_dir) ? 0 : 1;
    }
}