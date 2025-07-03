# Sprint 22: Advanced Input Processing - Implementation Guide
## CGame Engine - Developer Reference

**Date:** July 2, 2025  
**Target:** C Developers implementing advanced gamepad input processing  
**Prerequisites:** Understanding of CGame ECS, Sokol input system, basic signal processing  
**Estimated Implementation Time:** 8 weeks (4 phases)

---

## Quick Start Implementation

### Core Files to Create/Modify

```c
// New files to create:
src/input_processing.h    // Public API and structures
src/input_processing.c    // Core implementation
src/neural_input.h        // Neural network definitions (optional)
src/neural_input.c        // Neural network implementation (optional)

// Files to modify:
src/input.c              // Integration with existing input system
src/control.c            // Update to use processed input
tests/test_input_processing.c  // Comprehensive test suite
```

### Basic Integration Pattern

```c
// In your main game loop:
void game_update(float delta_time) {
    // 1. Get raw gamepad state (existing code)
    sgamepad_gamepad_state raw_gamepad;
    if (sgamepad_get_gamepad_state(0, &raw_gamepad)) {
        
        // 2. Process through advanced input system (NEW)
        Vector6 processed_input = advanced_input_process(&input_processor, raw_gamepad, delta_time);
        
        // 3. Feed to control system (modified)
        control_system_update_with_processed_input(&world, processed_input, delta_time);
    }
    
    // Rest of game loop unchanged...
}
```

---

## Phase 1: Statistical Self-Calibration Implementation

### 1.1 Core Data Structures

```c
// input_processing.h
#ifndef INPUT_PROCESSING_H
#define INPUT_PROCESSING_H

#include "core.h"  // For Vector2, etc.

typedef struct {
    Vector2 mu;              // Running mean (center position)
    Vector2 M2;              // Second moment for variance
    Vector2 sigma;           // Standard deviation
    Vector2 m_max;           // Running maximum per axis
    Vector2 m_min;           // Running minimum per axis
    
    float alpha;             // Learning rate (default 0.001)
    uint32_t sample_count;   // Total samples processed
    
    // Configuration
    float rest_threshold;    // Magnitude threshold for "rest" detection
    float percentile_threshold; // Threshold for extreme value detection
    uint32_t min_samples;    // Minimum samples before trusting statistics
} StatisticalCalibrator;

typedef struct {
    StatisticalCalibrator calibrator;
    
    // Performance monitoring
    float last_update_time_ms;
    bool initialized;
} BasicInputProcessor;

// Public API
void basic_input_processor_init(BasicInputProcessor* processor);
Vector2 basic_input_process(BasicInputProcessor* processor, Vector2 raw_input, float delta_time);
void basic_input_processor_reset(BasicInputProcessor* processor);

#endif // INPUT_PROCESSING_H
```

### 1.2 Core Implementation

