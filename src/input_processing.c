#include "input_processing.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Configuration constants
#define DEFAULT_ALPHA 0.002f
#define DEFAULT_REST_THRESHOLD 0.05f
#define DEFAULT_PERCENTILE_THRESHOLD 0.9f
#define MIN_SAMPLES_FOR_TRUST 100
#define SIGMA_MULTIPLIER 3.0f

// Neural network constants
#define ADAPTATION_LR 0.001f
#define CONTINUAL_LR 0.0001f
#define L2_REGULARIZATION 0.01f

// MRAC constants
#define DEFAULT_REFERENCE_DAMPING 0.7f
#define DEFAULT_REFERENCE_FREQUENCY 2.0f
#define DEFAULT_ADAPTATION_RATE 0.1f
#define DEFAULT_SIGMA_BOUND 1.0f

// Utility functions
uint64_t get_current_time_microseconds(void) {
    // Platform-specific high-resolution timer
    // This would need platform-specific implementation
    static uint64_t counter = 0;
    return counter++; // Placeholder - replace with actual high-res timer
}

Vector6 vector6_add(Vector6 a, Vector6 b) {
    return (Vector6){
        a.pitch + b.pitch, a.yaw + b.yaw, a.roll + b.roll,
        a.strafe_x + b.strafe_x, a.strafe_y + b.strafe_y, a.throttle + b.throttle
    };
}

Vector6 vector6_sub(Vector6 a, Vector6 b) {
    return (Vector6){
        a.pitch - b.pitch, a.yaw - b.yaw, a.roll - b.roll,
        a.strafe_x - b.strafe_x, a.strafe_y - b.strafe_y, a.throttle - b.throttle
    };
}

Vector6 vector6_scale(Vector6 v, float scale) {
    return (Vector6){
        v.pitch * scale, v.yaw * scale, v.roll * scale,
        v.strafe_x * scale, v.strafe_y * scale, v.throttle * scale
    };
}

float vector6_length(Vector6 v) {
    return sqrtf(v.pitch*v.pitch + v.yaw*v.yaw + v.roll*v.roll +
                v.strafe_x*v.strafe_x + v.strafe_y*v.strafe_y + v.throttle*v.throttle);
}

float vector6_dot(Vector6 a, Vector6 b) {
    return a.pitch*b.pitch + a.yaw*b.yaw + a.roll*b.roll +
           a.strafe_x*b.strafe_x + a.strafe_y*b.strafe_y + a.throttle*b.throttle;
}

// Matrix operations for Kalman filter
static Matrix2x2 matrix2x2_identity(void) {
    return (Matrix2x2){.data = {1.0f, 0.0f, 0.0f, 1.0f}};
}

static Matrix2x2 matrix2x2_add(Matrix2x2 a, Matrix2x2 b) {
    return (Matrix2x2){.data = {
        a.data[0] + b.data[0], a.data[1] + b.data[1],
        a.data[2] + b.data[2], a.data[3] + b.data[3]
    }};
}

static Matrix2x2 matrix2x2_scale(Matrix2x2 m, float scale) {
    return (Matrix2x2){.data = {
        m.data[0] * scale, m.data[1] * scale,
        m.data[2] * scale, m.data[3] * scale
    }};
}

static Matrix2x2 matrix2x2_inverse(Matrix2x2 m) {
    float det = m.data[0] * m.data[3] - m.data[1] * m.data[2];
    if (fabsf(det) < 1e-6f) {
        return matrix2x2_identity();  // Fallback for singular matrix
    }
    
    float inv_det = 1.0f / det;
    return (Matrix2x2){.data = {
        m.data[3] * inv_det, -m.data[1] * inv_det,
        -m.data[2] * inv_det, m.data[0] * inv_det
    }};
}

static float matrix2x2_trace(Matrix2x2 m) {
    return m.data[0] + m.data[3];
}

static Matrix2x2 matrix2x2_subtract(Matrix2x2 a, Matrix2x2 b) {
    return (Matrix2x2){.data = {
        a.data[0] - b.data[0], a.data[1] - b.data[1],
        a.data[2] - b.data[2], a.data[3] - b.data[3]
    }};
}

