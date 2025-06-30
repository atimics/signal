#include "graphics_api.h"

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

#include "core.h"
#include "systems.h"
#include "render.h"
#include "gpu_resources.h"
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

// Loading screen state
typedef struct {
    bool active;
    float rotation;
    EntityID cube_entity;
    float progress;
    char status_text[256];
} LoadingScreen;

static struct {
    // Core ECS
    struct World world;
    struct SystemScheduler scheduler;
    
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
    
    // Loading screen
    LoadingScreen loading_screen;
} app_state = {0};

// ============================================================================
// LOADING SCREEN MANAGEMENT
// ============================================================================

static void loading_screen_init(LoadingScreen* loading) {
    loading->active = true;
    loading->rotation = 0.0f;
    loading->progress = 0.0f;
    loading->cube_entity = INVALID_ENTITY;
    strcpy(loading->status_text, "Initializing...");
}

static void loading_screen_create_cube(LoadingScreen* loading, struct World* world, AssetRegistry* assets) {
    // Create a simple rotating cube entity for the loading screen
    loading->cube_entity = world_create_entity(world);
    
    if (loading->cube_entity == INVALID_ENTITY) {
        printf("❌ Failed to create loading screen cube entity\n");
        return;
    }
    
    // Add transform component
    struct Entity* entity = world_get_entity(world, loading->cube_entity);
    entity->component_mask |= COMPONENT_TRANSFORM;
    entity->transform = calloc(1, sizeof(struct Transform));
    if (entity->transform) {
        entity->transform->position = (Vector3){0.0f, 0.0f, 0.0f};
        entity->transform->rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
        entity->transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
        entity->transform->dirty = true;
    }
    
    // Add mesh component - we'll create a simple cube mesh
    entity->component_mask |= COMPONENT_MESH;
    entity->mesh_name = calloc(1, 64);
    if (entity->mesh_name) {
        strcpy(entity->mesh_name, "loading_cube");
    }
    
    printf("📦 Created loading screen cube entity: %d\n", loading->cube_entity);
}

static void loading_screen_update(LoadingScreen* loading, float dt) {
    if (!loading->active) return;
    
    // Rotate the cube
    loading->rotation += dt * 2.0f; // 2 radians per second
    if (loading->rotation > 2.0f * M_PI) {
        loading->rotation -= 2.0f * M_PI;
    }
    
    // Update cube rotation if entity exists
    if (loading->cube_entity != INVALID_ENTITY) {
        struct Entity* entity = world_get_entity(&app_state.world, loading->cube_entity);
        if (entity && entity->transform) {
            // Rotate around Y axis
            float half_angle = loading->rotation * 0.5f;
            entity->transform->rotation = (Quaternion){
                0.0f, 
                sinf(half_angle), 
                0.0f, 
                cosf(half_angle)
            };
            entity->transform->dirty = true;
        }
    }
}

static void loading_screen_set_progress(LoadingScreen* loading, float progress, const char* status) {
    loading->progress = progress;
    if (status) {
        strncpy(loading->status_text, status, sizeof(loading->status_text) - 1);
        loading->status_text[sizeof(loading->status_text) - 1] = '\0';
    }
    printf("📋 Loading: %.1f%% - %s\n", progress * 100.0f, loading->status_text);
}

static void loading_screen_finish(LoadingScreen* loading) {
    loading->active = false;
    printf("✅ Loading screen completed\n");
}

// ============================================================================
// SCENE MANAGEMENT
// ============================================================================

