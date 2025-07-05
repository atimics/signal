// Include standard headers first
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>  // For directory scanning
#include <stdlib.h>  // For malloc/free

#include "core.h"
#include "config.h"
#include "ui.h"

// Include graphics API (contains Nuklear setup)
#include "graphics_api.h"

static UIState ui_state = { 0 };
static bool ui_visible = true;
static bool debug_ui_visible = false;  // Start hidden

// Dynamic scene discovery functions
static void load_available_scenes(void);
static void free_scene_list(void);
static const char* get_scene_description(const char* scene_name);

void ui_init(void)
{
    snk_setup(&(snk_desc_t){ 0 });

    // Initialize UI state
    ui_state.show_debug_panel = false;  // Start hidden
    ui_state.show_hud = false;          // Start hidden
    ui_state.show_wireframe = false;
    ui_state.show_scene_selector = false;
    ui_state.camera_speed = 10.0f;
    ui_state.time_scale = 1.0f;
    ui_state.fps = 0.0f;
    ui_state.frame_count = 0;
    ui_state.fps_timer = 0.0f;
    strcpy(ui_state.selected_scene, "logo");
    ui_state.scene_change_requested = false;
    
    // Initialize scene browser state
    ui_state.selected_scene_index = -1;  // No scene selected initially
    ui_state.show_scene_details = false;
    
    // Initialize dynamic scene list
    ui_state.scene_names = NULL;
    ui_state.scene_descriptions = NULL;
    ui_state.scene_count = 0;
    ui_state.scenes_loaded = false;

    printf("✅ Nuklear UI initialized\n");
}

void ui_shutdown(void)
{
    free_scene_list();
    snk_shutdown();
    printf("✅ Nuklear UI shutdown\n");
}

static void draw_hud(struct nk_context* ctx, struct World* world)
{
    if (!ui_state.show_hud) return;

    // Create a HUD window in the top-left corner
    if (nk_begin(ctx, "HUD", nk_rect(10, 10, 300, 200),
                 NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND | NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 20, 1);

        // Display FPS
        nk_labelf(ctx, NK_TEXT_LEFT, "FPS: %.1f", ui_state.fps);

        // Display entity count
        nk_labelf(ctx, NK_TEXT_LEFT, "Entities: %d", world->entity_count);

        // Find and display camera information
        EntityID camera_id = INVALID_ENTITY;
        for (uint32_t i = 0; i < world->entity_count; i++)
        {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA)
            {
                camera_id = entity->id;
                break;
            }
        }

        if (camera_id != INVALID_ENTITY)
        {
            struct Transform* cam_transform = entity_get_transform(world, camera_id);
            struct Camera* camera = entity_get_camera(world, camera_id);

            if (cam_transform)
            {
                nk_labelf(ctx, NK_TEXT_LEFT, "Camera Pos: (%.1f, %.1f, %.1f)",
                          cam_transform->position.x, cam_transform->position.y,
                          cam_transform->position.z);
            }

            if (camera)
            {
                nk_labelf(ctx, NK_TEXT_LEFT, "FOV: %.1f°", camera->fov);
                nk_labelf(ctx, NK_TEXT_LEFT, "Type: Camera");
            }
        }

        // Find and display player information
        for (uint32_t i = 0; i < world->entity_count; i++)
        {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_PLAYER)
            {
                struct Transform* transform = entity_get_transform(world, entity->id);
                struct Physics* physics = entity_get_physics(world, entity->id);

                if (transform)
                {
                    nk_labelf(ctx, NK_TEXT_LEFT, "Player: (%.1f, %.1f, %.1f)",
                              transform->position.x, transform->position.y, transform->position.z);
                }

                if (physics)
                {
                    float velocity = sqrtf(physics->velocity.x * physics->velocity.x +
                                           physics->velocity.y * physics->velocity.y +
                                           physics->velocity.z * physics->velocity.z);
                    nk_labelf(ctx, NK_TEXT_LEFT, "Velocity: %.1f", velocity);
                }
                break;
            }
        }
    }
    nk_end(ctx);
}