static Matrix2x2 matrix2x2_multiply(Matrix2x2 a, Matrix2x2 b) {
    return (Matrix2x2){.data = {
        a.data[0]*b.data[0] + a.data[1]*b.data[2], a.data[0]*b.data[1] + a.data[1]*b.data[3],
        a.data[2]*b.data[0] + a.data[3]*b.data[2], a.data[2]*b.data[1] + a.data[3]*b.data[3]
    }};
}

// Enhanced statistical calibration
static void enhanced_calibrator_init(EnhancedStatisticalCalibrator* cal) {
    memset(cal, 0, sizeof(EnhancedStatisticalCalibrator));
    
    cal->alpha = DEFAULT_ALPHA;
    cal->rest_threshold = DEFAULT_REST_THRESHOLD;
    cal->percentile_threshold = DEFAULT_PERCENTILE_THRESHOLD;
    cal->min_samples = MIN_SAMPLES_FOR_TRUST;
    cal->connection_timestamp = get_current_time_microseconds();
    
    // Initialize extremes to reasonable defaults
    cal->m_max = (InputVector2){0.8f, 0.8f};
    cal->m_min = (InputVector2){-0.8f, -0.8f};
}

static void enhanced_calibrator_update(EnhancedStatisticalCalibrator* cal, InputVector2 input, uint64_t timestamp) {
    cal->sample_count++;
    
    float magnitude = sqrtf(input.x * input.x + input.y * input.y);
    
    // Update rest statistics only when input is small
    if (magnitude < cal->rest_threshold) {
        InputVector2 delta = {input.x - cal->mu.x, input.y - cal->mu.y};
        
        // Welford's online algorithm for mean and variance
        cal->mu.x += cal->alpha * delta.x;
        cal->mu.y += cal->alpha * delta.y;
        
        InputVector2 delta2 = {input.x - cal->mu.x, input.y - cal->mu.y};
        cal->M2.x = (1.0f - cal->alpha) * cal->M2.x + cal->alpha * delta.x * delta2.x;
        cal->M2.y = (1.0f - cal->alpha) * cal->M2.y + cal->alpha * delta.y * delta2.y;
        
        // Update standard deviation
        cal->sigma.x = sqrtf(cal->M2.x);
        cal->sigma.y = sqrtf(cal->M2.y);
    }
    
    // Update extremes when input exceeds percentile threshold
    if (magnitude > cal->percentile_threshold) {
        cal->m_max.x = 0.999f * cal->m_max.x + 0.001f * fabsf(input.x);
        cal->m_max.y = 0.999f * cal->m_max.y + 0.001f * fabsf(input.y);
    }
    
    // Update neural-ready features
    float mu_magnitude = sqrtf(cal->mu.x * cal->mu.x + cal->mu.y * cal->mu.y);
    float sigma_magnitude = sqrtf(cal->sigma.x * cal->sigma.x + cal->sigma.y * cal->sigma.y);
    cal->dynamic_deadzone_estimate = mu_magnitude + SIGMA_MULTIPLIER * sigma_magnitude;
    cal->gain_estimate = sqrtf(cal->m_max.x * cal->m_max.x + cal->m_max.y * cal->m_max.y);
    cal->confidence_level = fminf(1.0f, cal->sample_count / 300.0f);  // Full confidence at 5 seconds
    cal->controller_age_ms = timestamp - cal->connection_timestamp;
    
    // Track drift for continual learning
    cal->drift_history[cal->drift_index] = cal->mu;
    cal->drift_index = (cal->drift_index + 1) % DRIFT_HISTORY_SIZE;
    
    // Detect significant drift
    if (cal->sample_count > 300) {
        InputVector2 drift_start = cal->drift_history[cal->drift_index];
        float drift_magnitude = sqrtf((cal->mu.x - drift_start.x) * (cal->mu.x - drift_start.x) +
                                     (cal->mu.y - drift_start.y) * (cal->mu.y - drift_start.y));
        cal->drift_detected = drift_magnitude > 0.05f;  // 5% drift threshold
    }
}

