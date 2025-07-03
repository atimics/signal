/**
 * @file hud_system.c
 * @brief Modular HUD system with camera mode support
 */

#include "hud_system.h"
#include "system/input.h"
#include "component/look_target.h"
#include "graphics_api.h"  // This includes nuklear properly
#include "sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Global HUD system state
static HUDSystem g_hud_system = {0};

// ============================================================================
// INTERNAL HELPER FUNCTIONS
// ============================================================================

static void setup_default_component_layouts(void);
static void configure_cockpit_mode(void);
static void configure_chase_near_mode(void);
static void configure_chase_far_mode(void);
static bool is_component_visible_in_mode(HUDComponentType type, HUDCameraMode mode);

// ============================================================================
// HUD SYSTEM CORE API
// ============================================================================

void hud_system_init(void)
{
    if (g_hud_system.initialized) {
        printf("⚠️  HUD system already initialized\n");
        return;
    }
    
    // Initialize system state
    memset(&g_hud_system, 0, sizeof(HUDSystem));
    g_hud_system.current_mode = HUD_CAMERA_MODE_CHASE_NEAR;
    g_hud_system.transition_time = 0.5f; // 500ms transitions
    g_hud_system.transition_progress = 1.0f; // No transition in progress
    
    // Set up component render functions
    g_hud_system.components[HUD_COMPONENT_TARGETING_RETICLE].render_func = hud_render_targeting_reticle;
    g_hud_system.components[HUD_COMPONENT_SPEEDOMETER].render_func = hud_render_speedometer;
    g_hud_system.components[HUD_COMPONENT_THRUST_INDICATOR].render_func = hud_render_thrust_indicator;
    g_hud_system.components[HUD_COMPONENT_INSTRUMENT_PANEL].render_func = hud_render_instrument_panel;
    g_hud_system.components[HUD_COMPONENT_COMMUNICATIONS].render_func = hud_render_communications;
    g_hud_system.components[HUD_COMPONENT_TRADE_INTERFACE].render_func = hud_render_trade_interface;
    g_hud_system.components[HUD_COMPONENT_WAYPOINT_DISPLAY].render_func = hud_render_waypoint_display;
    g_hud_system.components[HUD_COMPONENT_TACTICAL_OVERLAY].render_func = hud_render_tactical_overlay;
    g_hud_system.components[HUD_COMPONENT_PROXIMITY_WARNINGS].render_func = hud_render_proximity_warnings;
    g_hud_system.components[HUD_COMPONENT_WEAPON_SYSTEMS].render_func = hud_render_weapon_systems;
    g_hud_system.components[HUD_COMPONENT_EXPLORATION_SCANNER].render_func = hud_render_exploration_scanner;
    
    // Configure default layouts and visibility
    setup_default_component_layouts();
    configure_cockpit_mode();
    configure_chase_near_mode();
    configure_chase_far_mode();
    
    g_hud_system.initialized = true;
    printf("✅ HUD system initialized with %d components\n", HUD_COMPONENT_COUNT);
    printf("🎯 Default camera mode: CHASE_NEAR\n");
}

void hud_system_shutdown(void)
{
    if (!g_hud_system.initialized) return;
    
    memset(&g_hud_system, 0, sizeof(HUDSystem));
    printf("✅ HUD system shut down\n");
}

