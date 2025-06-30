#include "sokol_gfx.h"
#include "data.h"
#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert Euler angles (degrees) to quaternion
static Quaternion euler_to_quaternion(Vector3 euler) {
    // Convert degrees to radians
    float pitch = euler.x * M_PI / 180.0f;
    float yaw = euler.y * M_PI / 180.0f;
    float roll = euler.z * M_PI / 180.0f;
    
    // Calculate half angles
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);
    
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    
    return q;
}

// ============================================================================
// DATA REGISTRY IMPLEMENTATION
// ============================================================================

bool data_registry_init(DataRegistry* registry, const char* data_root) {
    if (!registry || !data_root) return false;
    
    memset(registry, 0, sizeof(DataRegistry));
    strncpy(registry->data_root, data_root, sizeof(registry->data_root) - 1);
    
    printf("📋 Data registry initialized\n");
    printf("   Data root: %s\n", registry->data_root);
    
    return true;
}

void data_registry_cleanup(DataRegistry* registry) {
    if (!registry) return;
    printf("📋 Data registry cleaned up\n");
}

// ============================================================================
// TEMPLATE LOADING (TEXT FORMAT)
// ============================================================================

// Simple text parser for entity templates
bool load_entity_templates(DataRegistry* registry, const char* templates_path) {
    if (!registry || !templates_path) return false;
    
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", registry->data_root, templates_path);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        printf("⚠️  Could not open entity templates: %s\n", full_path);
        return false;
    }
    
    printf("📝 Loading entity templates from %s\n", full_path);
    
    char line[512];
    EntityTemplate* current_template = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        // Parse template definition
        if (strncmp(line, "template:", 9) == 0) {
            if (registry->entity_template_count >= 128) {
                printf("❌ Too many entity templates (max 128)\n");
                break;
            }
            
            current_template = &registry->entity_templates[registry->entity_template_count++];
            memset(current_template, 0, sizeof(EntityTemplate));
            
            // Extract template name, trimming whitespace
            char* name_start = line + 9;
            while (*name_start == ' ' || *name_start == '\t') name_start++;  // Skip leading whitespace
            strncpy(current_template->name, name_start, sizeof(current_template->name) - 1);
            
            // Set defaults
            current_template->scale = (Vector3){1.0f, 1.0f, 1.0f};
            current_template->mass = 1.0f;
            current_template->drag = 0.99f;
            current_template->collision_radius = 1.0f;
            current_template->layer_mask = 0xFFFFFFFF;
            current_template->visible = true;
            current_template->ai_update_frequency = 5.0f;
            
            continue;
        }
        
        if (!current_template) continue;
        
        // Parse properties
        char* equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = 0;
        char* key = line;
        char* value = equals + 1;
        
        // Trim whitespace
        while (isspace(*key)) key++;
        while (isspace(*value)) value++;
        
        // Parse different property types
        if (strcmp(key, "description") == 0) {
            strncpy(current_template->description, value, sizeof(current_template->description) - 1);
        }
        else if (strcmp(key, "has_transform") == 0) {
            current_template->has_transform = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "has_physics") == 0) {
            current_template->has_physics = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "has_collision") == 0) {
            current_template->has_collision = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "has_renderable") == 0) {
            current_template->has_renderable = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "has_ai") == 0) {
            current_template->has_ai = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "has_player") == 0) {
            current_template->has_player = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "has_camera") == 0) {
            current_template->has_camera = (strcmp(value, "true") == 0);
        }
        else if (strcmp(key, "camera_behavior") == 0) {
            if (strcmp(value, "third_person") == 0) {
                current_template->camera_behavior = 0; // CAMERA_BEHAVIOR_THIRD_PERSON
            } else if (strcmp(value, "first_person") == 0) {
                current_template->camera_behavior = 1; // CAMERA_BEHAVIOR_FIRST_PERSON
            } else if (strcmp(value, "static") == 0) {
                current_template->camera_behavior = 2; // CAMERA_BEHAVIOR_STATIC
            } else if (strcmp(value, "chase") == 0) {
                current_template->camera_behavior = 3; // CAMERA_BEHAVIOR_CHASE
            } else if (strcmp(value, "orbital") == 0) {
                current_template->camera_behavior = 4; // CAMERA_BEHAVIOR_ORBITAL
            } else {
                current_template->camera_behavior = 0; // Default to third person
            }
        }
        else if (strcmp(key, "fov") == 0) {
            current_template->fov = atof(value);
        }
        else if (strcmp(key, "near_plane") == 0) {
            current_template->near_plane = atof(value);
        }
        else if (strcmp(key, "far_plane") == 0) {
            current_template->far_plane = atof(value);
        }
        else if (strcmp(key, "follow_distance") == 0) {
            current_template->follow_distance = atof(value);
        }
        else if (strcmp(key, "follow_offset_x") == 0) {
            current_template->follow_offset.x = atof(value);
        }
        else if (strcmp(key, "follow_offset_y") == 0) {
            current_template->follow_offset.y = atof(value);
        }
        else if (strcmp(key, "follow_offset_z") == 0) {
            current_template->follow_offset.z = atof(value);
        }
        else if (strcmp(key, "follow_smoothing") == 0) {
            current_template->follow_smoothing = atof(value);
        }
        else if (strcmp(key, "mass") == 0) {
            current_template->mass = atof(value);
        }
        else if (strcmp(key, "collision_radius") == 0) {
            current_template->collision_radius = atof(value);
        }
        else if (strcmp(key, "mesh_name") == 0) {
            strncpy(current_template->mesh_name, value, sizeof(current_template->mesh_name) - 1);
        }
        else if (strcmp(key, "material_name") == 0) {
            strncpy(current_template->material_name, value, sizeof(current_template->material_name) - 1);
        }
        else if (strcmp(key, "kinematic") == 0) {
            current_template->kinematic = (strcmp(value, "true") == 0);
        }
    }
    
    fclose(file);
    printf("   ✅ Loaded %d entity templates\n", registry->entity_template_count);
    return true;
}

