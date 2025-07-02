#ifndef INPUT_PROCESSING_H
#define INPUT_PROCESSING_H

/** @brief A 2D vector with float components for input processing. */
typedef struct {
    float x, y;
} InputVector2;

#include "core.h"
#include <stdint.h>
#include <stdbool.h>

// Neural network architecture constants
#define NEURAL_INPUT_SIZE 14
#define NEURAL_HIDDEN_SIZE 32
#define NEURAL_OUTPUT_SIZE 6

// Performance and memory constants
#define REPLAY_BUFFER_SIZE 480  // 8 seconds at 60fps
#define DRIFT_HISTORY_SIZE 60   // 1 second at 60fps
#define CALIBRATION_SAMPLES 100

// Enhanced 6DOF vector for neural network output
typedef struct {
    float pitch;     // x-axis rotation
    float yaw;       // y-axis rotation  
    float roll;      // z-axis rotation
    float strafe_x;  // lateral movement
    float strafe_y;  // vertical movement
    float throttle;  // forward/backward movement
} Vector6;

// Neural network feature vector (14 dimensions)
typedef struct {
    // Raw sensor data (preserve quirks for NN learning)
    float raw_lx, raw_ly;        // Raw left stick [-1, 1]
    float raw_magnitude;         // |raw|
    
    // Temporal derivatives
    float delta_lx, delta_ly;    // 1-frame derivatives
    
    // Statistical context from Layer 1
    float rdz_est;              // Dynamic dead-zone estimate
    float g_est;                // Gain estimate from stats
    
    // Temporal context
    float age_norm;             // Minutes since pad connected [0, 1]
    
    // Control history (helps model inertia)
    int16_t prev_output[6];     // Previous 6-DoF command (quantized)
} NeuralFeatureVector;

// Enhanced statistical calibrator with neural-ready features
typedef struct {
    // Core statistics
    InputVector2 mu;                  // Running mean (rest position)
    InputVector2 M2;                  // Second moment for variance calculation
    InputVector2 sigma;               // Standard deviation
    InputVector2 m_max;               // Running maximum per axis
    InputVector2 m_min;               // Running minimum per axis
    float alpha;                 // Learning rate (~0.001-0.003)
    uint32_t sample_count;       // Total samples processed
    
    // Neural-ready enhancements
    float dynamic_deadzone_estimate;
    float gain_estimate;
    float confidence_level;
    uint64_t controller_age_ms;  // Time since connection
    uint64_t connection_timestamp;
    
    // Drift tracking for continual learning
    InputVector2 drift_history[DRIFT_HISTORY_SIZE];
    uint32_t drift_index;
    bool drift_detected;
    
    // Configuration
    float rest_threshold;        // Magnitude threshold for "rest" detection
    float percentile_threshold;  // Threshold for extreme value detection
    uint32_t min_samples;        // Minimum samples before trusting statistics
} EnhancedStatisticalCalibrator;

// Adaptive Kalman filter for noise reduction
typedef struct {
    float data[4];  // 2x2 matrix stored as [m00, m01, m10, m11]
} Matrix2x2;

typedef struct {
    InputVector2 state;           // Current estimated position
    Matrix2x2 P;            // State covariance matrix
    Matrix2x2 Q;            // Process noise covariance
    Matrix2x2 R;            // Measurement noise covariance
    InputVector2 innovation;     // Innovation (measurement - prediction)
    float confidence;       // Filter confidence [0, 1]
    
    // Adaptive parameters
    float base_R_value;     // Base measurement noise
    float spike_multiplier; // Multiplier during outlier detection
    uint32_t outlier_count; // Count of outliers detected
} AdaptiveKalmanFilter;

// Meta-trained neural network (int8 quantized for performance)
typedef struct {
    // Quantized weights (int8 for <50μs inference)
    int8_t weights_fc1[NEURAL_INPUT_SIZE * NEURAL_HIDDEN_SIZE];   // 448 bytes
    int8_t weights_fc2[NEURAL_HIDDEN_SIZE * NEURAL_HIDDEN_SIZE];  // 1024 bytes  
    int8_t weights_fc3[NEURAL_HIDDEN_SIZE * NEURAL_OUTPUT_SIZE];  // 192 bytes
    
    // Biases (int8)
    int8_t bias_fc1[NEURAL_HIDDEN_SIZE];
    int8_t bias_fc2[NEURAL_HIDDEN_SIZE];
    int8_t bias_fc3[NEURAL_OUTPUT_SIZE];
    
    // Scaling factors for quantization
    float input_scale, fc1_scale, fc2_scale, output_scale;
    
    // Meta-learning state
    bool meta_trained;
    float adaptation_lr;          // Learning rate for few-shot adaptation
    uint32_t adaptation_steps;    // Steps completed in current adaptation
    
    // Continual learning buffer (8-second FIFO)
    NeuralFeatureVector replay_buffer[REPLAY_BUFFER_SIZE];
    Vector6 replay_targets[REPLAY_BUFFER_SIZE];
    uint32_t replay_index;
    
    // Performance tracking
    float inference_time_us;
    uint32_t total_inferences;
    
    // Previous input for velocity calculation
    InputVector2 previous_input;
    bool has_previous;
} MetaTrainedNeuralNet;