```c
// input_processing.c
#include "input_processing.h"
#include <math.h>
#include <string.h>

// Configuration constants
#define DEFAULT_ALPHA 0.001f
#define DEFAULT_REST_THRESHOLD 0.05f
#define DEFAULT_PERCENTILE_THRESHOLD 0.9f
#define MIN_SAMPLES_FOR_TRUST 100
#define SIGMA_MULTIPLIER 3.0f

void basic_input_processor_init(BasicInputProcessor* processor) {
    memset(processor, 0, sizeof(BasicInputProcessor));
    
    // Initialize calibrator
    processor->calibrator.alpha = DEFAULT_ALPHA;
    processor->calibrator.rest_threshold = DEFAULT_REST_THRESHOLD;
    processor->calibrator.percentile_threshold = DEFAULT_PERCENTILE_THRESHOLD;
    processor->calibrator.min_samples = MIN_SAMPLES_FOR_TRUST;
    
    // Initialize extremes to reasonable defaults
    processor->calibrator.m_max = (Vector2){0.8f, 0.8f};
    processor->calibrator.m_min = (Vector2){-0.8f, -0.8f};
    
    processor->initialized = true;
}

static void update_statistics(StatisticalCalibrator* cal, Vector2 input) {
    cal->sample_count++;
    
    float magnitude = sqrtf(input.x * input.x + input.y * input.y);
    
    // Update rest statistics only when input is small
    if (magnitude < cal->rest_threshold) {
        Vector2 delta = {input.x - cal->mu.x, input.y - cal->mu.y};
        
        // Welford's online algorithm for mean and variance
        cal->mu.x += cal->alpha * delta.x;
        cal->mu.y += cal->alpha * delta.y;
        
        Vector2 delta2 = {input.x - cal->mu.x, input.y - cal->mu.y};
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
}

static float calculate_dynamic_deadzone(const StatisticalCalibrator* cal) {
    float mu_magnitude = sqrtf(cal->mu.x * cal->mu.x + cal->mu.y * cal->mu.y);
    float sigma_magnitude = sqrtf(cal->sigma.x * cal->sigma.x + cal->sigma.y * cal->sigma.y);
    
    return mu_magnitude + SIGMA_MULTIPLIER * sigma_magnitude;
}

Vector2 basic_input_process(BasicInputProcessor* processor, Vector2 raw_input, float delta_time) {
    if (!processor->initialized) {
        basic_input_processor_init(processor);
    }
    
    uint64_t start_time = get_current_time_microseconds();
    
    // Update statistics
    update_statistics(&processor->calibrator, raw_input);
    
    // Don't trust statistics until we have enough samples
    if (processor->calibrator.sample_count < processor->calibrator.min_samples) {
        // Return simple centered input with fixed deadzone
        Vector2 centered = {raw_input.x, raw_input.y};
        float magnitude = sqrtf(centered.x * centered.x + centered.y * centered.y);
        
        if (magnitude < 0.1f) {  // Fixed 10% deadzone
            return (Vector2){0.0f, 0.0f};
        }
        
        float scale = (magnitude - 0.1f) / (1.0f - 0.1f);
        return (Vector2){
            centered.x * scale / magnitude,
            centered.y * scale / magnitude
        };
    }
    
    // Apply statistical correction
    Vector2 centered = {
        raw_input.x - processor->calibrator.mu.x,
        raw_input.y - processor->calibrator.mu.y
    };
    
    float dynamic_deadzone = calculate_dynamic_deadzone(&processor->calibrator);
    float input_magnitude = sqrtf(centered.x * centered.x + centered.y * centered.y);
    
    if (input_magnitude < dynamic_deadzone) {
        return (Vector2){0.0f, 0.0f};
    }
    
    // Normalize to [-1, 1] using learned extremes
    Vector2 normalized = {
        centered.x / (processor->calibrator.m_max.x - fabsf(processor->calibrator.mu.x)),
        centered.y / (processor->calibrator.m_max.y - fabsf(processor->calibrator.mu.y))
    };
    
    // Clamp to [-1, 1] range
    normalized.x = fmaxf(-1.0f, fminf(1.0f, normalized.x));
    normalized.y = fmaxf(-1.0f, fminf(1.0f, normalized.y));
    
    // Apply radial scaling outside deadzone
    float scale_factor = (input_magnitude - dynamic_deadzone) / (1.0f - dynamic_deadzone);
    float norm_magnitude = sqrtf(normalized.x * normalized.x + normalized.y * normalized.y);
    
    if (norm_magnitude > 0.0001f) {  // Avoid division by zero
        return (Vector2){
            (normalized.x / norm_magnitude) * scale_factor,
            (normalized.y / norm_magnitude) * scale_factor
        };
    }
    
    // Record processing time
    uint64_t end_time = get_current_time_microseconds();
    processor->last_update_time_ms = (end_time - start_time) / 1000.0f;
    
    return (Vector2){0.0f, 0.0f};
}

void basic_input_processor_reset(BasicInputProcessor* processor) {
    basic_input_processor_init(processor);
}
```

### 1.3 Integration with Existing Input System