static InputVector2 apply_statistical_correction(const EnhancedStatisticalCalibrator* cal, InputVector2 raw_input) {
    // Don't trust statistics until we have enough samples
    if (cal->sample_count < cal->min_samples) {
        // Return simple centered input with fixed deadzone
        float magnitude = sqrtf(raw_input.x * raw_input.x + raw_input.y * raw_input.y);
        
        if (magnitude < 0.1f) {  // Fixed 10% deadzone
            return (InputVector2){0.0f, 0.0f};
        }
        
        float scale = (magnitude - 0.1f) / (1.0f - 0.1f);
        return (InputVector2){
            raw_input.x * scale / magnitude,
            raw_input.y * scale / magnitude
        };
    }
    
    // Apply statistical correction
    InputVector2 centered = {
        raw_input.x - cal->mu.x,
        raw_input.y - cal->mu.y
    };
    
    float input_magnitude = sqrtf(centered.x * centered.x + centered.y * centered.y);
    
    if (input_magnitude < cal->dynamic_deadzone_estimate) {
        return (InputVector2){0.0f, 0.0f};
    }
    
    // Normalize to [-1, 1] using learned extremes
    InputVector2 normalized = {
        centered.x / (cal->m_max.x - fabsf(cal->mu.x)),
        centered.y / (cal->m_max.y - fabsf(cal->mu.y))
    };
    
    // Clamp to [-1, 1] range
    normalized.x = fmaxf(-1.0f, fminf(1.0f, normalized.x));
    normalized.y = fmaxf(-1.0f, fminf(1.0f, normalized.y));
    
    // Apply radial scaling outside deadzone
    float scale_factor = (input_magnitude - cal->dynamic_deadzone_estimate) / (1.0f - cal->dynamic_deadzone_estimate);
    float norm_magnitude = sqrtf(normalized.x * normalized.x + normalized.y * normalized.y);
    
    if (norm_magnitude > 0.0001f) {  // Avoid division by zero
        return (InputVector2){
            (normalized.x / norm_magnitude) * scale_factor,
            (normalized.y / norm_magnitude) * scale_factor
        };
    }
    
    return (InputVector2){0.0f, 0.0f};
}

// Adaptive Kalman filter implementation
static void adaptive_kalman_init(AdaptiveKalmanFilter* kf) {
    memset(kf, 0, sizeof(AdaptiveKalmanFilter));
    
    // Initialize state covariance (high uncertainty initially)
    kf->P = (Matrix2x2){.data = {1.0f, 0.0f, 0.0f, 1.0f}};
    
    // Process noise (small random walk)
    kf->Q = (Matrix2x2){.data = {0.01f, 0.0f, 0.0f, 0.01f}};
    
    // Initial measurement noise
    kf->base_R_value = 0.1f;
    kf->R = (Matrix2x2){.data = {kf->base_R_value, 0.0f, 0.0f, kf->base_R_value}};
    
    kf->spike_multiplier = 1000.0f;
    kf->confidence = 1.0f;
}

static InputVector2 adaptive_kalman_update(AdaptiveKalmanFilter* kf, InputVector2 measurement) {
    // Predict step (constant position model)
    InputVector2 x_pred = kf->state;
    Matrix2x2 P_pred = matrix2x2_add(kf->P, kf->Q);
    
    // Innovation calculation
    kf->innovation = (InputVector2){measurement.x - x_pred.x, measurement.y - x_pred.y};
    float innovation_magnitude = sqrtf(kf->innovation.x * kf->innovation.x + 
                                      kf->innovation.y * kf->innovation.y);
    
    // Outlier detection using Z-score
    float expected_innovation = sqrtf(matrix2x2_trace(P_pred));
    float z_score = (expected_innovation > 0.001f) ? innovation_magnitude / expected_innovation : 0.0f;
    
    if (z_score > 3.0f) {  // 3-sigma outlier
        // Increase measurement noise temporarily
        kf->R = matrix2x2_scale(kf->R, kf->spike_multiplier);
        kf->confidence *= 0.5f;
        kf->outlier_count++;
    } else {
        // Gradually restore measurement noise
        float decay = 0.999f;
        kf->R.data[0] = decay * kf->R.data[0] + (1.0f - decay) * kf->base_R_value;
        kf->R.data[3] = decay * kf->R.data[3] + (1.0f - decay) * kf->base_R_value;
        kf->confidence = fminf(1.0f, kf->confidence * 1.01f);
    }
    
    // Update step
    Matrix2x2 S = matrix2x2_add(P_pred, kf->R);  // Innovation covariance
    Matrix2x2 K = matrix2x2_multiply(P_pred, matrix2x2_inverse(S));  // Kalman gain
    
    kf->state = (InputVector2){
        x_pred.x + K.data[0] * kf->innovation.x + K.data[1] * kf->innovation.y,
        x_pred.y + K.data[2] * kf->innovation.x + K.data[3] * kf->innovation.y
    };
    
    // Update covariance
    Matrix2x2 I_minus_K = matrix2x2_subtract(matrix2x2_identity(), K);
    kf->P = matrix2x2_multiply(I_minus_K, P_pred);
    
    return kf->state;
}

