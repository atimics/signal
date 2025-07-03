#include "scene_script.h"
#include "../system/input.h"
#include "../system/control.h"
#include "../component/physics.h"
#include "../component/thruster_system.h"
#include "../hud_system.h"
#include <stdio.h>
#include <math.h>

// ============================================================================
// THRUSTER TEST SCENE - Gyroscopic Control Prototype
// ============================================================================

static bool thruster_test_initialized = false;
static EntityID test_ship_id = INVALID_ENTITY;
static float test_time = 0.0f;

// Debug display state
static bool show_thrust_vectors = true;
static bool show_physics_debug = true;
static bool show_thruster_groups = true;

void thruster_test_enter(struct World* world, SceneStateManager* state) {
    (void)state;
    
    printf("🚀 Initializing Thruster Test Scene - Gyroscopic Control Prototype\n");
    
    // Find the player ship
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            test_ship_id = entity->id;
            printf("🎯 Test ship found: Entity ID %d\n", test_ship_id);
            break;
        }
    }
    
    if (test_ship_id == INVALID_ENTITY) {
        printf("❌ No player ship found in scene!\n");
        return;
    }
    
    // Enhance ship with gyroscopic control
    struct Physics* physics = entity_get_physics(world, test_ship_id);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, test_ship_id);
    struct ControlAuthority* control = entity_get_control_authority(world, test_ship_id);
    
    if (physics && thrusters && control) {
        // Configure for gyroscopic stabilization testing
        physics->drag_angular = 0.5f;  // Low angular drag for realistic space physics
        physics->has_6dof = true;
        
        // Configure thruster system for RCS
        thrusters->ship_type = SHIP_TYPE_FIGHTER;
        thrusters->max_linear_force = (Vector3){5000, 5000, 15000};  // Strong main engines
        thrusters->max_angular_torque = (Vector3){1000, 1000, 800};  // RCS torque
        thrusters->thrust_response_time = 0.1f;  // Quick response
        thrusters->vacuum_efficiency = 1.0f;
        thrusters->thrusters_enabled = true;
        
        // Control settings
        control->control_sensitivity = 1.5f;
        control->stability_assist = 0.0f;  // Let gyroscopic system handle stability
        control->flight_assist_enabled = false;  // Pure manual control
        control->control_mode = CONTROL_MANUAL;
        
        printf("✅ Ship configured for gyroscopic control testing\n");
        printf("   Angular Drag: %.2f\n", physics->drag_angular);
        printf("   Max Torque: [%.0f, %.0f, %.0f] N⋅m\n", 
               thrusters->max_angular_torque.x,
               thrusters->max_angular_torque.y, 
               thrusters->max_angular_torque.z);
    }
    
    // Initialize input system
    input_init();
    control_set_player_entity(world, test_ship_id);
    
    // Set HUD to chase near mode for testing
    hud_system_set_camera_mode(HUD_CAMERA_MODE_CHASE_NEAR);
    
    thruster_test_initialized = true;
    test_time = 0.0f;
    
    printf("\n📋 THRUSTER TEST CONTROLS:\n");
    printf("   F1 - Toggle thrust vector display\n");
    printf("   F2 - Toggle physics debug info\n");
    printf("   F3 - Toggle thruster group display\n");
    printf("   F5 - Reset ship position\n");
    printf("\n🎮 GYROSCOPIC STABILIZATION:\n");
    printf("   - Ship automatically counters rotation when stick released\n");
    printf("   - RCS thrusters fire to stop spin\n");
    printf("   - Realistic momentum and inertia\n");
}