void hud_system_update(float delta_time)
{
    if (!g_hud_system.initialized) return;
    
    // Handle camera mode transitions
    if (g_hud_system.transition_progress < 1.0f) {
        g_hud_system.transition_progress += delta_time / g_hud_system.transition_time;
        
        if (g_hud_system.transition_progress >= 1.0f) {
            g_hud_system.transition_progress = 1.0f;
            g_hud_system.current_mode = g_hud_system.transition_to;
            
            // Debug transition completion
            const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
            printf("📷 HUD transition complete: %s\n", mode_names[g_hud_system.current_mode]);
        }
    }
    
    // Update component visibility based on current mode
    for (int i = 0; i < HUD_COMPONENT_COUNT; i++) {
        HUDComponent* comp = &g_hud_system.components[i];
        bool should_be_visible = is_component_visible_in_mode(i, g_hud_system.current_mode);
        
        // Smooth fade in/out during transitions
        if (g_hud_system.transition_progress < 1.0f) {
            bool visible_from = is_component_visible_in_mode(i, g_hud_system.transition_from);
            bool visible_to = is_component_visible_in_mode(i, g_hud_system.transition_to);
            
            if (visible_from && !visible_to) {
                // Fading out
                comp->opacity = 1.0f - hud_smooth_step(g_hud_system.transition_progress);
                comp->visible = comp->opacity > 0.01f;
            } else if (!visible_from && visible_to) {
                // Fading in
                comp->opacity = hud_smooth_step(g_hud_system.transition_progress);
                comp->visible = comp->opacity > 0.01f;
            } else if (visible_from && visible_to) {
                // Visible in both modes
                comp->visible = true;
                comp->opacity = 1.0f;
            } else {
                // Hidden in both modes
                comp->visible = false;
                comp->opacity = 0.0f;
            }
        } else {
            // No transition - set final state
            comp->visible = should_be_visible;
            comp->opacity = should_be_visible ? 1.0f : 0.0f;
        }
    }
}

void hud_system_render(struct nk_context* ctx, struct World* world)
{
    if (!g_hud_system.initialized || !ctx || !world) return;
    
    float screen_width = (float)sapp_width();
    float screen_height = (float)sapp_height();
    
    // Create transparent overlay window for all HUD components
    struct nk_rect overlay_area = nk_rect(0, 0, screen_width, screen_height);
    struct nk_style_item original_bg = ctx->style.window.fixed_background;
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0,0,0,0));
    
    if (nk_begin(ctx, "HUD_Overlay", overlay_area, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NOT_INTERACTIVE)) {
        
        // Render all visible components
        for (int i = 0; i < HUD_COMPONENT_COUNT; i++) {
            HUDComponent* comp = &g_hud_system.components[i];
            
            if (comp->visible && comp->render_func && comp->opacity > 0.01f) {
                // Convert normalized coordinates to screen coordinates
                Vector2 screen_pos = hud_normalized_to_screen(comp->position, screen_width, screen_height);
                Vector2 screen_size = hud_normalized_size_to_screen(comp->size, screen_width, screen_height);
                
                // Render the component
                comp->render_func(ctx, world, screen_pos, screen_size, comp->opacity);
            }
        }
        
        // Debug: Show current camera mode
        static uint32_t debug_counter = 0;
        if (++debug_counter % 300 == 0) { // Every 5 seconds at 60fps
            const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
            printf("🎯 HUD Mode: %s (transition: %.1f%%)\n", 
                   mode_names[g_hud_system.current_mode], 
                   g_hud_system.transition_progress * 100.0f);
        }
    }
    nk_end(ctx);
    
    // Restore window background style
    ctx->style.window.fixed_background = original_bg;
}

void hud_system_set_camera_mode(HUDCameraMode mode)
{
    if (!g_hud_system.initialized || mode >= HUD_CAMERA_MODE_COUNT) return;
    
    if (mode != g_hud_system.current_mode) {
        // Start transition
        g_hud_system.transition_from = g_hud_system.current_mode;
        g_hud_system.transition_to = mode;
        g_hud_system.transition_progress = 0.0f;
        
        const char* mode_names[] = {"COCKPIT", "CHASE_NEAR", "CHASE_FAR"};
        printf("📷 HUD transitioning: %s → %s\n", 
               mode_names[g_hud_system.transition_from], 
               mode_names[g_hud_system.transition_to]);
    }
}

HUDCameraMode hud_system_get_camera_mode(void)
{
    return g_hud_system.current_mode;
}

// ============================================================================
// COMPONENT CONFIGURATION
// ============================================================================