bool load_scene_templates(DataRegistry* registry, const char* scenes_path) {
    if (!registry || !scenes_path) return false;
    
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", registry->data_root, scenes_path);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        printf("⚠️  Could not open scene templates: %s\n", full_path);
        return false;
    }
    
    printf("🏗️  Loading scene templates from %s\n", full_path);
    
    char line[512];
    SceneTemplate* current_scene = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        // Parse scene definition
        if (strncmp(line, "scene:", 6) == 0) {
            if (registry->scene_template_count >= 32) {
                printf("❌ Too many scene templates (max 32)\n");
                break;
            }
            
            current_scene = &registry->scene_templates[registry->scene_template_count++];
            memset(current_scene, 0, sizeof(SceneTemplate));
            
            // Extract scene name, trimming whitespace
            char* name_start = line + 6;
            while (*name_start == ' ' || *name_start == '\t') name_start++;  // Skip leading whitespace
            strncpy(current_scene->name, name_start, sizeof(current_scene->name) - 1);
            continue;
        }
        
        if (!current_scene) continue;
        
        // Parse spawn entries: "spawn: entity_type x y z" (handle indentation)
        char* spawn_keyword = "spawn:";
        char* trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;  // Skip leading whitespace
        if (strncmp(trimmed_line, spawn_keyword, strlen(spawn_keyword)) == 0) {
            if (current_scene->spawn_count >= 256) {
                printf("❌ Too many spawns in scene (max 256)\n");
                continue;
            }
            
            EntitySpawn* spawn = &current_scene->spawns[current_scene->spawn_count++];
            memset(spawn, 0, sizeof(EntitySpawn));
            
            // Default scale
            spawn->scale = (Vector3){1.0f, 1.0f, 1.0f};
            
            // Parse spawn line: "spawn: entity_type x y z"
            char* tokens = trimmed_line + strlen(spawn_keyword);
            while (isspace(*tokens)) tokens++;
            
            sscanf(tokens, "%63s %f %f %f", 
                   spawn->entity_type, 
                   &spawn->position.x, 
                   &spawn->position.y, 
                   &spawn->position.z);
        }
        // Parse description or other scene properties
        else if (strncmp(line, "description=", 12) == 0) {
            // Could parse description if needed
        }
    }
    
    fclose(file);
    printf("   ✅ Loaded %d scene templates\n", registry->scene_template_count);
    return true;
}

// ============================================================================
// ENTITY CREATION FROM TEMPLATES
// ============================================================================

