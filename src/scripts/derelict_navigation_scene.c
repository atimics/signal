// src/scripts/derelict_navigation_scene.c
// Derelict navigation scene - 3D magnetic racing through broken ship sections
// Demonstrates "sticky ship" physics where gravity/orientation relative to nearest mass

#include "../scene_script.h"
#include "../core.h"
#include "../render.h"
#include "../system/material.h"
#include "../system/input.h"
#include "../sokol_app.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Derelict navigation state
static bool derelict_nav_initialized = false;
static float navigation_time = 0.0f;
static bool navigation_active = false;
static EntityID player_ship_id = INVALID_ENTITY;

// Camera system state
typedef enum {
    CAMERA_MODE_COCKPIT = 0,
    CAMERA_MODE_CHASE_NEAR = 1,
    CAMERA_MODE_CHASE_FAR = 2,
    CAMERA_MODE_COUNT = 3
} CameraMode;

static CameraMode current_camera_mode = CAMERA_MODE_CHASE_NEAR;
static Vector3 camera_velocity = {0, 0, 0};  // For elastic camera movement
static float camera_zoom_factor = 1.0f;      // Dynamic zoom based on speed
static float camera_target_zoom = 1.0f;

// Physics constants for magnetic ship behavior
#define GRAVITY_RANGE 120.0f
#define MAGNETIC_ATTRACTION_FORCE 0.1f    // Much more reduced - was still too strong
#define ORIENTATION_ADJUSTMENT_SPEED 0.05f // Much more reduced - was still too strong  
#define DERELICT_SECTION_COUNT 8

// Player control constants
#define PLAYER_THRUST_FORCE 15.0f
#define PLAYER_BOOST_MULTIPLIER 2.5f
#define PLAYER_MANEUVER_FORCE 8.0f
#define PLAYER_BRAKE_FACTOR 0.85f

// Derelict navigation zones
typedef struct {
    Vector3 position;
    float mass;
    float magnetic_field;
    bool is_hazardous;
} DerelictSection;

static DerelictSection derelict_sections[DERELICT_SECTION_COUNT];

void derelict_navigation_init(struct World* world, SceneStateManager* state) {
    (void)state;
    
    if (derelict_nav_initialized) return;
    
    printf("🧲 Initializing Derelict Navigation Scene - Aethelian Command Ship\n");
    
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
    }
    
    // Clear old input state and initialize new input system
    if (!input_init()) {
        printf("⚠️  Input initialization failed\n");
    }
    
    // Initialize derelict sections as gravity/magnetic sources
    derelict_sections[0] = (DerelictSection){{0, 0, 0}, 50.0f, 25.0f, false};      // Central command
    derelict_sections[1] = (DerelictSection){{-80, 0, 0}, 30.0f, 15.0f, false};   // Port section
    derelict_sections[2] = (DerelictSection){{80, 0, 0}, 30.0f, 15.0f, false};    // Starboard section
    derelict_sections[3] = (DerelictSection){{0, 0, -100}, 25.0f, 20.0f, true};   // Forward (damaged)
    derelict_sections[4] = (DerelictSection){{0, 0, 100}, 25.0f, 20.0f, false};   // Aft section
    derelict_sections[5] = (DerelictSection){{0, 40, 0}, 20.0f, 10.0f, false};    // Upper section
    derelict_sections[6] = (DerelictSection){{0, -40, 0}, 20.0f, 10.0f, true};    // Lower (breached)
    derelict_sections[7] = (DerelictSection){{-40, 20, -60}, 15.0f, 30.0f, true}; // Engine core (unstable)
    
    navigation_time = 0.0f;
    navigation_active = true;
    derelict_nav_initialized = true;
    
    // Initialize camera system
    current_camera_mode = CAMERA_MODE_CHASE_NEAR;
    camera_velocity = (Vector3){0, 0, 0};
    camera_zoom_factor = 1.0f;
    camera_target_zoom = 1.0f;
    
    // Create our own dynamic camera entity to override scene cameras
    printf("🔍 DEBUG: Looking for cockpit camera instead of creating new one...\n");
    
    // Find the cockpit camera (Entity 26 based on logs) and override its position
    EntityID cockpit_camera = INVALID_ENTITY;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA) {
            struct Camera* cam = entity_get_camera(world, entity->id);
            if (cam && cam->fov >= 70.0f && cam->fov <= 80.0f) {  // Cockpit camera has fov 75
                cockpit_camera = entity->id;
                break;
            }
        }
    }
    
    if (cockpit_camera != INVALID_ENTITY) {
        printf("🔍 DEBUG: Found cockpit camera Entity %d, setting as active\n", cockpit_camera);
        world_set_active_camera(world, cockpit_camera);
        
        // Adjust its position to be closer to the player ship
        struct Transform* cam_transform = entity_get_transform(world, cockpit_camera);
        struct Camera* cam_camera = entity_get_camera(world, cockpit_camera);
        
        if (cam_transform && cam_camera) {
            // Start the camera at a good chase position behind the player
            cam_transform->position = (Vector3){10, -3, -105};  // Behind and above player start
            cam_camera->target = (Vector3){0, -8, -120};        // Look at player ship start position
            cam_camera->matrices_dirty = true;
            printf("📷 Repositioned camera for dynamic chase behind player ship\n");
        }
    } else {
        printf("❌ Could not find suitable camera to override\n");
    }
    
    printf("🧲 Derelict navigation initialized - %d sections detected\n", DERELICT_SECTION_COUNT);
    printf("📡 Magnetic field mapping complete - Sticky ship physics active\n");
    printf("🎮 Player Controls:\n");
    printf("   Keyboard: W/S - Forward/Backward thrust\n");
    printf("             A/D - Strafe left/right\n");
    printf("             Q/E - Vertical maneuver\n");
    printf("             Shift - Boost\n");
    printf("             Ctrl - Brake\n");
    printf("             C - Cycle camera modes\n");
    printf("   Gamepad:  Left Stick - Thrust/Strafe\n");
    printf("             Right Stick Y - Vertical\n");
    printf("             Right Trigger - Boost\n");
    printf("             Left Trigger - Brake\n");
    printf("📷 Camera Modes: COCKPIT → CHASE_NEAR → CHASE_FAR\n");
}

