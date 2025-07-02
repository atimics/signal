// src/scripts/flight_test_scene.c
// Flight test scene - Pure visceral flight experience with obstacles on a plain
// Features enhanced chase camera, dynamic FOV, camera shake, and responsive controls

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
#include "../system/input.h"
#include "../system/control.h"
#include "../system/thrusters.h"
#include "../hidapi.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Flight test state
static bool flight_test_initialized = false;
static float flight_time = 0.0f;
static EntityID player_ship_id = INVALID_ENTITY;

// Camera system state
typedef enum {
    CAMERA_MODE_COCKPIT = 0,
    CAMERA_MODE_CHASE_NEAR = 1,
    CAMERA_MODE_CHASE_FAR = 2,
    CAMERA_MODE_COUNT = 3
} FlightCameraMode;

static FlightCameraMode current_camera_mode = CAMERA_MODE_CHASE_NEAR;

// Enhanced flight physics constants for maximum visceral feel
#define FLIGHT_THRUST_FORCE 35.0f          // Higher than derelict scene for more responsiveness
#define FLIGHT_BOOST_MULTIPLIER 4.0f       // More dramatic boost
#define FLIGHT_MANEUVER_FORCE 18.0f        // More agile maneuvering
#define FLIGHT_BRAKE_FACTOR 0.65f          // More aggressive braking
#define FLIGHT_MAX_VELOCITY 80.0f          // Higher top speed for thrills

// Obstacle and environment setup
#define OBSTACLE_COUNT 12
#define PLAIN_SIZE 300.0f

typedef struct {
    Vector3 position;
    Vector3 size;
    bool is_moving;
    float move_speed;
    Vector3 move_direction;
    float move_time_offset;
} FlightObstacle;

static FlightObstacle obstacles[OBSTACLE_COUNT];

// Forward declarations
void diagnose_gamepad_issues(void);  // Defined in derelict_navigation_scene.c

// Visual thruster system
typedef struct {
    EntityID main_engines[2];    // Left and right main engines
    EntityID rcs_thrusters[4];   // RCS thrusters: forward, back, left, right
    EntityID vertical_thrusters[2]; // Up and down thrusters
    bool initialized;
} VisualThrusterSystem;

static VisualThrusterSystem visual_thrusters = {0};

// Create visual thruster entities
EntityID create_visual_thruster(struct World* world, Vector3 local_pos, Vector3 size, Vector3 glow_color);
void update_thruster_glow_intensity(struct World* world, EntityID thruster_id, float intensity);
void setup_visual_thrusters(struct World* world, EntityID ship_id);
void update_visual_thrusters(struct World* world, float delta_time);

