#include "unity.h"
#include "component/unified_flight_control.h"
#include "core.h"
#include <math.h>

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_flight_assist_mode_switching(void) {
    UnifiedFlightControl* control = unified_flight_control_create();
    TEST_ASSERT_NOT_NULL(control);
    
    // Initially assist should be disabled
    TEST_ASSERT_FALSE(control->assist_enabled);
    
    // Enable assist directly
    unified_flight_control_enable_assist(control, true);
    TEST_ASSERT_TRUE(control->assist_enabled);
    
    // Disable assist
    unified_flight_control_enable_assist(control, false);
    TEST_ASSERT_FALSE(control->assist_enabled);
    
    // Test that switching to ASSISTED mode enables assist
    // First need to request authority
    unified_flight_control_request_authority(control, AUTHORITY_PLAYER, 1);
    unified_flight_control_set_mode(control, FLIGHT_CONTROL_ASSISTED);
    TEST_ASSERT_EQUAL(FLIGHT_CONTROL_ASSISTED, control->mode);
    TEST_ASSERT_TRUE(control->assist_enabled);
    
    // Switch back to manual disables assist
    unified_flight_control_set_mode(control, FLIGHT_CONTROL_MANUAL);
    TEST_ASSERT_FALSE(control->assist_enabled);
    
    unified_flight_control_destroy(control);
}

void test_flight_assist_target_calculation(void) {
    UnifiedFlightControl* control = unified_flight_control_create();
    struct Transform transform = {0};
    transform.position = (Vector3){0, 0, 0};
    transform.rotation = (Quaternion){0, 0, 0, 1};  // Identity
    
    // Test with forward input
    Vector3 input = {0, 0, 1};  // Forward
    Vector3 target = unified_flight_control_calculate_assist_target(control, &transform, input);
    
    // Target should be sphere_radius * responsiveness units forward
    float expected_distance = control->assist_sphere_radius * control->assist_responsiveness;
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, target.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, target.y);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_distance, target.z);
    
    // Test with diagonal input
    input = (Vector3){1, 0, 1};  // Forward-right
    target = unified_flight_control_calculate_assist_target(control, &transform, input);
    
    // Target should be on sphere at 45 degrees
    float expected_component = expected_distance * 0.7071f;  // sin(45°)
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_component, target.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, target.y);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_component, target.z);
    
    unified_flight_control_destroy(control);
}

void test_flight_assist_acceleration_calculation(void) {
    UnifiedFlightControl* control = unified_flight_control_create();
    unified_flight_control_enable_assist(control, true);
    
    struct Transform transform = {0};
    transform.position = (Vector3){0, 0, 0};
    
    struct Physics physics = {0};
    physics.velocity = (Vector3){0, 0, 0};
    
    // Set a target 10 units ahead
    control->assist_target_position = (Vector3){0, 0, 10};
    control->assist_target_velocity = (Vector3){0, 0, 0};
    
    Vector3 accel = unified_flight_control_get_assist_acceleration(control, &transform, &physics);
    
    // Acceleration should be positive Z (forward)
    TEST_ASSERT_TRUE(accel.z > 0);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, accel.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, accel.y);
    
    // Check it respects max acceleration
    float magnitude = vector3_length(accel);
    TEST_ASSERT_TRUE(magnitude <= control->assist_max_acceleration);
    
    unified_flight_control_destroy(control);
}

void test_flight_assist_responsiveness(void) {
    UnifiedFlightControl* control = unified_flight_control_create();
    
    // Test responsiveness setting
    unified_flight_control_set_assist_responsiveness(control, 0.5f);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, control->assist_responsiveness);
    
    // Test clamping
    unified_flight_control_set_assist_responsiveness(control, 2.0f);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, control->assist_responsiveness);
    
    unified_flight_control_set_assist_responsiveness(control, -1.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->assist_responsiveness);
    
    unified_flight_control_destroy(control);
}

void test_flight_assist_pd_parameters(void) {
    UnifiedFlightControl* control = unified_flight_control_create();
    
    // Test parameter setting
    unified_flight_control_set_assist_params(control, 3.0f, 0.8f, 40.0f);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, control->assist_kp);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, control->assist_kd);
    TEST_ASSERT_EQUAL_FLOAT(40.0f, control->assist_max_acceleration);
    
    // Test negative clamping
    unified_flight_control_set_assist_params(control, -1.0f, -2.0f, -10.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->assist_kp);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, control->assist_kd);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, control->assist_max_acceleration);  // Min is 1.0
    
    unified_flight_control_destroy(control);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_flight_assist_mode_switching);
    RUN_TEST(test_flight_assist_target_calculation);
    RUN_TEST(test_flight_assist_acceleration_calculation);
    RUN_TEST(test_flight_assist_responsiveness);
    RUN_TEST(test_flight_assist_pd_parameters);
    
    return UNITY_END();
}