```c
// Modify src/input.c to add advanced processing option
// Add to input system state:
typedef struct {
    // ... existing fields ...
    BasicInputProcessor gamepad_processor[GAMEPAD_MAX_COUNT];
    bool enable_advanced_processing;
} InputSystem;

// Modify gamepad update function:
void input_system_update_gamepad(InputSystem* input, float delta_time) {
    for (int i = 0; i < GAMEPAD_MAX_COUNT; i++) {
        sgamepad_gamepad_state raw_state;
        if (sgamepad_get_gamepad_state(i, &raw_state)) {
            Vector2 raw_stick = {raw_state.thumb_lx, raw_state.thumb_ly};
            
            if (input->enable_advanced_processing) {
                Vector2 processed_stick = basic_input_process(&input->gamepad_processor[i], raw_stick, delta_time);
                // Update gamepad state with processed values
                input->gamepads[i].left_stick = processed_stick;
            } else {
                // Keep existing raw processing
                input->gamepads[i].left_stick = raw_stick;
            }
            
            // Process other inputs normally...
        }
    }
}
```

### 1.4 Testing Implementation

```c
// tests/test_input_processing.c
#include "input_processing.h"
#include "unity.h"

void setUp(void) {
    // Set up test fixtures
}

void tearDown(void) {
    // Clean up test fixtures
}

void test_basic_processor_initialization(void) {
    BasicInputProcessor processor;
    basic_input_processor_init(&processor);
    
    TEST_ASSERT_TRUE(processor.initialized);
    TEST_ASSERT_EQUAL_FLOAT(0.001f, processor.calibrator.alpha);
    TEST_ASSERT_EQUAL_UINT32(0, processor.calibrator.sample_count);
}

void test_zero_input_produces_zero_output(void) {
    BasicInputProcessor processor;
    basic_input_processor_init(&processor);
    
    Vector2 input = {0.0f, 0.0f};
    Vector2 output = basic_input_process(&processor, input, 0.016f);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output.y);
}

void test_drift_compensation(void) {
    BasicInputProcessor processor;
    basic_input_processor_init(&processor);
    
    // Simulate stick with constant drift
    Vector2 drift = {0.1f, -0.05f};
    
    // Feed many "rest" samples with drift
    for (int i = 0; i < 200; i++) {
        Vector2 input = {drift.x + (rand() / (float)RAND_MAX) * 0.01f - 0.005f,
                        drift.y + (rand() / (float)RAND_MAX) * 0.01f - 0.005f};
        basic_input_process(&processor, input, 0.016f);
    }
    
    // Now test that drift is compensated
    Vector2 drifted_input = drift;
    Vector2 output = basic_input_process(&processor, drifted_input, 0.016f);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output.y);
}

void test_performance_requirements(void) {
    BasicInputProcessor processor;
    basic_input_processor_init(&processor);
    
    // Process many samples to get stable performance measurement
    for (int i = 0; i < 1000; i++) {
        Vector2 input = {sinf(i * 0.1f), cosf(i * 0.1f)};
        basic_input_process(&processor, input, 0.016f);
    }
    
    // Verify processing time is under budget
    TEST_ASSERT_LESS_THAN_FLOAT(0.02f, processor.last_update_time_ms);  // <20μs
}

// Add Unity test runner
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_processor_initialization);
    RUN_TEST(test_zero_input_produces_zero_output);
    RUN_TEST(test_drift_compensation);
    RUN_TEST(test_performance_requirements);
    
    return UNITY_END();
}
```

---

## Phase 2: Adaptive Kalman Filter Implementation

### 2.1 Extended Data Structures

