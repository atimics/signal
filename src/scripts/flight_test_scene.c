// src/scripts/flight_test_scene.c
// Flight test scene - Pure visceral flight experience with obstacles on a plain
// Features enhanced chase camera, dynamic FOV, camera shake, and responsive controls

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
// Note: Now using InputService directly instead of legacy InputState
#include "../system/unified_control_system.h"
#include "../system/thrusters.h"
#include "../system/scripted_flight.h"
#include "../hidapi.h"
#include "../graphics_api.h"
#include "../game_input.h"
#include "../services/input_service.h"
#include "../systems.h"  // For get_asset_registry()
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Flight test state
static bool flight_test_initialized = false;
static float flight_time = 0.0f;
static EntityID player_ship_id = INVALID_ENTITY;

// Scripted flight state
static ScriptedFlight* player_scripted_flight = NULL;
static bool scripted_flight_active = false;

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
#define PLAIN_SIZE 10000.0f  // Massive space area (10km x 10km)

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
static void create_solar_system(struct World* world, EntityID parent);

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
    
    // Switch to gameplay input context for flight controls (Sprint 25)
    struct InputService* input_service = game_input_get_service();
    if (input_service) {
        input_service->push_context(input_service, INPUT_CONTEXT_GAMEPLAY);
        printf("🎮 Switched to gameplay input context for flight controls\n");
    }
    
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
        if (!entity_has_component(world, player_ship_id, COMPONENT_THRUSTER_SYSTEM)) {
            entity_add_component(world, player_ship_id, COMPONENT_THRUSTER_SYSTEM);
        }
        
        // Add UnifiedFlightControl component
        unified_control_system_configure_as_player_ship(world, player_ship_id);
        
        // Set this as the player entity for the unified control system
        printf("🎮 Setting player entity ID: %d\n", player_ship_id);
        unified_control_system_set_player_entity(player_ship_id);
        
        // Get components for any scene-specific adjustments
        struct Physics* physics = entity_get_physics(world, player_ship_id);
        struct ThrusterSystem* thrusters = entity_get_thruster_system(world, player_ship_id);
        
        if (thrusters) {
            // Configure as RACER class with balanced thrust values
            thrusters->max_linear_force = (Vector3){ 7000.0f, 5000.0f, 10000.0f };   // 25 m/s² forward acceleration
            thrusters->max_angular_torque = (Vector3){ 5000.0f, 8500.0f, 3000.0f };   // Increased yaw torque for more responsive turning
            thrusters->thrust_response_time = 0.02f;  // Near-instant response
            thrusters->thrusters_enabled = true;
            thrusters->auto_deceleration = true;
            
            printf("   ✅ Ship configured as RACER class\n");
            printf("   📊 Max thrust: [%.0f, %.0f, %.0f] N\n",
                   thrusters->max_linear_force.x, thrusters->max_linear_force.y, thrusters->max_linear_force.z);
            printf("   🔄 Max torque: [%.1f, %.1f, %.1f] N⋅m\n",
                   thrusters->max_angular_torque.x, thrusters->max_angular_torque.y, thrusters->max_angular_torque.z);
        }
        
        if (physics) {
            physics->drag_linear = 0.01f;   // Reduced drag (1% velocity loss per frame) for better acceleration
            physics->drag_angular = 0.08f;  // Slightly less angular damping for quicker turns
            physics->environment = PHYSICS_SPACE; // Zero gravity space flight
            
            // Set realistic moment of inertia for a 5x3x8m ship
            // This prevents unrealistic spinning speeds
            physics->moment_of_inertia = (Vector3){ 2400.0f, 3000.0f, 1100.0f };
            physics->has_6dof = true;  // Ensure 6DOF physics is enabled
            
            printf("   ✅ 6DOF Physics enabled with realistic inertia\n");
            printf("   📐 Moment of inertia: [%.0f, %.0f, %.0f] kg⋅m²\n",
                   physics->moment_of_inertia.x, physics->moment_of_inertia.y, physics->moment_of_inertia.z);
        }
        
        printf("🚀 Player ship upgrade complete - Enhanced 6DOF flight mechanics ready!\n");
        
        // Setup visual thrusters
        setup_visual_thrusters(world, player_ship_id);
    }
    
    // Input system initialization removed - handled by game_input service
    // diagnose_gamepad_issues(); // TODO: Update this function to use new input system
    
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
    
    // Initialize scripted flight system for the player ship
    if (player_ship_id != INVALID_ENTITY) {
        player_scripted_flight = scripted_flight_create_component(player_ship_id);
        if (player_scripted_flight) {
            printf("🛩️  Scripted flight system ready for player ship\n");
            
            // Don't auto-start - let user manually activate with '1' key
            printf("🛩️  Press '1' to start circuit flight pattern\n");
            printf("🛩️  Press '2' for figure-8 pattern\n");
            printf("🛩️  Press '3' for landing approach\n");
        }
    }
    
    // Create mini solar system with lots of reference points
    create_solar_system(world, INVALID_ENTITY);
    
    // Add scattered suns throughout the playground for better visibility
    printf("☀️ Adding %d scattered suns for better navigation...\n", 50);
    for (int i = 0; i < 50; i++) {
        EntityID sun = entity_create(world);
        if (sun == INVALID_ENTITY) continue;
        
        entity_add_component(world, sun, COMPONENT_TRANSFORM);
        entity_add_component(world, sun, COMPONENT_RENDERABLE);
        
        struct Transform* transform = entity_get_transform(world, sun);
        struct Renderable* renderable = entity_get_renderable(world, sun);
        
        if (transform && renderable) {
            // Random position in a large 3D grid
            float x = (((float)rand() / RAND_MAX) - 0.5f) * PLAIN_SIZE * 0.8f;
            float y = (((float)rand() / RAND_MAX) - 0.5f) * 2000.0f + 500.0f;  // -500 to +1500 height
            float z = (((float)rand() / RAND_MAX) - 0.5f) * PLAIN_SIZE * 0.8f;
            
            transform->position = (Vector3){x, y, z};
            
            // Varied sizes for depth perception
            float size = 8.0f + ((float)rand() / RAND_MAX) * 25.0f;  // 8-33 units
            transform->scale = (Vector3){size, size, size};
            transform->rotation = (Quaternion){0, 0, 0, 1};
            transform->dirty = true;
            
            // Use sun mesh if available, fallback to logo_cube
            AssetRegistry* assets = get_asset_registry();
            if (assets && assets_create_renderable_from_mesh(assets, "sun", renderable)) {
                renderable->visible = true;
                renderable->lod_level = LOD_HIGH;
            } else {
                assets_create_renderable_from_mesh(assets, "logo_cube", renderable);
                renderable->visible = true;
                renderable->lod_level = LOD_MEDIUM;
            }
        }
    }
    
    // Add grid reference markers for better spatial awareness
    printf("📍 Adding grid reference markers...\n");
    for (int grid_x = -2; grid_x <= 2; grid_x++) {
        for (int grid_z = -2; grid_z <= 2; grid_z++) {
            if (grid_x == 0 && grid_z == 0) continue; // Skip center (where player starts)
            
            EntityID marker = entity_create(world);
            if (marker == INVALID_ENTITY) continue;
            
            entity_add_component(world, marker, COMPONENT_TRANSFORM);
            entity_add_component(world, marker, COMPONENT_RENDERABLE);
            
            struct Transform* transform = entity_get_transform(world, marker);
            struct Renderable* renderable = entity_get_renderable(world, marker);
            
            if (transform && renderable) {
                // Grid spacing of 1000 units
                float x = grid_x * 1000.0f;
                float z = grid_z * 1000.0f;
                float y = 20.0f; // Just above ground level
                
                transform->position = (Vector3){x, y, z};
                transform->scale = (Vector3){15, 40, 15}; // Tall, visible markers
                transform->rotation = (Quaternion){0, 0, 0, 1};
                transform->dirty = true;
                
                // Use control tower for grid markers
                AssetRegistry* assets = get_asset_registry();
                if (assets && assets_create_renderable_from_mesh(assets, "control_tower", renderable)) {
                    renderable->visible = true;
                    renderable->lod_level = LOD_HIGH;
                } else {
                    assets_create_renderable_from_mesh(assets, "logo_cube", renderable);
                    renderable->visible = true;
                    renderable->lod_level = LOD_HIGH;
                }
            }
        }
    }
    
    // Set up camera for flight testing
    current_camera_mode = CAMERA_MODE_CHASE_NEAR;
    
    printf("🚀 Flight test initialized\n");
    printf("🌍 Space size: %.0fx%.0f units\n", PLAIN_SIZE, PLAIN_SIZE);
    printf("🏎️ LAYERED FLIGHT Controls (Aircraft-style):\n");
    printf("   KEYBOARD:\n");
    printf("     W/S - Forward/Backward thrust\n");
    printf("     A/D - BANKING TURNS (120%% roll + yaw)\n");
    printf("     Q/E - Descend/Ascend (vertical movement)\n");
    printf("     Space/LCtrl - Pitch Up/Down (dive/climb)\n");
    printf("     ←/→ - Pure roll (barrel roll)\n");
    printf("     Shift - Boost (%.1fx multiplier)\n", FLIGHT_BOOST_MULTIPLIER);
    printf("     Alt - Brake + Auto-deceleration\n");
    printf("     Tab - Cycle camera modes\n");
    printf("   XBOX CONTROLLER:\n");
    printf("     Right Trigger - Accelerate\n");
    printf("     Left Stick Y - Up/Down (vertical movement)\n");
    printf("     Left Stick X - BANKING TURNS (120%% roll)\n");
    printf("     Right Stick X - Camera Look (±60°)\n");
    printf("     Left Trigger - Brake\n");
    printf("     LB/RB - Pitch Up/Down\n");
    printf("\n🏎️ LAYERED FLIGHT: Ships bank heavily into turns for dynamic maneuvering!\n");
    printf("🛩️  SCRIPTED FLIGHT CONTROLS:\n");
    printf("     1 - Start circuit flight pattern\n");
    printf("     2 - Start figure-8 flight pattern\n");
    printf("     3 - Start landing approach\n");
    printf("     0 - Stop scripted flight (manual control)\n");
    printf("     P - Pause/Resume scripted flight\n");
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
            base_offset = (Vector3){0, 0.5f, -0.5f}; // Inside cockpit, slightly back and up
            new_smoothing = 15.0f;                   // Very responsive for cockpit
            camera->fov = 85.0f;                     // Narrower for cockpit realism
            break;
            
        case CAMERA_MODE_CHASE_NEAR:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 5, -15};      // Try negative Z for behind ship
            new_smoothing = 15.0f;                   // More responsive following
            camera->fov = 95.0f;                     // Good visibility
            break;
            
        case CAMERA_MODE_CHASE_FAR:
            camera->behavior = CAMERA_BEHAVIOR_CHASE;
            base_offset = (Vector3){0, 10, -30};     // Further back, negative Z
            new_smoothing = 12.0f;                   // Smooth cinematic feel
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
    
    // Debug camera following
    static float last_cam_log = 0.0f;
    if (flight_time - last_cam_log > 5.0f) {  // Every 5 seconds
        const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
        printf("📷 Camera: %s mode, target=%d, smoothing=%.1f\n", 
               mode_names[current_camera_mode], camera->follow_target, new_smoothing);
        printf("📷 Camera offset: [%.1f, %.1f, %.1f]\n", 
               base_offset.x, base_offset.y, base_offset.z);
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
    
    // Input updates handled automatically by game_input service
    
    // Update scripted flight system
    scripted_flight_update(world, NULL, delta_time);
    
    // Update camera system
    update_flight_camera_system(world, delta_time);
    
    // Update visual thrusters
    update_visual_thrusters(world, delta_time);
    
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
        printf("🎮 Flight test: Key pressed - code=%d\n", ev->key_code);
        
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
            
            // HUD system removed - camera mode tracking handled locally
            
            // Apply camera changes immediately
            update_flight_camera_system(world, 0.0f);
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_1) {
            printf("\n🎮 === '1' KEY PRESSED - CIRCUIT FLIGHT DEBUG ===\n");
            printf("   Player ship ID: %d\n", player_ship_id);
            printf("   Scripted flight component: %p\n", player_scripted_flight);
            printf("   Flight active flag: %d\n", scripted_flight_active);
            
            // Start scripted circuit flight
            if (player_scripted_flight) {
                printf("   ✓ Scripted flight component exists\n");
                if (scripted_flight_active) {
                    printf("   ⚠️  Flight already active - stopping current flight\n");
                    scripted_flight_stop(player_scripted_flight);
                    scripted_flight_active = false;
                } else {
                    FlightPath* circuit = scripted_flight_create_circuit_path();
                    if (circuit) {
                        printf("   📍 Created circuit path with %d waypoints\n", circuit->waypoint_count);
                        scripted_flight_start(player_scripted_flight, circuit);
                        scripted_flight_active = true;
                        printf("   ✅ Started circuit flight pattern\n");
                        printf("   Flight active: %d\n", player_scripted_flight->active);
                        
                        // Check unified control system state
                        struct UnifiedFlightControl* control = entity_get_unified_flight_control(world, player_ship_id);
                        if (control) {
                            printf("   Control: controlled_by=%d, mode=%d\n", 
                                   control->controlled_by, control->mode);
                        }
                    } else {
                        printf("   ❌ Failed to create circuit path\n");
                    }
                }
            } else {
                printf("   ❌ No scripted flight component available\n");
            }
            printf("===========================================\n\n");
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_2) {
            // Start scripted figure-8 flight
            if (player_scripted_flight) {
                FlightPath* figure8 = scripted_flight_create_figure_eight_path();
                scripted_flight_start(player_scripted_flight, figure8);
                scripted_flight_active = true;
                printf("🛩️  Started figure-8 flight pattern\n");
            }
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_3) {
            // Start landing approach
            if (player_scripted_flight) {
                struct Transform* transform = entity_get_transform(world, player_ship_id);
                if (transform) {
                    Vector3 start_pos = transform->position;
                    Vector3 landing_pos = {0, 5, 0}; // Landing pad position
                    FlightPath* approach = scripted_flight_create_landing_approach_path(start_pos, landing_pos);
                    scripted_flight_start(player_scripted_flight, approach);
                    scripted_flight_active = true;
                    printf("🛩️  Started landing approach pattern\n");
                }
            }
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_0) {
            // Stop scripted flight
            if (player_scripted_flight) {
                scripted_flight_stop(player_scripted_flight);
                scripted_flight_active = false;
                printf("🛩️  Stopped scripted flight - manual control resumed\n");
            }
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_P) {
            // Pause/resume scripted flight
            if (player_scripted_flight && scripted_flight_active) {
                if (player_scripted_flight->manual_override) {
                    scripted_flight_resume(player_scripted_flight);
                } else {
                    scripted_flight_pause(player_scripted_flight);
                }
            }
            return true;
        }
        
        // Legacy input handling removed - events processed through game_input service
    }
    
    if (ev->type == SAPP_EVENTTYPE_KEY_UP) {
        // Legacy input handling removed - events processed through game_input service
    }
    
    return false;
}

