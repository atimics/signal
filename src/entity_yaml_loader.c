#include "entity_yaml_loader.h"
#include "data.h"
#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static bool g_entity_yaml_loader_initialized = false;

bool entity_yaml_loader_init(void)
{
    if (g_entity_yaml_loader_initialized) {
        return true;
    }
    
    g_entity_yaml_loader_initialized = true;
    printf("📝 Entity YAML loader initialized\n");
    return true;
}

void entity_yaml_loader_shutdown(void)
{
    if (!g_entity_yaml_loader_initialized) {
        return;
    }
    
    g_entity_yaml_loader_initialized = false;
    printf("📝 Entity YAML loader shutdown\n");
}

// Helper function to parse camera behavior string
static int parse_camera_behavior(const char* behavior_str)
{
    if (!behavior_str) return 0;
    
    if (strcmp(behavior_str, "third_person") == 0) return 0;
    if (strcmp(behavior_str, "first_person") == 0) return 1;
    if (strcmp(behavior_str, "static") == 0) return 2;
    if (strcmp(behavior_str, "chase") == 0) return 3;
    if (strcmp(behavior_str, "orbital") == 0) return 4;
    
    return 0; // Default to third person
}

// Parse state for YAML processing
typedef struct {
    DataRegistry* registry;
    EntityTemplate* current_template;
    int depth;
    bool in_templates;
    bool in_template;
    bool in_components;
    bool in_properties; 
    bool in_follow_offset;
    char current_key[64];
    char current_template_name[64];
} YAMLParseState;

// Initialize parse state
static void init_parse_state(YAMLParseState* state, DataRegistry* registry)
{
    memset(state, 0, sizeof(YAMLParseState));
    state->registry = registry;
}

// Set component flags based on YAML
static void set_component_flag(EntityTemplate* template, const char* component_name, bool value)
{
    if (!template || !component_name) return;
    
    if (strcmp(component_name, "transform") == 0) {
        template->has_transform = value;
    } else if (strcmp(component_name, "physics") == 0) {
        template->has_physics = value;
    } else if (strcmp(component_name, "collision") == 0) {
        template->has_collision = value;
    } else if (strcmp(component_name, "renderable") == 0) {
        template->has_renderable = value;
    } else if (strcmp(component_name, "ai") == 0) {
        template->has_ai = value;
    } else if (strcmp(component_name, "player") == 0) {
        template->has_player = value;
    } else if (strcmp(component_name, "thrusters") == 0) {
        template->has_thrusters = value;
    } else if (strcmp(component_name, "control_authority") == 0) {
        template->has_control_authority = value;
    } else if (strcmp(component_name, "camera") == 0) {
        template->has_camera = value;
    }
}

// Set property values based on YAML
static void set_property_value(EntityTemplate* template, const char* property_name, const char* value)
{
    if (!template || !property_name || !value) return;
    
    if (strcmp(property_name, "mass") == 0) {
        template->mass = (float)atof(value);
    } else if (strcmp(property_name, "collision_radius") == 0) {
        template->collision_radius = (float)atof(value);
    } else if (strcmp(property_name, "drag") == 0) {
        template->drag = (float)atof(value);
    } else if (strcmp(property_name, "kinematic") == 0) {
        template->kinematic = (strcmp(value, "true") == 0);
    } else if (strcmp(property_name, "mesh_name") == 0) {
        strncpy(template->mesh_name, value, sizeof(template->mesh_name) - 1);
    } else if (strcmp(property_name, "material_name") == 0) {
        strncpy(template->material_name, value, sizeof(template->material_name) - 1);
    } else if (strcmp(property_name, "camera_behavior") == 0) {
        template->camera_behavior = parse_camera_behavior(value);
    } else if (strcmp(property_name, "fov") == 0) {
        template->fov = (float)atof(value);
    } else if (strcmp(property_name, "near_plane") == 0) {
        template->near_plane = (float)atof(value);
    } else if (strcmp(property_name, "far_plane") == 0) {
        template->far_plane = (float)atof(value);
    } else if (strcmp(property_name, "follow_distance") == 0) {
        template->follow_distance = (float)atof(value);
    } else if (strcmp(property_name, "follow_smoothing") == 0) {
        template->follow_smoothing = (float)atof(value);
    } else if (strcmp(property_name, "description") == 0) {
        strncpy(template->description, value, sizeof(template->description) - 1);
    }
}

// Handle follow_offset properties
static void set_follow_offset_property(EntityTemplate* template, const char* axis, const char* value)
{
    if (!template || !axis || !value) return;
    
    float val = (float)atof(value);
    if (strcmp(axis, "x") == 0) {
        template->follow_offset.x = val;
    } else if (strcmp(axis, "y") == 0) {
        template->follow_offset.y = val;
    } else if (strcmp(axis, "z") == 0) {
        template->follow_offset.z = val;
    }
}

