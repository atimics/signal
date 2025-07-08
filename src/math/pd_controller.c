#include "pd_controller.h"
#include <math.h>

void pd_controller_3d_init(PDController3D* controller, float kp, float kd, float max_output) {
    controller->kp = kp;
    controller->kd = kd;
    controller->max_output = max_output;
    controller->smoothing_factor = 0.0f;
    controller->last_output = (Vector3){0, 0, 0};
    controller->initialized = false;
}

Vector3 pd_controller_3d_update(PDController3D* controller,
                               Vector3 current_position,
                               Vector3 target_position,
                               Vector3 current_velocity,
                               Vector3 target_velocity,
                               float dt) {
    (void)dt;  // Currently unused, but available for future time-based features
    // Calculate position error
    Vector3 position_error = vector3_subtract(target_position, current_position);
    
    // Calculate velocity error (target velocity - current velocity)
    Vector3 velocity_error = vector3_subtract(target_velocity, current_velocity);
    
    // PD control law: output = Kp * position_error + Kd * velocity_error
    Vector3 p_term = vector3_multiply(position_error, controller->kp);
    Vector3 d_term = vector3_multiply(velocity_error, controller->kd);
    
    Vector3 output = vector3_add(p_term, d_term);
    
    // Clamp output magnitude
    float magnitude = vector3_length(output);
    if (magnitude > controller->max_output && magnitude > 0.0001f) {
        output = vector3_multiply(output, controller->max_output / magnitude);
    }
    
    // Apply smoothing if enabled
    if (controller->smoothing_factor > 0.0f && controller->initialized) {
        output = vector3_lerp(controller->last_output, output, 1.0f - controller->smoothing_factor);
    }
    
    controller->last_output = output;
    controller->initialized = true;
    
    return output;
}

void pd_controller_3d_reset(PDController3D* controller) {
    controller->last_output = (Vector3){0, 0, 0};
    controller->initialized = false;
}

void pd_controller_3d_set_smoothing(PDController3D* controller, float smoothing) {
    controller->smoothing_factor = fmaxf(0.0f, fminf(1.0f, smoothing));
}

void pd_controller_1d_init(PDController1D* controller, float kp, float kd, float max_output) {
    controller->kp = kp;
    controller->kd = kd;
    controller->max_output = max_output;
    controller->smoothing_factor = 0.0f;
    controller->last_output = 0.0f;
    controller->initialized = false;
}

float pd_controller_1d_update(PDController1D* controller,
                             float current_value,
                             float target_value,
                             float current_rate,
                             float target_rate,
                             float dt) {
    (void)dt;  // Currently unused, but available for future time-based features
    
    // Calculate errors
    float position_error = target_value - current_value;
    float rate_error = target_rate - current_rate;
    
    // PD control law
    float output = controller->kp * position_error + controller->kd * rate_error;
    
    // Clamp output
    output = fmaxf(-controller->max_output, fminf(controller->max_output, output));
    
    // Apply smoothing if enabled
    if (controller->smoothing_factor > 0.0f && controller->initialized) {
        output = controller->last_output + (output - controller->last_output) * (1.0f - controller->smoothing_factor);
    }
    
    controller->last_output = output;
    controller->initialized = true;
    
    return output;
}

void pd_controller_1d_reset(PDController1D* controller) {
    controller->last_output = 0.0f;
    controller->initialized = false;
}

float pd_controller_calculate_critical_damping(float natural_frequency, float mass) {
    // Critical damping coefficient: c = 2 * sqrt(k * m)
    // Where k = m * wn^2 (spring constant from natural frequency)
    // So c = 2 * m * wn
    return 2.0f * mass * natural_frequency;
}

void pd_controller_auto_tune(float* out_kp, float* out_kd,
                            float desired_response_time,
                            float damping_ratio,
                            float system_mass) {
    // Natural frequency based on desired response time
    // For a critically damped system, settling time ≈ 4.6 / wn
    // So wn ≈ 4.6 / desired_response_time
    float natural_frequency = 4.6f / desired_response_time;
    
    // Calculate gains
    // For a second-order system: Kp = m * wn^2
    // Kd = 2 * zeta * m * wn (where zeta is damping ratio)
    *out_kp = system_mass * natural_frequency * natural_frequency;
    *out_kd = 2.0f * damping_ratio * system_mass * natural_frequency;
}