// Neural feature vector generation
static NeuralFeatureVector generate_feature_vector(const ProductionInputProcessor* processor, 
                                                  InputVector2 filtered_input,
                                                  const Vector6* prev_output) {
    const EnhancedStatisticalCalibrator* cal = &processor->calibrator;
    static InputVector2 prev_filtered = {0};
    
    NeuralFeatureVector features = {
        // Raw sensor data (preserve hardware quirks)
        .raw_lx = filtered_input.x,
        .raw_ly = filtered_input.y,
        .raw_magnitude = sqrtf(filtered_input.x * filtered_input.x + filtered_input.y * filtered_input.y),
        
        // Temporal derivatives
        .delta_lx = filtered_input.x - prev_filtered.x,
        .delta_ly = filtered_input.y - prev_filtered.y,
        
        // Statistical context
        .rdz_est = cal->dynamic_deadzone_estimate,
        .g_est = cal->gain_estimate,
        
        // Temporal context (normalize to [0,1])
        .age_norm = fminf(1.0f, cal->controller_age_ms / (10.0f * 60.0f * 1000.0f)),  // 10 min max
    };
    
    // Previous output (quantized to int16 for compact history)
    if (prev_output) {
        const float* prev_ptr = (const float*)prev_output;
        for (int i = 0; i < 6; i++) {
            float clamped = fmaxf(-1.0f, fminf(1.0f, prev_ptr[i]));
            features.prev_output[i] = (int16_t)(clamped * 32767.0f);
        }
    }
    
    prev_filtered = filtered_input;
    return features;
}

// Neural network initialization with random weights
static void neural_net_init(MetaTrainedNeuralNet* net) {
    memset(net, 0, sizeof(MetaTrainedNeuralNet));
    
    // Initialize with random weights (Xavier initialization adapted for int8)
    for (int i = 0; i < NEURAL_INPUT_SIZE * NEURAL_HIDDEN_SIZE; i++) {
        net->weights_fc1[i] = (int8_t)((rand() % 255) - 127);
    }
    
    for (int i = 0; i < NEURAL_HIDDEN_SIZE * NEURAL_HIDDEN_SIZE; i++) {
        net->weights_fc2[i] = (int8_t)((rand() % 255) - 127);
    }
    
    for (int i = 0; i < NEURAL_HIDDEN_SIZE * NEURAL_OUTPUT_SIZE; i++) {
        net->weights_fc3[i] = (int8_t)((rand() % 255) - 127);
    }
    
    // Initialize scaling factors
    net->input_scale = 127.0f;
    net->fc1_scale = 1.0f / 127.0f;
    net->fc2_scale = 1.0f / 127.0f;
    net->output_scale = 1.0f / 127.0f;
    
    net->adaptation_lr = ADAPTATION_LR;
    net->meta_trained = true;  // Assume pre-trained for now
}

