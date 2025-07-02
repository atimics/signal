// src/scripts/flight_test_scene.c
// Flight test scene - Pure visceral flight experience with obstacles on a plain
// Features enhanced chase camera, dynamic FOV, camera shake, and responsive controls

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
#include "../system/input.h"
#include "../system/control.h"
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
    CAMERA_MODE_OVERHEAD = 3,
    CAMERA_MODE_COUNT = 4
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
                // Configure thruster capabilities for responsive flight
                thrusters->max_linear_force = (Vector3){ FLIGHT_THRUST_FORCE, FLIGHT_MANEUVER_FORCE, FLIGHT_MANEUVER_FORCE };
                thrusters->max_angular_torque = (Vector3){ 25.0f, 25.0f, 20.0f }; // Pitch, Yaw, Roll
                thrusters->thrust_response_time = 0.05f; // Very responsive
                thrusters->atmosphere_efficiency = 1.0f; // Full power in atmosphere
                thrusters->vacuum_efficiency = 1.0f;     // Full power in vacuum
                thrusters->thrusters_enabled = true;
                printf("   ✅ ThrusterSystem configured\n");
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
            physics->drag_angular = 0.85f; // Some angular drag for stability
            physics->environment = PHYSICS_ATMOSPHERE; // Atmospheric flight
            printf("   ✅ 6DOF Physics enabled\n");
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
    printf("🎮 Enhanced 6DOF Flight Controls:\n");
    printf("   LINEAR MOVEMENT:\n");
    printf("     W/S - Forward/Backward thrust (%.0f force)\n", FLIGHT_THRUST_FORCE);
    printf("     A/D - Strafe left/right (%.0f force)\n", FLIGHT_MANEUVER_FORCE);
    printf("     Q/E - Vertical up/down\n");
    printf("   ANGULAR MOVEMENT (6DOF):\n");
    printf("     Arrow Keys - Pitch (Up/Down) and Yaw (Left/Right)\n");
    printf("     Z/C - Roll left/right\n");
    printf("   MODIFIERS:\n");
    printf("     Shift - Boost (%.1fx multiplier)\n", FLIGHT_BOOST_MULTIPLIER);
    printf("     Ctrl - Brake\n");
    printf("     C - Cycle camera modes\n");
    printf("   GAMEPAD:\n");
    printf("     Left Stick - Thrust/Strafe\n");
    printf("     Right Stick - Pitch/Yaw\n");
    printf("     Triggers - Vertical movement\n");
    printf("     Bumpers - Roll\n");
    printf("     A Button - Boost\n");
    printf("📷 Camera Modes: COCKPIT → CHASE_NEAR → CHASE_FAR → OVERHEAD\n");
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
    
    // Get player data
    struct Entity* player_entity = NULL;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        if (world->entities[i].id == player_ship_id) {
            player_entity = &world->entities[i];
            break;
        }
    }
    
    Vector3 player_velocity = {0, 0, 0};
    Vector3 player_acceleration = {0, 0, 0};
    if (player_entity && player_entity->physics) {
        player_velocity = player_entity->physics->velocity;
        player_acceleration = player_entity->physics->acceleration;
    }
    
    float player_speed = sqrtf(player_velocity.x * player_velocity.x + 
                              player_velocity.y * player_velocity.y + 
                              player_velocity.z * player_velocity.z);
    
    float acceleration_magnitude = sqrtf(player_acceleration.x * player_acceleration.x + 
                                        player_acceleration.y * player_acceleration.y + 
                                        player_acceleration.z * player_acceleration.z);
    
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
            base_offset = (Vector3){0, 1.0f, 0};    // Slightly higher in cockpit
            new_smoothing = 12.0f;                  // Very responsive
            camera->fov = 90.0f;                    // Standard cockpit FOV
            break;
            
        case CAMERA_MODE_CHASE_NEAR:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 15, 30};     // Higher and further back
            new_smoothing = 3.0f;                   // Moderate lag for dynamic feel
            camera->fov = 100.0f;                   // Wide FOV for visibility
            break;
            
        case CAMERA_MODE_CHASE_FAR:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){10, 25, 65};    // Much further back, offset to side
            new_smoothing = 1.5f;                   // Slow for cinematic effect
            camera->fov = 110.0f;                   // Very wide for cinematic shots
            break;
            
        case CAMERA_MODE_OVERHEAD:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 80, 15};     // High overhead, slightly behind
            new_smoothing = 2.0f;                   // Moderate for overview
            camera->fov = 85.0f;                    // Narrower for tactical view
            break;
            
        case CAMERA_MODE_COUNT:
        default:
            current_camera_mode = CAMERA_MODE_CHASE_NEAR;
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 15, 30};
            new_smoothing = 3.0f;
            camera->fov = 100.0f;
            break;
    }
    
    // Enhanced dynamic offset calculation
    Vector3 dynamic_offset = base_offset;
    if (camera->behavior == CAMERA_BEHAVIOR_CHASE) {
        // Velocity-based lag (more pronounced than derelict scene)
        float velocity_factor = 0.0f;
        switch (current_camera_mode) {
            case CAMERA_MODE_CHASE_NEAR:
                velocity_factor = 0.2f;   // More velocity lag for visceral feel
                break;
            case CAMERA_MODE_CHASE_FAR:
                velocity_factor = 0.35f;  // Heavy lag for cinematic effect
                break;
            case CAMERA_MODE_OVERHEAD:
                velocity_factor = 0.1f;   // Less lag for tactical view
                break;
            default:
                velocity_factor = 0.15f;
                break;
        }
        
        // Add velocity lag
        dynamic_offset.x += player_velocity.x * velocity_factor;
        dynamic_offset.y += player_velocity.y * velocity_factor * 0.3f; // Less vertical
        dynamic_offset.z += player_velocity.z * velocity_factor;
        
        // Speed-based distance (more dramatic pullback)
        float speed_factor = fminf(player_speed / FLIGHT_MAX_VELOCITY, 1.0f);
        switch (current_camera_mode) {
            case CAMERA_MODE_CHASE_NEAR:
                dynamic_offset.z += speed_factor * 12.0f;  // Pull back up to 12 units
                dynamic_offset.y += speed_factor * 6.0f;   // Raise up to 6 units
                break;
            case CAMERA_MODE_CHASE_FAR:
                dynamic_offset.z += speed_factor * 25.0f;  // Dramatic pullback
                dynamic_offset.y += speed_factor * 12.0f;  // Higher for speed
                break;
            case CAMERA_MODE_OVERHEAD:
                dynamic_offset.y += speed_factor * 20.0f;  // Go higher when fast
                break;
            default:
                break;
        }
    }
    
    // Apply settings
    camera->follow_offset = dynamic_offset;
    camera->follow_smoothing = new_smoothing;
    camera->matrices_dirty = true;
    
    // Enhanced camera shake for flight (more intense than derelict)
    if (camera->behavior == CAMERA_BEHAVIOR_CHASE && 
        (player_speed > 3.0f || acceleration_magnitude > 1.5f)) {
        
        float shake_intensity = fminf((player_speed / 80.0f) + (acceleration_magnitude / 30.0f), 0.6f);
        float shake_freq = flight_time * 30.0f;  // Higher frequency for more intense feel
        
        Vector3 shake_offset = {
            sinf(shake_freq * 1.2f) * shake_intensity * 0.8f,
            sinf(shake_freq * 1.6f) * shake_intensity * 0.6f,
            sinf(shake_freq * 1.4f) * shake_intensity * 0.4f
        };
        
        // Apply shake
        camera->follow_offset.x += shake_offset.x;
        camera->follow_offset.y += shake_offset.y;
        camera->follow_offset.z += shake_offset.z;
        
        // Dynamic FOV for warp effect (more dramatic)
        if (acceleration_magnitude > 8.0f) {
            float fov_boost = fminf(acceleration_magnitude / 80.0f, 0.25f);
            camera->fov += fov_boost * 30.0f;  // Up to 30 degrees FOV increase
        }
        
        // Speed-based FOV increase for sensation of speed
        if (player_speed > 40.0f) {
            float speed_fov_boost = fminf((player_speed - 40.0f) / 40.0f, 0.2f);
            camera->fov += speed_fov_boost * 15.0f;  // Up to 15 degrees for high speed
        }
    }
    
    // Debug output
    static float last_cam_log = 0.0f;
    if (flight_time - last_cam_log > 3.0f) {
        const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR", "OVERHEAD"};
        struct Transform* player_transform = entity_get_transform(world, player_ship_id);
        Vector3 player_pos = player_transform ? player_transform->position : (Vector3){0, 0, 0};
        Vector3 cam_pos = camera->position;
        float cam_distance = sqrtf((cam_pos.x - player_pos.x) * (cam_pos.x - player_pos.x) + 
                                  (cam_pos.y - player_pos.y) * (cam_pos.y - player_pos.y) + 
                                  (cam_pos.z - player_pos.z) * (cam_pos.z - player_pos.z));
        printf("📷 [%s] Speed:%.1f Accel:%.1f FOV:%.1f Dist:%.1f\n",
               mode_names[current_camera_mode], player_speed, acceleration_magnitude, 
               camera->fov, cam_distance);
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
        
        if (ev->key_code == SAPP_KEYCODE_C) {
            // Cycle camera modes
            current_camera_mode = (FlightCameraMode)((current_camera_mode + 1) % CAMERA_MODE_COUNT);
            const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR", "OVERHEAD"};
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
