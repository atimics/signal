#include "../scene_script.h"
#include "../ship_assembly.h"
#include "../ship_part_meshes.h"
#include "../system/ode_physics.h"
#include "../system/ode_thrusters.h"
#include "../core.h"
#include <stdio.h>

static ShipPartLibrary* part_library = NULL;
static ShipAssembly* current_assembly = NULL;
static bool assembly_initialized = false;

void ship_assembly_test_enter(struct World* world, SceneStateManager* state) {
    (void)state;
    
    printf("\n🛠️ Ship Assembly Test Scene\n");
    printf("=========================\n");
    
    // Initialize part library
    part_library = calloc(1, sizeof(ShipPartLibrary));
    if (ship_parts_init(part_library)) {
        // Load part definitions
        ship_parts_load_directory(part_library, "data/ship_parts");
        
        // Initialize procedural meshes
        ship_part_meshes_init(get_asset_registry());
    }
    
    // Create a test assembly
    current_assembly = ship_assembly_create("Test Rocket Alpha");
    
    if (current_assembly && part_library) {
        printf("\n🚀 Building test rocket...\n");
        
        // Get parts
        ShipPart* pod = ship_parts_get(part_library, "hull_basic_pod");
        ShipPart* engine = ship_parts_get(part_library, "engine_rl10");
        ShipPart* rcs = ship_parts_get(part_library, "rcs_4way_block");
        
        if (pod) {
            // Add command pod as root
            Vector3 pod_pos = {0, 5, 0};
            ship_assembly_add_part(current_assembly, pod, NULL, -1, &pod_pos, NULL);
        }
        
        if (engine && current_assembly->root_part) {
            // Add engine below pod
            Vector3 engine_offset = {0, -2.5f, 0};
            ship_assembly_add_part(current_assembly, engine, 
                                 current_assembly->root_part, 0,
                                 &engine_offset, NULL);
        }
        
        if (rcs && current_assembly->root_part) {
            // Add RCS blocks
            Vector3 rcs_positions[4] = {
                {1.0f, 0.5f, 0},
                {-1.0f, 0.5f, 0},
                {0, 0.5f, 1.0f},
                {0, 0.5f, -1.0f}
            };
            
            for (int i = 0; i < 4; i++) {
                ship_assembly_add_part(current_assembly, rcs,
                                     current_assembly->root_part, i + 1,
                                     &rcs_positions[i], NULL);
            }
        }
        
        // Validate assembly
        if (ship_assembly_validate(current_assembly)) {
            printf("\n✅ Assembly validated successfully!\n");
            
            // Create physics representation
            ship_assembly_create_physics(current_assembly, world);
            
            // Create visual entities for each part
            for (int i = 0; i < current_assembly->num_parts; i++) {
                ShipPartInstance* part = current_assembly->parts[i];
                
                // Create entity for visual representation
                EntityID vis_entity = entity_create(world);
                entity_add_component(world, vis_entity, COMPONENT_TRANSFORM);
                entity_add_component(world, vis_entity, COMPONENT_RENDERABLE);
                
                struct Transform* transform = entity_get_transform(world, vis_entity);
                if (transform) {
                    transform->position = part->position;
                    transform->rotation = part->orientation;
                    transform->scale = (Vector3){1, 1, 1};
                }
                
                struct Renderable* renderable = entity_get_renderable(world, vis_entity);
                if (renderable && part->part_definition->mesh_name) {
                    assets_create_renderable_from_mesh(get_asset_registry(), 
                                                     part->part_definition->mesh_name,
                                                     renderable);
                }
            }
        }
    }
    
    assembly_initialized = true;
    
    printf("\n📋 CONTROLS:\n");
    printf("   F1 - Toggle part browser\n");
    printf("   F2 - Toggle assembly info\n");
    printf("   F3 - Test ignition sequence\n");
    printf("   SPACE - Launch assembled ship\n");
}

void ship_assembly_test_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)world;
    (void)state;
    
    if (!assembly_initialized) return;
    
    // Update assembly physics
    if (current_assembly) {
        ship_assembly_update(current_assembly, delta_time);
    }
}

void ship_assembly_test_exit(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    // Cleanup
    if (current_assembly) {
        ship_assembly_destroy(current_assembly);
        current_assembly = NULL;
    }
    
    if (part_library) {
        ship_parts_cleanup(part_library);
        free(part_library);
        part_library = NULL;
    }
    
    assembly_initialized = false;
    
    printf("🛠️ Ship assembly test scene cleaned up\n");
}