void thruster_test_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)state;
    
    if (!thruster_test_initialized) return;
    
    test_time += delta_time;
    
    // Get ship components
    struct Physics* physics = entity_get_physics(world, test_ship_id);
    struct Transform* transform = entity_get_transform(world, test_ship_id);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, test_ship_id);
    
    if (!physics || !transform || !thrusters) return;
    
    // Debug output every second
    static float debug_timer = 0.0f;
    debug_timer += delta_time;
    
    if (debug_timer >= 1.0f && show_physics_debug) {
        debug_timer = 0.0f;
        
        // Calculate speeds
        float linear_speed = vector3_length(physics->velocity);
        float angular_speed = vector3_length(physics->angular_velocity);
        
        printf("\n🚀 THRUSTER TEST STATUS (T=%.1fs):\n", test_time);
        printf("   Position: [%.1f, %.1f, %.1f]\n", 
               transform->position.x, transform->position.y, transform->position.z);
        printf("   Linear: %.1f m/s | Angular: %.2f rad/s\n", linear_speed, angular_speed);
        printf("   Angular Vel: [%.3f, %.3f, %.3f] rad/s\n",
               physics->angular_velocity.x, physics->angular_velocity.y, physics->angular_velocity.z);
        printf("   Current Thrust: Linear[%.2f, %.2f, %.2f] Angular[%.2f, %.2f, %.2f]\n",
               thrusters->current_linear_thrust.x, 
               thrusters->current_linear_thrust.y,
               thrusters->current_linear_thrust.z,
               thrusters->current_angular_thrust.x,
               thrusters->current_angular_thrust.y,
               thrusters->current_angular_thrust.z);
        
        // Check if stabilization is working
        if (fabsf(thrusters->current_angular_thrust.x) < 0.01f &&
            fabsf(thrusters->current_angular_thrust.y) < 0.01f &&
            fabsf(thrusters->current_angular_thrust.z) < 0.01f &&
            angular_speed > 0.1f) {
            printf("   ⚠️  Ship rotating but no counter-thrust applied!\n");
        } else if (angular_speed < 0.05f) {
            printf("   ✅ Ship stabilized (rotation stopped)\n");
        }
    }
}

bool thruster_test_handle_event(struct World* world, SceneStateManager* state, const sapp_event* ev) {
    (void)state;
    
    if (!thruster_test_initialized) return false;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch (ev->key_code) {
            case SAPP_KEYCODE_F1:
                show_thrust_vectors = !show_thrust_vectors;
                printf("🎯 Thrust vectors: %s\n", show_thrust_vectors ? "ON" : "OFF");
                return true;
                
            case SAPP_KEYCODE_F2:
                show_physics_debug = !show_physics_debug;
                printf("📊 Physics debug: %s\n", show_physics_debug ? "ON" : "OFF");
                return true;
                
            case SAPP_KEYCODE_F3:
                show_thruster_groups = !show_thruster_groups;
                printf("🔥 Thruster groups: %s\n", show_thruster_groups ? "ON" : "OFF");
                return true;
                
            case SAPP_KEYCODE_F5:
                // Reset ship position
                struct Transform* transform = entity_get_transform(world, test_ship_id);
                struct Physics* physics = entity_get_physics(world, test_ship_id);
                if (transform && physics) {
                    transform->position = (Vector3){0, 10, 0};
                    transform->rotation = (Quaternion){0, 0, 0, 1};
                    physics->velocity = (Vector3){0, 0, 0};
                    physics->angular_velocity = (Vector3){0, 0, 0};
                    printf("🔄 Ship position reset\n");
                }
                return true;
                
            default:
                break;
        }
    }
    
    // Pass other input to the input system
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN || ev->type == SAPP_EVENTTYPE_KEY_UP) {
        return input_handle_keyboard(ev->key_code, ev->type == SAPP_EVENTTYPE_KEY_DOWN);
    }
    
    return false;
}

void thruster_test_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (!thruster_test_initialized) return;
    
    thruster_test_initialized = false;
    input_shutdown();
    
    printf("🚀 Thruster test cleanup complete\n");
}

// Scene script definition
const SceneScript thruster_test_script = {
    .scene_name = "thruster_test",
    .on_enter = thruster_test_enter,
    .on_update = thruster_test_update,
    .on_exit = thruster_test_cleanup,
    .on_input = thruster_test_handle_event
};