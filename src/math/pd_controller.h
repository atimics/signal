#ifndef PD_CONTROLLER_H
#define PD_CONTROLLER_H

#include "../core.h"

// PD Controller for 3D position and velocity control
// Used for flight assist, autopilot, and AI navigation
typedef struct {
    float kp;                    // Proportional gain (position error)
    float kd;                    // Derivative gain (velocity error)
    float max_output;            // Maximum control output magnitude
    float smoothing_factor;      // Optional output smoothing (0.0 = none, 1.0 = max)
    
    // Internal state for smoothing
    Vector3 last_output;
    bool initialized;
} PDController3D;

// Single-axis PD controller for specific applications
typedef struct {
    float kp;
    float kd;
    float max_output;
    float smoothing_factor;
    
    float last_output;
    bool initialized;
} PDController1D;

// Initialize a 3D PD controller
void pd_controller_3d_init(PDController3D* controller, float kp, float kd, float max_output);

// Update the controller and get control output
// Returns the control force/acceleration to apply
Vector3 pd_controller_3d_update(PDController3D* controller,
                               Vector3 current_position,
                               Vector3 target_position,
                               Vector3 current_velocity,
                               Vector3 target_velocity,
                               float dt);

// Reset controller state (useful when switching modes)
void pd_controller_3d_reset(PDController3D* controller);

// Set smoothing factor (0.0 to 1.0)
void pd_controller_3d_set_smoothing(PDController3D* controller, float smoothing);

// 1D versions for single-axis control (e.g., altitude hold)
void pd_controller_1d_init(PDController1D* controller, float kp, float kd, float max_output);
float pd_controller_1d_update(PDController1D* controller,
                             float current_value,
                             float target_value,
                             float current_rate,
                             float target_rate,
                             float dt);
void pd_controller_1d_reset(PDController1D* controller);

// Utility functions for tuning
// Calculate critical damping coefficient for given natural frequency
float pd_controller_calculate_critical_damping(float natural_frequency, float mass);

// Auto-tune gains based on desired response time and damping ratio
void pd_controller_auto_tune(float* out_kp, float* out_kd,
                            float desired_response_time,
                            float damping_ratio,  // 0.7-1.0 for good response
                            float system_mass);

#endif // PD_CONTROLLER_H