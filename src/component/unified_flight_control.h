/**
 * @file unified_flight_control.h
 * @brief Unified flight control component - consolidates ControlAuthority, Controllable, and ScriptedFlight
 * 
 * This component provides a single, comprehensive control system that supports:
 * - Manual control (direct input)
 * - Assisted control (flight computer assistance)
 * - Scripted flight (automated paths)
 * - Autonomous flight (AI control) - ready for Sprint 26
 */

#ifndef UNIFIED_FLIGHT_CONTROL_H
#define UNIFIED_FLIGHT_CONTROL_H

#include "../core.h"
#include "../services/input_service.h"

// Forward declarations
struct World;

// Maximum waypoints in a flight path
#define MAX_FLIGHT_WAYPOINTS 16

// Control modes
typedef enum {
    FLIGHT_CONTROL_MANUAL,      // Direct manual control (Sprint 25)
    FLIGHT_CONTROL_ASSISTED,    // Flight computer assisted (Sprint 26)
    FLIGHT_CONTROL_SCRIPTED,    // Following predefined path
    FLIGHT_CONTROL_AUTONOMOUS,  // AI-controlled (Sprint 26+)
    FLIGHT_CONTROL_FORMATION    // Formation flying (future)
} FlightControlMode;

// Control authority levels (for conflict resolution)
typedef enum {
    AUTHORITY_PLAYER = 100,     // Player always has highest authority
    AUTHORITY_ASSISTANT = 80,   // Flight computer assistance
    AUTHORITY_SCRIPT = 60,      // Scripted paths
    AUTHORITY_AI = 40,          // AI control
    AUTHORITY_NONE = 0          // No control
} ControlAuthority;

// Waypoint types for scripted flight
typedef enum {
    WAYPOINT_POSITION,          // Just reach this position
    WAYPOINT_PASS_THROUGH,      // Pass through with specific speed
    WAYPOINT_HOVER,             // Hover at position for duration
    WAYPOINT_LOOP_START         // Marks start of loop
} WaypointType;

// Individual waypoint
typedef struct {
    Vector3 position;
    WaypointType type;
    float target_speed;         // Desired speed when reaching waypoint
    float hover_duration;       // For WAYPOINT_HOVER type
    float tolerance;            // Distance tolerance to consider waypoint reached
} FlightWaypoint;

// Flight path definition
typedef struct {
    FlightWaypoint waypoints[MAX_FLIGHT_WAYPOINTS];
    int waypoint_count;
    bool loop;                  // Whether to loop back to start
    float default_speed;        // Default flight speed
    float max_acceleration;     // Maximum acceleration
    float max_turn_rate;        // Maximum turn rate (radians/second)
} FlightPath;

// Input configuration for manual control
typedef struct {
    // Action mappings
    InputActionID thrust_forward;
    InputActionID thrust_back;
    InputActionID pitch_up;
    InputActionID pitch_down;
    InputActionID yaw_left;
    InputActionID yaw_right;
    InputActionID roll_left;
    InputActionID roll_right;
    InputActionID boost;
    InputActionID brake;
    
    // Sensitivity settings
    float linear_sensitivity;
    float angular_sensitivity;
    float dead_zone;
    
    // Response curves
    bool use_quadratic_curve;
    bool invert_pitch;
    bool invert_yaw;
} InputConfiguration;

// Control state and commands
typedef struct {
    // Current input values (processed)
    Vector3 linear_input;       // X=strafe, Y=vertical, Z=thrust
    Vector3 angular_input;      // X=pitch, Y=yaw, Z=roll
    float boost_input;          // 0-1 boost intensity
    float brake_input;          // 0-1 brake intensity
    
    // Flight computer assistance (Sprint 26)
    Vector3 target_position;    // Where we want to go
    Vector3 target_velocity;    // How fast we want to go there
    float assistance_level;     // 0=manual, 1=full assistance
    
    // Control limits and safety
    float max_linear_acceleration;
    float max_angular_acceleration;
    float g_force_limit;        // Prevent pilot blackout
    bool collision_avoidance;   // Enable collision avoidance
} ControlState;