// Calculate the dominant gravity/magnetic source for ship orientation
Vector3 calculate_dominant_mass_direction(Vector3 ship_pos, float* out_field_strength) {
    Vector3 net_direction = {0, 0, 0};
    float total_influence = 0.0f;
    float strongest_field = 0.0f;
    
    // Check all derelict sections for gravitational influence
    for (int i = 0; i < DERELICT_SECTION_COUNT; i++) {
        DerelictSection* section = &derelict_sections[i];
        
        Vector3 diff = {
            section->position.x - ship_pos.x,
            section->position.y - ship_pos.y,
            section->position.z - ship_pos.z
        };
        
        float distance = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        if (distance < GRAVITY_RANGE && distance > 0.1f) {
            // Calculate gravitational influence (inverse square law with magnetic field)
            float influence = (section->mass * section->magnetic_field) / (distance * distance + 1.0f);
            
            // Normalize direction vector
            Vector3 unit_dir = {diff.x / distance, diff.y / distance, diff.z / distance};
            
            // Add weighted direction to net direction
            net_direction.x += unit_dir.x * influence;
            net_direction.y += unit_dir.y * influence;
            net_direction.z += unit_dir.z * influence;
            
            total_influence += influence;
            
            if (influence > strongest_field) {
                strongest_field = influence;
            }
            
            // Add hazard effects for damaged sections
            if (section->is_hazardous && distance < 30.0f) {
                // Magnetic instability causes wobble
                float wobble = sinf(navigation_time * 3.0f + (float)i) * 0.3f;
                net_direction.x += wobble;
                net_direction.y += wobble * 0.5f;
            }
        }
    }
    
    // Normalize the net direction if there's any influence
    if (total_influence > 0.01f) {
        float magnitude = sqrtf(net_direction.x * net_direction.x + 
                               net_direction.y * net_direction.y + 
                               net_direction.z * net_direction.z);
        if (magnitude > 0.01f) {
            net_direction.x /= magnitude;
            net_direction.y /= magnitude;
            net_direction.z /= magnitude;
        }
    } else {
        // Default orientation if no mass influence
        net_direction = (Vector3){0, -1, 0};
    }
    
    if (out_field_strength) {
        *out_field_strength = strongest_field;
    }
    
    return net_direction;
}

