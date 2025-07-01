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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "assets.h"
#include "core.h"
#include "data.h"
#include "gpu_resources.h"
#include "nuklear.h"
#include "render.h"
#include "systems.h"
#include "system/camera.h"
#include "ui.h"
#include "scene_state.h"
#include "scene_script.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// APPLICATION STATE
// ============================================================================

static struct
{
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
    SceneStateManager scene_state;
    
    // Test modes
    bool capture_golden_reference;
    float golden_reference_timer;
} app_state = { 0 };

// ============================================================================
// SCENE MANAGEMENT
// ============================================================================

static void load_scene_by_name(struct World* world, const char* scene_name, EntityID* player_id)
{
    printf("🏗️  Loading scene '%s' from data...\n", scene_name);

    if (!load_scene(world, get_data_registry(), get_asset_registry(), scene_name))
    {
        printf("❌ Failed to load scene: %s\n", scene_name);
        return;
    }

    // Find the player entity
    *player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_PLAYER)
        {
            *player_id = entity->id;
            break;
        }
    }

    if (*player_id != INVALID_ENTITY)
    {
        printf("🎯 Player found: Entity ID %d\n", *player_id);
    }
    else
    {
        printf("⚠️ No player entity found in scene.\n");
    }

    // Set up scene lighting
    lighting_set_ambient(&app_state.render_config.lighting, (Vector3){ 0.1f, 0.15f, 0.2f }, 0.3f);
    lighting_add_directional_light(&app_state.render_config.lighting,
                                   (Vector3){ 0.3f, -0.7f, 0.2f }, (Vector3){ 0.4f, 0.5f, 0.6f },
                                   0.4f);
    printf("💡 Scene lighting configured\n");

    printf("🌍 Scene loaded with %d entities\n", world->entity_count);
}

static void list_available_cameras(struct World* world)
{
    printf("\n📹 Available cameras:\n");
    int camera_count = 0;
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_CAMERA)
        {
            camera_count++;
            struct Camera* camera = entity->camera;
            const char* behavior_name = "Unknown";
            switch (camera->behavior)
            {
                case 0:
                    behavior_name = "Third Person";
                    break;
                case 1:
                    behavior_name = "First Person";
                    break;
                case 2:
                    behavior_name = "Static";
                    break;
                case 3:
                    behavior_name = "Chase";
                    break;
                case 4:
                    behavior_name = "Orbital";
                    break;
            }
            printf("   %d: Entity %d (%s) pos:(%.1f,%.1f,%.1f) target:(%.1f,%.1f,%.1f) fov:%.1f\n",
                   camera_count, entity->id, behavior_name, camera->position.x, camera->position.y,
                   camera->position.z, camera->target.x, camera->target.y, camera->target.z,
                   camera->fov);
        }
    }

    // Show active camera
    EntityID active_id = world_get_active_camera(world);
    if (active_id != INVALID_ENTITY)
    {
        printf("   🎯 Active camera: Entity %d\n", active_id);
    }
    else
    {
        printf("   ⚠️  No active camera set\n");
    }
    printf("\n");
}

// ============================================================================
// SOKOL CALLBACKS
// ============================================================================