static void draw_debug_panel(struct nk_context* ctx, struct World* world,
                             SystemScheduler* scheduler)
{
    if (!ui_state.show_debug_panel) return;

    if (nk_begin(ctx, "Debug Panel", nk_rect(50, 250, 400, 500),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE |
                     NK_WINDOW_TITLE))
    {
        // System Performance Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Performance", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 20, 2);
            nk_label(ctx, "FPS:", NK_TEXT_LEFT);
            nk_labelf(ctx, NK_TEXT_LEFT, "%.1f", ui_state.fps);

            nk_label(ctx, "Frame Count:", NK_TEXT_LEFT);
            nk_labelf(ctx, NK_TEXT_LEFT, "%d", scheduler->frame_count);

            nk_label(ctx, "Total Time:", NK_TEXT_LEFT);
            nk_labelf(ctx, NK_TEXT_LEFT, "%.1fs", scheduler->total_time);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "System Times:", NK_TEXT_LEFT);

            for (int i = 0; i < SYSTEM_COUNT; i++)
            {
                if (scheduler->system_calls[i] > 0)
                {
                    float avg_time = scheduler->system_times[i] / scheduler->system_calls[i];
                    nk_layout_row_dynamic(ctx, 15, 1);
                    nk_labelf(ctx, NK_TEXT_LEFT, "  %s: %.3fms", scheduler->systems[i].name,
                              avg_time * 1000);
                }
            }

            nk_tree_pop(ctx);
        }

        // Render Settings Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Render Settings", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);

            // Wireframe toggle - using int for nuklear compatibility
            int wireframe_int = ui_state.show_wireframe ? 1 : 0;
            if (nk_checkbox_label(ctx, "Wireframe Mode", &wireframe_int))
            {
                ui_state.show_wireframe = wireframe_int ? true : false;
                RenderConfig* render_config = get_render_config();
                if (render_config)
                {
                    render_config->mode =
                        ui_state.show_wireframe ? RENDER_MODE_WIREFRAME : RENDER_MODE_SOLID;
                }
            }

            // Debug visualization toggles
            RenderConfig* render_config = get_render_config();
            if (render_config)
            {
                int debug_info = render_config->show_debug_info ? 1 : 0;
                int show_velocities = render_config->show_velocities ? 1 : 0;
                int show_collision = render_config->show_collision_bounds ? 1 : 0;
                int show_orbits = render_config->show_orbits ? 1 : 0;

                if (nk_checkbox_label(ctx, "Show Debug Info", &debug_info))
                {
                    render_config->show_debug_info = debug_info ? true : false;
                }
                if (nk_checkbox_label(ctx, "Show Velocities", &show_velocities))
                {
                    render_config->show_velocities = show_velocities ? true : false;
                }
                if (nk_checkbox_label(ctx, "Show Collision Bounds", &show_collision))
                {
                    render_config->show_collision_bounds = show_collision ? true : false;
                }
                if (nk_checkbox_label(ctx, "Show Orbits", &show_orbits))
                {
                    render_config->show_orbits = show_orbits ? true : false;
                }
            }

            nk_tree_pop(ctx);
        }

        // Camera Controls Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Camera Controls", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);

            nk_property_float(ctx, "Camera Speed", 0.1f, &ui_state.camera_speed, 50.0f, 0.1f, 0.1f);
            nk_property_float(ctx, "Time Scale", 0.0f, &ui_state.time_scale, 5.0f, 0.1f, 0.1f);

            nk_tree_pop(ctx);
        }

        // Entity Browser Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Entities", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Total Entities: %d", world->entity_count);

            for (uint32_t i = 0; i < world->entity_count; i++)
            {
                struct Entity* entity = &world->entities[i];

                // Create a collapsible tree for each entity
                char entity_name[64];
                snprintf(entity_name, sizeof(entity_name), "Entity %d", entity->id);

                if (nk_tree_push_id(ctx, NK_TREE_NODE, entity_name, NK_MINIMIZED, entity->id))
                {
                    nk_layout_row_dynamic(ctx, 15, 1);

                    // Show component mask
                    nk_labelf(ctx, NK_TEXT_LEFT, "  Mask: 0x%08X", entity->component_mask);

                    // Show individual components
                    if (entity->component_mask & COMPONENT_TRANSFORM)
                    {
                        struct Transform* t = entity_get_transform(world, entity->id);
                        if (t)
                        {
                            nk_labelf(ctx, NK_TEXT_LEFT, "  Pos: (%.1f, %.1f, %.1f)", t->position.x,
                                      t->position.y, t->position.z);
                        }
                    }

                    if (entity->component_mask & COMPONENT_PHYSICS)
                    {
                        struct Physics* p = entity_get_physics(world, entity->id);
                        if (p)
                        {
                            nk_labelf(ctx, NK_TEXT_LEFT, "  Mass: %.1f", p->mass);
                        }
                    }

                    if (entity->component_mask & COMPONENT_CAMERA)
                    {
                        nk_labelf(ctx, NK_TEXT_LEFT, "  Type: Camera");
                    }

                    if (entity->component_mask & COMPONENT_PLAYER)
                    {
                        nk_labelf(ctx, NK_TEXT_LEFT, "  Type: Player");
                    }

                    nk_tree_pop(ctx);
                }
            }
            nk_tree_pop(ctx);
        }

        // System Controls Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "System Controls", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);

            for (int i = 0; i < SYSTEM_COUNT; i++)
            {
                SystemInfo* system = &scheduler->systems[i];

                nk_layout_row_dynamic(ctx, 25, 2);
                int enabled = system->enabled ? 1 : 0;
                if (nk_checkbox_label(ctx, system->name, &enabled))
                {
                    system->enabled = enabled ? true : false;
                }
                nk_labelf(ctx, NK_TEXT_LEFT, "%.1f Hz", system->frequency);
            }

            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
}

