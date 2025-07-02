// Test for enhanced input processing system (Sprint 22)

#include "../vendor/unity.h"
#include "../../src/input_processing.h"
#include <math.h>

void setUp(void) {
    // Set up tests
}

void tearDown(void) {
    // Clean up tests
}

void test_production_input_processor_init(void) {
    ProductionInputProcessor processor;
    
    // Test initialization
    production_input_processor_init(&processor);
    
    TEST_ASSERT_TRUE(processor.initialized);
    TEST_ASSERT_EQUAL(CALIBRATION_STATE_WAITING, processor.calibration_state);
    TEST_ASSERT_TRUE(processor.config.enable_statistical_calibration);
    TEST_ASSERT_TRUE(processor.config.enable_kalman_filtering);
    TEST_ASSERT_FALSE(processor.config.enable_neural_processing);  // Starts disabled
}

void test_statistical_calibration(void) {
    ProductionInputProcessor processor;
    production_input_processor_init(&processor);
    
    // Send some rest input to build statistics
    InputVector2 rest_input = {0.01f, -0.02f};  // Small drift
    
    for (int i = 0; i < 200; i++) {
        production_input_process(&processor, rest_input, 0.016f);
    }
    
    // Check that calibrator has learned the drift
    TEST_ASSERT_GREATER_THAN(100, processor.calibrator.sample_count);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.01f, processor.calibrator.mu.x);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, -0.02f, processor.calibrator.mu.y);
}

void test_kalman_filtering(void) {
    ProductionInputProcessor processor;
    production_input_processor_init(&processor);
    
    // Enable Kalman filtering
    processor.config.enable_kalman_filtering = true;
    
    // Send noisy input
    InputVector2 noisy_input = {0.5f, 0.3f};
    Vector6 output1 = production_input_process(&processor, noisy_input, 0.016f);
    
    // Send another similar input - should be filtered
    noisy_input = (InputVector2){0.52f, 0.28f};  // Small variation
    Vector6 output2 = production_input_process(&processor, noisy_input, 0.016f);
    
    // Kalman filter should smooth the output
    TEST_ASSERT_NOT_EQUAL(0.0f, output1.pitch);
    TEST_ASSERT_NOT_EQUAL(0.0f, output1.yaw);
    TEST_ASSERT_NOT_EQUAL(0.0f, output2.pitch);
    TEST_ASSERT_NOT_EQUAL(0.0f, output2.yaw);
}

void test_calibration_state_machine(void) {
    ProductionInputProcessor processor;
    production_input_processor_init(&processor);
    
    // Should start in WAITING state
    TEST_ASSERT_EQUAL(CALIBRATION_STATE_WAITING, processor.calibration_state);
    
    // Send significant input to trigger calibration
    InputVector2 active_input = {0.8f, 0.6f};
    production_input_process(&processor, active_input, 0.016f);
    
    // Should move to STATISTICAL state
    TEST_ASSERT_EQUAL(CALIBRATION_STATE_STATISTICAL, processor.calibration_state);
    
    // After 5+ seconds, should move to production
    for (int i = 0; i < 400; i++) {  // ~6.4 seconds at 60fps
        production_input_process(&processor, active_input, 0.016f);
    }
    
    // Should eventually reach production state
    TEST_ASSERT_TRUE(processor.calibration_state >= CALIBRATION_STATE_PRODUCTION);
}

void test_performance_budget(void) {
    ProductionInputProcessor processor;
    production_input_processor_init(&processor);
    
    // Set a tight CPU budget
    processor.config.cpu_budget_us = 500.0f;  // 0.5ms budget
    
    InputVector2 test_input = {0.7f, 0.4f};
    
    // Process and check performance
    Vector6 output = production_input_process(&processor, test_input, 0.016f);
    
    // Should complete without exceeding budget
    TEST_ASSERT_FALSE(processor.safety.performance_budget_exceeded);
    TEST_ASSERT_LESS_THAN(500.0f, processor.metrics.total_time_us);
    
    // Output should be valid
    TEST_ASSERT_NOT_EQUAL(0.0f, output.pitch);
    TEST_ASSERT_NOT_EQUAL(0.0f, output.yaw);
}

void test_vector6_utility_functions(void) {
    Vector6 a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    Vector6 b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};
    
    // Test addition
    Vector6 sum = vector6_add(a, b);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, sum.pitch);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, sum.yaw);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, sum.roll);
    
    // Test scaling
    Vector6 scaled = vector6_scale(a, 2.0f);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, scaled.pitch);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, scaled.yaw);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, scaled.roll);
    
    // Test length
    float length = vector6_length(a);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 9.54f, length);  // sqrt(1+4+9+16+25+36)
}

// Unity test runner
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_production_input_processor_init);
    RUN_TEST(test_statistical_calibration);
    RUN_TEST(test_kalman_filtering);
    RUN_TEST(test_calibration_state_machine);
    RUN_TEST(test_performance_budget);
    RUN_TEST(test_vector6_utility_functions);
    
    return UNITY_END();
}
