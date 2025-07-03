#ifndef SCRIPTED_FLIGHT_H
#define SCRIPTED_FLIGHT_H

#include "../core.h"
#include "../render.h"

// Maximum waypoints in a flight path
#define MAX_WAYPOINTS 16

// Waypoint types
typedef enum {
    WAYPOINT_POSITION,    // Just reach this position
    WAYPOINT_PASS_THROUGH, // Pass through with specific speed
    WAYPOINT_HOVER,       // Hover at position for duration
    WAYPOINT_LOOP_START   // Marks start of loop
} WaypointType;

// Individual waypoint
typedef struct {
    Vector3 position;
    WaypointType type;
    float target_speed;     // Desired speed when reaching waypoint
    float hover_duration;   // For WAYPOINT_HOVER type
    float tolerance;        // Distance tolerance to consider waypoint reached
} Waypoint;

// Flight path definition
typedef struct {
    Waypoint waypoints[MAX_WAYPOINTS];
    int waypoint_count;
    bool loop;              // Whether to loop back to start
    float default_speed;    // Default flight speed
    float max_acceleration; // Maximum acceleration
    float max_turn_rate;    // Maximum turn rate (radians/second)
} FlightPath;

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

// Debug visualization
void scripted_flight_debug_render(struct World* world, const ScriptedFlight* flight);

#endif // SCRIPTED_FLIGHT_H