static void draw_logo_overlay(struct nk_context* ctx)
{
    // Create a centered overlay window for the "[Press ENTER to begin]" text
    int screen_width = sapp_width();
    int screen_height = sapp_height();
    
    int overlay_width = 300;
    int overlay_height = 80;
    int x = (screen_width - overlay_width) / 2;
    int y = screen_height - overlay_height - 50; // Near bottom of screen
    
    if (nk_begin(ctx, "[Press ENTER to begin]", nk_rect(x, y, overlay_width, overlay_height),
                 NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(ctx, 15, 1);
        nk_label(ctx, "[Press ENTER for Scene Selector]", NK_TEXT_CENTERED);
        nk_label(ctx, "[Press TAB from any scene]", NK_TEXT_CENTERED);
    }
    nk_end(ctx);
}

static void draw_scene_selector(struct nk_context* ctx, const char* current_scene)
{
    // Load scenes dynamically if not already loaded
    if (!ui_state.scenes_loaded) {
        load_available_scenes();
    }
    
    // Create a full-screen scene manager
    int screen_width = sapp_width();
    int screen_height = sapp_height();
    
    if (nk_begin(ctx, "Scene Manager", nk_rect(0, 0, screen_width, screen_height),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        // Title bar
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label(ctx, "CGame Scene Manager", NK_TEXT_CENTERED);
        
        // Two-column layout: Scene List (left) and Options (right)
        nk_layout_row_begin(ctx, NK_STATIC, screen_height - 120, 2);
        
        // LEFT PANEL: Scene List
        nk_layout_row_push(ctx, screen_width * 0.6f);
        if (nk_group_begin(ctx, "Scene List", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Available Scenes (%d)", ui_state.scene_count);
            
            if (ui_state.scene_count == 0) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "No scenes found in data/scenes/", NK_TEXT_CENTERED);
            } else {
                // Get current config for highlighting
                const char* startup_scene = config_get_startup_scene();
                
                for (int i = 0; i < ui_state.scene_count; i++)
                {
                    bool is_current = (current_scene && strcmp(current_scene, ui_state.scene_names[i]) == 0);
                    bool is_startup = (strcmp(startup_scene, ui_state.scene_names[i]) == 0);
                    bool is_selected = (ui_state.selected_scene_index == i);
                    
                    // Scene entry layout: button + indicators
                    nk_layout_row_begin(ctx, NK_STATIC, 40, 3);
                    
                    // Main scene button (80% width)
                    nk_layout_row_push(ctx, (screen_width * 0.6f - 40) * 0.8f);
                    
                    if (is_current) {
                        nk_style_push_color(ctx, &ctx->style.button.normal.data.color, nk_rgb(70, 120, 200));
                        nk_style_push_color(ctx, &ctx->style.button.hover.data.color, nk_rgb(80, 130, 210));
                    } else if (is_startup) {
                        nk_style_push_color(ctx, &ctx->style.button.normal.data.color, nk_rgb(120, 120, 70));
                        nk_style_push_color(ctx, &ctx->style.button.hover.data.color, nk_rgb(130, 130, 80));
                    } else if (is_selected) {
                        nk_style_push_color(ctx, &ctx->style.button.normal.data.color, nk_rgb(90, 90, 120));
                        nk_style_push_color(ctx, &ctx->style.button.hover.data.color, nk_rgb(100, 100, 130));
                    }
                    
                    if (nk_button_label(ctx, ui_state.scene_descriptions[i]))
                    {
                        strcpy(ui_state.selected_scene, ui_state.scene_names[i]);
                        ui_state.selected_scene_index = i;
                        ui_state.show_scene_details = true;
                        printf("🎯 Selected scene: %s\n", ui_state.scene_names[i]);
                    }
                    
                    if (is_current || is_startup || is_selected) {
                        nk_style_pop_color(ctx);
                        nk_style_pop_color(ctx);
                    }
                    
                    // Current indicator (10% width)
                    nk_layout_row_push(ctx, (screen_width * 0.6f - 40) * 0.1f);
                    if (is_current) {
                        nk_label(ctx, "●", NK_TEXT_CENTERED);
                    } else {
                        nk_spacing(ctx, 1);
                    }
                    
                    // Startup indicator (10% width)
                    nk_layout_row_push(ctx, (screen_width * 0.6f - 40) * 0.1f);
                    if (is_startup) {
                        nk_label(ctx, "★", NK_TEXT_CENTERED);
                    } else {
                        nk_spacing(ctx, 1);
                    }
                    
                    nk_layout_row_end(ctx);
                }
            }
            
            nk_group_end(ctx);
        }
        
        // RIGHT PANEL: Scene Options and Configuration
        nk_layout_row_push(ctx, screen_width * 0.38f);
        if (nk_group_begin(ctx, "Scene Options", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            // Current scene info
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Current Scene:", NK_TEXT_LEFT);
            
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "  %s", current_scene ? current_scene : "unknown");
            
            // Selected scene info
            if (ui_state.selected_scene_index >= 0 && ui_state.selected_scene_index < ui_state.scene_count) {
                const char* selected_scene_name = ui_state.scene_names[ui_state.selected_scene_index];
                
                nk_layout_row_dynamic(ctx, 10, 1); // Spacer
                nk_spacing(ctx, 1);
                
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Selected Scene:", NK_TEXT_LEFT);
                
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "  %s", ui_state.scene_descriptions[ui_state.selected_scene_index]);
                
                // Action buttons for selected scene
                nk_layout_row_dynamic(ctx, 10, 1); // Spacer
                nk_spacing(ctx, 1);
                
                nk_layout_row_dynamic(ctx, 40, 1);
                if (nk_button_label(ctx, "Launch Scene"))
                {
                    ui_state.scene_change_requested = true;
                    ui_state.show_scene_selector = false;
                    printf("🚀 Launching scene: %s\n", ui_state.selected_scene);
                }
                
                nk_layout_row_dynamic(ctx, 35, 1);
                if (nk_button_label(ctx, "Set as Startup Scene"))
                {
                    config_set_startup_scene(selected_scene_name);
                    config_save();
                    printf("⭐ Set startup scene: %s\n", selected_scene_name);
                }
            } else {
                nk_layout_row_dynamic(ctx, 40, 1); // Spacer
                nk_spacing(ctx, 1);
                
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Select a scene from the list", NK_TEXT_CENTERED);
                
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "to see options here.", NK_TEXT_CENTERED);
            }
            
            // Configuration section
            nk_layout_row_dynamic(ctx, 20, 1); // Spacer
            nk_spacing(ctx, 1);
            
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Configuration:", NK_TEXT_LEFT);
            
            // Get current config
            const char* startup_scene = config_get_startup_scene();
            bool auto_start = config_get_auto_start();
            
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Startup: %s", startup_scene);
            
            nk_layout_row_dynamic(ctx, 25, 1);
            static int temp_auto_start = 0;
            temp_auto_start = auto_start ? 1 : 0;
            
            if (nk_checkbox_label(ctx, "Auto-start enabled", &temp_auto_start)) {
                config_set_auto_start(temp_auto_start != 0);
                config_save();
            }
            
            // Legend
            nk_layout_row_dynamic(ctx, 30, 1); // Spacer
            nk_spacing(ctx, 1);
            
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "Legend:", NK_TEXT_LEFT);
            
            nk_layout_row_dynamic(ctx, 18, 1);
            nk_label(ctx, "● Current scene", NK_TEXT_LEFT);
            
            nk_layout_row_dynamic(ctx, 18, 1);
            nk_label(ctx, "★ Startup scene", NK_TEXT_LEFT);
            
            nk_group_end(ctx);
        }
        
        nk_layout_row_end(ctx);
        
        // Bottom buttons
        nk_layout_row_dynamic(ctx, 40, 4);
        
        if (nk_button_label(ctx, "Back to Game"))
        {
            ui_state.show_scene_selector = false;
        }
        
        if (nk_button_label(ctx, "Refresh Scenes"))
        {
            free_scene_list();
            load_available_scenes();
            ui_state.selected_scene_index = -1; // Clear selection
        }
        
        if (nk_button_label(ctx, "Save Config"))
        {
            config_save();
        }
        
        if (nk_button_label(ctx, "Exit Game"))
        {
            // This will be handled by the scene script
            ui_state.show_scene_selector = false;
            sapp_request_quit();
        }
    }
    
    // Handle window close button (though it won't show with no close button flag)
    if (nk_window_is_closed(ctx, "Scene Manager"))
    {
        ui_state.show_scene_selector = false;
    }
    
    nk_end(ctx);
}