static void load_scene_by_name(struct World* world, const char* scene_name, EntityID* player_id) {
    printf("🏗️  Loading scene '%s' from data...\n", scene_name);
    
    DataRegistry* data_registry = get_data_registry();
    
    // Load the scene from template
    if (!load_scene(world, data_registry, get_asset_registry(), scene_name)) {
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
    lighting_set_ambient(&app_state.render_config.lighting, (Vector3){0.1f, 0.15f, 0.2f}, 0.3f);
    lighting_add_directional_light(&app_state.render_config.lighting, 
                                  (Vector3){0.3f, -0.7f, 0.2f}, 
                                  (Vector3){0.4f, 0.5f, 0.6f}, 
                                  0.4f);
    printf("💡 Scene lighting configured\n");
    
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
            printf("   %d: Entity %d (%s) pos:(%.1f,%.1f,%.1f) target:(%.1f,%.1f,%.1f) fov:%.1f\n", 
                   camera_count, entity->id, behavior_name,
                   camera->position.x, camera->position.y, camera->position.z,
                   camera->target.x, camera->target.y, camera->target.z,
                   camera->fov);
        }
    }
    
    // Show active camera
    EntityID active_id = world_get_active_camera(world);
    if (active_id != INVALID_ENTITY) {
        printf("   🎯 Active camera: Entity %d\n", active_id);
    } else {
        printf("   ⚠️  No active camera set\n");
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
    if (!scheduler_init(&app_state.scheduler, &app_state.render_config)) {
        printf("❌ Failed to initialize scheduler\n");
        sapp_quit();
        return;
    }
    
    // Assets are initialized by the scheduler - use global registry
    
    // Initialize render config
    app_state.render_config = (RenderConfig){
        .screen_width = (int)sapp_width(),
        .screen_height = (int)sapp_height(),
        .assets = get_asset_registry(),
        .camera = {
            .position = {5.0f, 3.0f, 10.0f},
            .target = {0.0f, 0.0f, 0.0f},
            .up = {0.0f, 1.0f, 0.0f},
            .fov = 45.0f * M_PI / 180.0f,
            .near_plane = 0.1f,
            .far_plane = 100.0f,
            .aspect_ratio = (float)sapp_width() / (float)sapp_height()
        },
        .show_debug_info = true,
        .wireframe_mode = false  // Start with solid rendering
    };
    
    // Initialize renderer
    if (!render_init(&app_state.render_config, get_asset_registry(), 
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
    const char* scene_to_load = "camera_test";
    strcpy(app_state.current_scene, scene_to_load);
    printf("ℹ️ Loading default scene: %s\n", scene_to_load);
    
    load_scene_by_name(&app_state.world, scene_to_load, &app_state.player_id);
    list_available_cameras(&app_state.world);
    
    // Load logo texture for demonstration
    printf("🖼️  Loading logo texture...\n");
    AssetRegistry* asset_registry = get_asset_registry();
    if (load_texture(asset_registry, "logo.png", "game_logo")) {
        printf("✅ Logo texture loaded successfully!\n");
    } else {
        printf("⚠️  Logo texture loading failed (trying from textures/ directory)\n");
        // Try from textures directory as fallback
        if (load_texture(asset_registry, "textures/logo.png", "game_logo")) {
            printf("✅ Logo texture loaded from textures/ directory!\n");
        }
    }
    
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

    // Update world and systems
    world_update(&app_state.world, dt);
    scheduler_update(&app_state.scheduler, &app_state.world, &app_state.render_config, dt);
    
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
    // Assets are cleaned up by the scheduler
    scheduler_destroy(&app_state.scheduler, &app_state.render_config);
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
                
                // Use the new switch_to_camera function
                if (switch_to_camera(&app_state.world, camera_index)) {
                    printf("📹 Switched to camera %d\n", camera_index + 1);
                    
                    // Update aspect ratio for the new camera
                    float aspect_ratio = (float)app_state.render_config.screen_width / 
                                       (float)app_state.render_config.screen_height;
                    update_camera_aspect_ratio(&app_state.world, aspect_ratio);
                } else {
                    printf("📹 Camera %d not found\n", camera_index + 1);
                }
            }
            // Toggle wireframe mode with W key
            else if (ev->key_code == SAPP_KEYCODE_W) {
                app_state.render_config.wireframe_mode = !app_state.render_config.wireframe_mode;
                printf("🔧 Wireframe mode: %s\n", app_state.render_config.wireframe_mode ? "ON" : "OFF");
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