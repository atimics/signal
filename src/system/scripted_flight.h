#ifndef SCRIPTED_FLIGHT_H
#define SCRIPTED_FLIGHT_H

#include "../core.h"
#include "../render.h"
#include "../component/unified_flight_control.h"

// Use waypoint definitions from unified_flight_control.h
// Legacy compatibility typedefs
typedef FlightWaypoint Waypoint;

// Maximum waypoints compatibility
#define MAX_WAYPOINTS MAX_FLIGHT_WAYPOINTS

// Scripted flight state for an entity
typedef struct {
    FlightPath path;
    int current_waypoint;   // Current target waypoint
    float state_timer;      // Timer for current state
    bool active;            // Whether scripted flight is active
    bool manual_override;   // Whether manual control is active
    
    // Flight dynamics
    Vector3 target_velocity;
    Vector3 last_position;
    float current_speed;
    
    // Hover state
    float hover_start_time;
} ScriptedFlight;

// Flight path creation helpers
FlightPath* scripted_flight_create_circuit_path(void);
FlightPath* scripted_flight_create_figure_eight_path(void);
FlightPath* scripted_flight_create_landing_approach_path(Vector3 start_pos, Vector3 landing_pos);

// Component management
ScriptedFlight* scripted_flight_create_component(EntityID entity_id);
void scripted_flight_destroy_component(ScriptedFlight* component);

// Flight control
void scripted_flight_start(ScriptedFlight* flight, const FlightPath* path);
void scripted_flight_stop(ScriptedFlight* flight);
void scripted_flight_pause(ScriptedFlight* flight);
void scripted_flight_resume(ScriptedFlight* flight);

// System update
void scripted_flight_update(struct World* world, RenderConfig* render_config, float delta_time);

#endif // SCRIPTED_FLIGHT_H