static void draw_scene_browser(struct nk_context* ctx, const char* current_scene)
{
    // Load scenes dynamically if not already loaded
    if (!ui_state.scenes_loaded) {
        load_available_scenes();
    }
    
    // Scene browser panel on the left side
    int panel_width = 300;
    int panel_height = sapp_height() - 40; // Leave space for menu bar
    
    if (nk_begin(ctx, "Scene Browser", nk_rect(10, 30, panel_width, panel_height),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE))
    {
        // Configuration section at top
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Scene Manager", NK_TEXT_CENTERED);
        
        // Current scene info
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "Current: %s", current_scene ? current_scene : "unknown");
        
        // Startup configuration
        const char* startup_scene = config_get_startup_scene();
        bool auto_start = config_get_auto_start();
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "Startup: %s", startup_scene);
        
        static int temp_auto_start = 0;
        temp_auto_start = auto_start ? 1 : 0; // Convert bool to int for nuklear
        
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_checkbox_label(ctx, "Auto-start", &temp_auto_start)) {
            config_set_auto_start(temp_auto_start != 0);
            config_save();
        }
        
        nk_layout_row_dynamic(ctx, 5, 1); // Spacer
        nk_spacing(ctx, 1);
        
        // Scene list
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Available Scenes:", NK_TEXT_LEFT);
        
        if (ui_state.scene_count == 0) {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "No scenes found", NK_TEXT_CENTERED);
        } else {
            // Scene list with selection
            nk_layout_row_dynamic(ctx, 200, 1); // Fixed height for scrollable list
            if (nk_group_begin(ctx, "scene_list", NK_WINDOW_BORDER)) {
                for (int i = 0; i < ui_state.scene_count; i++) {
                    bool is_current = (current_scene && strcmp(current_scene, ui_state.scene_names[i]) == 0);
                    bool is_startup = (strcmp(startup_scene, ui_state.scene_names[i]) == 0);
                    bool is_selected = (ui_state.selected_scene_index == i);
                    
                    nk_layout_row_dynamic(ctx, 25, 1);
                    
                    // Style for different scene states
                    if (is_current) {
                        nk_style_push_color(ctx, &ctx->style.selectable.normal.data.color, nk_rgb(70, 120, 200));
                    } else if (is_startup) {
                        nk_style_push_color(ctx, &ctx->style.selectable.normal.data.color, nk_rgb(120, 120, 70));
                    }
                    
                    // Scene name with indicators
                    char scene_label[256];
                    snprintf(scene_label, sizeof(scene_label), "%s%s%s", 
                             ui_state.scene_descriptions[i],
                             is_current ? " ●" : "",
                             is_startup ? " ⚡" : "");
                    
                    if (nk_selectable_label(ctx, scene_label, NK_TEXT_LEFT, &is_selected)) {
                        ui_state.selected_scene_index = i;
                        ui_state.show_scene_details = true;
                    }
                    
                    if (is_current || is_startup) {
                        nk_style_pop_color(ctx);
                    }
                }
                nk_group_end(ctx);
            }
        }
        
        nk_layout_row_dynamic(ctx, 5, 1); // Spacer
        nk_spacing(ctx, 1);
        
        // Actions for selected scene
        if (ui_state.selected_scene_index >= 0 && ui_state.selected_scene_index < ui_state.scene_count) {
            const char* selected_scene_name = ui_state.scene_names[ui_state.selected_scene_index];
            
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Selected: %s", ui_state.scene_descriptions[ui_state.selected_scene_index]);
            
            // Action buttons
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_button_label(ctx, "Launch")) {
                strcpy(ui_state.selected_scene, selected_scene_name);
                ui_state.scene_change_requested = true;
                printf("🎬 Scene launch requested: %s\n", selected_scene_name);
            }
            
            if (nk_button_label(ctx, "Set Default")) {
                config_set_startup_scene(selected_scene_name);
                config_save();
                printf("⚙️  Set startup scene: %s\n", selected_scene_name);
            }
            
            // Future: Scene graph/details button
            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Scene Details")) {
                // TODO: Show scene graph, entities, etc.
                printf("📊 Scene details for: %s (TODO: implement)\n", selected_scene_name);
            }
        }
        
        nk_layout_row_dynamic(ctx, 10, 1); // Spacer
        nk_spacing(ctx, 1);
        
        // Utility buttons
        nk_layout_row_dynamic(ctx, 25, 2);
        if (nk_button_label(ctx, "Refresh")) {
            free_scene_list();
            load_available_scenes();
            ui_state.selected_scene_index = -1; // Clear selection
        }
        
        if (nk_button_label(ctx, "Config")) {
            config_save();
            printf("💾 Configuration saved\n");
        }
    }
    nk_end(ctx);
}

