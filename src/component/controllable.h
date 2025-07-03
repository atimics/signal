// src/component/controllable.h
// Controllable component - allows entities to receive input and convert to forces/movement

#ifndef COMPONENT_CONTROLLABLE_H
#define COMPONENT_CONTROLLABLE_H

#include "../core.h"
#include "../system/input_mapping.h"
#include <stdbool.h>

// Forward declarations
struct World;

// Input scheme types
typedef enum {
    INPUT_SCHEME_SPACESHIP_6DOF,    // Full 6DOF spaceship controls
    INPUT_SCHEME_SPACESHIP_ARCADE,  // Simplified arcade spaceship
    INPUT_SCHEME_FPS_CAMERA,        // First person camera
    INPUT_SCHEME_ORBIT_CAMERA,      // Orbiting third person camera
    INPUT_SCHEME_RTS_CAMERA,        // Strategy game camera
    INPUT_SCHEME_VEHICLE_CAR,       // Ground vehicle
    INPUT_SCHEME_CUSTOM             // Custom scheme
} InputSchemeType;

// Input-to-force mapping
typedef struct {
    // Linear force mappings (normalized -1 to 1)
    float forward_backward;   // Z axis
    float left_right;        // X axis  
    float up_down;           // Y axis
    
    // Angular torque mappings (normalized -1 to 1)
    float pitch;             // X rotation
    float yaw;               // Y rotation
    float roll;              // Z rotation
    
    // Special actions
    float boost;             // 0-1 multiplier
    float brake;             // 0-1 multiplier
    bool stabilize;          // Auto-stabilization
    
    // Camera-specific (for camera entities)
    float look_x;            // Mouse/stick look X
    float look_y;            // Mouse/stick look Y
    float zoom;              // Zoom in/out
} InputForceMapping;

// Response curves for input
typedef enum {
    RESPONSE_LINEAR,         // Direct 1:1 mapping
    RESPONSE_QUADRATIC,      // x^2 for fine control
    RESPONSE_CUBIC,          // x^3 for very fine control
    RESPONSE_EXPONENTIAL,    // Exponential curve
    RESPONSE_CUSTOM          // Custom curve function
} ResponseCurveType;

// Axis configuration
typedef struct {
    InputAction action;          // Which input action drives this axis
    float sensitivity;           // Sensitivity multiplier
    float dead_zone;            // Dead zone threshold
    bool inverted;              // Invert axis
    ResponseCurveType curve;    // Response curve type
    float max_value;            // Maximum output value
} AxisConfig;

// Controllable component
typedef struct Controllable {
    // Input configuration
    InputSchemeType scheme_type;
    bool enabled;               // Is input enabled for this entity?
    uint32_t player_id;         // Which player controls this (0 = player 1)
    
    // Axis configurations
    AxisConfig forward_axis;
    AxisConfig strafe_axis;
    AxisConfig vertical_axis;
    AxisConfig pitch_axis;
    AxisConfig yaw_axis;
    AxisConfig roll_axis;
    
    // Action mappings
    InputAction boost_action;
    InputAction brake_action;
    InputAction stabilize_action;
    InputAction primary_action;
    InputAction secondary_action;
    
    // Force multipliers (entity-specific tuning)
    float linear_force_scale;    // Overall linear force multiplier
    float angular_force_scale;   // Overall angular force multiplier
    float boost_multiplier;      // Boost force multiplier
    float brake_multiplier;      // Brake force multiplier
    
    // Current input state
    InputForceMapping current_input;
    InputForceMapping previous_input;
    
    // Input smoothing
    float input_smoothing;       // 0 = no smoothing, 1 = max smoothing
    
    // Custom input processing function (optional)
    void (*process_input)(struct Controllable* controllable, float delta_time);
} Controllable;

// Component functions
Controllable* controllable_create(void);
void controllable_destroy(Controllable* controllable);
void controllable_reset(Controllable* controllable);

// Configuration
void controllable_set_scheme(Controllable* controllable, InputSchemeType scheme);
void controllable_configure_axis(Controllable* controllable, const char* axis_name, 
                               InputAction action, float sensitivity, bool inverted);
void controllable_set_force_scales(Controllable* controllable, float linear, float angular);

// Input processing
void controllable_update_input(Controllable* controllable, float delta_time);
InputForceMapping controllable_get_input_forces(const Controllable* controllable);

// Preset configurations
void controllable_setup_spaceship_6dof(Controllable* controllable);
void controllable_setup_spaceship_arcade(Controllable* controllable);
void controllable_setup_fps_camera(Controllable* controllable);
void controllable_setup_orbit_camera(Controllable* controllable);

// Helper functions
float controllable_apply_response_curve(float input, ResponseCurveType curve);
float controllable_apply_dead_zone(float input, float dead_zone);

// Component registration
void register_controllable_component(struct World* world);

#endif // COMPONENT_CONTROLLABLE_H