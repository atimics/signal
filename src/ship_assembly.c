#include "ship_assembly.h"
#include "system/ode_physics.h"
#include "asset_loader/asset_loader_mesh.h"
#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// PART LIBRARY MANAGEMENT
// ============================================================================

bool ship_parts_init(ShipPartLibrary* library) {
    if (!library) return false;
    
    library->capacity = 128;
    library->num_parts = 0;
    library->parts = calloc(library->capacity, sizeof(ShipPart*));
    
    // Initialize category arrays
    library->parts_by_category = calloc(PART_PAYLOAD + 1, sizeof(ShipPart**));
    library->counts_by_category = calloc(PART_PAYLOAD + 1, sizeof(int));
    
    for (int i = 0; i <= PART_PAYLOAD; i++) {
        library->parts_by_category[i] = calloc(32, sizeof(ShipPart*));
    }
    
    printf("🚀 Ship part library initialized\n");
    return true;
}

// Parse a single part from YAML
static ShipPart* parse_ship_part_yaml(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("❌ Failed to open part file: %s\n", filepath);
        return NULL;
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    if (!yaml_parser_initialize(&parser)) {
        fclose(file);
        return NULL;
    }
    
    yaml_parser_set_input_file(&parser, file);
    
    ShipPart* part = calloc(1, sizeof(ShipPart));
    char current_key[64] = {0};
    bool in_physical = false;
    bool in_visual = false;
    bool in_attachment = false;
    bool in_properties = false;
    int attachment_index = -1;
    
    bool done = false;
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("❌ YAML parse error: %s\n", parser.problem);
            break;
        }
        
        switch (event.type) {
            case YAML_STREAM_END_EVENT:
                done = true;
                break;
                
            case YAML_MAPPING_START_EVENT:
                if (strcmp(current_key, "physical") == 0) {
                    in_physical = true;
                } else if (strcmp(current_key, "visual") == 0) {
                    in_visual = true;
                } else if (strcmp(current_key, "attachment_nodes") == 0) {
                    in_attachment = true;
                    part->attachment_nodes = calloc(8, sizeof(AttachmentNode));
                } else if (strcmp(current_key, "properties") == 0) {
                    in_properties = true;
                }
                break;
                
            case YAML_MAPPING_END_EVENT:
                if (in_physical) in_physical = false;
                else if (in_visual) in_visual = false;
                else if (attachment_index >= 0) {
                    attachment_index = -1;
                    part->num_attachments++;
                }
                break;
                
            case YAML_SEQUENCE_START_EVENT:
                if (in_attachment) {
                    attachment_index = 0;
                }
                break;
                
            case YAML_SCALAR_EVENT: {
                char* value = (char*)event.data.scalar.value;
                
                if (strlen(current_key) == 0) {
                    strncpy(current_key, value, sizeof(current_key) - 1);
                } else {
                    // Process key-value pair
                    if (strcmp(current_key, "part_id") == 0) {
                        part->part_id = strdup(value);
                    } else if (strcmp(current_key, "display_name") == 0) {
                        part->display_name = strdup(value);
                    } else if (strcmp(current_key, "category") == 0) {
                        if (strcmp(value, "HULL") == 0) part->category = PART_HULL;
                        else if (strcmp(value, "THRUSTER") == 0) part->category = PART_THRUSTER;
                        else if (strcmp(value, "TANK") == 0) part->category = PART_TANK;
                        // ... etc
                    } else if (in_physical) {
                        if (strcmp(current_key, "mass") == 0) {
                            part->mass = atof(value);
                        } else if (strcmp(current_key, "drag_coefficient") == 0) {
                            part->drag_coefficient = atof(value);
                        }
                    } else if (in_visual) {
                        if (strcmp(current_key, "mesh") == 0) {
                            part->mesh_name = strdup(value);
                        } else if (strcmp(current_key, "material") == 0) {
                            part->material_name = strdup(value);
                        }
                    }
                    
                    current_key[0] = '\0';
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
    
    printf("✅ Loaded ship part: %s (%s)\n", part->part_id, part->display_name);
    return part;
}

bool ship_parts_load_directory(ShipPartLibrary* library, const char* directory) {
    if (!library || !directory) return false;
    
    // TODO: Implement directory scanning
    // For now, manually load known parts
    
    const char* part_files[] = {
        "data/ship_parts/hulls/basic_pod.yaml",
        "data/ship_parts/thrusters/rl10_engine.yaml",
        "data/ship_parts/thrusters/rcs_block.yaml",
        NULL
    };
    
    for (int i = 0; part_files[i] != NULL; i++) {
        ShipPart* part = parse_ship_part_yaml(part_files[i]);
        if (part) {
            // Add to main library
            if (library->num_parts < library->capacity) {
                library->parts[library->num_parts++] = part;
                
                // Add to category array
                int cat = part->category;
                library->parts_by_category[cat][library->counts_by_category[cat]++] = part;
            }
        }
    }
    
    printf("📚 Loaded %d ship parts\n", library->num_parts);
    return true;
}

ShipPart* ship_parts_get(ShipPartLibrary* library, const char* part_id) {
    if (!library || !part_id) return NULL;
    
    for (int i = 0; i < library->num_parts; i++) {
        if (strcmp(library->parts[i]->part_id, part_id) == 0) {
            return library->parts[i];
        }
    }
    return NULL;
}

// ============================================================================
// SHIP ASSEMBLY
// ============================================================================

ShipAssembly* ship_assembly_create(const char* name) {
    ShipAssembly* assembly = calloc(1, sizeof(ShipAssembly));
    if (!assembly) return NULL;
    
    assembly->ship_name = strdup(name);
    assembly->parts = calloc(64, sizeof(ShipPartInstance*));
    assembly->num_parts = 0;
    
    // Create ODE collision space for ship
    assembly->collision_space = dHashSpaceCreate(0);
    
    printf("🛠️ Created ship assembly: %s\n", name);
    return assembly;
}

bool ship_assembly_add_part(ShipAssembly* assembly, 
                           ShipPart* part,
                           ShipPartInstance* attach_to,
                           int attachment_index,
                           const Vector3* position,
                           const Quaternion* orientation) {
    if (!assembly || !part) return false;
    
    ShipPartInstance* instance = calloc(1, sizeof(ShipPartInstance));
    instance->part_definition = part;
    
    // Set transform
    if (position) instance->position = *position;
    if (orientation) instance->orientation = *orientation;
    else instance->orientation = (Quaternion){0, 0, 0, 1};
    
    // Create ODE body
    instance->body = dBodyCreate(ode_get_world());
    
    // Set mass properties
    dMass mass;
    dMassSetBoxTotal(&mass, part->mass, 1.0, 1.0, 1.0); // TODO: Use actual dimensions
    dBodySetMass(instance->body, &mass);
    
    // Set position
    dBodySetPosition(instance->body, 
                    instance->position.x,
                    instance->position.y, 
                    instance->position.z);
    
    // Create joint to parent
    if (attach_to && attach_to->body) {
        // Create fixed joint for now
        dJointID joint = dJointCreateFixed(ode_get_world(), 0);
        dJointAttach(joint, attach_to->body, instance->body);
        dJointSetFixed(joint);
        
        instance->joints = calloc(1, sizeof(dJointID));
        instance->joints[0] = joint;
        instance->num_joints = 1;
        
        printf("  📎 Attached %s to parent\n", part->display_name);
    } else if (!assembly->root_part) {
        // This is the root part
        assembly->root_part = instance;
        printf("  🏠 Set as root part: %s\n", part->display_name);
    }
    
    // Add to assembly
    assembly->parts[assembly->num_parts++] = instance;
    
    // Update assembly properties
    assembly->total_mass += part->mass;
    
    printf("✅ Added part to assembly: %s (total parts: %d)\n", 
           part->display_name, assembly->num_parts);
    
    return true;
}

bool ship_assembly_validate(ShipAssembly* assembly) {
    if (!assembly || !assembly->root_part) {
        printf("❌ Invalid assembly: no root part\n");
        return false;
    }
    
    // Check structural integrity
    bool all_connected = true;
    for (int i = 0; i < assembly->num_parts; i++) {
        ShipPartInstance* part = assembly->parts[i];
        if (part != assembly->root_part && part->num_joints == 0) {
            printf("⚠️ Part not connected: %s\n", part->part_definition->display_name);
            all_connected = false;
        }
    }
    
    // Calculate center of mass
    Vector3 com = {0, 0, 0};
    float total_mass = 0;
    
    for (int i = 0; i < assembly->num_parts; i++) {
        ShipPartInstance* part = assembly->parts[i];
        float m = part->part_definition->mass;
        com.x += part->position.x * m;
        com.y += part->position.y * m;
        com.z += part->position.z * m;
        total_mass += m;
    }
    
    if (total_mass > 0) {
        com.x /= total_mass;
        com.y /= total_mass;
        com.z /= total_mass;
        assembly->center_of_mass = com;
    }
    
    printf("📊 Assembly validation:\n");
    printf("   Total mass: %.1f kg\n", total_mass);
    printf("   Center of mass: (%.2f, %.2f, %.2f)\n", com.x, com.y, com.z);
    printf("   All connected: %s\n", all_connected ? "✅" : "❌");
    
    return all_connected;
}

bool ship_assembly_create_physics(ShipAssembly* assembly, struct World* world) {
    if (!assembly || !world) return false;
    
    // Create composite body by combining all parts
    // For now, keep individual bodies for flexibility
    
    // TODO: Optimize by creating single composite body for performance
    
    printf("🔧 Created physics representation for %d parts\n", assembly->num_parts);
    return true;
}

void ship_assembly_update(ShipAssembly* assembly, float delta_time) {
    if (!assembly) return;
    
    // Update part states
    for (int i = 0; i < assembly->num_parts; i++) {
        ShipPartInstance* part = assembly->parts[i];
        
        // Get position/orientation from ODE
        if (part->body) {
            const dReal* pos = dBodyGetPosition(part->body);
            const dReal* rot = dBodyGetQuaternion(part->body);
            
            part->position = (Vector3){pos[0], pos[1], pos[2]};
            part->orientation = (Quaternion){rot[1], rot[2], rot[3], rot[0]};
        }
    }
}

void ship_assembly_destroy(ShipAssembly* assembly) {
    if (!assembly) return;
    
    // Destroy all ODE bodies and joints
    for (int i = 0; i < assembly->num_parts; i++) {
        ShipPartInstance* part = assembly->parts[i];
        
        // Destroy joints
        for (int j = 0; j < part->num_joints; j++) {
            if (part->joints[j]) {
                dJointDestroy(part->joints[j]);
            }
        }
        free(part->joints);
        
        // Destroy body
        if (part->body) {
            dBodyDestroy(part->body);
        }
        
        free(part);
    }
    
    // Destroy collision space
    if (assembly->collision_space) {
        dSpaceDestroy(assembly->collision_space);
    }
    
    free(assembly->parts);
    free(assembly->ship_name);
    free(assembly);
}

void ship_parts_cleanup(ShipPartLibrary* library) {
    if (!library) return;
    
    // Free all parts
    for (int i = 0; i < library->num_parts; i++) {
        ShipPart* part = library->parts[i];
        free(part->part_id);
        free(part->display_name);
        free(part->mesh_name);
        free(part->material_name);
        free(part->attachment_nodes);
        free(part->properties);
        free(part);
    }
    
    // Free arrays
    for (int i = 0; i <= PART_PAYLOAD; i++) {
        free(library->parts_by_category[i]);
    }
    free(library->parts_by_category);
    free(library->counts_by_category);
    free(library->parts);
}