#ifndef SYSTEM_CONTROL_H
#define SYSTEM_CONTROL_H

#include "../core.h"
#include "../render.h"

// Control system update - processes input and converts to thruster commands
void control_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Control configuration functions
void control_set_player_entity(struct World* world, EntityID player_entity);
void control_set_sensitivity(struct ControlAuthority* control, float sensitivity);
void control_set_flight_assist(struct ControlAuthority* control, bool enabled);
void control_toggle_flight_assist(struct ControlAuthority* control);

// Flight assistance functions
Vector3 apply_stability_assist(Vector3 input, Vector3 current_angular_velocity, float assist_strength);
Vector3 apply_sensitivity_curve(Vector3 input, float sensitivity);

// Ship configuration presets
typedef enum {
    SHIP_CONFIG_FIGHTER,      // Fast, agile fighter
    SHIP_CONFIG_RACER,        // Canyon racing optimized
    SHIP_CONFIG_FREIGHTER,    // Heavy cargo ship
    SHIP_CONFIG_RC_ROCKET     // RC model rocket (test)
} ShipConfigPreset;

// Unified ship control configuration
void control_configure_ship(struct World* world, EntityID ship_id, ShipConfigPreset preset);

#endif // SYSTEM_CONTROL_H