void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene)
{
    // Early exit if UI is not visible
    if (!ui_visible) return;
    
    // Get new frame context from sokol_nuklear
    struct nk_context* ctx = snk_new_frame();

    // Update FPS calculation
    ui_state.frame_count++;
    ui_state.fps_timer += delta_time;

    if (ui_state.fps_timer >= 1.0f)
    {
        ui_state.fps = ui_state.frame_count / ui_state.fps_timer;
        ui_state.frame_count = 0;
        ui_state.fps_timer = 0.0f;
    }

    // Draw UI components
    draw_hud(ctx, world);
    
    // Draw logo overlay if in logo scene
    if (current_scene && strcmp(current_scene, "logo") == 0)
    {
        draw_logo_overlay(ctx);
    }
    
    // Draw scene selector if visible
    if (ui_state.show_scene_selector)
    {
        draw_scene_selector(ctx, current_scene);
    }
    
    // Only draw debug panel if debug UI is visible
    if (debug_ui_visible)
    {
        draw_debug_panel(ctx, world, scheduler);
    }

    // Render the UI
    snk_render(sapp_width(), sapp_height());
}

bool ui_handle_event(const void* ev)
{
    const sapp_event* event = (const sapp_event*)ev;

    // Handle UI-specific hotkeys before passing to Nuklear
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN)
    {
        switch (event->key_code)
        {
            case SAPP_KEYCODE_F1:
                ui_toggle_debug_panel();
                return true;  // UI captured this event

            case SAPP_KEYCODE_F2:
                ui_toggle_hud();
                return true;  // UI captured this event

            case SAPP_KEYCODE_F3:
                ui_state.show_wireframe = !ui_state.show_wireframe;
                RenderConfig* render_config = get_render_config();
                if (render_config)
                {
                    render_config->mode =
                        ui_state.show_wireframe ? RENDER_MODE_WIREFRAME : RENDER_MODE_SOLID;
                }
                return true;  // UI captured this event

            case SAPP_KEYCODE_ESCAPE:
                // ESC key handling is now scene-specific
                return false;  // Don't capture, let scene handle it

            case SAPP_KEYCODE_TAB:
                // TAB key can still trigger scene selector from any scene
                // Request transition to scene selector
                return false;  // Let main game loop handle the transition

            default:
                break;
        }
    }

    // Pass event to Nuklear and return whether it was captured
    return snk_handle_event(event);
}