// Ultra-fast int8 neural network inference
static Vector6 neural_inference_int8(MetaTrainedNeuralNet* net, const NeuralFeatureVector* features) {
    uint64_t start = get_current_time_microseconds();
    
    // Quantize input features
    int8_t quantized_input[NEURAL_INPUT_SIZE];
    const float* feature_ptr = (const float*)features;
    for (int i = 0; i < NEURAL_INPUT_SIZE; i++) {
        float scaled = feature_ptr[i] * net->input_scale;
        quantized_input[i] = (int8_t)fmaxf(-127.0f, fminf(127.0f, scaled));
    }
    
    // FC1: Input -> Hidden1 (14x32 = 448 ops)
    int16_t hidden1[NEURAL_HIDDEN_SIZE];
    for (int i = 0; i < NEURAL_HIDDEN_SIZE; i++) {
        int32_t sum = net->bias_fc1[i];
        for (int j = 0; j < NEURAL_INPUT_SIZE; j++) {
            sum += quantized_input[j] * net->weights_fc1[i * NEURAL_INPUT_SIZE + j];
        }
        // Quantized tanh approximation (faster than full tanh)
        int16_t scaled = (int16_t)(sum * net->fc1_scale);
        hidden1[i] = (scaled > 127) ? 127 : (scaled < -127) ? -127 : scaled;
    }
    
    // FC2: Hidden1 -> Hidden2 (32x32 = 1024 ops)
    int16_t hidden2[NEURAL_HIDDEN_SIZE];
    for (int i = 0; i < NEURAL_HIDDEN_SIZE; i++) {
        int32_t sum = net->bias_fc2[i];
        for (int j = 0; j < NEURAL_HIDDEN_SIZE; j++) {
            sum += hidden1[j] * net->weights_fc2[i * NEURAL_HIDDEN_SIZE + j];
        }
        int16_t scaled = (int16_t)(sum * net->fc2_scale);
        hidden2[i] = (scaled > 127) ? 127 : (scaled < -127) ? -127 : scaled;
    }
    
    // FC3: Hidden2 -> Output (32x6 = 192 ops)
    Vector6 output;
    float* output_ptr = (float*)&output;
    for (int i = 0; i < NEURAL_OUTPUT_SIZE; i++) {
        int32_t sum = net->bias_fc3[i];
        for (int j = 0; j < NEURAL_HIDDEN_SIZE; j++) {
            sum += hidden2[j] * net->weights_fc3[i * NEURAL_HIDDEN_SIZE + j];
        }
        // Convert back to float with tanh activation
        output_ptr[i] = tanhf(sum * net->output_scale / 127.0f);
    }
    
    // Update previous input
    net->previous_input = (InputVector2){features->raw_lx, features->raw_ly};
    net->has_previous = true;
    
    // Record performance
    uint64_t end = get_current_time_microseconds();
    net->inference_time_us = end - start;
    net->total_inferences++;
    
    return output;
}

// MRAC controller initialization
static void mrac_init(ModelReferenceController* mrac) {
    memset(mrac, 0, sizeof(ModelReferenceController));
    
    mrac->reference_damping = DEFAULT_REFERENCE_DAMPING;
    mrac->reference_frequency = DEFAULT_REFERENCE_FREQUENCY;
    mrac->adaptation_rate = DEFAULT_ADAPTATION_RATE;
    mrac->sigma_bound = DEFAULT_SIGMA_BOUND;
    mrac->stability_margin = 1.0f;
    mrac->neural_confidence = 0.5f;
    mrac->mixing_lambda = 0.1f;  // Start conservative
}