```c
// Add to input_processing.h
typedef struct {
    float data[4];  // 2x2 matrix stored as [m00, m01, m10, m11]
} Matrix2x2;

typedef struct {
    Vector2 state;           // Current estimated position
    Matrix2x2 P;            // State covariance matrix
    Matrix2x2 Q;            // Process noise covariance
    Matrix2x2 R;            // Measurement noise covariance
    Vector2 innovation;     // Innovation (measurement - prediction)
    float confidence;       // Filter confidence [0, 1]
    
    // Adaptive parameters
    float base_R_value;     // Base measurement noise
    float spike_multiplier; // Multiplier during outlier detection
    uint32_t outlier_count; // Count of outliers detected
} AdaptiveKalmanFilter;

typedef struct {
    StatisticalCalibrator calibrator;
    AdaptiveKalmanFilter kalman_filter;
    
    bool enable_kalman_filtering;
    float last_update_time_ms;
    bool initialized;
} AdvancedInputProcessor;
```

### 2.2 Matrix Operations

```c
// Matrix helper functions (add to input_processing.c)
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

static Vector2 matrix2x2_vector2_multiply(Matrix2x2 m, Vector2 v) {
    return (Vector2){
        m.data[0] * v.x + m.data[1] * v.y,
        m.data[2] * v.x + m.data[3] * v.y
    };
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
```

### 2.3 Kalman Filter Implementation

```c
// Kalman filter functions
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

static Vector2 adaptive_kalman_update(AdaptiveKalmanFilter* kf, Vector2 measurement) {
    // Predict step (constant position model)
    Vector2 x_pred = kf->state;
    Matrix2x2 P_pred = matrix2x2_add(kf->P, kf->Q);
    
    // Innovation calculation
    kf->innovation = (Vector2){measurement.x - x_pred.x, measurement.y - x_pred.y};
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
    
    kf->state = (Vector2){
        x_pred.x + K.data[0] * kf->innovation.x + K.data[1] * kf->innovation.y,
        x_pred.y + K.data[2] * kf->innovation.x + K.data[3] * kf->innovation.y
    };
    
    // Update covariance
    Matrix2x2 I_minus_K = matrix2x2_subtract(matrix2x2_identity(), K);
    kf->P = matrix2x2_multiply(I_minus_K, P_pred);
    
    return kf->state;
}
```

### 2.4 Integrated Processing Pipeline

```c
Vector2 advanced_input_process(AdvancedInputProcessor* processor, Vector2 raw_input, float delta_time) {
    if (!processor->initialized) {
        advanced_input_processor_init(processor);
    }
    
    uint64_t start_time = get_current_time_microseconds();
    
    // Layer 1: Statistical calibration
    update_statistics(&processor->calibrator, raw_input);
    Vector2 calibrated_input = apply_statistical_correction(&processor->calibrator, raw_input);
    
    // Layer 2: Adaptive Kalman filtering (if enabled)
    Vector2 final_output = calibrated_input;
    if (processor->enable_kalman_filtering && 
        processor->calibrator.sample_count >= processor->calibrator.min_samples) {
        final_output = adaptive_kalman_update(&processor->kalman_filter, calibrated_input);
    }
    
    uint64_t end_time = get_current_time_microseconds();
    processor->last_update_time_ms = (end_time - start_time) / 1000.0f;
    
    return final_output;
}
```

---

## Phase 3: Neural Network Foundation

### 3.1 Network Architecture