// Model Reference Adaptive Controller (MRAC) for stability
typedef struct {
    // Reference model (desired closed-loop behavior)
    Vector6 reference_state;
    Vector6 reference_output;
    float reference_damping;      // Desired damping ratio
    float reference_frequency;    // Desired natural frequency
    
    // Adaptive parameters
    Vector6 adaptive_gains;       // Adaptive compensation gains
    float adaptation_rate;        // Learning rate for adaptation
    float sigma_bound;           // Adaptation law saturation
    
    // Lyapunov stability monitoring
    float lyapunov_energy;       // Current Lyapunov function value
    float stability_margin;      // Safety margin for stability
    bool stability_assured;      // Lyapunov stability confirmed
    
    // Mixing parameter
    float neural_confidence;     // Confidence in neural output [0,1]
    float mixing_lambda;         // λ for: u = λ*u_nn + (1-λ)*u_stats
} ModelReferenceController;

// Calibration micro-game for few-shot adaptation
typedef struct {
    InputVector2 target_position;      // Current target location
    InputVector2 target_velocity;      // Target movement
    float target_size;           // Size of target zone
    InputVector2 reticle_position;    // Player reticle location
    
    uint32_t episode_count;      // Calibration episodes completed
    float cumulative_error;      // Total tracking error
    bool calibration_complete;   // Ready for normal use
    
    // Calibration data collection
    NeuralFeatureVector calibration_inputs[CALIBRATION_SAMPLES];
    Vector6 calibration_targets[CALIBRATION_SAMPLES];
    uint32_t calibration_sample_count;
} CalibrationGame;

// Processing configuration
typedef struct {
    bool enable_statistical_calibration;
    bool enable_kalman_filtering;
    bool enable_neural_processing;
    bool enable_mrac_safety;
    float cpu_budget_us;         // CPU time budget in microseconds
} ProcessingConfig;

// Performance metrics
typedef struct {
    // Timing measurements (μs)
    float statistical_time_us;
    float kalman_time_us;
    float neural_inference_time_us;
    float mrac_time_us;
    float total_time_us;
    
    // Accuracy metrics
    float tracking_accuracy;
    float drift_compensation_effectiveness;
    float stability_margin;
    
    // Resource usage
    uint32_t memory_footprint_bytes;
    uint32_t frames_processed;
} PerformanceMetrics;

// Safety monitor
typedef struct {
    bool performance_budget_exceeded;
    bool stability_compromised;
    bool neural_confidence_low;
    uint32_t fallback_activations;
} SafetyMonitor;

// Calibration state machine
typedef enum {
    CALIBRATION_STATE_WAITING,     // Waiting for controller connection
    CALIBRATION_STATE_STATISTICAL, // Building statistical foundation (first 5 seconds)
    CALIBRATION_STATE_MICRO_GAME,  // Running calibration micro-game (10 seconds)
    CALIBRATION_STATE_ADAPTATION,  // Performing few-shot adaptation
    CALIBRATION_STATE_PRODUCTION,  // Normal operation
    CALIBRATION_STATE_CONTINUAL    // Background continual learning
} CalibrationState;

// Complete production input processor
typedef struct {
    // Layer 1: Statistical Foundation (Always Active)
    EnhancedStatisticalCalibrator calibrator;
    
    // Layer 2: Adaptive Kalman Filter (Noise Control)
    AdaptiveKalmanFilter kalman_filter;
    
    // Layer 3: Neural Network Processor (Advanced Compensation)
    MetaTrainedNeuralNet neural_net;
    
    // Layer 4: MRAC Safety Shell (Stability Guarantee)
    ModelReferenceController mrac_controller;
    
    // Calibration system
    CalibrationGame calibration_game;
    CalibrationState calibration_state;
    float calibration_timer;
    
    // Configuration and Performance
    ProcessingConfig config;
    PerformanceMetrics metrics;
    SafetyMonitor safety;
    
    bool initialized;
} ProductionInputProcessor;

// Public API
void production_input_processor_init(ProductionInputProcessor* processor);
Vector6 production_input_process(ProductionInputProcessor* processor, 
                                InputVector2 raw_input, 
                                float delta_time);
void production_input_processor_reset(ProductionInputProcessor* processor);

// Calibration API
void update_calibration_state_machine(ProductionInputProcessor* processor, 
                                     InputVector2 raw_input,
                                     float delta_time);

// Debug and monitoring API
void debug_input_processor_state(const ProductionInputProcessor* processor);
void run_performance_benchmark(ProductionInputProcessor* processor);

// Utility functions
Vector6 vector6_add(Vector6 a, Vector6 b);
Vector6 vector6_sub(Vector6 a, Vector6 b);
Vector6 vector6_scale(Vector6 v, float scale);
float vector6_length(Vector6 v);
float vector6_dot(Vector6 a, Vector6 b);

#endif // INPUT_PROCESSING_H
