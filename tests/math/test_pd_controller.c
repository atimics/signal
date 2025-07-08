#include "unity.h"
#include "math/pd_controller.h"
#include <math.h>

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_pd_controller_3d_init(void) {
    PDController3D controller;
    pd_controller_3d_init(&controller, 2.0f, 0.5f, 100.0f);
    
    TEST_ASSERT_EQUAL_FLOAT(2.0f, controller.kp);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, controller.kd);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, controller.max_output);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.smoothing_factor);
    TEST_ASSERT_FALSE(controller.initialized);
}

void test_pd_controller_3d_basic_position_control(void) {
    PDController3D controller;
    pd_controller_3d_init(&controller, 2.0f, 0.5f, 100.0f);
    
    Vector3 current_pos = {0, 0, 0};
    Vector3 target_pos = {10, 0, 0};
    Vector3 current_vel = {0, 0, 0};
    Vector3 target_vel = {0, 0, 0};
    
    Vector3 output = pd_controller_3d_update(&controller, current_pos, target_pos, current_vel, target_vel, 0.016f);
    
    // With Kp=2, position error of 10 should give output of 20
    TEST_ASSERT_EQUAL_FLOAT(20.0f, output.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output.z);
}

void test_pd_controller_3d_velocity_damping(void) {
    PDController3D controller;
    pd_controller_3d_init(&controller, 0.0f, 1.0f, 100.0f);  // Only derivative gain
    
    Vector3 current_pos = {0, 0, 0};
    Vector3 target_pos = {0, 0, 0};
    Vector3 current_vel = {10, 0, 0};
    Vector3 target_vel = {0, 0, 0};
    
    Vector3 output = pd_controller_3d_update(&controller, current_pos, target_pos, current_vel, target_vel, 0.016f);
    
    // With Kd=1, velocity error of -10 should give output of -10
    TEST_ASSERT_EQUAL_FLOAT(-10.0f, output.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, output.z);
}

void test_pd_controller_3d_max_output_clamping(void) {
    PDController3D controller;
    pd_controller_3d_init(&controller, 10.0f, 0.0f, 50.0f);  // High gain, low max output
    
    Vector3 current_pos = {0, 0, 0};
    Vector3 target_pos = {100, 0, 0};  // Large error
    Vector3 current_vel = {0, 0, 0};
    Vector3 target_vel = {0, 0, 0};
    
    Vector3 output = pd_controller_3d_update(&controller, current_pos, target_pos, current_vel, target_vel, 0.016f);
    
    // Output should be clamped to max_output
    float magnitude = vector3_length(output);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, magnitude);
}

void test_pd_controller_3d_multi_axis_control(void) {
    PDController3D controller;
    pd_controller_3d_init(&controller, 1.0f, 0.5f, 100.0f);
    
    Vector3 current_pos = {0, 0, 0};
    Vector3 target_pos = {10, 5, -3};
    Vector3 current_vel = {2, -1, 0};
    Vector3 target_vel = {0, 0, 0};
    
    Vector3 output = pd_controller_3d_update(&controller, current_pos, target_pos, current_vel, target_vel, 0.016f);
    
    // Position errors: (10, 5, -3), velocity errors: (-2, 1, 0)
    // Output = Kp * pos_error + Kd * vel_error
    TEST_ASSERT_EQUAL_FLOAT(10.0f - 1.0f, output.x);  // 1*10 + 0.5*(-2) = 9
    TEST_ASSERT_EQUAL_FLOAT(5.0f + 0.5f, output.y);   // 1*5 + 0.5*1 = 5.5
    TEST_ASSERT_EQUAL_FLOAT(-3.0f, output.z);         // 1*(-3) + 0.5*0 = -3
}

void test_pd_controller_1d_basic(void) {
    PDController1D controller;
    pd_controller_1d_init(&controller, 2.0f, 0.5f, 100.0f);
    
    float output = pd_controller_1d_update(&controller, 0.0f, 10.0f, 0.0f, 0.0f, 0.016f);
    
    TEST_ASSERT_EQUAL_FLOAT(20.0f, output);  // 2 * 10 = 20
}

void test_pd_controller_auto_tune(void) {
    float kp, kd;
    float desired_response_time = 1.0f;  // 1 second to reach target
    float damping_ratio = 0.7f;          // Slightly underdamped for good response
    float mass = 1000.0f;                // 1000kg spacecraft
    
    pd_controller_auto_tune(&kp, &kd, desired_response_time, damping_ratio, mass);
    
    // Natural frequency should be approximately 4.6 rad/s
    float expected_wn = 4.6f / desired_response_time;
    float expected_kp = mass * expected_wn * expected_wn;
    float expected_kd = 2.0f * damping_ratio * mass * expected_wn;
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_kp, kp);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_kd, kd);
}

void test_pd_controller_3d_reset(void) {
    PDController3D controller;
    pd_controller_3d_init(&controller, 2.0f, 0.5f, 100.0f);
    
    // Do an update to initialize the controller
    Vector3 dummy = {0, 0, 0};
    pd_controller_3d_update(&controller, dummy, dummy, dummy, dummy, 0.016f);
    TEST_ASSERT_TRUE(controller.initialized);
    
    // Reset should clear the initialized flag
    pd_controller_3d_reset(&controller);
    TEST_ASSERT_FALSE(controller.initialized);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_pd_controller_3d_init);
    RUN_TEST(test_pd_controller_3d_basic_position_control);
    RUN_TEST(test_pd_controller_3d_velocity_damping);
    RUN_TEST(test_pd_controller_3d_max_output_clamping);
    RUN_TEST(test_pd_controller_3d_multi_axis_control);
    RUN_TEST(test_pd_controller_1d_basic);
    RUN_TEST(test_pd_controller_auto_tune);
    RUN_TEST(test_pd_controller_3d_reset);
    
    return UNITY_END();
}