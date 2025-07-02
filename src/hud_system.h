#ifndef HUD_SYSTEM_H
#define HUD_SYSTEM_H

#include "core.h"
#include "component/look_target.h"

// Forward declarations
struct nk_context;
struct World;

// ============================================================================
// HUD COMPONENT SYSTEM
// ============================================================================

// HUD component types
typedef enum {
    HUD_COMPONENT_TARGETING_RETICLE = 0,
    HUD_COMPONENT_SPEEDOMETER,
    HUD_COMPONENT_THRUST_INDICATOR,
    HUD_COMPONENT_INSTRUMENT_PANEL,
    HUD_COMPONENT_COMMUNICATIONS,
    HUD_COMPONENT_TRADE_INTERFACE,
    HUD_COMPONENT_WAYPOINT_DISPLAY,
    HUD_COMPONENT_TACTICAL_OVERLAY,
    HUD_COMPONENT_PROXIMITY_WARNINGS,
    HUD_COMPONENT_WEAPON_SYSTEMS,
    HUD_COMPONENT_EXPLORATION_SCANNER,
    HUD_COMPONENT_COUNT
} HUDComponentType;

// Camera modes that determine which HUD components are visible
typedef enum {
    HUD_CAMERA_MODE_COCKPIT = 0,     // First-person with full instrument panel
    HUD_CAMERA_MODE_CHASE_NEAR = 1,  // Close chase with racing HUD
    HUD_CAMERA_MODE_CHASE_FAR = 2,   // Far chase with tactical HUD
    HUD_CAMERA_MODE_COUNT = 3
} HUDCameraMode;

// HUD component configuration
typedef struct {
    bool visible;                    // Is this component currently visible?
    Vector2 position;               // Screen position (0-1 normalized coordinates)
    Vector2 size;                   // Component size (0-1 normalized coordinates)
    float opacity;                  // Component opacity (0-1)
    HUDCameraMode valid_modes[HUD_CAMERA_MODE_COUNT]; // Which camera modes show this component
    int valid_mode_count;           // Number of valid camera modes
    void (*render_func)(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
} HUDComponent;

// Main HUD system state
typedef struct {
    HUDComponent components[HUD_COMPONENT_COUNT];
    HUDCameraMode current_mode;
    bool initialized;
    float transition_time;          // Time for smooth transitions between modes
    float transition_progress;      // Current transition progress (0-1)
    HUDCameraMode transition_from;  // Mode we're transitioning from
    HUDCameraMode transition_to;    // Mode we're transitioning to
} HUDSystem;

// ============================================================================
// HUD SYSTEM API
// ============================================================================

// Initialize the HUD system
void hud_system_init(void);

// Shutdown the HUD system
void hud_system_shutdown(void);

// Update HUD system (handle transitions, animations)
void hud_system_update(float delta_time);

// Render the current HUD based on camera mode
void hud_system_render(struct nk_context* ctx, struct World* world);

// Switch to a different camera mode with smooth transition
void hud_system_set_camera_mode(HUDCameraMode mode);

// Get current camera mode
HUDCameraMode hud_system_get_camera_mode(void);

// Configure which components are visible in each camera mode
void hud_system_configure_mode(HUDCameraMode mode, HUDComponentType* components, int component_count);

// Set component properties
void hud_system_set_component_position(HUDComponentType type, Vector2 position);
void hud_system_set_component_size(HUDComponentType type, Vector2 size);
void hud_system_set_component_opacity(HUDComponentType type, float opacity);

// ============================================================================
// HUD COMPONENT RENDERERS
// ============================================================================

// Individual component render functions
void hud_render_targeting_reticle(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_speedometer(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_thrust_indicator(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_instrument_panel(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_communications(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_trade_interface(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_waypoint_display(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_tactical_overlay(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_proximity_warnings(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_weapon_systems(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);
void hud_render_exploration_scanner(struct nk_context* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert normalized coordinates to screen coordinates
Vector2 hud_normalized_to_screen(Vector2 normalized_pos, float screen_width, float screen_height);

// Convert normalized size to screen size
Vector2 hud_normalized_size_to_screen(Vector2 normalized_size, float screen_width, float screen_height);

// Smooth interpolation for transitions
float hud_smooth_step(float t);

#endif // HUD_SYSTEM_H