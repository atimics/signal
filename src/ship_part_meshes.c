#include "ship_part_meshes.h"
#include "assets.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generate a cylindrical pod mesh
static Mesh* generate_pod_hull_mesh() {
    const int segments = 16;
    const int rings = 8;
    const float radius = 1.0f;
    const float height = 2.4f;
    const float taper_top = 0.7f;
    const float taper_bottom = 0.8f;
    
    Mesh* mesh = calloc(1, sizeof(Mesh));
    strncpy(mesh->name, "pod_hull", sizeof(mesh->name) - 1);
    
    // Calculate vertex count
    int vertex_count = segments * (rings + 1) + 2; // +2 for caps
    mesh->vertices = calloc(vertex_count, sizeof(Vertex));
    mesh->vertex_count = vertex_count;
    
    // Generate vertices
    int v_idx = 0;
    
    // Body vertices
    for (int ring = 0; ring <= rings; ring++) {
        float t = (float)ring / rings;
        float y = -height/2 + height * t;
        
        // Taper radius
        float ring_radius = radius;
        if (t < 0.3f) {
            ring_radius *= taper_bottom + (1.0f - taper_bottom) * (t / 0.3f);
        } else if (t > 0.7f) {
            ring_radius *= 1.0f - (1.0f - taper_top) * ((t - 0.7f) / 0.3f);
        }
        
        for (int seg = 0; seg < segments; seg++) {
            float angle = (2.0f * M_PI * seg) / segments;
            
            Vertex* v = &mesh->vertices[v_idx++];
            v->position.x = ring_radius * cosf(angle);
            v->position.y = y;
            v->position.z = ring_radius * sinf(angle);
            
            // Normal pointing outward
            v->normal.x = cosf(angle);
            v->normal.y = 0.2f * (t - 0.5f); // Slight vertical component
            v->normal.z = sinf(angle);
            
            // Normalize
            float len = sqrtf(v->normal.x * v->normal.x + 
                            v->normal.y * v->normal.y + 
                            v->normal.z * v->normal.z);
            v->normal.x /= len;
            v->normal.y /= len;
            v->normal.z /= len;
            
            // UV coordinates
            v->texcoord.x = (float)seg / segments;
            v->texcoord.y = t;
        }
    }
    
    // Top cap center
    mesh->vertices[v_idx].position = (Vector3){0, height/2, 0};
    mesh->vertices[v_idx].normal = (Vector3){0, 1, 0};
    mesh->vertices[v_idx].texcoord = (Vector2){0.5f, 0.5f};
    int top_center = v_idx++;
    
    // Bottom cap center
    mesh->vertices[v_idx].position = (Vector3){0, -height/2, 0};
    mesh->vertices[v_idx].normal = (Vector3){0, -1, 0};
    mesh->vertices[v_idx].texcoord = (Vector2){0.5f, 0.5f};
    int bottom_center = v_idx++;
    
    // Generate indices
    int face_count = segments * rings * 2 + segments * 2; // Body + caps
    mesh->indices = calloc(face_count * 3, sizeof(int));
    mesh->index_count = 0;
    
    // Body faces
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int curr = ring * segments + seg;
            int next = ring * segments + ((seg + 1) % segments);
            int curr_up = (ring + 1) * segments + seg;
            int next_up = (ring + 1) * segments + ((seg + 1) % segments);
            
            // Triangle 1
            mesh->indices[mesh->index_count++] = curr;
            mesh->indices[mesh->index_count++] = next;
            mesh->indices[mesh->index_count++] = next_up;
            
            // Triangle 2
            mesh->indices[mesh->index_count++] = curr;
            mesh->indices[mesh->index_count++] = next_up;
            mesh->indices[mesh->index_count++] = curr_up;
        }
    }
    
    // Top cap
    int top_ring_start = rings * segments;
    for (int seg = 0; seg < segments; seg++) {
        int curr = top_ring_start + seg;
        int next = top_ring_start + ((seg + 1) % segments);
        
        mesh->indices[mesh->index_count++] = top_center;
        mesh->indices[mesh->index_count++] = next;
        mesh->indices[mesh->index_count++] = curr;
    }
    
    // Bottom cap
    for (int seg = 0; seg < segments; seg++) {
        int curr = seg;
        int next = (seg + 1) % segments;
        
        mesh->indices[mesh->index_count++] = bottom_center;
        mesh->indices[mesh->index_count++] = curr;
        mesh->indices[mesh->index_count++] = next;
    }
    
    // Calculate bounds
    mesh->aabb_min = (Vector3){-radius, -height/2, -radius};
    mesh->aabb_max = (Vector3){radius, height/2, radius};
    
    mesh->loaded = true;
    
    printf("✅ Generated pod hull mesh: %d vertices, %d indices\n", 
           mesh->vertex_count, mesh->index_count);
    
    return mesh;
}