```c
// neural_input.h
#ifndef NEURAL_INPUT_H
#define NEURAL_INPUT_H

#include "core.h"

#define NEURAL_INPUT_SIZE 5      // [v_x, v_y, |v|, Δv_x, Δv_y]
#define NEURAL_HIDDEN_SIZE 32    // Hidden layer neurons
#define NEURAL_OUTPUT_SIZE 6     // [pitch, yaw, roll, strafe_x, strafe_y, throttle]

typedef struct {
    float x, y, z, w, u, v;  // 6DOF output vector
} Vector6;

typedef struct {
    // Network weights and biases
    float weights_input_hidden[NEURAL_INPUT_SIZE * NEURAL_HIDDEN_SIZE];
    float bias_hidden[NEURAL_HIDDEN_SIZE];
    float weights_hidden_output[NEURAL_HIDDEN_SIZE * NEURAL_OUTPUT_SIZE];
    float bias_output[NEURAL_OUTPUT_SIZE];
    
    // Inference state
    float hidden_activations[NEURAL_HIDDEN_SIZE];
    float input_buffer[NEURAL_INPUT_SIZE];
    float output_buffer[NEURAL_OUTPUT_SIZE];
    
    // Previous input for velocity calculation
    Vector2 previous_input;
    bool has_previous;
    
    // Training/adaptation
    bool training_enabled;
    float learning_rate;
    uint32_t training_samples;
    
    // Performance
    float inference_time_ms;
    bool initialized;
} NeuralInputProcessor;

// Public API
void neural_processor_init(NeuralInputProcessor* processor);
Vector6 neural_processor_inference(NeuralInputProcessor* processor, Vector2 input);
void neural_processor_train_sample(NeuralInputProcessor* processor, Vector2 input, Vector6 target);
void neural_processor_reset(NeuralInputProcessor* processor);

#endif // NEURAL_INPUT_H
```

### 3.2 Neural Network Implementation