// Update dynamic camera system
void update_camera_system(struct World* world, float delta_time) {
    if (player_ship_id == INVALID_ENTITY) return;
    
    // Find player entity
    struct Entity* player_entity = NULL;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->id == player_ship_id && (entity->component_mask & COMPONENT_PLAYER)) {
            player_entity = entity;
            break;
        }
    }
    
    if (!player_entity || !player_entity->transform) return;
    
    // Get player position and velocity
    Vector3 player_pos = player_entity->transform->position;
    Vector3 player_vel = {0, 0, 0};
    if (player_entity->physics) {
        player_vel = player_entity->physics->velocity;
    }
    
    // Calculate player speed for dynamic zoom
    float player_speed = sqrtf(player_vel.x * player_vel.x + 
                              player_vel.y * player_vel.y + 
                              player_vel.z * player_vel.z);
    
    // Get the active camera
    EntityID active_camera_id = world_get_active_camera(world);
    struct Entity* camera_entity = NULL;
    
    if (active_camera_id != INVALID_ENTITY) {
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->id == active_camera_id) {
                camera_entity = entity;
                break;
            }
        }
    }
    
    if (!camera_entity || !camera_entity->transform || !camera_entity->camera) return;
    
    Vector3 desired_pos = player_pos;
    Vector3 desired_target = player_pos;
    
    switch (current_camera_mode) {
        case CAMERA_MODE_COCKPIT: {
            // Cockpit view - camera inside/on the ship
            desired_pos.x = player_pos.x;
            desired_pos.y = player_pos.y + 1.0f;  // Slightly above player center
            desired_pos.z = player_pos.z + 2.0f;  // Slightly forward
            
            // Look in direction of movement
            Vector3 look_dir = {player_vel.x, player_vel.y, player_vel.z};
            float vel_mag = sqrtf(look_dir.x * look_dir.x + look_dir.y * look_dir.y + look_dir.z * look_dir.z);
            if (vel_mag > 0.1f) {
                look_dir.x /= vel_mag;
                look_dir.y /= vel_mag;
                look_dir.z /= vel_mag;
                desired_target.x = player_pos.x + look_dir.x * 10.0f;
                desired_target.y = player_pos.y + look_dir.y * 10.0f;
                desired_target.z = player_pos.z + look_dir.z * 10.0f;
            } else {
                // Default forward direction
                desired_target.z = player_pos.z - 10.0f;
            }
            break;
        }
        
        case CAMERA_MODE_CHASE_NEAR: {
            // Dynamic chase camera with speed-based zoom
            // Base distance varies with speed (zoom out on acceleration)
            float base_distance = 15.0f;
            float speed_factor = player_speed / 20.0f;  // Normalize speed
            camera_target_zoom = 1.0f + speed_factor * 1.5f;  // Zoom out with speed
            
            // Smooth zoom transition with elasticity
            float zoom_spring = 8.0f;
            float zoom_damping = 0.7f;
            float zoom_diff = camera_target_zoom - camera_zoom_factor;
            camera_zoom_factor += zoom_diff * zoom_spring * delta_time;
            camera_zoom_factor *= (1.0f - zoom_damping * delta_time);
            
            float distance = base_distance * camera_zoom_factor;
            
            // Camera position behind and above player
            Vector3 offset = {distance * 0.6f, distance * 0.4f, distance};
            
            // Add velocity prediction for smoother camera
            Vector3 predicted_pos = {
                player_pos.x + player_vel.x * 0.3f,
                player_pos.y + player_vel.y * 0.3f,
                player_pos.z + player_vel.z * 0.3f
            };
            
            desired_pos.x = predicted_pos.x + offset.x;
            desired_pos.y = predicted_pos.y + offset.y;
            desired_pos.z = predicted_pos.z + offset.z;
            desired_target = predicted_pos;
            break;
        }
        
        case CAMERA_MODE_CHASE_FAR: {
            // Stable far chase camera with less elasticity
            float distance = 40.0f;
            Vector3 offset = {distance * 0.7f, distance * 0.5f, distance * 0.8f};
            
            desired_pos.x = player_pos.x + offset.x;
            desired_pos.y = player_pos.y + offset.y;
            desired_pos.z = player_pos.z + offset.z;
            desired_target = player_pos;
            break;
        }
        
        case CAMERA_MODE_COUNT:
        default:
            // Fallback to chase near
            current_camera_mode = CAMERA_MODE_CHASE_NEAR;
            break;
    }
    
    // Apply camera movement with elasticity
    float elasticity = 0.0f;
    float damping = 0.0f;
    
    switch (current_camera_mode) {
        case CAMERA_MODE_COCKPIT:
            elasticity = 15.0f;  // Very responsive
            damping = 0.9f;
            break;
        case CAMERA_MODE_CHASE_NEAR:
            elasticity = 6.0f;   // Moderate elasticity
            damping = 0.8f;
            break;
        case CAMERA_MODE_CHASE_FAR:
            elasticity = 3.0f;   // Less elastic, more stable
            damping = 0.6f;
            break;
        case CAMERA_MODE_COUNT:
        default:
            elasticity = 6.0f;   // Default to chase near
            damping = 0.8f;
            break;
    }
    
    // Calculate camera movement forces
    Vector3 pos_diff = {
        desired_pos.x - camera_entity->transform->position.x,
        desired_pos.y - camera_entity->transform->position.y,
        desired_pos.z - camera_entity->transform->position.z
    };
    
    // Apply spring forces to camera velocity
    camera_velocity.x += pos_diff.x * elasticity * delta_time;
    camera_velocity.y += pos_diff.y * elasticity * delta_time;
    camera_velocity.z += pos_diff.z * elasticity * delta_time;
    
    // Apply damping
    camera_velocity.x *= (1.0f - damping * delta_time);
    camera_velocity.y *= (1.0f - damping * delta_time);
    camera_velocity.z *= (1.0f - damping * delta_time);
    
    // Update camera position
    camera_entity->transform->position.x += camera_velocity.x * delta_time;
    camera_entity->transform->position.y += camera_velocity.y * delta_time;
    camera_entity->transform->position.z += camera_velocity.z * delta_time;
    
    // Update camera target
    camera_entity->camera->target = desired_target;
    
    // Debug output (less frequent)
    static float last_cam_log = 0.0f;
    if (navigation_time - last_cam_log > 3.0f) {
        const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
        printf("📷 Camera [%s] Player:(%.1f,%.1f,%.1f) Speed:%.1f Zoom:%.2f\n",
               mode_names[current_camera_mode],
               player_pos.x, player_pos.y, player_pos.z,
               player_speed, camera_zoom_factor);
        last_cam_log = navigation_time;
    }
}
void apply_player_controls(struct World* world, float delta_time) {
    if (player_ship_id == INVALID_ENTITY) return;
    
    // Find the player ship entity
    struct Entity* player_entity = NULL;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        if (world->entities[i].id == player_ship_id) {
            player_entity = &world->entities[i];
            break;
        }
    }
    
    if (!player_entity || !(player_entity->component_mask & (COMPONENT_PHYSICS | COMPONENT_TRANSFORM))) {
        return;
    }
    
    struct Physics* physics = player_entity->physics;
    struct Transform* transform = player_entity->transform;
    
    if (!physics || !transform) return;
    
    // Get unified input state
    const InputState* input = input_get_state();
    if (!input) return;
    
    float thrust_force = PLAYER_THRUST_FORCE;
    float maneuver_force = PLAYER_MANEUVER_FORCE;
    
    // Apply boost multiplier
    float effective_boost = 1.0f + (PLAYER_BOOST_MULTIPLIER - 1.0f) * input->boost;
    thrust_force *= effective_boost;
    maneuver_force *= effective_boost;
    
    // Apply forces
    static float last_input_log = 0.0f;
    bool has_input = (fabsf(input->thrust) > 0.01f || fabsf(input->strafe) > 0.01f || fabsf(input->vertical) > 0.01f);
    
    if (has_input) {
        physics->acceleration.z -= input->thrust * thrust_force * delta_time; // Forward is negative Z
        physics->acceleration.x += input->strafe * maneuver_force * delta_time;
        physics->acceleration.y += input->vertical * maneuver_force * delta_time;
        
        // Log input activity occasionally
        if (navigation_time - last_input_log > 2.0f) {
            printf("🎮 Input: thrust:%.2f strafe:%.2f vertical:%.2f boost:%.2f\n", 
                   input->thrust, input->strafe, input->vertical, input->boost);
            last_input_log = navigation_time;
        }
    }
    
    // Apply braking
    if (input->brake) {
        physics->velocity.x *= PLAYER_BRAKE_FACTOR;
        physics->velocity.y *= PLAYER_BRAKE_FACTOR;
        physics->velocity.z *= PLAYER_BRAKE_FACTOR;
    }
}