// Generate engine bell mesh
static Mesh* generate_engine_bell_mesh() {
    const int segments = 24;
    const int rings = 12;
    const float top_radius = 0.5f;
    const float bottom_radius = 1.0f;
    const float height = 1.5f;
    const float curve_power = 1.5f; // Bell curve shape
    
    Mesh* mesh = calloc(1, sizeof(Mesh));
    strncpy(mesh->name, "engine_bell", sizeof(mesh->name) - 1);
    
    int vertex_count = segments * (rings + 1) + 1; // +1 for top center
    mesh->vertices = calloc(vertex_count, sizeof(Vertex));
    mesh->vertex_count = vertex_count;
    
    int v_idx = 0;
    
    // Generate bell curve vertices
    for (int ring = 0; ring <= rings; ring++) {
        float t = (float)ring / rings;
        float y = -height * t;
        
        // Bell curve radius
        float curve_t = powf(t, curve_power);
        float ring_radius = top_radius + (bottom_radius - top_radius) * curve_t;
        
        for (int seg = 0; seg < segments; seg++) {
            float angle = (2.0f * M_PI * seg) / segments;
            
            Vertex* v = &mesh->vertices[v_idx++];
            v->position.x = ring_radius * cosf(angle);
            v->position.y = y;
            v->position.z = ring_radius * sinf(angle);
            
            // Normal calculation for bell curve
            float dx = (bottom_radius - top_radius) * curve_power * powf(t, curve_power - 1) / rings;
            float dy = -height / rings;
            float len = sqrtf(dx * dx + dy * dy);
            
            v->normal.x = cosf(angle) * dy / len;
            v->normal.y = dx / len;
            v->normal.z = sinf(angle) * dy / len;
            
            v->texcoord.x = (float)seg / segments;
            v->texcoord.y = t;
        }
    }
    
    // Top center vertex
    mesh->vertices[v_idx].position = (Vector3){0, 0, 0};
    mesh->vertices[v_idx].normal = (Vector3){0, 1, 0};
    mesh->vertices[v_idx].texcoord = (Vector2){0.5f, 0.5f};
    int top_center = v_idx++;
    
    // Indices
    mesh->indices = calloc(segments * rings * 2 * 3 + segments * 3, sizeof(int));
    mesh->index_count = 0;
    
    // Bell surface
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int curr = ring * segments + seg;
            int next = ring * segments + ((seg + 1) % segments);
            int curr_down = (ring + 1) * segments + seg;
            int next_down = (ring + 1) * segments + ((seg + 1) % segments);
            
            mesh->indices[mesh->index_count++] = curr;
            mesh->indices[mesh->index_count++] = curr_down;
            mesh->indices[mesh->index_count++] = next_down;
            
            mesh->indices[mesh->index_count++] = curr;
            mesh->indices[mesh->index_count++] = next_down;
            mesh->indices[mesh->index_count++] = next;
        }
    }
    
    // Top cap
    for (int seg = 0; seg < segments; seg++) {
        int curr = seg;
        int next = (seg + 1) % segments;
        
        mesh->indices[mesh->index_count++] = top_center;
        mesh->indices[mesh->index_count++] = curr;
        mesh->indices[mesh->index_count++] = next;
    }
    
    mesh->aabb_min = (Vector3){-bottom_radius, -height, -bottom_radius};
    mesh->aabb_max = (Vector3){bottom_radius, 0, bottom_radius};
    
    mesh->loaded = true;
    
    printf("✅ Generated engine bell mesh: %d vertices, %d indices\n",
           mesh->vertex_count, mesh->index_count);
    
    return mesh;
}