static void setup_default_component_layouts(void)
{
    // Targeting reticle - center screen
    g_hud_system.components[HUD_COMPONENT_TARGETING_RETICLE].position = (Vector2){0.5f, 0.5f};
    g_hud_system.components[HUD_COMPONENT_TARGETING_RETICLE].size = (Vector2){0.1f, 0.1f};
    
    // Speedometer - bottom left
    g_hud_system.components[HUD_COMPONENT_SPEEDOMETER].position = (Vector2){0.05f, 0.85f};
    g_hud_system.components[HUD_COMPONENT_SPEEDOMETER].size = (Vector2){0.15f, 0.1f};
    
    // Thrust indicator - bottom right
    g_hud_system.components[HUD_COMPONENT_THRUST_INDICATOR].position = (Vector2){0.8f, 0.85f};
    g_hud_system.components[HUD_COMPONENT_THRUST_INDICATOR].size = (Vector2){0.15f, 0.1f};
    
    // Instrument panel - top area for cockpit
    g_hud_system.components[HUD_COMPONENT_INSTRUMENT_PANEL].position = (Vector2){0.1f, 0.05f};
    g_hud_system.components[HUD_COMPONENT_INSTRUMENT_PANEL].size = (Vector2){0.8f, 0.3f};
    
    // Communications - left side
    g_hud_system.components[HUD_COMPONENT_COMMUNICATIONS].position = (Vector2){0.02f, 0.3f};
    g_hud_system.components[HUD_COMPONENT_COMMUNICATIONS].size = (Vector2){0.25f, 0.4f};
    
    // Trade interface - right side  
    g_hud_system.components[HUD_COMPONENT_TRADE_INTERFACE].position = (Vector2){0.73f, 0.3f};
    g_hud_system.components[HUD_COMPONENT_TRADE_INTERFACE].size = (Vector2){0.25f, 0.4f};
    
    // Waypoint display - top right
    g_hud_system.components[HUD_COMPONENT_WAYPOINT_DISPLAY].position = (Vector2){0.7f, 0.05f};
    g_hud_system.components[HUD_COMPONENT_WAYPOINT_DISPLAY].size = (Vector2){0.28f, 0.2f};
    
    // Tactical overlay - top left
    g_hud_system.components[HUD_COMPONENT_TACTICAL_OVERLAY].position = (Vector2){0.02f, 0.05f};
    g_hud_system.components[HUD_COMPONENT_TACTICAL_OVERLAY].size = (Vector2){0.3f, 0.25f};
    
    // Proximity warnings - center top
    g_hud_system.components[HUD_COMPONENT_PROXIMITY_WARNINGS].position = (Vector2){0.35f, 0.05f};
    g_hud_system.components[HUD_COMPONENT_PROXIMITY_WARNINGS].size = (Vector2){0.3f, 0.15f};
    
    // Weapon systems - right middle
    g_hud_system.components[HUD_COMPONENT_WEAPON_SYSTEMS].position = (Vector2){0.75f, 0.4f};
    g_hud_system.components[HUD_COMPONENT_WEAPON_SYSTEMS].size = (Vector2){0.23f, 0.3f};
    
    // Exploration scanner - left middle
    g_hud_system.components[HUD_COMPONENT_EXPLORATION_SCANNER].position = (Vector2){0.02f, 0.4f};
    g_hud_system.components[HUD_COMPONENT_EXPLORATION_SCANNER].size = (Vector2){0.25f, 0.3f};
}

static void configure_cockpit_mode(void)
{
    // Cockpit mode: Full instrument panel, comms, trade interface
    HUDComponentType cockpit_components[] = {
        HUD_COMPONENT_INSTRUMENT_PANEL,
        HUD_COMPONENT_COMMUNICATIONS,
        HUD_COMPONENT_TRADE_INTERFACE,
        HUD_COMPONENT_SPEEDOMETER,
        HUD_COMPONENT_THRUST_INDICATOR
    };
    
    for (size_t i = 0; i < sizeof(cockpit_components) / sizeof(HUDComponentType); i++) {
        HUDComponent* comp = &g_hud_system.components[cockpit_components[i]];
        comp->valid_modes[comp->valid_mode_count++] = HUD_CAMERA_MODE_COCKPIT;
    }
}