void flight_test_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (!flight_test_initialized) return;
    
    flight_test_initialized = false;
    
    // Pop gameplay input context when exiting flight test (Sprint 25)
    struct InputService* input_service = game_input_get_service();
    if (input_service) {
        input_service->pop_context(input_service);
        printf("🎮 Popped gameplay input context\n");
    }
    
    printf("🚀 Flight test cleanup complete\n");
}

// ============================================================================
// VISUAL THRUSTER IMPLEMENTATION
// ============================================================================

EntityID create_visual_thruster(struct World* world, Vector3 local_pos, Vector3 size, Vector3 glow_color) {
    (void)glow_color;  // TODO: Use when material system supports emissive colors
    
    EntityID thruster_id = entity_create(world);
    if (thruster_id == INVALID_ENTITY) {
        printf("❌ Failed to create visual thruster entity\n");
        return INVALID_ENTITY;
    }
    
    // Add transform component
    if (!entity_add_component(world, thruster_id, COMPONENT_TRANSFORM)) {
        printf("❌ Failed to add transform to thruster\n");
        return INVALID_ENTITY;
    }
    
    struct Transform* transform = entity_get_transform(world, thruster_id);
    if (transform) {
        transform->position = local_pos;
        transform->scale = size;
        transform->rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
        transform->dirty = true;
    }
    
    // Add renderable component for visual representation
    if (!entity_add_component(world, thruster_id, COMPONENT_RENDERABLE)) {
        printf("❌ Failed to add renderable to thruster\n");
        return INVALID_ENTITY;
    }
    
    struct Renderable* renderable = entity_get_renderable(world, thruster_id);
    if (renderable) {
        // Set up basic renderable for thruster nozzle
        renderable->visible = false;  // Start invisible
        renderable->lod_level = LOD_HIGH;
        renderable->material_id = 1;  // Use a basic material for now
        renderable->index_count = 36; // Basic cube indices
    }
    
    // Add scene node for hierarchy
    if (!entity_add_component(world, thruster_id, COMPONENT_SCENENODE)) {
        printf("❌ Failed to add scene node to thruster\n");
        return INVALID_ENTITY;
    }
    
    printf("✅ Created visual thruster at pos(%.1f,%.1f,%.1f) size(%.2f,%.2f,%.2f)\n",
           local_pos.x, local_pos.y, local_pos.z, size.x, size.y, size.z);
    
    return thruster_id;
}