// Scripted flight state
typedef struct {
    FlightPath path;
    int current_waypoint;       // Current target waypoint
    float state_timer;          // Timer for current state
    bool active;                // Whether scripted flight is active
    bool manual_override;       // Whether manual control is overriding
    
    // Flight dynamics
    Vector3 target_velocity;
    Vector3 last_position;
    float current_speed;
    float hover_timer;          // For hovering waypoints
    
    // Path following parameters
    float path_following_strength;
    float speed_control_strength;
} ScriptedFlightState;

// Main unified flight control component
typedef struct UnifiedFlightControl {
    // Core control settings
    FlightControlMode mode;
    ControlAuthority authority_level;
    EntityID controlled_by;     // Who controls this entity
    bool enabled;               // Is control enabled
    
    // Input configuration
    InputConfiguration input_config;
    
    // Current control state
    ControlState state;
    
    // Scripted flight (when mode == FLIGHT_CONTROL_SCRIPTED)
    ScriptedFlightState scripted;
    
    // Flight assistance settings
    float stability_assist;     // 0-1 stability assistance strength
    float inertia_dampening;    // 0-1 automatic counter-thrust
    bool flight_assist_enabled; // Enable flight assistance systems
    
    // Performance monitoring
    float last_update_time;
    float computation_time_ms;  // For performance tracking
    uint32_t update_count;
    
    // Ship-specific parameters (for tuning)
    float mass_scaling;
    float thruster_efficiency;
    Vector3 moment_of_inertia;
    
    // Future: Autonomous flight data (Sprint 26)
    void* autonomous_data;      // Opaque pointer for AI/autonomous systems
} UnifiedFlightControl;

// Component management functions
UnifiedFlightControl* unified_flight_control_create(void);
void unified_flight_control_destroy(UnifiedFlightControl* control);
void unified_flight_control_reset(UnifiedFlightControl* control);

// Mode management
void unified_flight_control_set_mode(UnifiedFlightControl* control, FlightControlMode mode);
FlightControlMode unified_flight_control_get_mode(const UnifiedFlightControl* control);
bool unified_flight_control_can_switch_mode(const UnifiedFlightControl* control, FlightControlMode new_mode);

// Manual control configuration
void unified_flight_control_configure_input(UnifiedFlightControl* control, const InputConfiguration* config);
void unified_flight_control_set_sensitivity(UnifiedFlightControl* control, float linear, float angular);
void unified_flight_control_set_assistance(UnifiedFlightControl* control, float stability, float inertia);

// Scripted flight functions
void unified_flight_control_load_path(UnifiedFlightControl* control, const FlightPath* path);
void unified_flight_control_start_script(UnifiedFlightControl* control);
void unified_flight_control_stop_script(UnifiedFlightControl* control);
bool unified_flight_control_is_script_active(const UnifiedFlightControl* control);

// Control state access
const ControlState* unified_flight_control_get_state(const UnifiedFlightControl* control);
Vector3 unified_flight_control_get_linear_command(const UnifiedFlightControl* control);
Vector3 unified_flight_control_get_angular_command(const UnifiedFlightControl* control);

// Authority management
void unified_flight_control_request_authority(UnifiedFlightControl* control, ControlAuthority level, EntityID requester);
void unified_flight_control_release_authority(UnifiedFlightControl* control, EntityID releaser);
bool unified_flight_control_has_authority(const UnifiedFlightControl* control, EntityID entity);

// System integration
void unified_flight_control_update(UnifiedFlightControl* control, float delta_time);
void unified_flight_control_process_input(UnifiedFlightControl* control, const InputService* input_service);

// Preset configurations
void unified_flight_control_setup_manual_flight(UnifiedFlightControl* control);
void unified_flight_control_setup_assisted_flight(UnifiedFlightControl* control);
void unified_flight_control_setup_autonomous_flight(UnifiedFlightControl* control);

// Compatibility functions (for migration from old components)
void unified_flight_control_migrate_from_control_authority(UnifiedFlightControl* unified, const void* old_control);
void unified_flight_control_migrate_from_controllable(UnifiedFlightControl* unified, const void* old_controllable);
void unified_flight_control_migrate_from_scripted_flight(UnifiedFlightControl* unified, const void* old_scripted);

#endif // UNIFIED_FLIGHT_CONTROL_H