static void configure_chase_near_mode(void)
{
    // Near chase mode: Racing HUD with targeting, speed, thrust, proximity
    HUDComponentType near_components[] = {
        HUD_COMPONENT_TARGETING_RETICLE,
        HUD_COMPONENT_SPEEDOMETER,
        HUD_COMPONENT_THRUST_INDICATOR,
        HUD_COMPONENT_PROXIMITY_WARNINGS
    };
    
    for (size_t i = 0; i < sizeof(near_components) / sizeof(HUDComponentType); i++) {
        HUDComponent* comp = &g_hud_system.components[near_components[i]];
        comp->valid_modes[comp->valid_mode_count++] = HUD_CAMERA_MODE_CHASE_NEAR;
    }
}

static void configure_chase_far_mode(void)
{
    // Far chase mode: Combat/exploration HUD with tactical overlay, waypoints, weapons, scanner
    HUDComponentType far_components[] = {
        HUD_COMPONENT_TARGETING_RETICLE,
        HUD_COMPONENT_WAYPOINT_DISPLAY,
        HUD_COMPONENT_TACTICAL_OVERLAY,
        HUD_COMPONENT_WEAPON_SYSTEMS,
        HUD_COMPONENT_EXPLORATION_SCANNER,
        HUD_COMPONENT_SPEEDOMETER
    };
    
    for (size_t i = 0; i < sizeof(far_components) / sizeof(HUDComponentType); i++) {
        HUDComponent* comp = &g_hud_system.components[far_components[i]];
        comp->valid_modes[comp->valid_mode_count++] = HUD_CAMERA_MODE_CHASE_FAR;
    }
}

static bool is_component_visible_in_mode(HUDComponentType type, HUDCameraMode mode)
{
    if (type >= HUD_COMPONENT_COUNT || mode >= HUD_CAMERA_MODE_COUNT) return false;
    
    HUDComponent* comp = &g_hud_system.components[type];
    for (int i = 0; i < comp->valid_mode_count; i++) {
        if (comp->valid_modes[i] == mode) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

Vector2 hud_normalized_to_screen(Vector2 normalized_pos, float screen_width, float screen_height)
{
    return (Vector2){
        normalized_pos.u * screen_width,
        normalized_pos.v * screen_height
    };
}

Vector2 hud_normalized_size_to_screen(Vector2 normalized_size, float screen_width, float screen_height)
{
    return (Vector2){
        normalized_size.u * screen_width,
        normalized_size.v * screen_height
    };
}

float hud_smooth_step(float t)
{
    // Smooth hermite interpolation
    t = fmaxf(0.0f, fminf(1.0f, t));
    return t * t * (3.0f - 2.0f * t);
}

// ============================================================================
// COMPONENT RENDERERS (Placeholder implementations)
// ============================================================================

void hud_render_targeting_reticle(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world; // Not used in this implementation
    (void)pos; // Use input-based positioning instead
    (void)size; // Use input-based positioning instead
    
    const InputState* input = input_get_state();
    if (!input) return;
    
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    
    // Calculate reticle position from look target
    float screen_width = (float)sapp_width();
    float screen_height = (float)sapp_height();
    
    float reticle_x = screen_width * 0.5f;
    float reticle_y = screen_height * 0.5f;
    
    // Apply look target offset
    if (input->look_target.distance > 0.0f) {
        float offset_x = sinf(input->look_target.azimuth) * 100.0f;
        float offset_y = -sinf(input->look_target.elevation) * 100.0f;
        
        reticle_x = screen_width * 0.5f + offset_x;
        reticle_y = screen_height * 0.5f + offset_y;
        
        // Clamp to screen bounds
        reticle_x = fmaxf(20.0f, fminf(screen_width - 20.0f, reticle_x));
        reticle_y = fmaxf(20.0f, fminf(screen_height - 20.0f, reticle_y));
    }
    
    // Apply opacity to colors
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color reticle_color = nk_rgba(255, 100, 100, alpha);
    
    // Main crosshair
    float reticle_size = 15.0f;
    nk_stroke_line(canvas, 
                  reticle_x - reticle_size, reticle_y,
                  reticle_x + reticle_size, reticle_y,
                  3.0f, reticle_color);
    nk_stroke_line(canvas,
                  reticle_x, reticle_y - reticle_size,
                  reticle_x, reticle_y + reticle_size,
                  3.0f, reticle_color);
    
    // Outer circle for targeting feedback
    if (input->thrust > 0.0f) {
        struct nk_color thrust_color = nk_rgba(100, 255, 100, alpha);
        nk_stroke_circle(canvas, nk_rect(reticle_x - 20, reticle_y - 20, 40, 40), 2.0f, thrust_color);
    } else {
        nk_stroke_circle(canvas, nk_rect(reticle_x - 15, reticle_y - 15, 30, 30), 1.0f, reticle_color);
    }
}

void hud_render_speedometer(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world; // TODO: Get ship velocity
    
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    
    // Apply opacity
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color text_color = nk_rgba(100, 255, 100, alpha);
    struct nk_color bg_color = nk_rgba(0, 0, 0, (uint8_t)(128 * opacity));
    
    // Background
    nk_fill_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 4.0f, bg_color);
    
    // Speed text (placeholder) - using canvas text overlay
    const char* speed_text = "SPEED: 150 m/s";
    struct nk_rect text_rect = nk_rect(pos.u + 5, pos.v + 5, size.u - 10, 20);
    
    // For now, we'll draw a simple filled rect as text background
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(0, 80, 0, (uint8_t)(128 * opacity)));
    (void)speed_text; // Suppress unused warning - will implement text later
}