void update_thruster_glow_intensity(struct World* world, EntityID thruster_id, float intensity) {
    if (thruster_id == INVALID_ENTITY) return;
    
    struct Renderable* renderable = entity_get_renderable(world, thruster_id);
    if (!renderable) return;
    
    // Clamp intensity to 0-1 range
    intensity = fmaxf(0.0f, fminf(1.0f, intensity));
    
    // Make thrusters visible when active, invisible when not
    renderable->visible = (intensity > 0.01f);
    
    // Scale the thruster size based on intensity for visual effect
    struct Transform* transform = entity_get_transform(world, thruster_id);
    if (transform) {
        float base_scale = 0.2f + intensity * 0.8f;  // Scale from 20% to 100%
        transform->scale = (Vector3){base_scale, base_scale, base_scale * 1.5f}; // Elongated for nozzle
        transform->dirty = true;
    }
    
    // Debug thruster activity
    static uint32_t thruster_debug_counter = 0;
    if (intensity > 0.5f && ++thruster_debug_counter % 60 == 0) {
        printf("🔥 Thruster %d: intensity=%.2f, visible=%s\n", 
               thruster_id, intensity, renderable->visible ? "YES" : "NO");
    }
}

void setup_visual_thrusters(struct World* world, EntityID ship_id) {
    if (visual_thrusters.initialized) {
        printf("🔥 Visual thrusters already initialized\n");
        return;
    }
    
    printf("🔥 Setting up visual thrusters for ship %d...\n", ship_id);
    printf("🔥 World has %d entities\n", world->entity_count);
    
    // Main engines (rear of ship) - Blue/white exhaust
    Vector3 blue_glow = {0.3f, 0.7f, 1.0f};
    visual_thrusters.main_engines[0] = create_visual_thruster(world, 
        (Vector3){-1.5f, -0.5f, 3.0f}, (Vector3){0.5f, 0.5f, 1.0f}, blue_glow);
    visual_thrusters.main_engines[1] = create_visual_thruster(world, 
        (Vector3){1.5f, -0.5f, 3.0f}, (Vector3){0.5f, 0.5f, 1.0f}, blue_glow);
    
    // RCS thrusters (smaller, orange glow)
    Vector3 orange_glow = {1.0f, 0.6f, 0.2f};
    visual_thrusters.rcs_thrusters[0] = create_visual_thruster(world, 
        (Vector3){0.0f, 0.5f, -2.0f}, (Vector3){0.3f, 0.3f, 0.5f}, orange_glow); // Forward RCS
    visual_thrusters.rcs_thrusters[1] = create_visual_thruster(world, 
        (Vector3){0.0f, -0.5f, 3.5f}, (Vector3){0.3f, 0.3f, 0.5f}, orange_glow); // Rear RCS
    visual_thrusters.rcs_thrusters[2] = create_visual_thruster(world, 
        (Vector3){-2.0f, 0.0f, 0.0f}, (Vector3){0.5f, 0.3f, 0.3f}, orange_glow); // Left RCS
    visual_thrusters.rcs_thrusters[3] = create_visual_thruster(world, 
        (Vector3){2.0f, 0.0f, 0.0f}, (Vector3){0.5f, 0.3f, 0.3f}, orange_glow); // Right RCS
    
    // Vertical thrusters (green glow)
    Vector3 green_glow = {0.2f, 1.0f, 0.4f};
    visual_thrusters.vertical_thrusters[0] = create_visual_thruster(world, 
        (Vector3){0.0f, -1.5f, 0.0f}, (Vector3){0.4f, 0.3f, 0.4f}, green_glow); // Down thruster
    visual_thrusters.vertical_thrusters[1] = create_visual_thruster(world, 
        (Vector3){0.0f, 1.5f, 0.0f}, (Vector3){0.4f, 0.3f, 0.4f}, green_glow); // Up thruster
    
    // For now, don't use scene hierarchy - just make thrusters visible for testing
    // TODO: Implement proper scene node hierarchy once rendering supports it
    
    // Make all thrusters initially visible for debugging
    for (int i = 0; i < 2; i++) {
        if (visual_thrusters.main_engines[i] != INVALID_ENTITY) {
            struct Renderable* renderable = entity_get_renderable(world, visual_thrusters.main_engines[i]);
            if (renderable) {
                renderable->visible = true;
                printf("🔥 Main engine %d set to visible\n", i);
            }
        }
        if (visual_thrusters.vertical_thrusters[i] != INVALID_ENTITY) {
            struct Renderable* renderable = entity_get_renderable(world, visual_thrusters.vertical_thrusters[i]);
            if (renderable) {
                renderable->visible = true;
                printf("🔥 Vertical thruster %d set to visible\n", i);
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        if (visual_thrusters.rcs_thrusters[i] != INVALID_ENTITY) {
            struct Renderable* renderable = entity_get_renderable(world, visual_thrusters.rcs_thrusters[i]);
            if (renderable) {
                renderable->visible = true;
                printf("🔥 RCS thruster %d set to visible\n", i);
            }
        }
    }
    
    visual_thrusters.initialized = true;
    printf("🔥 Visual thrusters setup complete!\n");
    printf("🔥 Created thruster IDs: Main[%d,%d] RCS[%d,%d,%d,%d] Vert[%d,%d]\n",
           visual_thrusters.main_engines[0], visual_thrusters.main_engines[1],
           visual_thrusters.rcs_thrusters[0], visual_thrusters.rcs_thrusters[1],
           visual_thrusters.rcs_thrusters[2], visual_thrusters.rcs_thrusters[3],
           visual_thrusters.vertical_thrusters[0], visual_thrusters.vertical_thrusters[1]);
}

void update_visual_thrusters(struct World* world, float delta_time) {
    (void)delta_time;
    if (!visual_thrusters.initialized || player_ship_id == INVALID_ENTITY) {
        static uint32_t debug_counter = 0;
        if (++debug_counter % 120 == 0) {
            printf("🔥 Thruster update: initialized=%s, player_ship=%d\n", 
                   visual_thrusters.initialized ? "YES" : "NO", player_ship_id);
        }
        return;
    }
    
    // Visual thrusters use processed input from unified flight control
    
    // Get unified flight control to access processed input
    struct UnifiedFlightControl* control = entity_get_unified_flight_control(world, player_ship_id);
    if (!control) {
        static uint32_t control_debug_counter = 0;
        if (++control_debug_counter % 120 == 0) {
            printf("🔥 No unified flight control for player ship %d\n", player_ship_id);
        }
        return;
    }
    
    // Get linear and angular commands from unified control
    Vector3 linear_command = unified_flight_control_get_linear_command(control);
    Vector3 angular_command = unified_flight_control_get_angular_command(control);
    
    // Debug input values
    static uint32_t thruster_input_debug = 0;
    if (++thruster_input_debug % 60 == 0) {
        printf("🔥 Input: linear[%.2f,%.2f,%.2f] angular[%.2f,%.2f,%.2f]\n",
               linear_command.x, linear_command.y, linear_command.z,
               angular_command.x, angular_command.y, angular_command.z);
    }
    
    // Update main engines based on forward/backward thrust
    float main_thrust_intensity = fabsf(linear_command.z);  // Z is forward/backward
    
    // Debug main thrust
    static uint32_t main_debug = 0;
    if (main_thrust_intensity > 0.01f && ++main_debug % 30 == 0) {
        printf("🚀 Main thrust: %.2f\n", main_thrust_intensity);
    }
    
    update_thruster_glow_intensity(world, visual_thrusters.main_engines[0], main_thrust_intensity);
    update_thruster_glow_intensity(world, visual_thrusters.main_engines[1], main_thrust_intensity);
    
    // Update RCS thrusters based on strafe and directional movement
    float strafe_intensity = fabsf(linear_command.x);
    float vertical_intensity = fabsf(linear_command.y);
    
    // Left/Right RCS based on strafe
    update_thruster_glow_intensity(world, visual_thrusters.rcs_thrusters[2], 
                                 linear_command.x < 0 ? strafe_intensity : 0.0f); // Left RCS
    update_thruster_glow_intensity(world, visual_thrusters.rcs_thrusters[3], 
                                 linear_command.x > 0 ? strafe_intensity : 0.0f); // Right RCS
    
    // Forward/Backward RCS for maneuvering
    update_thruster_glow_intensity(world, visual_thrusters.rcs_thrusters[0], 
                                 linear_command.z > 0 ? main_thrust_intensity * 0.3f : 0.0f); // Forward RCS
    update_thruster_glow_intensity(world, visual_thrusters.rcs_thrusters[1], 
                                 linear_command.z < 0 ? main_thrust_intensity * 0.3f : 0.0f); // Rear RCS
    
    // Vertical thrusters
    update_thruster_glow_intensity(world, visual_thrusters.vertical_thrusters[0], 
                                 linear_command.y < 0 ? vertical_intensity : 0.0f); // Down thruster
    update_thruster_glow_intensity(world, visual_thrusters.vertical_thrusters[1], 
                                 linear_command.y > 0 ? vertical_intensity : 0.0f); // Up thruster
}

// ============================================================================

// Create a mini solar system
static void create_solar_system(struct World* world, EntityID parent) {
    (void)parent;  // Unused for now
    if (!world) return;
    
    printf("☀️ Creating mini solar system...\n");
    
    // Get the asset registry to load meshes
    AssetRegistry* assets = get_asset_registry();
    if (!assets) {
        printf("❌ Failed to get asset registry for solar system\n");
        return;
    }
    
    // Central sun
    EntityID sun = entity_create(world);
    if (sun == INVALID_ENTITY) {
        printf("❌ Failed to create sun entity\n");
        return;
    }
    
    entity_add_component(world, sun, COMPONENT_TRANSFORM);
    entity_add_component(world, sun, COMPONENT_RENDERABLE);
    
    struct Transform* sun_transform = entity_get_transform(world, sun);
    struct Renderable* sun_renderable = entity_get_renderable(world, sun);
    
    if (sun_transform && sun_renderable) {
        sun_transform->position = (Vector3){0, 500, 0};  // 500 units up from origin
        sun_transform->scale = (Vector3){50, 50, 50};    // Large sun
        sun_transform->rotation = (Quaternion){0, 0, 0, 1};
        sun_transform->dirty = true;
        
        // Set up renderable for the sun mesh
        if (assets_create_renderable_from_mesh(assets, "sun", sun_renderable)) {
            sun_renderable->visible = true;
            sun_renderable->lod_level = LOD_HIGH;
            printf("☀️ Created sun at (0, 500, 0) with scale 50\n");
        } else {
            printf("⚠️ Failed to load sun mesh, using logo_cube fallback\n");
            assets_create_renderable_from_mesh(assets, "logo_cube", sun_renderable);
            sun_renderable->visible = true;
        }
    }
    
    // Planet positions and properties
    struct {
        const char* name;
        float distance;
        float size;
        int material_id;
        float orbit_speed;
        Vector3 color_tint;
    } planets[] = {
        {"Mercury", 300.0f, 5.0f, 1, 0.02f, {0.7f, 0.7f, 0.7f}},
        {"Venus", 500.0f, 12.0f, 2, 0.015f, {1.0f, 0.9f, 0.6f}},
        {"Earth", 800.0f, 15.0f, 3, 0.01f, {0.3f, 0.5f, 1.0f}},
        {"Mars", 1200.0f, 10.0f, 4, 0.008f, {1.0f, 0.4f, 0.2f}},
        {"Jupiter", 2000.0f, 40.0f, 5, 0.005f, {0.9f, 0.7f, 0.5f}},
        {"Saturn", 3000.0f, 35.0f, 6, 0.003f, {1.0f, 0.9f, 0.7f}},
    };
    
    // Create planets
    for (int i = 0; i < 6; i++) {
        EntityID planet = entity_create(world);
        if (planet == INVALID_ENTITY) {
            printf("❌ Failed to create planet %s\n", planets[i].name);
            continue;
        }
        
        entity_add_component(world, planet, COMPONENT_TRANSFORM);
        entity_add_component(world, planet, COMPONENT_RENDERABLE);
        
        struct Transform* transform = entity_get_transform(world, planet);
        struct Renderable* renderable = entity_get_renderable(world, planet);
        
        if (transform && renderable) {
            // Position in orbit
            float angle = ((float)i / 6.0f) * 2.0f * M_PI;
            transform->position = (Vector3){
                cosf(angle) * planets[i].distance,
                500.0f,  // Same height as sun
                sinf(angle) * planets[i].distance
            };
            transform->scale = (Vector3){planets[i].size, planets[i].size, planets[i].size};
            transform->rotation = (Quaternion){0, 0, 0, 1};
            transform->dirty = true;
            
            // Use logo_cube mesh for all planets (it's the only sphere-like mesh available)
            assets_create_renderable_from_mesh(assets, "logo_cube", renderable);
            renderable->visible = true;
            renderable->lod_level = LOD_HIGH;
            renderable->material_id = planets[i].material_id;
            
            printf("🪐 Created %s at distance %.0f with size %.0f\n", 
                   planets[i].name, planets[i].distance, planets[i].size);
        }
    }
    
    // Create asteroid belt between Mars and Jupiter
    int asteroid_count = 50;
    float inner_radius = 1500.0f;
    float outer_radius = 1800.0f;
    
    printf("   ☄️ Creating asteroid belt with %d asteroids\n", asteroid_count);
    
    for (int i = 0; i < asteroid_count; i++) {
        EntityID asteroid = entity_create(world);
        if (asteroid == INVALID_ENTITY) continue;
        
        entity_add_component(world, asteroid, COMPONENT_TRANSFORM);
        entity_add_component(world, asteroid, COMPONENT_RENDERABLE);
        
        struct Transform* transform = entity_get_transform(world, asteroid);
        struct Renderable* renderable = entity_get_renderable(world, asteroid);
        
        if (transform && renderable) {
            // Random position in belt
            float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
            float radius = inner_radius + ((float)rand() / RAND_MAX) * (outer_radius - inner_radius);
            float height_offset = (((float)rand() / RAND_MAX) - 0.5f) * 100.0f;
            
            transform->position = (Vector3){
                cosf(angle) * radius,
                500.0f + height_offset,
                sinf(angle) * radius
            };
            
            float size = 2.0f + ((float)rand() / RAND_MAX) * 5.0f;
            transform->scale = (Vector3){size, size, size};
            transform->rotation = (Quaternion){0, 0, 0, 1};
            transform->dirty = true;
            
            // Use logo_cube mesh for asteroids
            assets_create_renderable_from_mesh(assets, "logo_cube", renderable);
            renderable->visible = true;
            renderable->lod_level = LOD_MEDIUM;  // Lower LOD for performance
            renderable->material_id = 0;  // Default material
        }
    }
    
    printf("☀️ Solar system created!\n");
}

// ============================================================================

// Scene script definition
const SceneScript flight_test_script = {
    .scene_name = "flight_test",
    .on_enter = flight_test_init,
    .on_update = flight_test_update,
    .on_exit = flight_test_cleanup,
    .on_input = flight_test_input
};