// Generate RCS block mesh
static Mesh* generate_rcs_block_mesh() {
    const float size = 0.3f;
    const float nozzle_length = 0.1f;
    const float nozzle_radius = 0.05f;
    
    Mesh* mesh = calloc(1, sizeof(Mesh));
    strncpy(mesh->name, "rcs_block", sizeof(mesh->name) - 1);
    
    // Simple box with 4 nozzle protrusions
    // 8 box vertices + 4*8 nozzle vertices
    mesh->vertex_count = 8 + 4 * 8;
    mesh->vertices = calloc(mesh->vertex_count, sizeof(Vertex));
    
    // Box vertices
    float h = size / 2;
    Vector3 box_verts[8] = {
        {-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h},  // Bottom
        {-h, h, -h}, {h, h, -h}, {h, h, h}, {-h, h, h}       // Top
    };
    
    for (int i = 0; i < 8; i++) {
        mesh->vertices[i].position = box_verts[i];
        // Simple box normals (will be refined per face)
        mesh->vertices[i].normal = (Vector3){0, 0, 0};
    }
    
    // Add nozzles (simplified as small boxes)
    int v_idx = 8;
    Vector3 nozzle_positions[4] = {
        {size/2, 0, 0}, {-size/2, 0, 0},
        {0, 0, size/2}, {0, 0, -size/2}
    };
    
    for (int n = 0; n < 4; n++) {
        Vector3 pos = nozzle_positions[n];
        float nr = nozzle_radius;
        
        // Create small box for nozzle
        for (int i = 0; i < 8; i++) {
            mesh->vertices[v_idx + i].position = (Vector3){
                pos.x + (i & 1 ? nr : -nr) * (pos.x != 0 ? 2 : 1),
                pos.y + (i & 2 ? nr : -nr),
                pos.z + (i & 4 ? nr : -nr) * (pos.z != 0 ? 2 : 1)
            };
        }
        v_idx += 8;
    }
    
    // Generate indices (box + nozzles)
    mesh->index_count = 12 * 3 + 4 * 12 * 3; // 12 triangles per box
    mesh->indices = calloc(mesh->index_count, sizeof(int));
    
    // Box faces
    int box_faces[12][3] = {
        {0,2,1}, {0,3,2}, // Bottom
        {4,5,6}, {4,6,7}, // Top
        {0,1,5}, {0,5,4}, // Front
        {2,3,7}, {2,7,6}, // Back
        {0,4,7}, {0,7,3}, // Left
        {1,2,6}, {1,6,5}  // Right
    };
    
    int idx = 0;
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            mesh->indices[idx++] = box_faces[i][j];
        }
    }
    
    // Nozzle faces (simplified)
    for (int n = 0; n < 4; n++) {
        int base = 8 + n * 8;
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 3; j++) {
                mesh->indices[idx++] = base + box_faces[i][j];
            }
        }
    }
    
    mesh->aabb_min = (Vector3){-size/2 - nozzle_length, -size/2, -size/2 - nozzle_length};
    mesh->aabb_max = (Vector3){size/2 + nozzle_length, size/2, size/2 + nozzle_length};
    
    mesh->loaded = true;
    
    printf("✅ Generated RCS block mesh: %d vertices, %d indices\n",
           mesh->vertex_count, mesh->index_count);
    
    return mesh;
}

// Register procedural meshes with asset system
bool ship_part_meshes_init(AssetRegistry* registry) {
    if (!registry) return false;
    
    // Generate and register meshes
    Mesh* meshes[] = {
        generate_pod_hull_mesh(),
        generate_engine_bell_mesh(),
        generate_rcs_block_mesh(),
        NULL
    };
    
    for (int i = 0; meshes[i] != NULL; i++) {
        Mesh* mesh = meshes[i];
        
        // Upload to GPU
        if (!assets_upload_mesh_to_gpu(mesh)) {
            printf("❌ Failed to upload %s to GPU\n", mesh->name);
            continue;
        }
        
        // Add to registry
        if (registry->mesh_count < MAX_MESHES) {
            registry->meshes[registry->mesh_count++] = *mesh;
            free(mesh); // Registry has a copy now
        }
    }
    
    printf("🛠️ Registered %d procedural ship part meshes\n", 
           sizeof(meshes)/sizeof(meshes[0]) - 1);
    
    return true;
}