// Process YAML scalar events
static bool process_scalar_event(YAMLParseState* state, const char* value)
{
    if (state->depth == 1 && strcmp(value, "templates") == 0) {
        state->in_templates = true;
        return true;
    }
    
    if (state->depth == 2 && state->in_templates) {
        // Template name
        if (state->registry->entity_template_count >= 128) {
            printf("❌ Too many entity templates (max 128)\n");
            return false;
        }
        
        state->current_template = &state->registry->entity_templates[state->registry->entity_template_count++];
        memset(state->current_template, 0, sizeof(EntityTemplate));
        
        strncpy(state->current_template->name, value, sizeof(state->current_template->name) - 1);
        strncpy(state->current_template_name, value, sizeof(state->current_template_name) - 1);
        
        // Set defaults
        state->current_template->scale = (Vector3){ 5.0f, 5.0f, 5.0f };
        state->current_template->mass = 1.0f;
        state->current_template->drag = 0.99f;
        state->current_template->collision_radius = 1.0f;
        state->current_template->layer_mask = 0xFFFFFFFF;
        state->current_template->visible = true;
        state->current_template->ai_update_frequency = 5.0f;
        
        state->in_template = true;
        return true;
    }
    
    if (state->depth == 3 && state->current_template) {
        if (strcmp(value, "description") == 0) {
            strncpy(state->current_key, "description", sizeof(state->current_key) - 1);
        } else if (strcmp(value, "components") == 0) {
            state->in_components = true;
        } else if (strcmp(value, "properties") == 0) {
            state->in_properties = true;
        }
        return true;
    }
    
    if (state->depth == 4 && state->current_template) {
        if (state->in_components) {
            // Component key - next value will be boolean
            strncpy(state->current_key, value, sizeof(state->current_key) - 1);
        } else if (state->in_properties) {
            if (strcmp(value, "follow_offset") == 0) {
                state->in_follow_offset = true;
            } else {
                strncpy(state->current_key, value, sizeof(state->current_key) - 1);
            }
        } else if (strcmp(state->current_key, "description") == 0) {
            set_property_value(state->current_template, "description", value);
            state->current_key[0] = '\0';
        }
        return true;
    }
    
    if (state->depth == 5 && state->current_template) {
        if (state->in_components && strlen(state->current_key) > 0) {
            // Component value
            bool component_value = (strcmp(value, "true") == 0);
            set_component_flag(state->current_template, state->current_key, component_value);
            state->current_key[0] = '\0';
        } else if (state->in_properties && strlen(state->current_key) > 0) {
            // Property value
            set_property_value(state->current_template, state->current_key, value);
            state->current_key[0] = '\0';
        } else if (state->in_follow_offset) {
            // follow_offset axis key
            strncpy(state->current_key, value, sizeof(state->current_key) - 1);
        }
        return true;
    }
    
    if (state->depth == 6 && state->current_template && state->in_follow_offset) {
        // follow_offset value
        set_follow_offset_property(state->current_template, state->current_key, value);
        state->current_key[0] = '\0';
        return true;
    }
    
    return true;
}

bool load_entity_templates_yaml(DataRegistry* registry, const char* filename)
{
    if (!registry || !filename) {
        printf("❌ Invalid parameters for YAML entity template loading\n");
        return false;
    }
    
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", registry->data_root, filename);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        printf("⚠️  Could not open entity templates YAML: %s\n", full_path);
        return false;
    }
    
    printf("📝 Loading entity templates from YAML: %s\n", full_path);
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    if (!yaml_parser_initialize(&parser)) {
        printf("❌ Failed to initialize YAML parser\n");
        fclose(file);
        return false;
    }
    
    yaml_parser_set_input_file(&parser, file);
    
    YAMLParseState state;
    init_parse_state(&state, registry);
    
    bool done = false;
    bool success = true;
    
    while (!done && success) {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("❌ YAML parse error: %s\n", parser.problem);
            success = false;
            break;
        }
        
        switch (event.type) {
            case YAML_STREAM_START_EVENT:
                break;
                
            case YAML_DOCUMENT_START_EVENT:
                break;
                
            case YAML_MAPPING_START_EVENT:
                state.depth++;
                break;
                
            case YAML_MAPPING_END_EVENT:
                state.depth--;
                if (state.depth == 2 && state.in_template) {
                    // End of template
                    state.in_template = false;
                    state.current_template = NULL;
                    state.current_template_name[0] = '\0';
                }
                if (state.depth == 3) {
                    state.in_components = false;
                    state.in_properties = false;
                }
                if (state.depth == 4) {
                    state.in_follow_offset = false;
                }
                break;
                
            case YAML_SCALAR_EVENT: {
                const char* value = (const char*)event.data.scalar.value;
                if (!process_scalar_event(&state, value)) {
                    success = false;
                }
                break;
            }
            
            case YAML_STREAM_END_EVENT:
                done = true;
                break;
                
            default:
                break;
        }
        
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);
    fclose(file);
    
    if (success) {
        printf("   ✅ Loaded %d entity templates from YAML\n", registry->entity_template_count);
    } else {
        printf("   ❌ Failed to load entity templates from YAML\n");
    }
    
    return success;
}