EntityID create_entity_from_template(struct World* world, DataRegistry* registry, 
                                    AssetRegistry* assets, const char* template_name, Vector3 position) {
    if (!world || !registry || !template_name) return INVALID_ENTITY;
    
    EntityTemplate* template = find_entity_template(registry, template_name);
    if (!template) {
        printf("❌ Entity template not found: %s\n", template_name);
        return INVALID_ENTITY;
    }
    
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components based on template
    if (template->has_transform) {
        entity_add_component(world, id, COMPONENT_TRANSFORM);
        struct Transform* transform = entity_get_transform(world, id);
        transform->position = position;  // Use provided position
        // Convert Euler angles to quaternion (simplified for now)
        transform->rotation = euler_to_quaternion(template->rotation);
        transform->scale = template->scale;
    }
    
    if (template->has_physics) {
        entity_add_component(world, id, COMPONENT_PHYSICS);
        struct Physics* physics = entity_get_physics(world, id);
        physics->mass = template->mass;
        physics->drag = template->drag;
        physics->velocity = template->velocity;
        physics->acceleration = template->acceleration;
        physics->kinematic = template->kinematic;
    }
    
    if (template->has_collision) {
        entity_add_component(world, id, COMPONENT_COLLISION);
        struct Collision* collision = entity_get_collision(world, id);
        collision->shape = template->collision_shape;
        collision->radius = template->collision_radius;
        collision->layer_mask = template->layer_mask;
        collision->is_trigger = template->is_trigger;
    }
    
    if (template->has_renderable) {
        entity_add_component(world, id, COMPONENT_RENDERABLE);
        struct Renderable* renderable = entity_get_renderable(world, id);
        renderable->visible = template->visible;
        
        // Try to create renderable from mesh name
        if (strlen(template->mesh_name) > 0) {
            if (assets_create_renderable_from_mesh(assets, template->mesh_name, renderable)) {
                printf("✅ Entity %d assigned mesh: %s (%d indices)\n", 
                       id, template->mesh_name, renderable->index_count);
            } else {
                printf("⚠️  Entity %d failed to load mesh: %s - using fallback\n", 
                       id, template->mesh_name);
                // Set up invalid handles - will be skipped during rendering
                renderable->vbuf.id = SG_INVALID_ID;
                renderable->ibuf.id = SG_INVALID_ID;
                renderable->tex.id = SG_INVALID_ID;
                renderable->index_count = 0;
                renderable->visible = false;
            }
        } else {
            // No mesh specified - disable rendering
            renderable->vbuf.id = SG_INVALID_ID;
            renderable->ibuf.id = SG_INVALID_ID;
            renderable->tex.id = SG_INVALID_ID;
            renderable->index_count = 0;
            renderable->visible = false;
            printf("⚠️  Entity %d has no mesh specified\n", id);
        }
    }
    
    if (template->has_ai) {
        entity_add_component(world, id, COMPONENT_AI);
        struct AI* ai = entity_get_ai(world, id);
        ai->state = template->initial_ai_state;
        ai->update_frequency = template->ai_update_frequency;
        ai->decision_timer = world->total_time;
    }
    
    if (template->has_player) {
        entity_add_component(world, id, COMPONENT_PLAYER);
    }
    
    if (template->has_camera) {
        entity_add_component(world, id, COMPONENT_CAMERA);
        struct Camera* camera = entity_get_camera(world, id);
        if (camera) {
            camera->fov = template->fov > 0 ? template->fov : 60.0f;
            camera->near_plane = template->near_plane > 0 ? template->near_plane : 0.1f;
            camera->far_plane = template->far_plane > 0 ? template->far_plane : 1000.0f;
            camera->aspect_ratio = template->aspect_ratio > 0 ? template->aspect_ratio : 16.0f / 9.0f;
            camera->behavior = template->camera_behavior;
            camera->follow_distance = template->follow_distance > 0 ? template->follow_distance : 10.0f;
            camera->follow_offset = template->follow_offset;
            camera->follow_smoothing = template->follow_smoothing > 0 ? template->follow_smoothing : 0.02f;
            camera->is_active = false; // Will be set by camera system
            camera->follow_target = INVALID_ENTITY; // Will be set later
        }
    }
    
    return id;
}

// ============================================================================
// SCENE LOADING
// ============================================================================

bool load_scene(struct World* world, DataRegistry* registry, AssetRegistry* assets, const char* scene_name) {
    if (!world || !registry || !scene_name) return false;
    
    SceneTemplate* scene = find_scene_template(registry, scene_name);
    if (!scene) {
        printf("❌ Scene template not found: %s\n", scene_name);
        return false;
    }
    
    printf("🏗️  Loading scene: %s\n", scene->name);
    
    for (uint32_t i = 0; i < scene->spawn_count; i++) {
        EntitySpawn* spawn = &scene->spawns[i];
        
        EntityID id = create_entity_from_template(world, registry, assets,
                                                 spawn->entity_type, 
                                                 spawn->position);
        
        if (id != INVALID_ENTITY) {
            printf("   ✅ Spawned %s at (%.1f, %.1f, %.1f)\n", 
                   spawn->entity_type, 
                   spawn->position.x, 
                   spawn->position.y, 
                   spawn->position.z);
        }
    }
    
    printf("🏗️  Scene loaded: %d entities spawned\n", scene->spawn_count);
    return true;
}

// ============================================================================
// TEMPLATE QUERIES
// ============================================================================

EntityTemplate* find_entity_template(DataRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (uint32_t i = 0; i < registry->entity_template_count; i++) {
        if (strcmp(registry->entity_templates[i].name, name) == 0) {
            return &registry->entity_templates[i];
        }
    }
    return NULL;
}

SceneTemplate* find_scene_template(DataRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    
    for (uint32_t i = 0; i < registry->scene_template_count; i++) {
        if (strcmp(registry->scene_templates[i].name, name) == 0) {
            return &registry->scene_templates[i];
        }
    }
    return NULL;
}

// ============================================================================
// LISTING FUNCTIONS
// ============================================================================

void list_entity_templates(DataRegistry* registry) {
    if (!registry) return;
    
    printf("📋 Entity Templates (%d):\n", registry->entity_template_count);
    for (uint32_t i = 0; i < registry->entity_template_count; i++) {
        EntityTemplate* t = &registry->entity_templates[i];
        printf("   - %s: %s\n", t->name, t->description);
    }
}

void list_scene_templates(DataRegistry* registry) {
    if (!registry) return;
    
    printf("🏗️  Scene Templates (%d):\n", registry->scene_template_count);
    for (uint32_t i = 0; i < registry->scene_template_count; i++) {
        SceneTemplate* s = &registry->scene_templates[i];
        printf("   - %s: %d spawns\n", s->name, s->spawn_count);
    }
}