static void init(void)
{
    printf("🎮 CGGame - Sokol-based Component Engine\n");
    printf("==========================================\n\n");

    // Initialize random seed
    srand((unsigned int)time(NULL));

    printf("🔧 Initializing graphics...\n");

    // Initialize Sokol graphics
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    if (!sg_isvalid())
    {
        printf("❌ Failed to initialize Sokol graphics\n");
        sapp_quit();
        return;
    }

    printf("🌍 Setting up world...\n");

    // Initialize world
    if (!world_init(&app_state.world))
    {
        printf("❌ Failed to initialize world\n");
        sapp_quit();
        return;
    }

    printf("⚙️ Starting systems...\n");

    // Initialize system scheduler
    if (!scheduler_init(&app_state.scheduler, &app_state.render_config))
    {
        printf("❌ Failed to initialize scheduler\n");
        sapp_quit();
        return;
    }

    printf("🎨 Configuring renderer...\n");

    // Initialize render config
    app_state.render_config = (RenderConfig){
        .screen_width = (int)sapp_width(),
        .screen_height = (int)sapp_height(),
        .assets = get_asset_registry(),
        .camera = { .position = { 0.0f, 5.0f, 10.0f },  // Default camera position
                    .target = { 0.0f, 0.0f, 0.0f },
                    .up = { 0.0f, 1.0f, 0.0f },
                    .fov = 45.0f * M_PI / 180.0f,
                    .near_plane = 0.1f,
                    .far_plane = 100.0f,
                    .aspect_ratio = (float)sapp_width() / (float)sapp_height() },
        .show_debug_info = true,
        .wireframe_mode = false
    };

    // Initialize renderer
    if (!render_init(&app_state.render_config, get_asset_registry(), (float)sapp_width(),
                     (float)sapp_height()))
    {
        printf("❌ Failed to initialize renderer\n");
        sapp_quit();
        return;
    }

    // Set up pass action
    app_state.pass_action =
        (sg_pass_action){ .colors[0] = { .load_action = SG_LOADACTION_CLEAR,
                                         .clear_value = { 0.0f, 0.05f, 0.1f, 1.0f } } };

    printf("🏗️ Loading scene...\n");

    // Load logo scene directly - no loading screen needed
    const char* scene_to_load = "logo";
    strcpy(app_state.current_scene, scene_to_load);
    printf("ℹ️ Loading scene: %s\n", scene_to_load);

    load_scene_by_name(&app_state.world, scene_to_load, &app_state.player_id);
    
    // Initialize scene state management
    scene_state_init(&app_state.scene_state);
    strcpy(app_state.scene_state.current_scene_name, scene_to_load);
    
    // Start with debug UI and HUD hidden (~ to toggle)
    scene_state_set_debug_ui_visible(&app_state.scene_state, false);
    ui_set_debug_visible(false);  // Synchronize with UI system
    
    ui_init();
    
    // Initialize camera system after scene is loaded
    camera_system_init(&app_state.world, &app_state.render_config);
    
    list_available_cameras(&app_state.world);

    // Execute scene script enter callback for logo scene
    scene_script_execute_enter(scene_to_load, &app_state.world, &app_state.scene_state);

    // Initialize application state
    app_state.frame_count = 0;
    app_state.simulation_time = 0.0f;
    app_state.initialized = true;  // Ready to render immediately

    printf("✅ Engine initialized successfully!\n");
    printf("\n🎮 Starting simulation...\n");
    printf("Press ESC to exit, C to cycle cameras, W for wireframe\n");
}

static void frame(void)
{
    // Begin performance frame timing
    performance_frame_begin();
    
    const float dt = (float)sapp_frame_duration();
    app_state.simulation_time += dt;
    app_state.frame_count++;

    // Skip rendering if not initialized
    if (!app_state.initialized) {
        performance_frame_end();
        return;
    }

    // Update scene state and scripts
    scene_state_update(&app_state.scene_state, dt);
    scene_script_execute_update(app_state.scene_state.current_scene_name, &app_state.world, &app_state.scene_state, dt);
    
    // Handle scene transitions
    if (scene_state_has_pending_transition(&app_state.scene_state))
    {
        const char* next_scene = scene_state_get_next_scene(&app_state.scene_state);
        printf("🎬 Executing scene transition: %s -> %s\n", app_state.scene_state.current_scene_name, next_scene);
        
        // Clear current scene
        world_clear(&app_state.world);
        
        // Load new scene
        load_scene_by_name(&app_state.world, next_scene, &app_state.player_id);
        
        // Update scene state
        strcpy(app_state.scene_state.current_scene_name, next_scene);
        strcpy(app_state.current_scene, next_scene);
        app_state.scene_state.transition_pending = false;
        
        // Execute enter script for new scene
        scene_script_execute_enter(next_scene, &app_state.world, &app_state.scene_state);
        
        printf("🎬 Scene transition completed: now in %s\n", next_scene);
    }

    // Update world and systems
    world_update(&app_state.world, dt);
    scheduler_update(&app_state.scheduler, &app_state.world, &app_state.render_config, dt);

    // Render frame
    sg_begin_pass(&(sg_pass){ .swapchain = sglue_swapchain(), .action = app_state.pass_action });

    // Render entities
    render_frame(&app_state.world, &app_state.render_config, app_state.player_id, dt);

    ui_render(&app_state.world, &app_state.scheduler, dt, app_state.scene_state.current_scene_name);

    sg_end_pass();
    sg_commit();

    // End performance frame timing
    performance_frame_end();

    // Print status occasionally
    if (app_state.frame_count % 300 == 0 && app_state.frame_count > 0)
    {
        printf("⏱️  Time: %.1fs, Frame: %d, Entities: %d\n", app_state.simulation_time,
               app_state.frame_count, app_state.world.entity_count);
        
        // Log performance summary periodically
        performance_log_summary();
    }
}

static void cleanup(void)
{
    printf("\n🏁 Simulation complete!\n");

    ui_shutdown();
    render_cleanup(&app_state.render_config);
    // Assets are cleaned up by the scheduler
    scheduler_destroy(&app_state.scheduler, &app_state.render_config);
    world_destroy(&app_state.world);

    sg_shutdown();
    printf("✅ Cleanup complete\n");
}

