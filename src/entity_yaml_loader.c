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

// Helper function to parse boolean from string
static bool parse_bool(const char* str)
{
    if (!str) return false;
    return (strcmp(str, "true") == 0 || strcmp(str, "1") == 0 || strcmp(str, "yes") == 0);
}

// Helper to trim whitespace
static void trim_whitespace(char* str)
{
    if (!str) return;
    
    // Trim leading whitespace
    char* start = str;
    while (isspace(*start)) start++;
    
    // Trim trailing whitespace
    char* end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    // Null terminate
    *(end + 1) = '\0';
    
    // Move trimmed string to beginning
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// Parser state structure
typedef struct {
    DataRegistry* registry;
    EntityTemplate* current_template;
    char current_section[64];  // "components" or "properties"
    char current_key[64];
    bool in_templates;
    bool in_follow_offset;
    int template_count;
} YamlParserState;

// Process a key-value pair
static void process_yaml_pair(YamlParserState* state, const char* key, const char* value)
{
    if (!state || !key || !value || !state->current_template) return;
    
    char clean_key[64], clean_value[256];
    strncpy(clean_key, key, sizeof(clean_key) - 1);
    strncpy(clean_value, value, sizeof(clean_value) - 1);
    trim_whitespace(clean_key);
    trim_whitespace(clean_value);
    
    if (strcmp(state->current_section, "components") == 0) {
        // Handle component flags
        bool enable = parse_bool(clean_value);
        
        if (strcmp(clean_key, "transform") == 0) {
            state->current_template->has_transform = enable;
        } else if (strcmp(clean_key, "physics") == 0) {
            state->current_template->has_physics = enable;
        } else if (strcmp(clean_key, "collision") == 0) {
            state->current_template->has_collision = enable;
        } else if (strcmp(clean_key, "renderable") == 0) {
            state->current_template->has_renderable = enable;
        } else if (strcmp(clean_key, "ai") == 0) {
            state->current_template->has_ai = enable;
        } else if (strcmp(clean_key, "player") == 0) {
            state->current_template->has_player = enable;
        } else if (strcmp(clean_key, "thrusters") == 0) {
            state->current_template->has_thrusters = enable;
        } else if (strcmp(clean_key, "control_authority") == 0) {
            state->current_template->has_control_authority = enable;
        } else if (strcmp(clean_key, "camera") == 0) {
            state->current_template->has_camera = enable;
        }
    } else if (strcmp(state->current_section, "properties") == 0) {
        // Handle property values
        if (state->in_follow_offset) {
            // Handle follow_offset sub-properties
            if (strcmp(clean_key, "x") == 0) {
                state->current_template->follow_offset.x = atof(clean_value);
            } else if (strcmp(clean_key, "y") == 0) {
                state->current_template->follow_offset.y = atof(clean_value);
            } else if (strcmp(clean_key, "z") == 0) {
                state->current_template->follow_offset.z = atof(clean_value);
            }
        } else {
            // Handle regular properties
            if (strcmp(clean_key, "description") == 0) {
                strncpy(state->current_template->description, clean_value, 
                       sizeof(state->current_template->description) - 1);
            } else if (strcmp(clean_key, "mass") == 0) {
                state->current_template->mass = atof(clean_value);
            } else if (strcmp(clean_key, "collision_radius") == 0) {
                state->current_template->collision_radius = atof(clean_value);
            } else if (strcmp(clean_key, "drag") == 0) {
                state->current_template->drag = atof(clean_value);
            } else if (strcmp(clean_key, "kinematic") == 0) {
                state->current_template->kinematic = parse_bool(clean_value);
            } else if (strcmp(clean_key, "mesh_name") == 0) {
                strncpy(state->current_template->mesh_name, clean_value,
                       sizeof(state->current_template->mesh_name) - 1);
            } else if (strcmp(clean_key, "material_name") == 0) {
                strncpy(state->current_template->material_name, clean_value,
                       sizeof(state->current_template->material_name) - 1);
            } else if (strcmp(clean_key, "camera_behavior") == 0) {
                state->current_template->camera_behavior = parse_camera_behavior(clean_value);
            } else if (strcmp(clean_key, "fov") == 0) {
                state->current_template->fov = atof(clean_value);
            } else if (strcmp(clean_key, "near_plane") == 0) {
                state->current_template->near_plane = atof(clean_value);
            } else if (strcmp(clean_key, "far_plane") == 0) {
                state->current_template->far_plane = atof(clean_value);
            } else if (strcmp(clean_key, "follow_distance") == 0) {
                state->current_template->follow_distance = atof(clean_value);
            } else if (strcmp(clean_key, "follow_smoothing") == 0) {
                state->current_template->follow_smoothing = atof(clean_value);
            }
        }
    } else {
        // Handle top-level template properties
        if (strcmp(clean_key, "description") == 0) {
            strncpy(state->current_template->description, clean_value,
                   sizeof(state->current_template->description) - 1);
        }
    }
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
    yaml_document_t document;
    
    if (!yaml_parser_initialize(&parser)) {
        printf("❌ Failed to initialize YAML parser\n");
        fclose(file);
        return false;
    }
    
    yaml_parser_set_input_file(&parser, file);
    
    if (!yaml_parser_load(&parser, &document)) {
        printf("❌ Failed to load YAML document: %s\n", parser.problem);
        yaml_parser_delete(&parser);
        fclose(file);
        return false;
    }
    
    // Get the root node
    yaml_node_t* root = yaml_document_get_root_node(&document);
    if (!root || root->type != YAML_MAPPING_NODE) {
        printf("❌ YAML root is not a mapping\n");
        goto cleanup;
    }
    
    YamlParserState state = {0};
    state.registry = registry;
    
    // Find the "templates" mapping
    for (yaml_node_pair_t* pair = root->data.mapping.pairs.start;
         pair < root->data.mapping.pairs.top; pair++) {
        
        yaml_node_t* key_node = yaml_document_get_node(&document, pair->key);
        yaml_node_t* value_node = yaml_document_get_node(&document, pair->value);
        
        if (key_node->type == YAML_SCALAR_NODE &&
            strcmp((char*)key_node->data.scalar.value, "templates") == 0 &&
            value_node->type == YAML_MAPPING_NODE) {
            
            // Process each template
            for (yaml_node_pair_t* template_pair = value_node->data.mapping.pairs.start;
                 template_pair < value_node->data.mapping.pairs.top; template_pair++) {
                
                yaml_node_t* template_key = yaml_document_get_node(&document, template_pair->key);
                yaml_node_t* template_value = yaml_document_get_node(&document, template_pair->value);
                
                if (template_key->type != YAML_SCALAR_NODE || 
                    template_value->type != YAML_MAPPING_NODE) continue;
                
                // Create new template
                if (registry->entity_template_count >= 128) {
                    printf("❌ Too many entity templates (max 128)\n");
                    break;
                }
                
                state.current_template = &registry->entity_templates[registry->entity_template_count++];
                memset(state.current_template, 0, sizeof(EntityTemplate));
                
                // Set template name
                strncpy(state.current_template->name, (char*)template_key->data.scalar.value,
                       sizeof(state.current_template->name) - 1);
                
                // Set defaults
                state.current_template->scale = (Vector3){ 5.0f, 5.0f, 5.0f };
                state.current_template->mass = 1.0f;
                state.current_template->drag = 0.99f;
                state.current_template->collision_radius = 1.0f;
                state.current_template->layer_mask = 0xFFFFFFFF;
                state.current_template->visible = true;
                state.current_template->ai_update_frequency = 5.0f;
                
                // Process template properties
                for (yaml_node_pair_t* prop_pair = template_value->data.mapping.pairs.start;
                     prop_pair < template_value->data.mapping.pairs.top; prop_pair++) {
                    
                    yaml_node_t* prop_key = yaml_document_get_node(&document, prop_pair->key);
                    yaml_node_t* prop_value = yaml_document_get_node(&document, prop_pair->value);
                    
                    if (prop_key->type != YAML_SCALAR_NODE) continue;
                    
                    char* section_name = (char*)prop_key->data.scalar.value;
                    
                    if (strcmp(section_name, "description") == 0 && prop_value->type == YAML_SCALAR_NODE) {
                        strncpy(state.current_template->description, (char*)prop_value->data.scalar.value,
                               sizeof(state.current_template->description) - 1);
                    } else if (strcmp(section_name, "components") == 0 && prop_value->type == YAML_MAPPING_NODE) {
                        strncpy(state.current_section, "components", sizeof(state.current_section) - 1);
                        
                        // Process components
                        for (yaml_node_pair_t* comp_pair = prop_value->data.mapping.pairs.start;
                             comp_pair < prop_value->data.mapping.pairs.top; comp_pair++) {
                            
                            yaml_node_t* comp_key = yaml_document_get_node(&document, comp_pair->key);
                            yaml_node_t* comp_value = yaml_document_get_node(&document, comp_pair->value);
                            
                            if (comp_key->type == YAML_SCALAR_NODE && comp_value->type == YAML_SCALAR_NODE) {
                                process_yaml_pair(&state, (char*)comp_key->data.scalar.value,
                                                 (char*)comp_value->data.scalar.value);
                            }
                        }
                    } else if (strcmp(section_name, "properties") == 0 && prop_value->type == YAML_MAPPING_NODE) {
                        strncpy(state.current_section, "properties", sizeof(state.current_section) - 1);
                        
                        // Process properties
                        for (yaml_node_pair_t* prop_prop_pair = prop_value->data.mapping.pairs.start;
                             prop_prop_pair < prop_value->data.mapping.pairs.top; prop_prop_pair++) {
                            
                            yaml_node_t* prop_prop_key = yaml_document_get_node(&document, prop_prop_pair->key);
                            yaml_node_t* prop_prop_value = yaml_document_get_node(&document, prop_prop_pair->value);
                            
                            if (prop_prop_key->type != YAML_SCALAR_NODE) continue;
                            
                            char* prop_name = (char*)prop_prop_key->data.scalar.value;
                            
                            if (strcmp(prop_name, "follow_offset") == 0 && prop_prop_value->type == YAML_MAPPING_NODE) {
                                state.in_follow_offset = true;
                                
                                // Process follow_offset coordinates
                                for (yaml_node_pair_t* offset_pair = prop_prop_value->data.mapping.pairs.start;
                                     offset_pair < prop_prop_value->data.mapping.pairs.top; offset_pair++) {
                                    
                                    yaml_node_t* offset_key = yaml_document_get_node(&document, offset_pair->key);
                                    yaml_node_t* offset_value = yaml_document_get_node(&document, offset_pair->value);
                                    
                                    if (offset_key->type == YAML_SCALAR_NODE && offset_value->type == YAML_SCALAR_NODE) {
                                        process_yaml_pair(&state, (char*)offset_key->data.scalar.value,
                                                         (char*)offset_value->data.scalar.value);
                                    }
                                }
                                
                                state.in_follow_offset = false;
                            } else if (prop_prop_value->type == YAML_SCALAR_NODE) {
                                process_yaml_pair(&state, prop_name, (char*)prop_prop_value->data.scalar.value);
                            }
                        }
                    }
                }
                
                state.template_count++;
            }
            break;
        }
    }
    
cleanup:
    yaml_document_delete(&document);
    yaml_parser_delete(&parser);
    fclose(file);
    
    printf("   ✅ Loaded %d entity templates from YAML\n", state.template_count);
    return state.template_count > 0;
}
