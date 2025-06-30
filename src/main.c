#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

// Nuklear implementation
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "nuklear.h"

// Define nk_bool as int for sokol_nuklear compatibility
typedef int nk_bool;

#define SOKOL_NUKLEAR_IMPL
#include "sokol_nuklear.h"

#include "core.h"
#include "systems.h"
#include "render.h"
#include "data.h"
#include "assets.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// APPLICATION STATE
// ============================================================================

static struct {
    // Core ECS
    struct World world;
    struct SystemScheduler scheduler;
    AssetRegistry assets;
    
    // Application state
    EntityID player_id;
    int frame_count;
    float simulation_time;
    bool initialized;
    
    // Render state
    RenderConfig render_config;
    sg_pass_action pass_action;
    
    // Scene management
    char current_scene[64];
} app_state = {0};

// ============================================================================
// ENTITY FACTORY FUNCTIONS (from test.c)
// ============================================================================

static EntityID create_player(struct World* world, Vector3 position) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) return id;
    
    // Add components
    entity_add_component(world, id, COMPONENT_TRANSFORM);
    entity_add_component(world, id, COMPONENT_PHYSICS);
    entity_add_component(world, id, COMPONENT_COLLISION);
    entity_add_component(world, id, COMPONENT_PLAYER);
    entity_add_component(world, id, COMPONENT_RENDERABLE);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    transform->position = position;
    transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
    transform->rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
    transform->dirty = true;
    
    struct Physics* physics = entity_get_physics(world, id);
    physics->mass = 80.0f;
    physics->drag = 0.98f;
    
    struct Collision* collision = entity_get_collision(world, id);
    collision->radius = 4.0f;
    collision->layer_mask = 0xFFFFFFFF;
    
    // Configure renderable component - try to load wedge_ship mesh
    struct Renderable* renderable = entity_get_renderable(world, id);
    if (renderable && !assets_create_renderable_from_mesh(&app_state.assets, "wedge_ship", renderable)) {
        // If wedge_ship not available, try wedge_ship_mk2
        if (!assets_create_renderable_from_mesh(&app_state.assets, "wedge_ship_mk2", renderable)) {
            printf("⚠️  No suitable mesh found for player, using default resources\n");
            // Set up with invalid handles - will skip rendering
            renderable->vbuf.id = SG_INVALID_ID;
            renderable->ibuf.id = SG_INVALID_ID;
            renderable->tex.id = SG_INVALID_ID;
            renderable->index_count = 0;
            renderable->visible = false;
        }
    }
    
    printf("🚀 Created player ship at (%.1f, %.1f, %.1f)\n", 
           position.x, position.y, position.z);
    
    return id;
}

static void simulate_player_input(struct World* world, EntityID player_id, float time) {
    struct Physics* physics = entity_get_physics(world, player_id);
    if (!physics) return;
    
    // Gentle hovering movement for demonstration
    float hover_radius = 5.0f;
    float hover_speed = 0.5f;
    
    // Calculate gentle hovering position around starting point
    float center_x = 30.0f;
    float center_z = 0.0f;
    
    float desired_x = center_x + hover_radius * cosf(time * hover_speed);
    float desired_z = center_z + hover_radius * sinf(time * hover_speed);
    
    // Get current position
    struct Transform* transform = entity_get_transform(world, player_id);
    if (!transform) return;
    
    // Gentle steering toward hovering position
    float dx = desired_x - transform->position.x;
    float dz = desired_z - transform->position.z;
    
    physics->acceleration.x = dx * 0.1f;
    physics->acceleration.z = dz * 0.1f;
    physics->acceleration.y = sinf(time * 1.0f) * 0.5f;
}

static void load_scene_by_name(struct World* world, const char* scene_name, EntityID* player_id) {
    printf("🏗️  Loading scene '%s' from data...\n", scene_name);
    
    DataRegistry* data_registry = get_data_registry();
    
    // Load the scene from template
    if (!load_scene(world, data_registry, scene_name)) {
        printf("❌ Failed to load scene: %s\n", scene_name);
        return;
    }
    
    // Find the player entity
    *player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            *player_id = entity->id;
            break;
        }
    }
    
    if (*player_id != INVALID_ENTITY) {
        printf("🎯 Player found: Entity ID %d\n", *player_id);
    } else {
        printf("⚠️ No player entity found in scene.\n");
    }
    
    // Set up scene lighting
    RenderConfig* render_config = get_render_config();
    if (render_config) {
        lighting_set_ambient(&render_config->lighting, (Vector3){0.1f, 0.15f, 0.2f}, 0.3f);
        lighting_add_directional_light(&render_config->lighting, 
                                      (Vector3){0.3f, -0.7f, 0.2f}, 
                                      (Vector3){0.4f, 0.5f, 0.6f}, 
                                      0.4f);
        printf("💡 Scene lighting configured\n");
    }
    
    printf("🌍 Scene loaded with %d entities\n", world->entity_count);
}

static void list_available_cameras(struct World* world) {
    printf("\n📹 Available cameras:\n");
    int camera_count = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA) {
            camera_count++;
            struct Camera* camera = entity->camera;
            const char* behavior_name = "Unknown";
            switch (camera->behavior) {
                case 0: behavior_name = "Third Person"; break;
                case 1: behavior_name = "First Person"; break; 
                case 2: behavior_name = "Static"; break;
                case 3: behavior_name = "Chase"; break;
                case 4: behavior_name = "Orbital"; break;
            }
            printf("   %d: Entity %d (%s)\n", camera_count, entity->id, behavior_name);
        }
    }
    printf("\n");
}