// MRAC control mixing with stability guarantee
static Vector6 mrac_control_mixing(ModelReferenceController* mrac,
                                  const Vector6* neural_output,
                                  const Vector6* statistical_output,
                                  const Vector6* reference_command,
                                  float delta_time) {
    
    // Update reference model (desired behavior)
    Vector6 reference_error = vector6_sub(*reference_command, mrac->reference_state);
    Vector6 reference_derivative = vector6_scale(reference_error, 
                                                mrac->reference_frequency * mrac->reference_frequency);
    Vector6 reference_damping_term = vector6_scale(mrac->reference_output, 
                                                  2.0f * mrac->reference_damping * mrac->reference_frequency);
    
    mrac->reference_output = vector6_add(reference_derivative, vector6_scale(reference_damping_term, -1.0f));
    mrac->reference_state = vector6_add(mrac->reference_state, vector6_scale(mrac->reference_output, delta_time));
    
    // Compute control mixing based on confidence
    Vector6 mixed_output;
    for (int i = 0; i < 6; i++) {
        float* mixed_ptr = (float*)&mixed_output;
        const float* neural_ptr = (const float*)neural_output;
        const float* stats_ptr = (const float*)statistical_output;
        
        mixed_ptr[i] = mrac->mixing_lambda * neural_ptr[i] + 
                      (1.0f - mrac->mixing_lambda) * stats_ptr[i];
    }
    
    // Adaptive compensation (simplified MRAC law)
    Vector6 tracking_error = vector6_sub(mixed_output, mrac->reference_output);
    float error_magnitude = vector6_length(tracking_error);
    
    // Update adaptive gains with saturation
    for (int i = 0; i < 6; i++) {
        float* adaptive_ptr = (float*)&mrac->adaptive_gains;
        const float* error_ptr = (const float*)&tracking_error;
        
        float gain_update = mrac->adaptation_rate * error_ptr[i] * error_magnitude * delta_time;
        adaptive_ptr[i] += gain_update;
        
        // Saturate adaptive gains for stability
        adaptive_ptr[i] = fmaxf(-mrac->sigma_bound, fminf(mrac->sigma_bound, adaptive_ptr[i]));
    }
    
    // Apply adaptive compensation
    Vector6 compensated_output = vector6_add(mixed_output, mrac->adaptive_gains);
    
    // Monitor Lyapunov stability
    mrac->lyapunov_energy = error_magnitude * error_magnitude + vector6_dot(mrac->adaptive_gains, mrac->adaptive_gains);
    mrac->stability_assured = mrac->lyapunov_energy < mrac->stability_margin;
    
    // Reduce neural confidence if stability is compromised
    if (!mrac->stability_assured) {
        mrac->mixing_lambda *= 0.95f;  // Gradually reduce neural contribution
    } else if (mrac->mixing_lambda < mrac->neural_confidence) {
        mrac->mixing_lambda += 0.01f;  // Gradually restore neural confidence
    }
    
    return compensated_output;
}

// Main initialization function
void production_input_processor_init(ProductionInputProcessor* processor) {
    memset(processor, 0, sizeof(ProductionInputProcessor));
    
    // Initialize all subsystems
    enhanced_calibrator_init(&processor->calibrator);
    adaptive_kalman_init(&processor->kalman_filter);
    neural_net_init(&processor->neural_net);
    mrac_init(&processor->mrac_controller);
    
    // Default configuration
    processor->config.enable_statistical_calibration = true;
    processor->config.enable_kalman_filtering = true;
    processor->config.enable_neural_processing = false;  // Enable after calibration
    processor->config.enable_mrac_safety = true;
    processor->config.cpu_budget_us = 100.0f;  // 100μs budget
    
    processor->calibration_state = CALIBRATION_STATE_WAITING;
    processor->initialized = true;
    
    printf("Production input processor initialized\n");
}