static void event(const sapp_event* ev)
{
    // Handle UI events first - if UI captures the event, don't process it further
    if (ui_handle_event(ev))
    {
        return;  // UI captured this event
    }

    // Handle scene-specific input events
    if (scene_script_execute_input(app_state.scene_state.current_scene_name, &app_state.world, &app_state.scene_state, ev))
    {
        return;  // Scene script handled this event
    }

    // Process global game events only if UI and scene scripts didn't capture them
    switch (ev->type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (ev->key_code == SAPP_KEYCODE_ESCAPE)
            {
                printf("⎋ Escape key pressed - exiting\n");
                sapp_request_quit();
            }
            // Toggle debug UI with tilde (~) key
            else if (ev->key_code == SAPP_KEYCODE_GRAVE_ACCENT)
            {
                bool current_visible = scene_state_is_debug_ui_visible(&app_state.scene_state);
                scene_state_set_debug_ui_visible(&app_state.scene_state, !current_visible);
                ui_set_debug_visible(!current_visible);  // Synchronize with UI system
                ui_toggle_hud();  // Also toggle the HUD
                printf("🔧 Debug UI & HUD: %s\n", !current_visible ? "ON" : "OFF");
            }
            // Camera switching with number keys (legacy support)
            else if (ev->key_code >= SAPP_KEYCODE_1 && ev->key_code <= SAPP_KEYCODE_9)
            {
                int camera_index = ev->key_code - SAPP_KEYCODE_1;  // 0-8

                // Use the new switch_to_camera function
                if (switch_to_camera(&app_state.world, camera_index))
                {
                    printf("📹 Switched to camera %d\n", camera_index + 1);

                    // Update aspect ratio for the new camera
                    float aspect_ratio = (float)app_state.render_config.screen_width /
                                         (float)app_state.render_config.screen_height;
                    update_camera_aspect_ratio(&app_state.world, aspect_ratio);
                }
                else
                {
                    printf("📹 Camera %d not found\n", camera_index + 1);
                }
            }
            // Camera cycling with C key
            else if (ev->key_code == SAPP_KEYCODE_C)
            {
                if (cycle_to_next_camera(&app_state.world))
                {
                    // Get info about the new active camera
                    EntityID active_camera = world_get_active_camera(&app_state.world);
                    struct Camera* camera = entity_get_camera(&app_state.world, active_camera);
                    
                    const char* camera_type = "Unknown";
                    if (camera)
                    {
                        switch (camera->behavior)
                        {
                            case CAMERA_BEHAVIOR_FIRST_PERSON: camera_type = "Cockpit"; break;
                            case CAMERA_BEHAVIOR_THIRD_PERSON: camera_type = "Chase"; break;
                            case CAMERA_BEHAVIOR_STATIC: camera_type = "Static/Overhead"; break;
                            case CAMERA_BEHAVIOR_ORBITAL: camera_type = "Orbital"; break;
                            default: camera_type = "Unknown"; break;
                        }
                    }
                    
                    printf("📹 Cycled to %s camera (Entity %d)\n", camera_type, active_camera);

                    // Update aspect ratio for the new camera
                    float aspect_ratio = (float)app_state.render_config.screen_width /
                                         (float)app_state.render_config.screen_height;
                    update_camera_aspect_ratio(&app_state.world, aspect_ratio);
                }
                else
                {
                    printf("📹 No cameras available to cycle through\n");
                }
            }
            // Toggle wireframe mode with W key
            else if (ev->key_code == SAPP_KEYCODE_W)
            {
                app_state.render_config.wireframe_mode = !app_state.render_config.wireframe_mode;
                printf("🔧 Wireframe mode: %s\n",
                       app_state.render_config.wireframe_mode ? "ON" : "OFF");
            }
            // Take screenshot with S key
            else if (ev->key_code == SAPP_KEYCODE_S)
            {
                char filename[64];
                snprintf(filename, sizeof(filename), "screenshots/cube_test_%ld.bmp", time(NULL));
                if (render_take_screenshot(&app_state.render_config, filename))
                {
                    printf("📸 Screenshot saved: %s\n", filename);
                }
                else
                {
                    printf("📸 Screenshot attempted: %s (function not implemented)\n", filename);
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

sapp_desc sokol_main(int argc, char* argv[])
{
    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--golden-reference") == 0 || strcmp(argv[i], "-g") == 0)
        {
            app_state.capture_golden_reference = true;
            printf("🏆 Golden reference capture mode enabled\n");
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("CGame Engine Usage:\n");
            printf("  --golden-reference, -g    Capture golden reference screenshot of loading cube\n");
            printf("  --help, -h                Show this help message\n");
            printf("\nGame Controls:\n");
            printf("  ESC        Exit game\n");
            printf("  ENTER      Skip logo screen (on logo scene)\n");
            printf("  ~          Toggle debug UI & HUD\n");
            printf("  1-9        Switch cameras\n");
            printf("  C          Cycle cameras\n");
            printf("  W          Toggle wireframe mode\n");
            printf("  S          Take screenshot\n");
        }
    }

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