// ============================================================================
// SOKOL CALLBACKS
// ============================================================================

static void init(void) {
    printf("🎮 CGGame - Sokol-based Component Engine\n");
    printf("==========================================\n\n");
    
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    // Initialize Sokol graphics
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    
    if (!sg_isvalid()) {
        printf("❌ Failed to initialize Sokol graphics\n");
        sapp_quit();
        return;
    }
    
    // Initialize world
    if (!world_init(&app_state.world)) {
        printf("❌ Failed to initialize world\n");
        sapp_quit();
        return;
    }
    
    // Initialize system scheduler
    if (!scheduler_init(&app_state.scheduler)) {
        printf("❌ Failed to initialize scheduler\n");
        sapp_quit();
        return;
    }
    
    // Initialize assets
    if (!assets_init(&app_state.assets, "assets")) {
        printf("❌ Failed to initialize assets\n");
        sapp_quit();
        return;
    }
    
    // Initialize render config
    app_state.render_config = (RenderConfig){
        .screen_width = (int)sapp_width(),
        .screen_height = (int)sapp_height(),
        .assets = &app_state.assets,
        .camera = {
            .position = {5.0f, 3.0f, 10.0f},
            .target = {0.0f, 0.0f, 0.0f},
            .up = {0.0f, 1.0f, 0.0f},
            .fov = 45.0f * M_PI / 180.0f,
            .near_plane = 0.1f,
            .far_plane = 100.0f,
            .aspect_ratio = (float)sapp_width() / (float)sapp_height()
        },
        .show_debug_info = true
    };
    
    // Initialize renderer
    if (!render_init(&app_state.render_config, &app_state.assets, 
                     (float)sapp_width(), (float)sapp_height())) {
        printf("❌ Failed to initialize renderer\n");
        sapp_quit();
        return;
    }
    
    // Set up pass action
    app_state.pass_action = (sg_pass_action){
        .colors[0] = { 
            .load_action = SG_LOADACTION_CLEAR, 
            .clear_value = {0.0f, 0.05f, 0.1f, 1.0f} 
        }
    };
    
    // Load default scene
    const char* scene_to_load = "mesh_test";
    strcpy(app_state.current_scene, scene_to_load);
    printf("ℹ️ Loading default scene: %s\n", scene_to_load);
    
    load_scene_by_name(&app_state.world, scene_to_load, &app_state.player_id);
    list_available_cameras(&app_state.world);
    
    // Initialize application state
    app_state.frame_count = 0;
    app_state.simulation_time = 0.0f;
    app_state.initialized = true;
    
    ui_init();
    
    printf("\n🎮 Starting simulation...\n");
    printf("Press ESC to exit, 1-9 to switch cameras\n");
}

static void frame(void) {
    if (!app_state.initialized) return;
    
    const float dt = (float)sapp_frame_duration();
    app_state.simulation_time += dt;
    app_state.frame_count++;
    
    // Simulate player input
    if (app_state.player_id != INVALID_ENTITY) {
        simulate_player_input(&app_state.world, app_state.player_id, app_state.simulation_time);
    }
    
    // Update world and systems
    world_update(&app_state.world, dt);
    scheduler_update(&app_state.scheduler, &app_state.world, dt);
    
    // Render frame
    sg_begin_pass(&(sg_pass){
        .swapchain = sglue_swapchain(),
        .action = app_state.pass_action
    });
    
    // Render entities (basic implementation for now)
    render_frame(&app_state.world, &app_state.render_config, app_state.player_id, dt);
    
    ui_render(&app_state.world, &app_state.scheduler, dt);
    
    sg_end_pass();
    sg_commit();
    
    // Print status occasionally
    if (app_state.frame_count % 300 == 0 && app_state.frame_count > 0) {
        printf("⏱️  Time: %.1fs, Frame: %d, Entities: %d\n", 
               app_state.simulation_time, app_state.frame_count, app_state.world.entity_count);
    }
}

static void cleanup(void) {
    printf("\n🏁 Simulation complete!\n");
    
    ui_shutdown();
    render_cleanup(&app_state.render_config);
    assets_cleanup(&app_state.assets);
    scheduler_destroy(&app_state.scheduler);
    world_destroy(&app_state.world);
    
    sg_shutdown();
    printf("✅ Cleanup complete\n");
}

static void event(const sapp_event* ev) {
    // Handle UI events first - if UI captures the event, don't process it further
    if (ui_handle_event(ev)) {
        return;  // UI captured this event
    }
    
    // Process game events only if UI didn't capture them
    switch (ev->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
                printf("⎋ Escape key pressed - exiting\n");
                sapp_request_quit();
            }
            // Camera switching with number keys
            else if (ev->key_code >= SAPP_KEYCODE_1 && ev->key_code <= SAPP_KEYCODE_9) {
                int camera_index = ev->key_code - SAPP_KEYCODE_1; // 0-8
                
                // Find camera entities
                int found_cameras = 0;
                for (uint32_t i = 0; i < app_state.world.entity_count; i++) {
                    struct Entity* entity = &app_state.world.entities[i];
                    if (entity->component_mask & COMPONENT_CAMERA) {
                        if (found_cameras == camera_index) {
                            world_set_active_camera(&app_state.world, entity->id);
                            printf("📹 Switched to camera %d (Entity %d)\n", camera_index + 1, entity->id);
                            break;
                        }
                        found_cameras++;
                    }
                }
            }
            break;
        default:
            break;
    }
}

// ============================================================================
// SOKOL MAIN
// ============================================================================

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1280,
        .height = 720,
        .sample_count = 4,
        .window_title = "CGame - Entity-Component-System Engine (Sokol)",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}