void ui_toggle_debug_panel(void)
{
    ui_state.show_debug_panel = !ui_state.show_debug_panel;
    printf("🔧 Debug panel %s\n", ui_state.show_debug_panel ? "enabled" : "disabled");
}

void ui_toggle_hud(void)
{
    ui_state.show_hud = !ui_state.show_hud;
    printf("📊 HUD %s\n", ui_state.show_hud ? "enabled" : "disabled");
}

void ui_show_scene_selector(void)
{
    ui_state.show_scene_selector = true;
    printf("🎬 Scene selector opened\n");
}

void ui_hide_scene_selector(void)
{
    ui_state.show_scene_selector = false;
    printf("🎬 Scene selector closed\n");
}

bool ui_is_scene_selector_visible(void)
{
    return ui_state.show_scene_selector;
}

bool ui_has_scene_change_request(void)
{
    return ui_state.scene_change_requested;
}

const char* ui_get_requested_scene(void)
{
    return ui_state.selected_scene;
}

void ui_clear_scene_change_request(void)
{
    ui_state.scene_change_requested = false;
}

// ============================================================================
// UI VISIBILITY CONTROL
// ============================================================================

void ui_set_visible(bool visible)
{
    ui_visible = visible;
}

void ui_set_debug_visible(bool visible)
{
    debug_ui_visible = visible;
}

