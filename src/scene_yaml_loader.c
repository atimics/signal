#include "scene_yaml_loader.h"
#include "data.h"
#include "system/material.h"
#include "system/control.h"
#include "assets.h"
#include "gpu_resources.h"
#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// YAML parsing state
typedef struct {
    struct World* world;
    AssetRegistry* assets;
    EntityID current_entity;
    char current_key[256];
    bool in_entities;
    bool in_components;
    bool in_physics;
    bool in_collision;
    bool in_camera;
    bool in_lighting;
    bool in_position;
    bool in_rotation;
    bool in_scale;
    bool in_array;  // Track if we're inside any array
    int array_index;
    Vector3 temp_vec3;
    Quaternion temp_quat;
    bool expecting_value;
    int entity_mapping_depth;  // Track nesting level within entity
} YAMLParseState;

// Helper to parse float from scalar
static float parse_float(const char* str) {
    return strtof(str, NULL);
}

// Helper to parse bool from scalar
static bool parse_bool(const char* str) {
    return strcmp(str, "true") == 0 || strcmp(str, "1") == 0;
}

// Process a key-value pair in the current context
static void process_yaml_value(YAMLParseState* state, const char* value) {
    if (!state->current_entity || state->current_entity == INVALID_ENTITY) {
        return;
    }
    
    struct Entity* entity = entity_get(state->world, state->current_entity);
    if (!entity) return;
    
    // Handle different contexts
    if (state->in_physics && entity->physics) {
        struct Physics* physics = entity->physics;
        
        if (strcmp(state->current_key, "mass") == 0) {
            physics->mass = parse_float(value);
        } else if (strcmp(state->current_key, "drag_linear") == 0) {
            physics->drag_linear = parse_float(value);
        } else if (strcmp(state->current_key, "drag_angular") == 0) {
            physics->drag_angular = parse_float(value);
        } else if (strcmp(state->current_key, "has_6dof") == 0) {
            physics->has_6dof = parse_bool(value);
        } else if (strcmp(state->current_key, "use_ode") == 0) {
            physics->use_ode = parse_bool(value);
        } else if (strcmp(state->current_key, "kinematic") == 0) {
            physics->kinematic = parse_bool(value);
        }
    } else if (state->in_collision && entity->collision) {
        struct Collision* collision = entity->collision;
        
        if (strcmp(state->current_key, "shape") == 0) {
            if (strcmp(value, "sphere") == 0) {
                collision->shape = COLLISION_SPHERE;
            } else if (strcmp(value, "box") == 0) {
                collision->shape = COLLISION_BOX;
            } else if (strcmp(value, "capsule") == 0) {
                collision->shape = COLLISION_CAPSULE;
            }
        }
    } else if (state->in_camera && entity->camera) {
        struct Camera* camera = entity->camera;
        
        if (strcmp(state->current_key, "follow_target") == 0) {
            // Would need to resolve entity name to ID
            // For now, assume player_ship is entity 1
            if (strcmp(value, "player_ship") == 0) {
                camera->follow_target = 1;
            }
        } else if (strcmp(state->current_key, "follow_distance") == 0) {
            camera->follow_distance = parse_float(value);
        } else if (strcmp(state->current_key, "follow_smoothing") == 0) {
            camera->follow_smoothing = parse_float(value);
        } else if (strcmp(state->current_key, "behavior") == 0) {
            if (strcmp(value, "chase") == 0) {
                camera->behavior = CAMERA_BEHAVIOR_CHASE;
            } else if (strcmp(value, "third_person") == 0) {
                camera->behavior = CAMERA_BEHAVIOR_THIRD_PERSON;
            }
        }
    } else if (!state->in_components) {
        // Top-level entity properties
        if (strcmp(state->current_key, "type") == 0) {
            // Entity type handled during creation
        } else if (strcmp(state->current_key, "material") == 0) {
            MaterialProperties* material = material_get_by_name(value);
            if (material && entity->renderable) {
                // For now, just store a simple ID based on name hash
                entity->renderable->material_id = 0; // Default material
                printf("✅ Entity %d assigned material: %s\n", 
                       state->current_entity, value);
            }
        } else if (strcmp(state->current_key, "mesh") == 0) {
            // Load mesh from asset registry
            if (entity->renderable) {
                printf("🔍 Attempting to load mesh '%s' for entity %d\n", value, state->current_entity);
                if (state->assets && assets_create_renderable_from_mesh(state->assets, value, entity->renderable)) {
                    printf("✅ Entity %d assigned mesh: %s (%d indices)\n",
                           state->current_entity, value, entity->renderable->index_count);
                    printf("   GPU resources created for mesh\n");
                } else {
                    printf("⚠️  Entity %d failed to load mesh: %s\n",
                           state->current_entity, value);
                    // Don't override existing GPU resources
                    // Just create new ones if needed
                    entity->renderable->gpu_resources = gpu_resources_create();
                    entity->renderable->index_count = 0;
                    entity->renderable->visible = false;
                }
            } else {
                printf("❌ Entity %d has no renderable component for mesh %s\n", 
                       state->current_entity, value);
            }
        }
    }
}