void flight_test_init(struct World* world, SceneStateManager* state) {
    (void)state;
    
    if (flight_test_initialized) return;
    
    printf("🚀 Initializing Flight Test Scene - Open Plain Flying\n");
    
    // Find the player ship entity
    player_ship_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            player_ship_id = entity->id;
            printf("🎯 Player ship found: Entity ID %d\n", player_ship_id);
            break;
        }
    }
    
    if (player_ship_id == INVALID_ENTITY) {
        printf("⚠️  No player ship found - controls will be disabled\n");
    } else {
        // Add new flight mechanics components to the player ship
        printf("🚀 Upgrading player ship with 6DOF flight mechanics...\n");
        
        // Add ThrusterSystem component
        if (entity_add_component(world, player_ship_id, COMPONENT_THRUSTER_SYSTEM)) {
            struct ThrusterSystem* thrusters = entity_get_thruster_system(world, player_ship_id);
            if (thrusters) {
                // Sprint 21: Configure as Fighter-class ship with proper characteristics
                float base_thrust = FLIGHT_THRUST_FORCE * 500.0f;  // Strong base thrust for testing
                thruster_configure_ship_type(thrusters, SHIP_TYPE_FIGHTER, base_thrust);
                
                // Apply ship characteristics to physics
                struct Physics* physics = entity_get_physics(world, player_ship_id);
                if (physics) {
                    thruster_apply_ship_characteristics(thrusters, physics);
                }
                
                printf("   ✅ ThrusterSystem configured as FIGHTER class\n");
                printf("   📊 Max thrust: Forward=%.0fN, Maneuver=%.0fN, Strafe=%.0fN\n",
                       thrusters->max_linear_force.x, thrusters->max_linear_force.y, thrusters->max_linear_force.z);
                printf("   🔄 Max torque: [%.1f, %.1f, %.1f] N⋅m\n",
                       thrusters->max_angular_torque.x, thrusters->max_angular_torque.y, thrusters->max_angular_torque.z);
                printf("   ⚡ Response time: %.3fs, Efficiency: %.1f\n",
                       thrusters->thrust_response_time, thrusters->power_efficiency);
            }
        }
        
        // Add ControlAuthority component  
        if (entity_add_component(world, player_ship_id, COMPONENT_CONTROL_AUTHORITY)) {
            struct ControlAuthority* control = entity_get_control_authority(world, player_ship_id);
            if (control) {
                // Configure control settings for flight test
                control->controlled_by = player_ship_id; // Self-controlled
                control->control_sensitivity = 1.0f;
                control->stability_assist = 0.3f; // Light assistance for flight test
                control->flight_assist_enabled = true;
                control->control_mode = CONTROL_ASSISTED;
                printf("   ✅ ControlAuthority configured\n");
                
                // Set this as the player entity for the control system
                control_set_player_entity(world, player_ship_id);
            }
        }
        
        // Enable 6DOF physics on the player ship
        struct Physics* physics = entity_get_physics(world, player_ship_id);
        if (physics) {
            physics->has_6dof = true;
            physics->moment_of_inertia = (Vector3){ 2.0f, 2.0f, 1.5f }; // Ship-like inertia
            physics->drag_linear = 0.9999f;  // Minimal drag for space flight
            physics->drag_angular = 0.90f;   // Moderate angular drag for stability
            physics->environment = PHYSICS_ATMOSPHERE; // Atmospheric flight
            printf("   ✅ 6DOF Physics enabled with reduced drag\n");
        }
        
        printf("🚀 Player ship upgrade complete - Enhanced 6DOF flight mechanics ready!\n");
    }
    
    // Initialize input system
    if (!input_init()) {
        printf("⚠️  Input initialization failed\n");
    } else {
        diagnose_gamepad_issues();
    }
    
    // Generate obstacles around the plain
    printf("🗿 Generating %d obstacles across %.0fx%.0f plain...\n", OBSTACLE_COUNT, PLAIN_SIZE, PLAIN_SIZE);
    
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        FlightObstacle* obs = &obstacles[i];
        
        // Random position within the plain
        obs->position.x = ((float)rand() / RAND_MAX - 0.5f) * PLAIN_SIZE;
        obs->position.y = ((float)rand() / RAND_MAX) * 20.0f + 5.0f; // Height 5-25
        obs->position.z = ((float)rand() / RAND_MAX - 0.5f) * PLAIN_SIZE;
        
        // Random size
        float base_size = 5.0f + ((float)rand() / RAND_MAX) * 15.0f; // 5-20 units
        obs->size = (Vector3){base_size, base_size * (0.5f + (float)rand() / RAND_MAX), base_size};
        
        // Some obstacles move
        obs->is_moving = (rand() % 3 == 0); // 33% chance to move
        if (obs->is_moving) {
            obs->move_speed = 2.0f + ((float)rand() / RAND_MAX) * 8.0f; // 2-10 units/sec
            
            // Random horizontal movement direction
            float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
            obs->move_direction = (Vector3){cosf(angle), 0, sinf(angle)};
            obs->move_time_offset = ((float)rand() / RAND_MAX) * 10.0f;
        }
        
        printf("   🗿 Obstacle %d: pos(%.1f,%.1f,%.1f) size(%.1f,%.1f,%.1f) %s\n", 
               i, obs->position.x, obs->position.y, obs->position.z,
               obs->size.x, obs->size.y, obs->size.z,
               obs->is_moving ? "MOVING" : "static");
    }
    
    flight_time = 0.0f;
    flight_test_initialized = true;
    
    // Set up camera for flight testing
    current_camera_mode = CAMERA_MODE_CHASE_NEAR;
    
    printf("🚀 Flight test initialized\n");
    printf("🌍 Plain size: %.0fx%.0f units\n", PLAIN_SIZE, PLAIN_SIZE);
    printf("🎮 Modern 6DOF Flight Controls:\n");
    printf("   KEYBOARD (Mouse + WASD):\n");
    printf("     W/S - Forward/Backward thrust (%.0f force)\n", FLIGHT_THRUST_FORCE);
    printf("     A/D - Strafe left/right (%.0f force)\n", FLIGHT_MANEUVER_FORCE);
    printf("     Space/Ctrl - Vertical up/down\n");
    printf("     Q/E - Roll left/right\n");
    printf("     Mouse - Pitch/Yaw (like FPS aiming)\n");
    printf("     Shift - Boost (%.1fx multiplier)\n", FLIGHT_BOOST_MULTIPLIER);
    printf("     Alt - Brake\n");
    printf("     Tab - Cycle camera modes\n");
    printf("   XBOX CONTROLLER (Modern Layout):\n");
    printf("     Left Stick - Pitch/Yaw (primary flight control)\n");
    printf("     Right Stick - Lateral/Vertical thrust\n");
    printf("     Right Trigger - Forward thrust\n");
    printf("     Left Trigger - Reverse thrust\n");
    printf("     Bumpers - Roll left/right\n");
    printf("     A Button - Boost, B Button - Brake\n");
    printf("📷 Camera Modes: COCKPIT → CHASE_NEAR → CHASE_FAR\n");
    printf("🎯 Physics: 6DOF enabled with flight assistance\n");
}