bool ui_is_visible(void)
{
    return ui_visible;
}

bool ui_is_debug_visible(void)
{
    return debug_ui_visible;
}

static void load_available_scenes(void)
{
    if (ui_state.scenes_loaded) {
        return; // Already loaded
    }
    
    // Free existing data if any
    free_scene_list();
    
    DIR* dir = opendir("data/scenes");
    if (!dir) {
        printf("⚠️  Could not open data/scenes directory\n");
        return;
    }
    
    // First pass: count valid scene files
    ui_state.scene_count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt") && 
            strcmp(entry->d_name, "navigation_menu.txt") != 0) {  // Skip the navigation menu itself
            ui_state.scene_count++;
        }
    }
    
    if (ui_state.scene_count == 0) {
        closedir(dir);
        return;
    }
    
    // Allocate arrays
    ui_state.scene_names = malloc(ui_state.scene_count * sizeof(char*));
    ui_state.scene_descriptions = malloc(ui_state.scene_count * sizeof(char*));
    
    // Second pass: collect scene names
    rewinddir(dir);
    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < ui_state.scene_count) {
        if (strstr(entry->d_name, ".txt") && 
            strcmp(entry->d_name, "navigation_menu.txt") != 0) {
            
            // Remove .txt extension for scene name
            char* scene_name = malloc(strlen(entry->d_name) + 1);
            strcpy(scene_name, entry->d_name);
            char* dot = strrchr(scene_name, '.');
            if (dot) *dot = '\0';
            
            ui_state.scene_names[index] = scene_name;
            
            // Generate description
            const char* desc = get_scene_description(scene_name);
            ui_state.scene_descriptions[index] = malloc(strlen(desc) + 1);
            strcpy(ui_state.scene_descriptions[index], desc);
            
            index++;
        }
    }
    
    closedir(dir);
    ui_state.scenes_loaded = true;
    
    printf("✅ Loaded %d available scenes dynamically\n", ui_state.scene_count);
}

static void free_scene_list(void)
{
    if (ui_state.scene_names) {
        for (int i = 0; i < ui_state.scene_count; i++) {
            free(ui_state.scene_names[i]);
            free(ui_state.scene_descriptions[i]);
        }
        free(ui_state.scene_names);
        free(ui_state.scene_descriptions);
        ui_state.scene_names = NULL;
        ui_state.scene_descriptions = NULL;
    }
    ui_state.scene_count = 0;
    ui_state.scenes_loaded = false;
}

static const char* get_scene_description(const char* scene_name)
{
    // Generate lore-accurate descriptions for FTL navigation scenes
    if (strcmp(scene_name, "logo") == 0) {
        return "System Boot - Core engine validation sequence";
    } else if (strcmp(scene_name, "system_overview") == 0) {
        return "System Overview - Sector-wide FTL navigation hub";
    } else if (strcmp(scene_name, "slipstream_nav") == 0) {
        return "Slipstream Navigation - FTL threadline planning testbed";
    } else if (strcmp(scene_name, "derelict_alpha") == 0) {
        return "Derelict Alpha - Ancient station excavation site";
    } else if (strcmp(scene_name, "derelict_beta") == 0) {
        return "Derelict Beta - Deep-space archaeological exploration";
    } else if (strcmp(scene_name, "flight_test") == 0) {
        return "Flight Test - Open plain flight training ground";
    } else if (strcmp(scene_name, "navigation_menu") == 0) {
        return "Threadline Planner - Primary FTL navigation interface";
    } else {
        // Default description for unknown scenes
        static char default_desc[128];
        snprintf(default_desc, sizeof(default_desc), "%s - Uncharted location", scene_name);
        return default_desc;
    }
}