// Process array values for vectors
static void process_yaml_array_value(YAMLParseState* state, const char* value) {
    float f = parse_float(value);
    
    if (state->in_position && state->current_entity != INVALID_ENTITY) {
        struct Transform* transform = entity_get_transform(state->world, state->current_entity);
        if (transform) {
            if (state->array_index == 0) transform->position.x = f;
            else if (state->array_index == 1) transform->position.y = f;
            else if (state->array_index == 2) transform->position.z = f;
        }
    } else if (state->in_rotation && state->current_entity != INVALID_ENTITY) {
        struct Transform* transform = entity_get_transform(state->world, state->current_entity);
        if (transform) {
            if (state->array_index == 0) transform->rotation.x = f;
            else if (state->array_index == 1) transform->rotation.y = f;
            else if (state->array_index == 2) transform->rotation.z = f;
            else if (state->array_index == 3) transform->rotation.w = f;
        }
    } else if (state->in_scale && state->current_entity != INVALID_ENTITY) {
        struct Transform* transform = entity_get_transform(state->world, state->current_entity);
        if (transform) {
            if (state->array_index == 0) transform->scale.x = f;
            else if (state->array_index == 1) transform->scale.y = f;
            else if (state->array_index == 2) transform->scale.z = f;
        }
    } else if (state->in_camera && strcmp(state->current_key, "follow_offset") == 0) {
        struct Camera* camera = entity_get_camera(state->world, state->current_entity);
        if (camera) {
            if (state->array_index == 0) camera->follow_offset.x = f;
            else if (state->array_index == 1) camera->follow_offset.y = f;
            else if (state->array_index == 2) camera->follow_offset.z = f;
        }
    } else if (state->in_collision && strcmp(state->current_key, "size") == 0) {
        struct Collision* collision = entity_get_collision(state->world, state->current_entity);
        if (collision) {
            if (state->array_index == 0) collision->box_size.x = f;
            else if (state->array_index == 1) collision->box_size.y = f;
            else if (state->array_index == 2) collision->box_size.z = f;
        }
    }
    
    state->array_index++;
}