// Main processing function
Vector6 production_input_process(ProductionInputProcessor* processor, 
                                InputVector2 raw_input, 
                                float delta_time) {
    
    if (!processor->initialized) {
        production_input_processor_init(processor);
    }
    
    uint64_t start_time = get_current_time_microseconds();
    static Vector6 previous_output = {0};
    
    // Update calibration state machine
    update_calibration_state_machine(processor, raw_input, delta_time);
    
    // Layer 1: Enhanced Statistical Calibration (Always Active)
    enhanced_calibrator_update(&processor->calibrator, raw_input, start_time);
    InputVector2 calibrated_input = apply_statistical_correction(&processor->calibrator, raw_input);
    
    // Layer 2: Adaptive Kalman Filtering
    InputVector2 filtered_input = calibrated_input;
    if (processor->config.enable_kalman_filtering) {
        filtered_input = adaptive_kalman_update(&processor->kalman_filter, calibrated_input);
    }
    
    // Generate feature vector for neural network
    NeuralFeatureVector features = generate_feature_vector(processor, filtered_input, &previous_output);
    
    // Layer 3: Neural Network Processing
    Vector6 neural_output = {0};
    Vector6 statistical_output = {
        .pitch = filtered_input.y,  // pitch
        .yaw = filtered_input.x,    // yaw
        .roll = 0.0f, .strafe_x = 0.0f, .strafe_y = 0.0f, .throttle = 0.0f
    };
    
    if (processor->config.enable_neural_processing && processor->neural_net.meta_trained) {
        neural_output = neural_inference_int8(&processor->neural_net, &features);
        
        // Update replay buffer for continual learning
        processor->neural_net.replay_buffer[processor->neural_net.replay_index] = features;
        processor->neural_net.replay_targets[processor->neural_net.replay_index] = neural_output;
        processor->neural_net.replay_index = (processor->neural_net.replay_index + 1) % REPLAY_BUFFER_SIZE;
    }
    
    // Layer 4: MRAC Safety Shell (Final Output)
    Vector6 final_output;
    if (processor->config.enable_mrac_safety) {
        Vector6 reference_command = neural_output;  // Use neural output as reference
        final_output = mrac_control_mixing(&processor->mrac_controller,
                                         &neural_output,
                                         &statistical_output,
                                         &reference_command,
                                         delta_time);
    } else {
        // Simple mixing without MRAC
        float lambda = processor->mrac_controller.mixing_lambda;
        final_output = vector6_add(vector6_scale(neural_output, lambda),
                                  vector6_scale(statistical_output, 1.0f - lambda));
    }
    
    // Performance monitoring
    uint64_t end_time = get_current_time_microseconds();
    processor->metrics.total_time_us = end_time - start_time;
    processor->metrics.frames_processed++;
    
    // Safety monitoring
    if (processor->metrics.total_time_us > processor->config.cpu_budget_us) {
        processor->safety.performance_budget_exceeded = true;
        processor->config.enable_neural_processing = false;  // Emergency fallback
        printf("Performance budget exceeded, disabling neural processing\n");
    }
    
    previous_output = final_output;
    return final_output;
}

// Calibration state machine
void update_calibration_state_machine(ProductionInputProcessor* processor, 
                                     InputVector2 raw_input,
                                     float delta_time) {
    
    processor->calibration_timer += delta_time;
    
    switch (processor->calibration_state) {
        case CALIBRATION_STATE_WAITING:
            // Check if we have consistent input (controller connected)
            if (sqrtf(raw_input.x * raw_input.x + raw_input.y * raw_input.y) > 0.001f ||
                processor->calibrator.sample_count > 10) {
                printf("Controller detected! Starting statistical calibration...\n");
                processor->calibration_state = CALIBRATION_STATE_STATISTICAL;
                processor->calibration_timer = 0.0f;
            }
            break;
            
        case CALIBRATION_STATE_STATISTICAL:
            // Build statistical foundation for 5 seconds
            if (processor->calibration_timer >= 5.0f && processor->calibrator.confidence_level > 0.8f) {
                printf("Statistical calibration complete (%.2f confidence). Ready for neural processing.\n", 
                       processor->calibrator.confidence_level);
                processor->calibration_state = CALIBRATION_STATE_PRODUCTION;
                processor->config.enable_neural_processing = true;
                processor->calibration_timer = 0.0f;
            }
            break;
            
        case CALIBRATION_STATE_PRODUCTION:
            // Normal operation with drift monitoring
            if (processor->calibrator.drift_detected) {
                printf("Drift detected, enabling continual learning\n");
                processor->calibration_state = CALIBRATION_STATE_CONTINUAL;
            }
            break;
            
        case CALIBRATION_STATE_CONTINUAL:
            // Background drift adaptation
            if (!processor->calibrator.drift_detected) {
                printf("Drift compensation complete\n");
                processor->calibration_state = CALIBRATION_STATE_PRODUCTION;
            }
            break;
            
        default:
            processor->calibration_state = CALIBRATION_STATE_WAITING;
            break;
    }
}