```c
// neural_input.c
#include "neural_input.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Activation functions
static float tanh_activation(float x) {
    return tanhf(x);
}

static float tanh_derivative(float x) {
    float t = tanhf(x);
    return 1.0f - t * t;
}

void neural_processor_init(NeuralInputProcessor* processor) {
    memset(processor, 0, sizeof(NeuralInputProcessor));
    
    // Initialize weights with Xavier initialization
    float scale_input = sqrtf(2.0f / (NEURAL_INPUT_SIZE + NEURAL_HIDDEN_SIZE));
    float scale_output = sqrtf(2.0f / (NEURAL_HIDDEN_SIZE + NEURAL_OUTPUT_SIZE));
    
    for (int i = 0; i < NEURAL_INPUT_SIZE * NEURAL_HIDDEN_SIZE; i++) {
        processor->weights_input_hidden[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * scale_input;
    }
    
    for (int i = 0; i < NEURAL_HIDDEN_SIZE * NEURAL_OUTPUT_SIZE; i++) {
        processor->weights_hidden_output[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * scale_output;
    }
    
    // Initialize biases to zero
    memset(processor->bias_hidden, 0, sizeof(processor->bias_hidden));
    memset(processor->bias_output, 0, sizeof(processor->bias_output));
    
    processor->learning_rate = 0.001f;
    processor->initialized = true;
}

Vector6 neural_processor_inference(NeuralInputProcessor* processor, Vector2 input) {
    if (!processor->initialized) {
        neural_processor_init(processor);
    }
    
    uint64_t start_time = get_current_time_microseconds();
    
    // Prepare input vector [v_x, v_y, |v|, Δv_x, Δv_y]
    Vector2 velocity = {0.0f, 0.0f};
    if (processor->has_previous) {
        velocity.x = input.x - processor->previous_input.x;
        velocity.y = input.y - processor->previous_input.y;
    }
    
    processor->input_buffer[0] = input.x;
    processor->input_buffer[1] = input.y;
    processor->input_buffer[2] = sqrtf(input.x * input.x + input.y * input.y);
    processor->input_buffer[3] = velocity.x;
    processor->input_buffer[4] = velocity.y;
    
    // Forward pass: input to hidden layer
    for (int h = 0; h < NEURAL_HIDDEN_SIZE; h++) {
        float sum = processor->bias_hidden[h];
        for (int i = 0; i < NEURAL_INPUT_SIZE; i++) {
            sum += processor->input_buffer[i] * processor->weights_input_hidden[i * NEURAL_HIDDEN_SIZE + h];
        }
        processor->hidden_activations[h] = tanh_activation(sum);
    }
    
    // Forward pass: hidden to output layer
    for (int o = 0; o < NEURAL_OUTPUT_SIZE; o++) {
        float sum = processor->bias_output[o];
        for (int h = 0; h < NEURAL_HIDDEN_SIZE; h++) {
            sum += processor->hidden_activations[h] * processor->weights_hidden_output[h * NEURAL_OUTPUT_SIZE + o];
        }
        processor->output_buffer[o] = sum;  // Linear output
    }
    
    // Update previous input
    processor->previous_input = input;
    processor->has_previous = true;
    
    uint64_t end_time = get_current_time_microseconds();
    processor->inference_time_ms = (end_time - start_time) / 1000.0f;
    
    // Convert to Vector6
    Vector6 output = {
        .x = processor->output_buffer[0],  // pitch
        .y = processor->output_buffer[1],  // yaw
        .z = processor->output_buffer[2],  // roll
        .w = processor->output_buffer[3],  // strafe_x
        .u = processor->output_buffer[4],  // strafe_y
        .v = processor->output_buffer[5]   // throttle
    };
    
    return output;
}

void neural_processor_train_sample(NeuralInputProcessor* processor, Vector2 input, Vector6 target) {
    if (!processor->training_enabled) return;
    
    // Forward pass (reuse inference function to populate internal state)
    Vector6 predicted = neural_processor_inference(processor, input);
    
    // Compute output layer errors
    float output_errors[NEURAL_OUTPUT_SIZE];
    float* target_array = (float*)&target;
    for (int o = 0; o < NEURAL_OUTPUT_SIZE; o++) {
        output_errors[o] = target_array[o] - processor->output_buffer[o];
    }
    
    // Compute hidden layer errors (backpropagation)
    float hidden_errors[NEURAL_HIDDEN_SIZE];
    for (int h = 0; h < NEURAL_HIDDEN_SIZE; h++) {
        hidden_errors[h] = 0.0f;
        for (int o = 0; o < NEURAL_OUTPUT_SIZE; o++) {
            hidden_errors[h] += output_errors[o] * processor->weights_hidden_output[h * NEURAL_OUTPUT_SIZE + o];
        }
        hidden_errors[h] *= tanh_derivative(processor->hidden_activations[h]);
    }
    
    // Update weights and biases
    for (int h = 0; h < NEURAL_HIDDEN_SIZE; h++) {
        for (int o = 0; o < NEURAL_OUTPUT_SIZE; o++) {
            processor->weights_hidden_output[h * NEURAL_OUTPUT_SIZE + o] += 
                processor->learning_rate * output_errors[o] * processor->hidden_activations[h];
        }
    }
    
    for (int i = 0; i < NEURAL_INPUT_SIZE; i++) {
        for (int h = 0; h < NEURAL_HIDDEN_SIZE; h++) {
            processor->weights_input_hidden[i * NEURAL_HIDDEN_SIZE + h] += 
                processor->learning_rate * hidden_errors[h] * processor->input_buffer[i];
        }
    }
    
    // Update biases
    for (int o = 0; o < NEURAL_OUTPUT_SIZE; o++) {
        processor->bias_output[o] += processor->learning_rate * output_errors[o];
    }
    
    for (int h = 0; h < NEURAL_HIDDEN_SIZE; h++) {
        processor->bias_hidden[h] += processor->learning_rate * hidden_errors[h];
    }
    
    processor->training_samples++;
}

void neural_processor_reset(NeuralInputProcessor* processor) {
    neural_processor_init(processor);
}
```

---

## Phase 4: Full Integration & Optimization

### 4.1 Complete Advanced Input Processor