// Enhanced camera system for flight testing with even more dynamic behavior
void update_flight_camera_system(struct World* world, float delta_time) {
    (void)delta_time;
    
    if (player_ship_id == INVALID_ENTITY) return;
    
    EntityID active_camera_id = world_get_active_camera(world);
    if (active_camera_id == INVALID_ENTITY) return;
    
    struct Camera* camera = entity_get_camera(world, active_camera_id);
    if (!camera) return;
    
    // Velocity tracking removed for camera stability
    // TODO: Re-add when implementing subtle velocity-based effects
    
    // Ensure camera follows player
    if (camera->follow_target != player_ship_id) {
        camera->follow_target = player_ship_id;
        camera->behavior = CAMERA_BEHAVIOR_CHASE;
    }
    
    // Camera mode configurations with enhanced settings for flight
    Vector3 base_offset;
    float new_smoothing;
    
    switch (current_camera_mode) {
        case CAMERA_MODE_COCKPIT:
            camera->behavior = CAMERA_BEHAVIOR_FIRST_PERSON;
            base_offset = (Vector3){0, 0.5f, 0.5f}; // Inside cockpit, slightly forward and up
            new_smoothing = 15.0f;                   // Very responsive for cockpit
            camera->fov = 85.0f;                     // Narrower for cockpit realism
            break;
            
        case CAMERA_MODE_CHASE_NEAR:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 8, 20};       // Closer, stable position
            new_smoothing = 10.0f;                   // Smooth but responsive
            camera->fov = 95.0f;                     // Good visibility
            break;
            
        case CAMERA_MODE_CHASE_FAR:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 15, 40};      // Further back, higher up
            new_smoothing = 8.0f;                    // Smooth cinematic feel
            camera->fov = 105.0f;                    // Wide for overview
            break;
            
            
        case CAMERA_MODE_COUNT:
        default:
            current_camera_mode = CAMERA_MODE_CHASE_NEAR;
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 8, 20};
            new_smoothing = 10.0f;
            camera->fov = 95.0f;
            break;
    }
    
    // Simple static offset - no velocity-based effects for now
    Vector3 dynamic_offset = base_offset;
    
    // Apply settings
    camera->follow_offset = dynamic_offset;
    camera->follow_smoothing = new_smoothing;
    camera->matrices_dirty = true;
    
    // Camera shake and FOV effects disabled for stability
    // TODO: Re-enable with more conservative settings once basic camera is stable
    
    // Reduced camera debug output
    static float last_cam_log = 0.0f;
    if (flight_time - last_cam_log > 30.0f) {  // Reduced frequency from 3s to 30s
        const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
        printf("📷 Camera: %s mode\n", mode_names[current_camera_mode]);
        last_cam_log = flight_time;
    }
}

