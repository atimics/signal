#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

// Nuklear configuration - declarations only (no implementation)
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "sokol_nuklear.h"

#include "ui.h"
#include "core.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

static UIState ui_state = {0};

void ui_init(void) {
    snk_setup(&(snk_desc_t){0});
    
    // Initialize UI state
    ui_state.show_debug_panel = true;
    ui_state.show_hud = true;
    ui_state.show_wireframe = false;
    ui_state.camera_speed = 10.0f;
    ui_state.time_scale = 1.0f;
    ui_state.fps = 0.0f;
    ui_state.frame_count = 0;
    ui_state.fps_timer = 0.0f;
    
    printf("✅ Nuklear UI initialized\n");
}

void ui_shutdown(void) {
    snk_shutdown();
    printf("✅ Nuklear UI shutdown\n");
}

static void draw_hud(struct nk_context* ctx, struct World* world) {
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
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_CAMERA) {
                camera_id = entity->id;
                break;
            }
        }
        
        if (camera_id != INVALID_ENTITY) {
            struct Transform* cam_transform = entity_get_transform(world, camera_id);
            struct Camera* camera = entity_get_camera(world, camera_id);
            
            if (cam_transform) {
                nk_labelf(ctx, NK_TEXT_LEFT, "Camera Pos: (%.1f, %.1f, %.1f)", 
                         cam_transform->position.x, 
                         cam_transform->position.y, 
                         cam_transform->position.z);
            }
            
            if (camera) {
                nk_labelf(ctx, NK_TEXT_LEFT, "FOV: %.1f°", camera->fov);
                nk_labelf(ctx, NK_TEXT_LEFT, "Type: Camera");
            }
        }
        
        // Find and display player information
        for (uint32_t i = 0; i < world->entity_count; i++) {
            struct Entity* entity = &world->entities[i];
            if (entity->component_mask & COMPONENT_PLAYER) {
                struct Transform* transform = entity_get_transform(world, entity->id);
                struct Physics* physics = entity_get_physics(world, entity->id);
                
                if (transform) {
                    nk_labelf(ctx, NK_TEXT_LEFT, "Player: (%.1f, %.1f, %.1f)", 
                             transform->position.x, 
                             transform->position.y, 
                             transform->position.z);
                }
                
                if (physics) {
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

static void draw_debug_panel(struct nk_context* ctx, struct World* world, SystemScheduler* scheduler) {
    if (!ui_state.show_debug_panel) return;
    
    if (nk_begin(ctx, "Debug Panel", nk_rect(50, 250, 400, 500),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
        NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
    {
        // System Performance Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Performance", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 20, 2);
            nk_label(ctx, "FPS:", NK_TEXT_LEFT);
            nk_labelf(ctx, NK_TEXT_LEFT, "%.1f", ui_state.fps);
            
            nk_label(ctx, "Frame Count:", NK_TEXT_LEFT);
            nk_labelf(ctx, NK_TEXT_LEFT, "%d", scheduler->frame_count);
            
            nk_label(ctx, "Total Time:", NK_TEXT_LEFT);
            nk_labelf(ctx, NK_TEXT_LEFT, "%.1fs", scheduler->total_time);
            
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "System Times:", NK_TEXT_LEFT);
            
            for (int i = 0; i < SYSTEM_COUNT; i++) {
                if (scheduler->system_calls[i] > 0) {
                    float avg_time = scheduler->system_times[i] / scheduler->system_calls[i];
                    nk_layout_row_dynamic(ctx, 15, 1);
                    nk_labelf(ctx, NK_TEXT_LEFT, "  %s: %.3fms", 
                             scheduler->systems[i].name, avg_time * 1000);
                }
            }
            
            nk_tree_pop(ctx);
        }
        
        // Render Settings Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Render Settings", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 30, 1);
            
            // Wireframe toggle - using int for nuklear compatibility
            int wireframe_int = ui_state.show_wireframe ? 1 : 0;
            if (nk_checkbox_label(ctx, "Wireframe Mode", &wireframe_int)) {
                ui_state.show_wireframe = wireframe_int ? true : false;
                RenderConfig* render_config = get_render_config();
                if (render_config) {
                    render_config->mode = ui_state.show_wireframe ? 
                                        RENDER_MODE_WIREFRAME : RENDER_MODE_SOLID;
                }
            }
            
            // Debug visualization toggles
            RenderConfig* render_config = get_render_config();
            if (render_config) {
                int debug_info = render_config->show_debug_info ? 1 : 0;
                int show_velocities = render_config->show_velocities ? 1 : 0;
                int show_collision = render_config->show_collision_bounds ? 1 : 0;
                int show_orbits = render_config->show_orbits ? 1 : 0;
                
                if (nk_checkbox_label(ctx, "Show Debug Info", &debug_info)) {
                    render_config->show_debug_info = debug_info ? true : false;
                }
                if (nk_checkbox_label(ctx, "Show Velocities", &show_velocities)) {
                    render_config->show_velocities = show_velocities ? true : false;
                }
                if (nk_checkbox_label(ctx, "Show Collision Bounds", &show_collision)) {
                    render_config->show_collision_bounds = show_collision ? true : false;
                }
                if (nk_checkbox_label(ctx, "Show Orbits", &show_orbits)) {
                    render_config->show_orbits = show_orbits ? true : false;
                }
            }
            
            nk_tree_pop(ctx);
        }
        
        // Camera Controls Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Camera Controls", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 30, 1);
            
            nk_property_float(ctx, "Camera Speed", 0.1f, &ui_state.camera_speed, 50.0f, 0.1f, 0.1f);
            nk_property_float(ctx, "Time Scale", 0.0f, &ui_state.time_scale, 5.0f, 0.1f, 0.1f);
            
            nk_tree_pop(ctx);
        }
        
        // Entity Browser Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "Entities", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Total Entities: %d", world->entity_count);
            
            for (uint32_t i = 0; i < world->entity_count; i++) {
                struct Entity* entity = &world->entities[i];
                
                // Create a collapsible tree for each entity
                char entity_name[64];
                snprintf(entity_name, sizeof(entity_name), "Entity %d", entity->id);
                
                if (nk_tree_push_id(ctx, NK_TREE_NODE, entity_name, NK_MINIMIZED, entity->id)) {
                    nk_layout_row_dynamic(ctx, 15, 1);
                    
                    // Show component mask
                    nk_labelf(ctx, NK_TEXT_LEFT, "  Mask: 0x%08X", entity->component_mask);
                    
                    // Show individual components
                    if (entity->component_mask & COMPONENT_TRANSFORM) {
                        struct Transform* t = entity_get_transform(world, entity->id);
                        if (t) {
                            nk_labelf(ctx, NK_TEXT_LEFT, "  Pos: (%.1f, %.1f, %.1f)", 
                                     t->position.x, t->position.y, t->position.z);
                        }
                    }
                    
                    if (entity->component_mask & COMPONENT_PHYSICS) {
                        struct Physics* p = entity_get_physics(world, entity->id);
                        if (p) {
                            nk_labelf(ctx, NK_TEXT_LEFT, "  Mass: %.1f", p->mass);
                        }
                    }
                    
                    if (entity->component_mask & COMPONENT_CAMERA) {
                        nk_labelf(ctx, NK_TEXT_LEFT, "  Type: Camera");
                    }
                    
                    if (entity->component_mask & COMPONENT_PLAYER) {
                        nk_labelf(ctx, NK_TEXT_LEFT, "  Type: Player");
                    }
                    
                    nk_tree_pop(ctx);
                }
            }
            nk_tree_pop(ctx);
        }
        
        // System Controls Section
        if (nk_tree_push(ctx, NK_TREE_TAB, "System Controls", NK_MINIMIZED)) {
            nk_layout_row_dynamic(ctx, 30, 1);
            
            for (int i = 0; i < SYSTEM_COUNT; i++) {
                SystemInfo* system = &scheduler->systems[i];
                
                nk_layout_row_dynamic(ctx, 25, 2);
                int enabled = system->enabled ? 1 : 0;
                if (nk_checkbox_label(ctx, system->name, &enabled)) {
                    system->enabled = enabled ? true : false;
                }
                nk_labelf(ctx, NK_TEXT_LEFT, "%.1f Hz", system->frequency);
            }
            
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
}

void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time) {
    // Get new frame context from sokol_nuklear
    struct nk_context* ctx = snk_new_frame();
    
    // Update FPS calculation
    ui_state.frame_count++;
    ui_state.fps_timer += delta_time;
    
    if (ui_state.fps_timer >= 1.0f) {
        ui_state.fps = ui_state.frame_count / ui_state.fps_timer;
        ui_state.frame_count = 0;
        ui_state.fps_timer = 0.0f;
    }
    
    // Draw UI components
    draw_hud(ctx, world);
    draw_debug_panel(ctx, world, scheduler);
    
    // Render the UI
    snk_render(sapp_width(), sapp_height());
}

bool ui_handle_event(const void* ev) {
    const sapp_event* event = (const sapp_event*)ev;
    
    // Handle UI-specific hotkeys before passing to Nuklear
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch (event->key_code) {
            case SAPP_KEYCODE_F1:
                ui_toggle_debug_panel();
                return true;  // UI captured this event
                
            case SAPP_KEYCODE_F2:
                ui_toggle_hud();
                return true;  // UI captured this event
                
            case SAPP_KEYCODE_F3:
                ui_state.show_wireframe = !ui_state.show_wireframe;
                RenderConfig* render_config = get_render_config();
                if (render_config) {
                    render_config->mode = ui_state.show_wireframe ? 
                                        RENDER_MODE_WIREFRAME : RENDER_MODE_SOLID;
                }
                return true;  // UI captured this event
                
            default:
                break;
        }
    }
    
    // Pass event to Nuklear and return whether it was captured
    return snk_handle_event(event);
}

void ui_toggle_debug_panel(void) {
    ui_state.show_debug_panel = !ui_state.show_debug_panel;
    printf("🔧 Debug panel %s\n", ui_state.show_debug_panel ? "enabled" : "disabled");
}

void ui_toggle_hud(void) {
    ui_state.show_hud = !ui_state.show_hud;
    printf("📊 HUD %s\n", ui_state.show_hud ? "enabled" : "disabled");
}