void derelict_navigation_update(struct World* world, SceneStateManager* state, float delta_time) {
    if (!derelict_nav_initialized || !navigation_active) return;
    
    navigation_time += delta_time;
    
    // Update input system
    input_update();
    
    // Apply player ship controls
    apply_player_controls(world, delta_time);
    
    // Update dynamic camera system
    update_camera_system(world, delta_time);
    
    // Apply magnetic ship physics to all ships with physics components
    // TEMPORARY: Disable magnetic physics to debug visibility
    bool enable_magnetic_physics = false;  // Set to true once ship is visible
    
    if (enable_magnetic_physics) {
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            
            if (!(entity->component_mask & (COMPONENT_PHYSICS | COMPONENT_TRANSFORM))) continue;
            
            struct Physics* physics = entity->physics;
            struct Transform* transform = entity->transform;
            
            if (!physics || !transform) continue;
            
            // DISABLED: Calculate dominant mass direction and field strength
            // float field_strength = 0.0f;
            // Vector3 gravity_dir = calculate_dominant_mass_direction(transform->position, &field_strength);
            
            // DISABLED: Apply magnetic attraction force toward dominant mass
            // float attraction_force = MAGNETIC_ATTRACTION_FORCE * field_strength * delta_time;
            // physics->acceleration.x += gravity_dir.x * attraction_force;
            // physics->acceleration.y += gravity_dir.y * attraction_force;
            // physics->acceleration.z += gravity_dir.z * attraction_force;
            
            // DISABLED: "Sticky ship" orientation: gradually align ship's up vector with away from mass
            // This creates the feeling that the ship's "down" is toward the nearest mass
            // Vector3 desired_up = {-gravity_dir.x, -gravity_dir.y, -gravity_dir.z};
            
            // DISABLED: Apply gradual orientation adjustment
            // float orientation_speed = ORIENTATION_ADJUSTMENT_SPEED * delta_time;
            
            // DISABLED: Simulate ship rotation by adjusting velocity based on orientation (much reduced effect)
            // physics->velocity.x += desired_up.x * orientation_speed * 0.05f;  // Much more reduced
            // physics->velocity.y += desired_up.y * orientation_speed * 0.05f;  // Much more reduced
            // physics->velocity.z += desired_up.z * orientation_speed * 0.05f;  // Much more reduced
            
            // DISABLED: Add autonomous navigation for AI ships (skip player ship)
            if (false && entity->id > 0 && entity->id != player_ship_id) {
                // DEBUG: Check if this is being applied to player ship by mistake
                if (entity->id == 21) {  // Player ship entity (Entity 21 from logs)
                    printf("❌ ERROR: AI navigation being applied to player ship! entity->id=%d, player_ship_id=%d\n", 
                           entity->id, player_ship_id);
                }
                
                float thrust_base = 8.0f;
                
                // Calculate distance to nearest obstacle for evasive maneuvers
                float min_obstacle_distance = 1000.0f;
                Vector3 avoidance_vector = {0, 0, 0};
                
                for (uint32_t j = 0; j < world->entity_count; j++) {
                    if (i == j) continue;
                    struct Entity* other = &world->entities[j];
                    if (!(other->component_mask & COMPONENT_TRANSFORM)) continue;
                    
                    struct Transform* other_transform = other->transform;
                    if (!other_transform) continue;
                    
                    float dx = transform->position.x - other_transform->position.x;
                    float dy = transform->position.y - other_transform->position.y;
                    float dz = transform->position.z - other_transform->position.z;
                    float distance = sqrtf(dx*dx + dy*dy + dz*dz);
                    
                    if (distance < min_obstacle_distance && distance > 0.1f) {
                        min_obstacle_distance = distance;
                        // Calculate avoidance direction
                        avoidance_vector.x = dx / distance;
                        avoidance_vector.y = dy / distance;
                        avoidance_vector.z = dz / distance;
                    }
                }
                
                // Enhanced navigation based on proximity to obstacles
                float navigation_multiplier = 1.0f;
                if (min_obstacle_distance < 40.0f) {
                    navigation_multiplier = 1.0f + (40.0f - min_obstacle_distance) / 40.0f * 3.0f;
                    
                    // Add avoidance thrust
                    physics->acceleration.x += avoidance_vector.x * thrust_base * navigation_multiplier * delta_time;
                    physics->acceleration.y += avoidance_vector.y * thrust_base * navigation_multiplier * delta_time;
                    physics->acceleration.z += avoidance_vector.z * thrust_base * navigation_multiplier * delta_time;
                }
                
                // Add patrol navigation - ships circle around derelict sections
                float patrol_radius = 60.0f + (float)(entity->id % 3) * 20.0f;
                float patrol_speed = 0.5f + (float)(entity->id % 2) * 0.3f;
                float patrol_angle = navigation_time * patrol_speed + (float)entity->id;
                
                Vector3 patrol_target = {
                    cosf(patrol_angle) * patrol_radius,
                    sinf(patrol_angle * 0.7f) * 15.0f,
                    sinf(patrol_angle) * patrol_radius
                };
                
                Vector3 to_patrol = {
                    patrol_target.x - transform->position.x,
                    patrol_target.y - transform->position.y,
                    patrol_target.z - transform->position.z
                };
                
                float patrol_distance = sqrtf(to_patrol.x*to_patrol.x + to_patrol.y*to_patrol.y + to_patrol.z*to_patrol.z);
                if (patrol_distance > 0.1f) {
                    physics->acceleration.x += (to_patrol.x / patrol_distance) * thrust_base * 0.5f * delta_time;
                    physics->acceleration.y += (to_patrol.y / patrol_distance) * thrust_base * 0.5f * delta_time;
                    physics->acceleration.z += (to_patrol.z / patrol_distance) * thrust_base * 0.5f * delta_time;
                }
            }
            
            // Apply atmospheric drag to prevent runaway acceleration
            float drag_factor = 0.96f;
            physics->velocity.x *= drag_factor;
            physics->velocity.y *= drag_factor;
            physics->velocity.z *= drag_factor;
            
            // Velocity limiting for stability
            float max_velocity = 50.0f;
            float velocity_magnitude = sqrtf(physics->velocity.x * physics->velocity.x + 
                                           physics->velocity.y * physics->velocity.y + 
                                           physics->velocity.z * physics->velocity.z);
            if (velocity_magnitude > max_velocity) {
                float scale = max_velocity / velocity_magnitude;
                physics->velocity.x *= scale;
                physics->velocity.y *= scale;
                physics->velocity.z *= scale;
            }
        }
    }
    
    // Periodic status updates
    static float last_debug = 0.0f;
    if (navigation_time - last_debug > 5.0f) {
        printf("🧲 Derelict Navigation: %.1fs - Magnetic field strength varies across %d sections\n", 
               navigation_time, DERELICT_SECTION_COUNT);
        last_debug = navigation_time;
    }
    
    (void)state;
}