```c
// Final complete structure
typedef struct {
    // Layer 1: Statistical calibration
    StatisticalCalibrator calibrator;
    
    // Layer 2: Adaptive Kalman filter
    AdaptiveKalmanFilter kalman_filter;
    
    // Layer 3: Neural network processor
    NeuralInputProcessor neural_processor;
    
    // Configuration flags
    bool enable_statistical_calibration;
    bool enable_kalman_filtering;
    bool enable_neural_processing;
    
    // Performance monitoring
    float total_processing_time_ms;
    uint32_t frames_processed;
    
    // Fallback system
    bool fallback_active;
    float fallback_threshold_ms;  // CPU budget limit
    
    bool initialized;
} CompleteInputProcessor;

Vector6 complete_input_process(CompleteInputProcessor* processor, Vector2 raw_input, float delta_time) {
    if (!processor->initialized) {
        complete_input_processor_init(processor);
    }
    
    uint64_t start_time = get_current_time_microseconds();
    
    Vector2 current_input = raw_input;
    
    // Layer 1: Statistical calibration (always enabled for basic quality)
    if (processor->enable_statistical_calibration) {
        update_statistics(&processor->calibrator, raw_input);
        current_input = apply_statistical_correction(&processor->calibrator, raw_input);
    }
    
    // Layer 2: Adaptive Kalman filtering (optional)
    if (processor->enable_kalman_filtering && !processor->fallback_active) {
        current_input = adaptive_kalman_update(&processor->kalman_filter, current_input);
    }
    
    Vector6 output;
    
    // Layer 3: Neural network processing (optional and performance-gated)
    if (processor->enable_neural_processing && !processor->fallback_active) {
        output = neural_processor_inference(&processor->neural_processor, current_input);
    } else {
        // Default 2D->6DOF mapping for fallback
        output = (Vector6){
            .x = current_input.y,    // pitch
            .y = current_input.x,    // yaw
            .z = 0.0f,              // roll
            .w = 0.0f,              // strafe_x
            .u = 0.0f,              // strafe_y
            .v = 0.0f               // throttle
        };
    }
    
    // Performance monitoring and fallback logic
    uint64_t end_time = get_current_time_microseconds();
    processor->total_processing_time_ms = (end_time - start_time) / 1000.0f;
    processor->frames_processed++;
    
    // Activate fallback if processing exceeds budget
    if (processor->total_processing_time_ms > processor->fallback_threshold_ms) {
        processor->fallback_active = true;
    } else if (processor->fallback_active && 
               processor->total_processing_time_ms < processor->fallback_threshold_ms * 0.5f) {
        processor->fallback_active = false;  // Re-enable advanced features
    }
    
    return output;
}
```

### 4.2 Integration with Control System

```c
// Modify control.c to accept 6DOF input
void control_system_update_with_processed_input(World* world, Vector6 input, float delta_time) {
    // Iterate through all entities with control components
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (!(entity->component_mask & COMPONENT_CONTROL_AUTHORITY)) continue;
        
        struct ControlAuthority* control = entity->control_authority;
        struct Physics* physics = entity_get_physics(world, i);
        struct Transform* transform = entity_get_transform(world, i);
        
        if (!physics || !transform) continue;
        
        // Apply 6DOF control input
        Vector3 linear_force = {
            input.w * control->linear_thrust[0],  // strafe_x
            input.u * control->linear_thrust[1],  // strafe_y
            input.v * control->linear_thrust[2]   // throttle (forward)
        };
        
        Vector3 angular_torque = {
            input.x * control->angular_thrust[0], // pitch
            input.y * control->angular_thrust[1], // yaw
            input.z * control->angular_thrust[2]  // roll
        };
        
        // Transform forces to world space
        Vector3 world_linear_force = quaternion_rotate_vector(transform->rotation, linear_force);
        Vector3 world_angular_torque = quaternion_rotate_vector(transform->rotation, angular_torque);
        
        // Apply to physics
        physics_add_force(physics, world_linear_force);
        physics_add_torque(physics, world_angular_torque);
    }
}
```

---

## Debug and Visualization Tools

### Debug UI Integration

