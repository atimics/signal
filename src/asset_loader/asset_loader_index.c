#include "../assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool load_assets_from_metadata(AssetRegistry* registry) {
    if (!registry) return false;
    
    // Load from index.json to get list of metadata files
    char index_path[512];
    snprintf(index_path, sizeof(index_path), "%s/meshes/index.json", registry->asset_root);
    
    FILE* file = fopen(index_path, "r");
    if (!file) {
        printf("⚠️  Could not open index.json: %s\n", index_path);
        printf("⚠️  Could not find modern asset index, check asset pipeline\n");
        return false;
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
    
    // Construct full absolute path to .cobj file
    char mesh_path[512];
    snprintf(mesh_path, sizeof(mesh_path), "%s/%s", mesh_dir, geometry_filename);
    
    printf("🔍 DEBUG: Full mesh path: '%s'\n", mesh_path);
    
    // Load compiled mesh files (.cobj format from asset compiler)
    printf("🔍 DEBUG: Calling load_mesh_from_file with: registry=%p, path='%s', name='%s'\n", 
           (void*)registry, mesh_path, mesh_name);
    bool loaded = load_mesh_from_file(registry, mesh_path, mesh_name);
    
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