static bool derelict_navigation_input(struct World* world, SceneStateManager* state, const void* event) {
    (void)world;
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        // System navigation keys
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            printf("🧲 Derelict Navigation: ESC pressed, returning to navigation menu\n");
            scene_state_request_transition(state, "navigation_menu");
            return true;
        }
        if (ev->key_code == SAPP_KEYCODE_TAB) {
            printf("🧲 Switching to system overview\n");
            scene_state_request_transition(state, "system_overview");
            return true;
        }
        if (ev->key_code == SAPP_KEYCODE_SPACE) {
            // Toggle navigation active/passive for testing
            navigation_active = !navigation_active;
            printf("🧲 Magnetic navigation %s\n", navigation_active ? "ACTIVE" : "PASSIVE");
            return true;
        }
        if (ev->key_code == SAPP_KEYCODE_C) {
            // Cycle camera modes
            current_camera_mode = (CameraMode)((current_camera_mode + 1) % CAMERA_MODE_COUNT);
            const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
            printf("📷 Camera mode: %s\n", mode_names[current_camera_mode]);
            
            // Reset camera state for smooth transition
            camera_velocity = (Vector3){0, 0, 0};
            camera_zoom_factor = 1.0f;
            camera_target_zoom = 1.0f;
            return true;
        }
        
        // Player ship controls - use input abstraction
        if (input_handle_keyboard(ev->key_code, true)) {
            return true;
        }
    }
    
    if (ev->type == SAPP_EVENTTYPE_KEY_UP) {
        // Player ship controls - use input abstraction
        if (input_handle_keyboard(ev->key_code, false)) {
            return true;
        }
    }
    
    return false;
}

void derelict_navigation_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    if (!derelict_nav_initialized) return;
    
    navigation_active = false;
    derelict_nav_initialized = false;
    
    // Shutdown input system
    input_shutdown();
    
    printf("🧲 Derelict navigation cleanup complete\n");
}

// ============================================================================
// SCENE SCRIPT DEFINITION
// ============================================================================

const SceneScript derelict_navigation_script = {
    .scene_name = "derelict_alpha",
    .on_enter = derelict_navigation_init,
    .on_update = derelict_navigation_update,
    .on_exit = derelict_navigation_cleanup,
    .on_input = derelict_navigation_input
};