void hud_render_thrust_indicator(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    
    const InputState* input = input_get_state();
    if (!input) return;
    
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    
    // Apply opacity
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color text_color = nk_rgba(255, 200, 100, alpha);
    struct nk_color bg_color = nk_rgba(0, 0, 0, (uint8_t)(128 * opacity));
    
    // Background
    nk_fill_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 4.0f, bg_color);
    
    // Thrust percentage - using canvas overlay
    struct nk_rect text_rect = nk_rect(pos.u + 5, pos.v + 5, size.u - 10, 20);
    
    // Draw colored indicator bar based on thrust level
    float thrust_width = (size.u - 20) * input->thrust;
    struct nk_rect thrust_bar = nk_rect(pos.u + 10, pos.v + size.v - 15, thrust_width, 8);
    nk_fill_rect(canvas, thrust_bar, 2.0f, text_color);
    
    // For now, we'll draw a simple filled rect as text background
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(80, 40, 0, (uint8_t)(128 * opacity)));
}

// Placeholder implementations for other components
void hud_render_instrument_panel(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(100, 150, 255, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_communications(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(150, 255, 150, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_trade_interface(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(255, 255, 100, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_waypoint_display(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(255, 150, 255, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_tactical_overlay(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(255, 100, 100, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_proximity_warnings(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(255, 150, 50, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_weapon_systems(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(255, 50, 50, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}

void hud_render_exploration_scanner(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity)
{
    (void)world;
    struct nk_command_buffer* canvas = nk_window_get_canvas(ctx);
    uint8_t alpha = (uint8_t)(255 * opacity);
    struct nk_color color = nk_rgba(100, 255, 255, alpha);
    nk_stroke_rect(canvas, nk_rect(pos.u, pos.v, size.u, size.v), 2.0f, 2.0f, color);
    
    // Simple text indicator - draw colored rect for now
    struct nk_rect text_rect = nk_rect(pos.u + 10, pos.v + 10, size.u - 20, 20);
    nk_fill_rect(canvas, text_rect, 2.0f, nk_rgba(color.r/4, color.g/4, color.b/4, alpha));
}