bool scene_load_from_yaml(struct World* world, AssetRegistry* assets, const char* filename) {
    if (!world || !assets || !filename) return false;
    
    // Build full path
    char full_path[512];
    const char* root_path = "data"; // Default data path
    snprintf(full_path, sizeof(full_path), "%s/scenes/%s", 
             root_path, filename);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        printf("❌ Failed to open YAML scene file: %s\n", full_path);
        return false;
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    if (!yaml_parser_initialize(&parser)) {
        printf("❌ Failed to initialize YAML parser\n");
        fclose(file);
        return false;
    }
    
    yaml_parser_set_input_file(&parser, file);
    
    YAMLParseState state = {0};
    state.world = world;
    state.assets = assets;
    state.current_entity = INVALID_ENTITY;
    state.expecting_value = false;
    state.entity_mapping_depth = 0;
    
    bool done = false;
    bool success = true;
    
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("❌ YAML parse error: %s\n", parser.problem);
            success = false;
            break;
        }
        
        switch (event.type) {
            case YAML_STREAM_END_EVENT:
                done = true;
                break;
                
            case YAML_MAPPING_START_EVENT:
                state.expecting_value = false; // Reset for new mapping
                
                // If we're in the entities sequence, this starts a new entity
                if (state.in_entities && state.current_entity == INVALID_ENTITY) {
                    // Don't create entity yet - wait for type
                    state.entity_mapping_depth = 1;  // Top level of entity mapping
                } else if (state.in_entities && state.current_entity != INVALID_ENTITY) {
                    // Nested mapping within entity
                    state.entity_mapping_depth++;
                }
                
                if (strcmp(state.current_key, "components") == 0) {
                    state.in_components = true;
                } else if (state.in_components && strcmp(state.current_key, "physics") == 0) {
                    state.in_physics = true;
                    if (!entity_has_component(world, state.current_entity, COMPONENT_PHYSICS)) {
                        entity_add_component(world, state.current_entity, COMPONENT_PHYSICS);
                    }
                } else if (state.in_components && strcmp(state.current_key, "collision") == 0) {
                    state.in_collision = true;
                    if (!entity_has_component(world, state.current_entity, COMPONENT_COLLISION)) {
                        entity_add_component(world, state.current_entity, COMPONENT_COLLISION);
                    }
                } else if (state.in_components && strcmp(state.current_key, "camera") == 0) {
                    state.in_camera = true;
                    if (!entity_has_component(world, state.current_entity, COMPONENT_CAMERA)) {
                        entity_add_component(world, state.current_entity, COMPONENT_CAMERA);
                    }
                } else if (state.in_components && strcmp(state.current_key, "thrusters") == 0) {
                    if (!entity_has_component(world, state.current_entity, COMPONENT_THRUSTER_SYSTEM)) {
                        entity_add_component(world, state.current_entity, COMPONENT_THRUSTER_SYSTEM);
                    }
                } else if (state.in_components && strcmp(state.current_key, "control_authority") == 0) {
                    if (!entity_has_component(world, state.current_entity, COMPONENT_CONTROL_AUTHORITY)) {
                        entity_add_component(world, state.current_entity, COMPONENT_CONTROL_AUTHORITY);
                    }
                }
                break;
                
            case YAML_MAPPING_END_EVENT:
                // Reset context flags
                if (state.in_physics) state.in_physics = false;
                else if (state.in_collision) state.in_collision = false;
                else if (state.in_camera) state.in_camera = false;
                else if (state.in_components) {
                    state.in_components = false;
                    printf("   🔍 DEBUG: Exiting components section\n");
                }
                
                // Handle entity mapping end based on nesting depth
                if (state.in_entities && state.current_entity != INVALID_ENTITY) {
                    state.entity_mapping_depth--;
                    printf("   🔍 DEBUG: Mapping end at depth %d for entity %d\n", 
                           state.entity_mapping_depth, state.current_entity);
                    
                    if (state.entity_mapping_depth == 0) {
                        // Only reset entity when we exit the top-level entity mapping
                        printf("   🔍 DEBUG: End of entity %d definition\n", state.current_entity);
                        state.current_entity = INVALID_ENTITY;
                    }
                }
                break;
                
            case YAML_SEQUENCE_START_EVENT:
                state.array_index = 0;
                state.expecting_value = false;  // Reset when starting a sequence
                if (strcmp(state.current_key, "entities") == 0) {
                    state.in_entities = true;
                    printf("📋 Loading entities...\n");
                } else if (strcmp(state.current_key, "position") == 0) {
                    state.in_position = true;
                    state.in_array = true;
                } else if (strcmp(state.current_key, "rotation") == 0) {
                    state.in_rotation = true;
                    state.in_array = true;
                } else if (strcmp(state.current_key, "scale") == 0) {
                    state.in_scale = true;
                    state.in_array = true;
                } else if (strcmp(state.current_key, "follow_offset") == 0 || 
                          strcmp(state.current_key, "size") == 0) {
                    state.in_array = true;
                }
                break;
                
            case YAML_SEQUENCE_END_EVENT:
                if (state.in_position) {
                    state.in_position = false;
                    state.in_array = false;
                } else if (state.in_rotation) {
                    state.in_rotation = false;
                    state.in_array = false;
                } else if (state.in_scale) {
                    state.in_scale = false;
                    state.in_array = false;
                } else if (state.in_entities) {
                    state.in_entities = false;
                    state.current_entity = INVALID_ENTITY;
                } else if (state.in_array) {
                    // Generic array end
                    state.in_array = false;
                }
                break;
                
            case YAML_SCALAR_EVENT: {
                char* value = (char*)event.data.scalar.value;

                // If we're in an array, all scalars are values
                if (state.in_array) {
                    if (state.in_position || state.in_rotation || state.in_scale ||
                        (strcmp(state.current_key, "follow_offset") == 0) ||
                        (strcmp(state.current_key, "size") == 0)) {
                        process_yaml_array_value(&state, value);
                    }
                } else if (!state.expecting_value) {
                    // This is a key
                    strncpy(state.current_key, value, sizeof(state.current_key) - 1);
                    state.expecting_value = true;
                } else {
                    // This is a value
                    state.expecting_value = false;
                    
                    // Handle entity creation
                    if (state.in_entities && strcmp(state.current_key, "type") == 0) {
                        // Create entity based on type
                        state.current_entity = entity_create(world);
                        if (state.current_entity != INVALID_ENTITY) {
                            printf("📦 Created entity %d of type: %s\n", state.current_entity, value);
                            
                            // Add basic components
                            entity_add_component(world, state.current_entity, COMPONENT_TRANSFORM);
                            
                            // Only add renderable component to non-camera entities
                            if (strcmp(value, "camera") != 0) {
                                entity_add_component(world, state.current_entity, COMPONENT_RENDERABLE);
                                
                                // Initialize renderable with empty resources
                                struct Renderable* renderable = entity_get_renderable(world, state.current_entity);
                                if (renderable) {
                                    renderable->gpu_resources = gpu_resources_create();
                                    renderable->visible = true;
                                    renderable->material_id = 0; // Default material
                                    renderable->index_count = 0; // Will be set when mesh loads
                                    printf("   Initialized renderable component (awaiting mesh)\n");
                                }
                            }
                            
                            // Add player component for player ships
                            if (strcmp(value, "player_ship") == 0) {
                                entity_add_component(world, state.current_entity, COMPONENT_PLAYER);
                                printf("   Added PLAYER component\n");
                                
                                // Configure ship with preset
                                control_configure_ship(world, state.current_entity, SHIP_CONFIG_FIGHTER);
                                printf("   Configured ship with FIGHTER preset\n");
                            }
                        }
                    } else {
                        process_yaml_value(&state, value);
                    }
                }
                break;
            }
                
            default:
                break;
        }
        
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);
    fclose(file);
    
    if (success) {
        printf("✅ Loaded YAML scene from %s\n", filename);
    }
    
    return success;
}

bool scene_yaml_loader_init(void) {
    // Nothing to initialize for now
    return true;
}

void scene_yaml_loader_shutdown(void) {
    // Nothing to cleanup for now
}