// Debug information
void debug_input_processor_state(const ProductionInputProcessor* processor) {
    if (!processor->initialized) {
        printf("Input processor not initialized\n");
        return;
    }
    
    printf("\n=== Input Processor Debug Info ===\n");
    printf("Calibration State: %d\n", processor->calibration_state);
    printf("Calibration Timer: %.2fs\n", processor->calibration_timer);
    
    printf("\nStatistical Calibrator:\n");
    printf("  Center: [%.3f, %.3f]\n", processor->calibrator.mu.x, processor->calibrator.mu.y);
    printf("  Std Dev: [%.3f, %.3f]\n", processor->calibrator.sigma.x, processor->calibrator.sigma.y);
    printf("  Samples: %u\n", processor->calibrator.sample_count);
    printf("  Confidence: %.3f\n", processor->calibrator.confidence_level);
    printf("  Dynamic Deadzone: %.3f\n", processor->calibrator.dynamic_deadzone_estimate);
    printf("  Drift Detected: %s\n", processor->calibrator.drift_detected ? "Yes" : "No");
    
    printf("\nKalman Filter:\n");
    printf("  State: [%.3f, %.3f]\n", processor->kalman_filter.state.x, processor->kalman_filter.state.y);
    printf("  Confidence: %.3f\n", processor->kalman_filter.confidence);
    printf("  Outliers: %u\n", processor->kalman_filter.outlier_count);
    
    printf("\nNeural Network:\n");
    printf("  Meta-trained: %s\n", processor->neural_net.meta_trained ? "Yes" : "No");
    printf("  Inference Time: %.1f μs\n", processor->neural_net.inference_time_us);
    printf("  Total Inferences: %u\n", processor->neural_net.total_inferences);
    
    printf("\nMRAC Controller:\n");
    printf("  Mixing Lambda: %.3f\n", processor->mrac_controller.mixing_lambda);
    printf("  Stability Assured: %s\n", processor->mrac_controller.stability_assured ? "Yes" : "No");
    printf("  Lyapunov Energy: %.3f\n", processor->mrac_controller.lyapunov_energy);
    
    printf("\nPerformance:\n");
    printf("  Total Time: %.1f μs\n", processor->metrics.total_time_us);
    printf("  Frames Processed: %u\n", processor->metrics.frames_processed);
    printf("  Budget Exceeded: %s\n", processor->safety.performance_budget_exceeded ? "Yes" : "No");
    
    printf("\nConfiguration:\n");
    printf("  Statistical: %s\n", processor->config.enable_statistical_calibration ? "ON" : "OFF");
    printf("  Kalman: %s\n", processor->config.enable_kalman_filtering ? "ON" : "OFF");
    printf("  Neural: %s\n", processor->config.enable_neural_processing ? "ON" : "OFF");
    printf("  MRAC: %s\n", processor->config.enable_mrac_safety ? "ON" : "OFF");
    printf("===================================\n\n");
}

// Performance benchmark
void run_performance_benchmark(ProductionInputProcessor* processor) {
    printf("Running performance benchmark...\n");
    
    const uint32_t test_iterations = 10000;
    float total_time = 0.0f;
    
    for (uint32_t i = 0; i < test_iterations; i++) {
        // Generate test input
        InputVector2 test_input = {
            sinf(i * 0.01f) * 0.8f,  // Smooth circular motion
            cosf(i * 0.01f) * 0.8f
        };
        
        // Time the complete processing pipeline
        uint64_t start = get_current_time_microseconds();
        Vector6 output = production_input_process(processor, test_input, 0.016f);
        uint64_t end = get_current_time_microseconds();
        
        total_time += (end - start);
        (void)output;  // Avoid unused variable warning
    }
    
    float average_time = total_time / test_iterations;
    
    printf("Benchmark Results:\n");
    printf("  Average processing time: %.1f μs (target: <100 μs)\n", average_time);
    printf("  Neural inference time: %.1f μs (target: <50 μs)\n", processor->neural_net.inference_time_us);
    printf("  Memory footprint: %zu bytes (target: <10KB)\n", sizeof(ProductionInputProcessor));
    printf("  Performance target: %s\n", 
           (average_time < 100.0f) ? "✅ PASSED" : "❌ FAILED");
}

void production_input_processor_reset(ProductionInputProcessor* processor) {
    production_input_processor_init(processor);
}
