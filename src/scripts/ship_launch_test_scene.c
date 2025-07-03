#include "../scene_script.h"
#include "../component/thruster_points_component.h"
#include "../render_thrust_cones.h"
#include "../system/physics.h"
#include "../system/control.h"
#include "../system/camera.h"
#include "../assets.h"
#include "../systems.h"  // For get_asset_registry
#include "sokol_app.h"  // For sapp_event
#include <stdio.h>
#include <stdlib.h>  // For calloc

// Scene state
typedef struct {
    EntityID ship_entity;
    EntityID camera_entity;
    EntityID launchpad_entity;
    ThrusterPointsComponent* thruster_component;
    float launch_countdown;
    bool launched;
    bool auto_launch;
} ShipLaunchState;

static ShipLaunchState* launch_state = NULL;

void ship_launch_test_enter(struct World* world, SceneStateManager* state) {
    (void)state;
    
    printf("\n🚀 Ship Launch Test Scene\n");
    printf("========================\n");
    
    // Create scene state
    launch_state = calloc(1, sizeof(ShipLaunchState));
    launch_state->launch_countdown = 3.0f;
    launch_state->auto_launch = true;
    
    // Create launchpad
    launch_state->launchpad_entity = entity_create(world);
    entity_add_component(world, launch_state->launchpad_entity, COMPONENT_TRANSFORM);
    entity_add_component(world, launch_state->launchpad_entity, COMPONENT_RENDERABLE);
    
    struct Transform* pad_transform = entity_get_transform(world, launch_state->launchpad_entity);
    if (pad_transform) {
        pad_transform->position = (Vector3){0, 0, 0};
        pad_transform->scale = (Vector3){10, 0.5f, 10};
    }
    
    struct Renderable* pad_render = entity_get_renderable(world, launch_state->launchpad_entity);
    if (pad_render) {
        assets_create_renderable_from_mesh(get_asset_registry(), "landing_pad", pad_render);
    }
    
    // Create ship on launchpad
    launch_state->ship_entity = entity_create(world);
    entity_add_components(world, launch_state->ship_entity, 
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_RENDERABLE | 
        COMPONENT_CONTROL_AUTHORITY | COMPONENT_PLAYER);
    
    struct Transform* ship_transform = entity_get_transform(world, launch_state->ship_entity);
    if (ship_transform) {
        ship_transform->position = (Vector3){0, 3, 0};  // On pad
        ship_transform->rotation = (Quaternion){0, 0, 0, 1};
        ship_transform->scale = (Vector3){1, 1, 1};
    }
    
    struct Physics* ship_physics = entity_get_physics(world, launch_state->ship_entity);
    if (ship_physics) {
        ship_physics->mass = 1000.0f;  // 1 ton ship
        ship_physics->drag_linear = 0.1f;
        ship_physics->drag_angular = 0.2f;
        ship_physics->has_6dof = true;
        ship_physics->moment_of_inertia = (Vector3){500, 800, 500};
        ship_physics->environment = PHYSICS_ATMOSPHERE;  // Launch from surface
    }
    
    struct Renderable* ship_render = entity_get_renderable(world, launch_state->ship_entity);
    if (ship_render) {
        assets_create_renderable_from_mesh(get_asset_registry(), "wedge_ship_mk2", ship_render);
    }
    
    // Initialize simplified thruster system
    launch_state->thruster_component = thruster_points_component_create("data/thrusters/wedge_ship_mk2.thrusters");
    if (launch_state->thruster_component) {
        // Load and validate thrusters
        const Mesh* ship_mesh = assets_get_mesh(get_asset_registry(), "wedge_ship_mk2");
        thruster_points_component_init(launch_state->thruster_component, ship_mesh);
        launch_state->thruster_component->visual_debug = true;
    }
    
    // Setup control authority for manual control
    struct ControlAuthority* control = entity_get_control_authority(world, launch_state->ship_entity);
    if (control) {
        control->control_mode = CONTROL_MANUAL;
        control->control_sensitivity = 2.0f;
        control->stability_assist = 0.5f;
        control->flight_assist_enabled = true;
    }
    
    // Create chase camera
    launch_state->camera_entity = entity_create(world);
    entity_add_component(world, launch_state->camera_entity, COMPONENT_CAMERA);
    entity_add_component(world, launch_state->camera_entity, COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(world, launch_state->camera_entity);
    if (camera) {
        camera->behavior = CAMERA_BEHAVIOR_CHASE;
        camera->follow_target = launch_state->ship_entity;
        camera->follow_distance = 15.0f;
        camera->follow_offset = (Vector3){0, 5, -10};
        camera->follow_smoothing = 0.1f;
        camera->is_active = true;
        camera->fov = 60.0f;
        camera->near_plane = 0.1f;
        camera->far_plane = 1000.0f;
    }
    
    // Add some reference objects
    for (int i = 0; i < 3; i++) {
        EntityID marker = entity_create(world);
        entity_add_component(world, marker, COMPONENT_TRANSFORM);
        entity_add_component(world, marker, COMPONENT_RENDERABLE);
        
        struct Transform* t = entity_get_transform(world, marker);
        if (t) {
            t->position = (Vector3){(i - 1) * 20.0f, 2, 50};
            t->scale = (Vector3){2, 10, 2};
        }
        
        struct Renderable* r = entity_get_renderable(world, marker);
        if (r) {
            assets_create_renderable_from_mesh(get_asset_registry(), "logo_cube", r);
        }
    }
    
    printf("\n📋 CONTROLS:\n");
    printf("   SPACE     - Manual launch (cancel auto-launch)\n");
    printf("   WASD      - Pitch/Yaw control\n");
    printf("   Q/E       - Roll\n");
    printf("   Shift/Ctrl - Vertical thrust\n");
    printf("   X         - Cut engines\n");
    printf("   V         - Toggle thruster visualization\n");
    printf("   ESC       - Return to menu\n");
    printf("\n⏱️  Auto-launch in 3 seconds...\n");
}

void ship_launch_test_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)state;
    
    if (!launch_state) return;
    
    // Handle countdown and auto-launch
    if (launch_state->auto_launch && !launch_state->launched) {
        launch_state->launch_countdown -= delta_time;
        
        if (launch_state->launch_countdown <= 0) {
            printf("🚀 LAUNCH! Main engines ignited!\n");
            launch_state->launched = true;
            launch_state->auto_launch = false;
        } else if ((int)(launch_state->launch_countdown + delta_time) != (int)launch_state->launch_countdown) {
            printf("⏱️  T-%d...\n", (int)launch_state->launch_countdown + 1);
        }
    }
    
    // Update thruster system
    if (launch_state->thruster_component && launch_state->thruster_component->initialized) {
        struct ControlAuthority* control = entity_get_control_authority(world, launch_state->ship_entity);
        
        if (control) {
            Vector3 linear_command = {0, 0, 0};
            Vector3 angular_command = {0, 0, 0};
            
            // Auto-launch sequence
            if (launch_state->launched && launch_state->launch_countdown > -5.0f) {
                linear_command.z = -1.0f;  // Main engines full
                linear_command.y = 0.3f;    // Some upward thrust
                launch_state->launch_countdown -= delta_time;
            } else {
                // Manual control
                linear_command = control->input_linear;
                angular_command = control->input_angular;
            }
            
            // Update thruster states
            thruster_points_component_update(launch_state->thruster_component,
                                           &linear_command, &angular_command);
            
            // Apply forces to physics
            struct Transform* transform = entity_get_transform(world, launch_state->ship_entity);
            struct Physics* physics = entity_get_physics(world, launch_state->ship_entity);
            
            if (transform && physics) {
                thruster_points_component_apply_forces(launch_state->thruster_component,
                                                     transform, physics);
            }
        }
    }
    
    // Transition to space when high enough
    struct Transform* ship_transform = entity_get_transform(world, launch_state->ship_entity);
    struct Physics* ship_physics = entity_get_physics(world, launch_state->ship_entity);
    
    if (ship_transform && ship_physics) {
        if (ship_transform->position.y > 100.0f && ship_physics->environment != PHYSICS_SPACE) {
            ship_physics->environment = PHYSICS_SPACE;
            printf("🌌 Reached space! Drag reduced.\n");
        }
        
        // Print altitude occasionally
        static float altitude_timer = 0;
        altitude_timer += delta_time;
        if (altitude_timer > 2.0f) {
            altitude_timer = 0;
            printf("📍 Altitude: %.1fm, Speed: %.1fm/s\n", 
                   ship_transform->position.y, vector3_length(ship_physics->velocity));
        }
    }
}

