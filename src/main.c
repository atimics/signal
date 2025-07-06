#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "assets.h"
#include "config.h"
#include "core.h"
#include "data.h"
#include "gpu_resources.h"
#include "render.h"
#include "systems.h"
#include "system/camera.h"
#include "system/performance.h"
#include "system/input.h"  // For canyon racing input handling
#include "ui.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "scene_state.h"
#include "scene_script.h"
#include "render_layers.h"  // Offscreen rendering system
#include "graphics_health.h"  // Graphics health monitoring
#include "render_pass_guard.h"  // Encoder state management

// UI system includes

// Sokol includes
#include "graphics_api.h"  // This includes sokol headers safely
#include "sokol_glue.h"
#include "sokol_log.h"

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
    LayerManager* layer_manager;  // Offscreen rendering layers

    // Scene management
    char current_scene[64];
    SceneStateManager scene_state;
    char requested_scene[64];  // CLI-requested scene override
    
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
    
    // DEBUG: Log all entities and their components for debugging
    printf("🔍 Entity debug after scene load:\n");
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        printf("  Entity %d: mask=0x%X (T:%d R:%d C:%d P:%d)\n", 
               entity->id, entity->component_mask,
               !!(entity->component_mask & COMPONENT_TRANSFORM),
               !!(entity->component_mask & COMPONENT_RENDERABLE),
               !!(entity->component_mask & COMPONENT_CAMERA),
               !!(entity->component_mask & COMPONENT_PHYSICS));
    }
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

    // Initialize configuration system
    printf("⚙️  Initializing configuration...\n");
    if (!config_init()) {
        printf("❌ Failed to initialize configuration system\n");
        sapp_quit();
        return;
    }

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
    
    // Initialize graphics health monitoring
    gfx_health_init();
    
    // Initialize UI renderer early (before any scenes load)
    ui_microui_init_renderer();

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

    // Determine initial scene: CLI override > config startup scene > default to logo
    const char* scene_to_load;
    if (strlen(app_state.requested_scene) > 0) {
        // CLI override takes precedence
        scene_to_load = app_state.requested_scene;
        printf("🎯 Using CLI-specified scene: %s\n", scene_to_load);
    } else if (config_get_auto_start()) {
        // Use configured startup scene if auto-start is enabled
        scene_to_load = config_get_startup_scene();
        printf("⚙️  Using configured startup scene: %s (auto-start enabled)\n", scene_to_load);
    } else {
        // Default to logo scene
        scene_to_load = "logo";
        printf("🎮 Using default scene: %s\n", scene_to_load);
    }
    
    strcpy(app_state.current_scene, scene_to_load);
    printf("ℹ️ Loading scene: %s\n", scene_to_load);

    load_scene_by_name(&app_state.world, scene_to_load, &app_state.player_id);
    
    // Initialize scene state management
    scene_state_init(&app_state.scene_state);
    strcpy(app_state.scene_state.current_scene_name, scene_to_load);
    
    // Initialize offscreen rendering layers
    app_state.layer_manager = layer_manager_create(
        app_state.render_config.screen_width, 
        app_state.render_config.screen_height
    );
    
    // Create render layers
    RenderLayerConfig scene_layer_config = {
        .name = "3d_scene",
        .width = app_state.render_config.screen_width,
        .height = app_state.render_config.screen_height,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    RenderLayer* scene_layer = layer_manager_add_layer(app_state.layer_manager, &scene_layer_config);
    layer_set_order(scene_layer, 0);  // Render first
    
    // CRITICAL FIX: Set opaque clear color for 3D scene layer
    // Transparent clear color (0,0,0,0) makes rendered content invisible in compositor
    scene_layer->clear_color = (sg_color){ 0.0f, 0.05f, 0.1f, 1.0f }; // Match main clear color
    
    RenderLayerConfig ui_layer_config = {
        .name = "ui",
        .width = app_state.render_config.screen_width,
        .height = app_state.render_config.screen_height,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    RenderLayer* ui_layer = layer_manager_add_layer(app_state.layer_manager, &ui_layer_config);
    layer_set_order(ui_layer, 1);  // Render on top
    layer_set_blend_mode(ui_layer, BLEND_MODE_NORMAL);
    
    // Initialize UI system first before any UI calls
    ui_init();
    
    // Set UI visibility based on initial scene
    if (strcmp(scene_to_load, "logo") == 0) {
        // Logo scene should start with UI hidden
        scene_state_set_ui_visible(&app_state.scene_state, false);
        scene_state_set_debug_ui_visible(&app_state.scene_state, false);
        ui_set_visible(false);  // Synchronize with UI system
        ui_set_debug_visible(false);  // Synchronize with UI system
    } else {
        // Other scenes start with UI visible but debug hidden
        scene_state_set_ui_visible(&app_state.scene_state, true);
        scene_state_set_debug_ui_visible(&app_state.scene_state, false);
        ui_set_visible(true);  // Synchronize with UI system
        ui_set_debug_visible(false);  // Synchronize with UI system
    }
    
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

static bool app_shutting_down = false;
static bool frame_rendering_active = false;

// Function to check if frame rendering is active (for UI safety)
bool is_frame_rendering_active(void) {
    return frame_rendering_active;
}

static void frame(void)
{
    // Begin performance frame timing
    performance_frame_begin();
    
    const float dt = (float)sapp_frame_duration();
    app_state.simulation_time += dt;
    app_state.frame_count++;

    // Check if app is shutting down or invalid
    if (!sapp_isvalid()) {
        app_shutting_down = true;
    }

    // Skip rendering if not initialized or app is shutting down
    if (!app_state.initialized || app_shutting_down) {
        performance_frame_end();
        return;
    }

    // Update scene state and scripts
    scene_state_update(&app_state.scene_state, dt);
    scene_script_execute_update(app_state.scene_state.current_scene_name, &app_state.world, &app_state.scene_state, dt);
    
    // Handle UI scene change requests (only when not actively rendering)
    if (ui_has_scene_change_request() && !frame_rendering_active)
    {
        const char* requested_scene = ui_get_requested_scene();
        printf("🎬 UI scene change request: %s -> %s\n", app_state.scene_state.current_scene_name, requested_scene);
        scene_state_request_transition(&app_state.scene_state, requested_scene);
        ui_clear_scene_change_request();
    }

    // Handle scene transitions (only when not actively rendering)
    if (scene_state_has_pending_transition(&app_state.scene_state) && !frame_rendering_active)
    {
        const char* next_scene = scene_state_get_next_scene(&app_state.scene_state);
        printf("🎬 Executing scene transition: %s -> %s\n", app_state.scene_state.current_scene_name, next_scene);
        
        // Reset camera system before clearing world
        camera_system_reset();
        
        // Clear current scene
        world_clear(&app_state.world);
        
        // Load new scene
        load_scene_by_name(&app_state.world, next_scene, &app_state.player_id);
        
        // Reinitialize camera system with new scene entities
        camera_system_init(&app_state.world, &app_state.render_config);
        
        // Update scene state
        strcpy(app_state.scene_state.current_scene_name, next_scene);
        strcpy(app_state.current_scene, next_scene);
        app_state.scene_state.transition_pending = false;
        
        // Execute enter script for new scene (this will set appropriate UI visibility)
        scene_script_execute_enter(next_scene, &app_state.world, &app_state.scene_state);
        
        printf("🎬 Scene transition completed: now in %s\n", next_scene);
    }

    // Update world and systems
    world_update(&app_state.world, dt);
    scheduler_update(&app_state.scheduler, &app_state.world, &app_state.render_config, dt);
    

    // === OFFSCREEN RENDERING SOLUTION ===
    // Render to separate offscreen targets to avoid pipeline conflicts
    
    if (!sg_isvalid()) {
        printf("⚠️ Skipping rendering - Graphics context invalid\n");
        performance_frame_end();
        return;
    }
    
    // Get screen dimensions
    const int screen_width = sapp_width();
    const int screen_height = sapp_height();
    
    // Update layer manager resolution if needed
    if (screen_width != app_state.layer_manager->screen_width || 
        screen_height != app_state.layer_manager->screen_height) {
        layer_manager_resize(app_state.layer_manager, screen_width, screen_height);
    }
    
    // === START ACTIVE RENDERING PHASE ===
    // Set flag to prevent scene transitions during rendering
    frame_rendering_active = true;
    
    // === RENDER TO 3D SCENE LAYER ===
    RenderLayer* scene_layer = layer_manager_get_layer(app_state.layer_manager, "3d_scene");
    bool should_update = layer_should_update(app_state.layer_manager, scene_layer);
    
    // DEBUG: Log 3D scene layer state periodically
    static int scene_debug_counter = 0;
    if (scene_debug_counter++ % 180 == 0) { // Log every 3 seconds at 60fps
        printf("🎨 3D SCENE LAYER DEBUG: exists=%d, should_update=%d, enabled=%d, entities=%d\n", 
               scene_layer ? 1 : 0, should_update, 
               scene_layer ? scene_layer->enabled : -1, app_state.world.entity_count);
    }
    
    if (scene_layer && should_update) {
        // Ensure context is valid before rendering
        if (sg_isvalid()) {
            render_set_offscreen_mode(true);  // Switch to offscreen pipeline
            layer_begin_render(scene_layer);
            
            // Render 3D entities to offscreen target
            render_frame(&app_state.world, &app_state.render_config, app_state.player_id, dt);
            
            layer_end_render();  // CRITICAL: Always end the render pass
            render_set_offscreen_mode(false);  // Switch back to default pipeline
        } else {
            printf("⚠️ Skipping 3D scene render - context already invalid\n");
        }
    }
    
    // === RENDER TO UI LAYER ===
    RenderLayer* ui_layer = layer_manager_get_layer(app_state.layer_manager, "ui");
    bool ui_visible = ui_is_visible();
    
    // Update UI layer enabled state BEFORE render attempt
    if (ui_layer) {
        layer_set_enabled(ui_layer, ui_visible);
    }
    
    // DEBUG: Log UI visibility state periodically
    static int ui_debug_counter = 0;
    if (ui_debug_counter++ % 180 == 0) { // Log every 3 seconds at 60fps
        printf("🎨 UI LAYER DEBUG: ui_visible=%d, scene='%s', layer_enabled=%d\n", 
               ui_visible, app_state.scene_state.current_scene_name, 
               ui_layer ? ui_layer->enabled : -1);
    }
    
    // CRITICAL FIX: Only render UI layer if UI is actually visible
    // This prevents empty UI context from being composited and causing magenta artifacts
    if (ui_layer && ui_visible && layer_should_update(app_state.layer_manager, ui_layer)) {
        // Track if we actually rendered UI
        bool ui_rendered = false;
        
        // 1. Build MicroUI command list (generates vertices)
        ui_render(&app_state.world, &app_state.scheduler, dt, 
                  app_state.scene_state.current_scene_name, screen_width, screen_height);
        
        // 2. Upload vertices while NO encoder is open
        if (ui_microui_ready()) {
            printf("🎨 Context valid before UI vertex upload\n");
            
            // CRITICAL: Ensure no encoder is active before vertex upload
            #ifdef DEBUG
            if (layer_is_encoder_active()) {
                printf("❌ CRITICAL: Encoder still active before UI vertex upload! This indicates a missing layer_end_render()!\n");
                // Force-end any active encoder to prevent crash
                layer_end_render();
            }
            #endif
            
            ui_microui_upload_vertices();
            ui_rendered = true;
        } else {
            printf("⚠️ Skipping UI upload - renderer not ready\n");
        }
        
        // 3. Only render if we successfully uploaded vertices
        if (ui_rendered) {
            render_set_offscreen_mode(true);          // Set mode
            layer_begin_render(ui_layer);             // Begin encoder
            ui_microui_render(screen_width, screen_height);
            layer_end_render();                       // ALWAYS end encoder
            render_set_offscreen_mode(false);         // Reset mode
        }
    }
    
    // UI layer enable/disable is now handled before render attempt
    
    // === COMPOSITE LAYERS TO SWAPCHAIN ===
    // Validate context before final composite pass
    if (!sg_isvalid()) {
        printf("⚠️ Graphics context invalid before composite pass - skipping frame\n");
        frame_rendering_active = false;  // Clear flag before early return
        performance_frame_end();
        return;
    }
    
    // Ensure app is still valid
    if (!sapp_isvalid()) {
        printf("⚠️ Sokol app context invalid - skipping frame\n");
        frame_rendering_active = false;  // Clear flag before early return
        performance_frame_end();
        return;
    }
    
    // DEBUG: Log composite pass periodically
    static int composite_debug_counter = 0;
    if (composite_debug_counter++ % 180 == 0) { // Log every 3 seconds
        printf("🎨 COMPOSITE DEBUG: Beginning swapchain pass...\n");
    }
    
    // Create pass descriptor first to avoid macro issues
    sg_pass swapchain_pass = { 
        .swapchain = sglue_swapchain(), 
        .action = app_state.pass_action 
    };
    
    // Use PASS_BEGIN macro for safe pass management
    PASS_BEGIN("swapchain_composite", &swapchain_pass);
    
    // Composite all layers
    layer_manager_composite(app_state.layer_manager);
    
    if ((composite_debug_counter-1) % 180 == 0) { // Log every 3 seconds
        printf("🎨 COMPOSITE DEBUG: Ending swapchain pass...\n");
    }
    
    // Use PASS_END macro to ensure proper cleanup
    PASS_END();
    
    // Commit frame
    sg_commit();
    
    // === END ACTIVE RENDERING PHASE ===
    // Clear flag to allow scene transitions for next frame
    frame_rendering_active = false;
    
    // Process deferred UI operations (safe to create/destroy resources now)
    ui_microui_end_of_frame();

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
    
    // Destroy layer manager and all offscreen targets
    if (app_state.layer_manager) {
        layer_manager_destroy(app_state.layer_manager);
        app_state.layer_manager = NULL;
    }
    
    render_cleanup(&app_state.render_config);
    // Assets are cleaned up by the scheduler
    scheduler_destroy(&app_state.scheduler, &app_state.render_config);
    world_destroy(&app_state.world);
    
    // Shutdown configuration system
    config_shutdown();

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

    // Handle scene-specific input events first
    if (scene_script_execute_input(app_state.scene_state.current_scene_name, &app_state.world, &app_state.scene_state, ev))
    {
        return;  // Scene script handled this event
    }

    // Process global game events only if UI and scene scripts didn't capture them
    switch (ev->type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
            
            // Handle other keys
            if (ev->key_code == SAPP_KEYCODE_ESCAPE)
            {
                // Only exit the application if we're in the navigation menu and ESC wasn't handled
                if (strcmp(app_state.scene_state.current_scene_name, "navigation_menu") == 0)
                {
                    printf("⎋ Escape key pressed in navigation menu - exiting\n");
                    sapp_request_quit();
                }
                // For other scenes, ESC is handled by scene scripts (return to navigation menu)
            }
            // TAB key opens navigation menu from any scene
            else if (ev->key_code == SAPP_KEYCODE_TAB)
            {
                if (strcmp(app_state.scene_state.current_scene_name, "navigation_menu") != 0)
                {
                    printf("🧭 TAB pressed - opening navigation menu\n");
                    scene_state_request_transition(&app_state.scene_state, "navigation_menu");
                }
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
            // For number keys 1-9, let's not forward to input system
            // since these might be used for camera switching or other scene-specific functions
            else if (ev->key_code >= SAPP_KEYCODE_1 && ev->key_code <= SAPP_KEYCODE_9)
            {
                // Do nothing - let scene scripts handle number keys
            }
            // Forward other keyboard events to canyon racing input system
            else
            {
                input_handle_keyboard(ev->key_code, true);
            }
            break;
            
        case SAPP_EVENTTYPE_KEY_UP:
            // Try scene script first for key up events
            if (scene_script_execute_input(app_state.scene_state.current_scene_name, &app_state.world, &app_state.scene_state, ev))
            {
                return;  // Scene script handled this event
            }
            // Forward key up events to input system (but not number keys)
            if (ev->key_code < SAPP_KEYCODE_1 || ev->key_code > SAPP_KEYCODE_9)
            {
                input_handle_keyboard(ev->key_code, false);
            }
            break;
            
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            // Forward mouse motion to canyon racing input
            input_handle_mouse_motion(ev->mouse_dx, ev->mouse_dy);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            // Right mouse button (1) for camera control
            if (ev->mouse_button == SAPP_MOUSEBUTTON_RIGHT)
            {
                input_handle_mouse_button(1, true);
                sapp_lock_mouse(true);  // Capture mouse for FPS-style control
            }
            break;
            
        case SAPP_EVENTTYPE_MOUSE_UP:
            if (ev->mouse_button == SAPP_MOUSEBUTTON_RIGHT)
            {
                input_handle_mouse_button(1, false);
                sapp_lock_mouse(false);  // Release mouse
            }
            break;
            
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            // Forward scroll events for camera zoom
            input_handle_mouse_wheel(ev->scroll_y);
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
    // Initialize CLI scene override
    app_state.requested_scene[0] = '\0';
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--golden-reference") == 0 || strcmp(argv[i], "-g") == 0)
        {
            app_state.capture_golden_reference = true;
            printf("🏆 Golden reference capture mode enabled\n");
        }
        else if (strcmp(argv[i], "--scene") == 0 || strcmp(argv[i], "-s") == 0)
        {
            if (i + 1 < argc)
            {
                strncpy(app_state.requested_scene, argv[i + 1], sizeof(app_state.requested_scene) - 1);
                app_state.requested_scene[sizeof(app_state.requested_scene) - 1] = '\0';
                printf("🎬 CLI scene override: %s\n", app_state.requested_scene);
                i++; // Skip the scene name argument
            }
            else
            {
                printf("❌ Error: --scene requires a scene name\n");
            }
        }
        else if (strcmp(argv[i], "--list-scenes") == 0 || strcmp(argv[i], "-l") == 0)
        {
            printf("🎬 Available scenes:\n");
            printf("  logo              - Engine logo and system test\n");
            printf("  navigation_menu   - FTL navigation interface\n");
            printf("  system_overview   - System map for FTL navigation\n");
            printf("  derelict_alpha    - Magnetic navigation through Aethelian Command Ship\n");
            printf("  derelict_beta     - Smaller derelict exploration\n");
            printf("  slipstream_nav    - FTL slipstream navigation test\n");
            exit(0);
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printf("CGame Engine Usage:\n");
            printf("  --scene NAME, -s NAME     Launch directly into specified scene\n");
            printf("  --list-scenes, -l         List all available scenes\n");
            printf("  --golden-reference, -g    Capture golden reference screenshot of loading cube\n");
            printf("  --help, -h                Show this help message\n");
            printf("\nExamples:\n");
            printf("  ./cgame --scene derelict_alpha    # Launch into magnetic navigation scene\n");
            printf("  ./cgame -s navigation_menu        # Launch into FTL navigation\n");
            printf("  ./cgame --list-scenes             # Show all available scenes\n");
            printf("\nGame Controls:\n");
            printf("  ESC        Exit game / Return to navigation menu\n");
            printf("  TAB        Switch between related scenes\n");
            printf("  SPACE      Toggle magnetic navigation (in derelict scenes)\n");
            printf("  ENTER      Skip logo screen (on logo scene)\n");
            printf("  ~          Toggle debug UI & HUD\n");
            printf("  1-9        Switch cameras\n");
            printf("  C          Cycle cameras\n");
            printf("  W          Toggle wireframe mode\n");
            printf("  S          Take screenshot\n");
            exit(0);
        }
    }

    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1280,
        .height = 720,
        .sample_count = 1,  // Must match render layer sample counts
        .window_title = "CGame - Entity-Component-System Engine (Sokol)",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}