// NOTE: Flight controls are now handled by the Control and Thruster systems automatically
// The input_update() call processes input and converts it to thruster commands
// The systems framework handles the rest automatically

void update_moving_obstacles(float delta_time) {
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        FlightObstacle* obs = &obstacles[i];
        if (!obs->is_moving) continue;
        
        float time_phase = flight_time + obs->move_time_offset;
        
        // Oscillating movement pattern
        float movement_scale = sinf(time_phase * obs->move_speed * 0.1f);
        
        obs->position.x += obs->move_direction.x * movement_scale * obs->move_speed * delta_time;
        obs->position.z += obs->move_direction.z * movement_scale * obs->move_speed * delta_time;
        
        // Keep obstacles within bounds
        float boundary = PLAIN_SIZE * 0.4f;
        if (fabsf(obs->position.x) > boundary) {
            obs->move_direction.x *= -1.0f;
        }
        if (fabsf(obs->position.z) > boundary) {
            obs->move_direction.z *= -1.0f;
        }
    }
}

void flight_test_update(struct World* world, SceneStateManager* state, float delta_time) {
    if (!flight_test_initialized) return;
    
    flight_time += delta_time;
    
    // Update input system (the Control and Thruster systems handle the rest automatically)
    input_update();
    
    // Update camera system
    update_flight_camera_system(world, delta_time);
    
    // Update moving obstacles
    update_moving_obstacles(delta_time);
    
    // Periodic status reporting with enhanced 6DOF info
    static float last_status = 0.0f;
    if (flight_time - last_status > 10.0f) {
        if (player_ship_id != INVALID_ENTITY) {
            struct Entity* player = NULL;
            for (uint32_t i = 0; i < world->entity_count; i++) {
                if (world->entities[i].id == player_ship_id) {
                    player = &world->entities[i];
                    break;
                }
            }
            
            if (player && player->physics) {
                float speed = sqrtf(player->physics->velocity.x * player->physics->velocity.x + 
                                   player->physics->velocity.y * player->physics->velocity.y + 
                                   player->physics->velocity.z * player->physics->velocity.z);
                
                float angular_speed = sqrtf(player->physics->angular_velocity.x * player->physics->angular_velocity.x + 
                                          player->physics->angular_velocity.y * player->physics->angular_velocity.y + 
                                          player->physics->angular_velocity.z * player->physics->angular_velocity.z);
                
                printf("🚀 Flight Test: %.1fs - Speed: %.1f u/s, Angular: %.2f rad/s, 6DOF: %s\n", 
                       flight_time, speed, angular_speed, player->physics->has_6dof ? "ON" : "OFF");
            }
        }
        last_status = flight_time;
    }
    
    (void)state;
}

static bool flight_test_input(struct World* world, SceneStateManager* state, const void* event) {
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            printf("🚀 Flight Test: ESC pressed, returning to navigation menu\n");
            scene_state_request_transition(state, "navigation_menu");
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_TAB) {
            // Cycle camera modes - moved from C to TAB to avoid conflict with roll control
            current_camera_mode = (FlightCameraMode)((current_camera_mode + 1) % CAMERA_MODE_COUNT);
            const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
            printf("📷 Flight camera mode: %s\n", mode_names[current_camera_mode]);
            
            // Apply immediately
            update_flight_camera_system(world, 0.0f);
            return true;
        }
        
        // Handle other input
        if (input_handle_keyboard(ev->key_code, true)) {
            return true;
        }
    }
    
    if (ev->type == SAPP_EVENTTYPE_KEY_UP) {
        if (input_handle_keyboard(ev->key_code, false)) {
            return true;
        }
    }
    
    return false;
}

void flight_test_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (!flight_test_initialized) return;
    
    flight_test_initialized = false;
    input_shutdown();
    
    printf("🚀 Flight test cleanup complete\n");
}

// Scene script definition
const SceneScript flight_test_script = {
    .scene_name = "flight_test",
    .on_enter = flight_test_init,
    .on_update = flight_test_update,
    .on_exit = flight_test_cleanup,
    .on_input = flight_test_input
};