bool ship_launch_test_input(struct World* world, SceneStateManager* state, const void* event) {
    (void)world;
    (void)state;
    
    if (!launch_state) return false;
    
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch (ev->key_code) {
            case SAPP_KEYCODE_SPACE:
                if (!launch_state->launched) {
                    printf("🚀 Manual launch activated!\n");
                    launch_state->launched = true;
                    launch_state->auto_launch = false;
                    launch_state->launch_countdown = 0;
                }
                return true;
                
            case SAPP_KEYCODE_X:
                printf("❌ Engines cut!\n");
                // Zero all thrusters
                if (launch_state->thruster_component) {
                    Vector3 zero = {0, 0, 0};
                    thruster_points_component_update(launch_state->thruster_component,
                                                   &zero, &zero);
                }
                return true;
                
            case SAPP_KEYCODE_V:
                if (launch_state->thruster_component) {
                    launch_state->thruster_component->visual_debug = 
                        !launch_state->thruster_component->visual_debug;
                    printf("👁️  Thruster visualization: %s\n",
                           launch_state->thruster_component->visual_debug ? "ON" : "OFF");
                }
                return true;
                
            default:
                // Handle all other keys
                break;
        }
    }
    
    return false;
}

void ship_launch_test_exit(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (launch_state) {
        if (launch_state->thruster_component) {
            thruster_points_component_destroy(launch_state->thruster_component);
        }
        free(launch_state);
        launch_state = NULL;
    }
    
    printf("🚀 Ship launch test scene cleaned up\n");
}

// Register scene script
const SceneScript ship_launch_test_script = {
    .scene_name = "ship_launch_test",
    .on_enter = ship_launch_test_enter,
    .on_update = ship_launch_test_update,
    .on_input = ship_launch_test_input,
    .on_exit = ship_launch_test_exit
};