```c
// Add to ui.c for runtime debugging
void ui_render_input_processor_debug(CompleteInputProcessor* processor) {
    if (!processor->initialized) return;
    
    igBegin("Advanced Input Processor Debug", NULL, 0);
    
    // Layer status
    igText("Layer Status:");
    igText("  Statistical: %s", processor->enable_statistical_calibration ? "ON" : "OFF");
    igText("  Kalman: %s", processor->enable_kalman_filtering ? "ON" : "OFF");
    igText("  Neural: %s", processor->enable_neural_processing ? "ON" : "OFF");
    igText("  Fallback: %s", processor->fallback_active ? "ACTIVE" : "NORMAL");
    
    igSeparator();
    
    // Performance metrics
    igText("Performance:");
    igText("  Processing Time: %.3f ms", processor->total_processing_time_ms);
    igText("  Frames Processed: %u", processor->frames_processed);
    igText("  Budget: %.3f ms", processor->fallback_threshold_ms);
    
    igSeparator();
    
    // Statistical calibration info
    if (processor->enable_statistical_calibration) {
        StatisticalCalibrator* cal = &processor->calibrator;
        igText("Statistical Calibration:");
        igText("  Center: [%.3f, %.3f]", cal->mu.x, cal->mu.y);
        igText("  Std Dev: [%.3f, %.3f]", cal->sigma.x, cal->sigma.y);
        igText("  Samples: %u", cal->sample_count);
        igText("  Dynamic Deadzone: %.3f", calculate_dynamic_deadzone(cal));
    }
    
    igSeparator();
    
    // Kalman filter info
    if (processor->enable_kalman_filtering) {
        AdaptiveKalmanFilter* kf = &processor->kalman_filter;
        igText("Kalman Filter:");
        igText("  State: [%.3f, %.3f]", kf->state.x, kf->state.y);
        igText("  Confidence: %.3f", kf->confidence);
        igText("  Outliers: %u", kf->outlier_count);
    }
    
    igSeparator();
    
    // Neural network info
    if (processor->enable_neural_processing) {
        NeuralInputProcessor* nn = &processor->neural_processor;
        igText("Neural Network:");
        igText("  Inference Time: %.3f ms", nn->inference_time_ms);
        igText("  Training Samples: %u", nn->training_samples);
        igText("  Training: %s", nn->training_enabled ? "ON" : "OFF");
    }
    
    // Configuration controls
    igSeparator();
    igText("Configuration:");
    igCheckbox("Enable Statistical", &processor->enable_statistical_calibration);
    igCheckbox("Enable Kalman", &processor->enable_kalman_filtering);
    igCheckbox("Enable Neural", &processor->enable_neural_processing);
    igSliderFloat("CPU Budget", &processor->fallback_threshold_ms, 0.01f, 2.0f, "%.3f ms", 0);
    
    igEnd();
}
```

---

## Final Integration Checklist

### Essential Integration Steps

- [ ] **Phase 1 Complete:** Statistical calibration working with existing input system
- [ ] **Phase 2 Complete:** Kalman filtering integrated and tested
- [ ] **Phase 3 Complete:** Neural network foundation implemented
- [ ] **Phase 4 Complete:** Full system integrated with fallback mechanisms
- [ ] **Testing:** Comprehensive test suite covers all layers
- [ ] **Performance:** CPU budget enforcement working
- [ ] **Debug Tools:** Runtime visualization and tuning available
- [ ] **Documentation:** Code documented and examples provided

### Performance Validation

- [ ] **Latency:** Total processing <1.0ms measured
- [ ] **CPU Usage:** <0.05ms per frame under normal conditions
- [ ] **Memory:** Total footprint <2KB
- [ ] **Reliability:** No crashes during 100+ hour stress test
- [ ] **Fallback:** Graceful degradation when CPU budget exceeded

### User Experience Validation

- [ ] **Zero Config:** Works immediately without setup
- [ ] **Hardware Compatibility:** Tested on 5+ different controller types
- [ ] **Quality Improvement:** Measurable improvement in control precision
- [ ] **Transparency:** Advanced processing not noticeable to users
- [ ] **Customization:** Power users can tune parameters

---

This implementation guide provides a complete roadmap for integrating advanced input processing into the CGame engine. Each phase builds incrementally, allowing for testing and validation at each step. The modular design ensures that even partial implementation provides user benefits, while the complete system offers state-of-the-art input processing capabilities.

**Next Step:** Begin Phase 1 implementation with statistical calibration system.
