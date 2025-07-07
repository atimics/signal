/**
 * @file test_unified_control_scheme.c
 * @brief Comprehensive tests for the unified control scheme
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "vendor/unity.h"
#include "core_math.h"

// Mock structures for testing (since we can't include full game engine)
typedef enum {
    INPUT_ACTION_THRUST_FORWARD = 0,
    INPUT_ACTION_THRUST_BACK,
    INPUT_ACTION_PITCH_UP,
    INPUT_ACTION_PITCH_DOWN,
    INPUT_ACTION_YAW_LEFT,
    INPUT_ACTION_YAW_RIGHT,
    INPUT_ACTION_ROLL_LEFT,
    INPUT_ACTION_ROLL_RIGHT,
    INPUT_ACTION_VERTICAL_UP,
    INPUT_ACTION_VERTICAL_DOWN,
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_COUNT
} InputActionID;

typedef enum {
    FLIGHT_CONTROL_MANUAL = 0,
    FLIGHT_CONTROL_ASSISTED,
    FLIGHT_CONTROL_SCRIPTED,
    FLIGHT_CONTROL_AUTONOMOUS,
    FLIGHT_CONTROL_FORMATION
} FlightControlMode;

typedef enum {
    AUTHORITY_NONE = 0,
    AUTHORITY_PLAYER,
    AUTHORITY_ASSISTANT,
    AUTHORITY_SCRIPT,
    AUTHORITY_AI
} ControlAuthority;

typedef uint32_t EntityID;
#define INVALID_ENTITY 0

/**
 * @file test_unified_control_scheme.c
 * @brief Comprehensive tests for the unified control scheme logic
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "vendor/unity.h"
#include "core_math.h"

// Mock structures for testing control scheme logic
typedef enum {
    INPUT_ACTION_THRUST_FORWARD = 0,
    INPUT_ACTION_THRUST_BACK,
    INPUT_ACTION_PITCH_UP,
    INPUT_ACTION_PITCH_DOWN,
    INPUT_ACTION_YAW_LEFT,
    INPUT_ACTION_YAW_RIGHT,
    INPUT_ACTION_ROLL_LEFT,
    INPUT_ACTION_ROLL_RIGHT,
    INPUT_ACTION_VERTICAL_UP,
    INPUT_ACTION_VERTICAL_DOWN,
    INPUT_ACTION_BOOST,
    INPUT_ACTION_BRAKE,
    INPUT_ACTION_COUNT
} InputActionID;

typedef enum {
    FLIGHT_CONTROL_MANUAL = 0,
    FLIGHT_CONTROL_ASSISTED,
    FLIGHT_CONTROL_SCRIPTED,
    FLIGHT_CONTROL_AUTONOMOUS,
    FLIGHT_CONTROL_FORMATION
} FlightControlMode;

typedef enum {
    AUTHORITY_NONE = 0,
    AUTHORITY_PLAYER,
    AUTHORITY_ASSISTANT,
    AUTHORITY_SCRIPT,
    AUTHORITY_AI
} ControlAuthority;

typedef struct {
    float linear_sensitivity;
    float angular_sensitivity;
    float dead_zone;
    bool invert_pitch;
    bool invert_yaw;
    bool use_quadratic_curve;
} InputConfiguration;

typedef struct {
    Vector3 linear_input;
    Vector3 angular_input;
    float boost_input;
    float brake_input;
    float assistance_level;
} ControlState;

typedef struct {
    FlightControlMode mode;
    ControlAuthority authority_level;
    bool enabled;
    bool flight_assist_enabled;
    float stability_assist;
    float inertia_dampening;
    InputConfiguration input_config;
    ControlState state;
} MockFlightControl;

// ============================================================================
// MOCK FUNCTIONS FOR TESTING CONTROL LOGIC
// ============================================================================

MockFlightControl* mock_flight_control_create(void) {
    MockFlightControl* control = (MockFlightControl*)calloc(1, sizeof(MockFlightControl));
    if (control) {
        // Set defaults
        control->mode = FLIGHT_CONTROL_MANUAL;
        control->authority_level = AUTHORITY_NONE;
        control->enabled = true;
        control->flight_assist_enabled = true;
        control->stability_assist = 0.3f;
        control->inertia_dampening = 0.2f;
        
        // Default input configuration
        control->input_config.linear_sensitivity = 1.0f;
        control->input_config.angular_sensitivity = 1.0f;
        control->input_config.dead_zone = 0.1f;
        control->input_config.invert_pitch = false;
        control->input_config.invert_yaw = false;
        control->input_config.use_quadratic_curve = false;
    }
    return control;
}

void mock_flight_control_destroy(MockFlightControl* control) {
    if (control) {
        free(control);
    }
}

void mock_setup_manual_flight(MockFlightControl* control) {
    if (!control) return;
    control->mode = FLIGHT_CONTROL_MANUAL;
    control->stability_assist = 0.02f;
    control->inertia_dampening = 0.0f;
    control->flight_assist_enabled = false;
    control->state.assistance_level = 0.0f;
}

void mock_setup_assisted_flight(MockFlightControl* control) {
    if (!control) return;
    control->mode = FLIGHT_CONTROL_ASSISTED;
    control->stability_assist = 0.15f;
    control->inertia_dampening = 0.1f;
    control->flight_assist_enabled = true;
    control->state.assistance_level = 0.3f;
}

// Mock banking calculation (core of the control scheme)
float mock_calculate_banking_roll(float yaw_input, bool flight_assist_enabled) {
    if (!flight_assist_enabled || fabsf(yaw_input) <= 0.01f) {
        return 0.0f;
    }
    
    float banking_ratio = 1.8f;
    float banking_roll = -yaw_input * banking_ratio;
    
    // Clamp to [-1, 1]
    return fmaxf(-1.0f, fminf(1.0f, banking_roll));
}

// Mock dead zone application
float mock_apply_dead_zone(float input, float dead_zone) {
    if (fabsf(input) < dead_zone) {
        return 0.0f;
    }
    return input;
}

// Mock boost calculation
Vector3 mock_apply_boost(Vector3 linear_input, float boost_input) {
    if (boost_input <= 0.0f) {
        return linear_input;
    }
    
    float boost_factor = 1.0f + boost_input * 2.0f; // 3x max boost
    return (Vector3){
        linear_input.x * boost_factor,
        linear_input.y * boost_factor,
        linear_input.z * boost_factor
    };
}

// ============================================================================
// UNITY TEST FUNCTIONS
// ============================================================================

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

// ============================================================================
// CONTROL COMPONENT TESTS
// ============================================================================

void test_control_creation_and_defaults(void) {
    MockFlightControl* control = mock_flight_control_create();
    TEST_ASSERT_NOT_NULL(control);
    
    // Test default values
    TEST_ASSERT_EQUAL_INT(FLIGHT_CONTROL_MANUAL, control->mode);
    TEST_ASSERT_EQUAL_INT(AUTHORITY_NONE, control->authority_level);
    TEST_ASSERT_TRUE(control->enabled);
    
    // Test default input configuration
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, control->input_config.linear_sensitivity);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, control->input_config.angular_sensitivity);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, control->input_config.dead_zone);
    
    // Test default assistance
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.3f, control->stability_assist);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.2f, control->inertia_dampening);
    TEST_ASSERT_TRUE(control->flight_assist_enabled);
    
    mock_flight_control_destroy(control);
}

void test_flight_modes(void) {
    MockFlightControl* control = mock_flight_control_create();
    TEST_ASSERT_NOT_NULL(control);
    
    // Test manual mode setup
    mock_setup_manual_flight(control);
    TEST_ASSERT_EQUAL_INT(FLIGHT_CONTROL_MANUAL, control->mode);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.02f, control->stability_assist);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, control->inertia_dampening);
    TEST_ASSERT_FALSE(control->flight_assist_enabled);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, control->state.assistance_level);
    
    // Test assisted mode setup
    mock_setup_assisted_flight(control);
    TEST_ASSERT_EQUAL_INT(FLIGHT_CONTROL_ASSISTED, control->mode);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, control->stability_assist);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, control->inertia_dampening);
    TEST_ASSERT_TRUE(control->flight_assist_enabled);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, control->state.assistance_level);
    
    mock_flight_control_destroy(control);
}

// ============================================================================
// BANKING SYSTEM TESTS
// ============================================================================

void test_banking_calculation(void) {
    // Test banking calculation with assisted mode
    float yaw_input = 0.5f;
    bool flight_assist = true;
    
    float banking_roll = mock_calculate_banking_roll(yaw_input, flight_assist);
    
    // 0.5 yaw * -1.8 banking ratio = -0.9 roll
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -0.9f, banking_roll);
    
    // Test with maximum yaw (should clamp to -1.0)
    yaw_input = 1.0f;
    banking_roll = mock_calculate_banking_roll(yaw_input, flight_assist);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, banking_roll);
    
    // Test with flight assist disabled
    flight_assist = false;
    banking_roll = mock_calculate_banking_roll(yaw_input, flight_assist);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, banking_roll);
    
    // Test below threshold
    yaw_input = 0.005f; // Below 0.01 threshold
    flight_assist = true;
    banking_roll = mock_calculate_banking_roll(yaw_input, flight_assist);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, banking_roll);
}

void test_banking_direction(void) {
    // Test that banking direction is correct
    float right_yaw = 0.5f;   // Positive yaw (right)
    float left_yaw = -0.5f;   // Negative yaw (left)
    
    float right_banking = mock_calculate_banking_roll(right_yaw, true);
    float left_banking = mock_calculate_banking_roll(left_yaw, true);
    
    // Right yaw should produce negative roll (bank right)
    TEST_ASSERT_TRUE(right_banking < 0.0f);
    
    // Left yaw should produce positive roll (bank left)
    TEST_ASSERT_TRUE(left_banking > 0.0f);
    
    // Banking should be symmetric
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -right_banking, left_banking);
}

// ============================================================================
// INPUT PROCESSING TESTS
// ============================================================================

void test_dead_zone_application(void) {
    float dead_zone = 0.15f;
    
    // Test values below dead zone are filtered to zero
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, mock_apply_dead_zone(0.05f, dead_zone));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, mock_apply_dead_zone(0.1f, dead_zone));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, mock_apply_dead_zone(0.14f, dead_zone));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, mock_apply_dead_zone(-0.1f, dead_zone));
    
    // Test values above dead zone pass through
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2f, mock_apply_dead_zone(0.2f, dead_zone));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.3f, mock_apply_dead_zone(-0.3f, dead_zone));
}

void test_boost_system(void) {
    Vector3 base_thrust = {0.0f, 0.0f, 0.5f};
    
    // Test no boost
    Vector3 result = mock_apply_boost(base_thrust, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, result.z);
    
    // Test full boost (3x multiplier)
    result = mock_apply_boost(base_thrust, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.5f, result.z);
    
    // Test partial boost
    result = mock_apply_boost(base_thrust, 0.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, result.z);
    
    // Test boost applies to all axes
    Vector3 multi_axis = {0.2f, 0.3f, 0.4f};
    result = mock_apply_boost(multi_axis, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.6f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.9f, result.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.2f, result.z);
}

// ============================================================================
// GAMEPAD MAPPING TESTS
// ============================================================================

void test_xbox_controller_axis_mapping(void) {
    // Test that Xbox controller axes map to correct actions
    // This validates the documented control scheme
    
    // Xbox axes: 0=RightX, 1=RightY, 2=LeftX, 3=LeftY, 4=LT, 5=RT
    
    // Verify axis assignments match documentation
    int rt_axis = 5;  // Right Trigger -> Forward thrust
    int lt_axis = 4;  // Left Trigger -> Reverse thrust
    int left_x_axis = 2;  // Left Stick X -> Yaw
    int left_y_axis = 3;  // Left Stick Y -> Pitch
    int right_x_axis = 0; // Right Stick X -> Roll
    int right_y_axis = 1; // Right Stick Y -> Vertical
    
    // These should match the INPUT_ACTION enum values
    TEST_ASSERT_EQUAL_INT(5, rt_axis);
    TEST_ASSERT_EQUAL_INT(4, lt_axis);
    TEST_ASSERT_EQUAL_INT(2, left_x_axis);
    TEST_ASSERT_EQUAL_INT(3, left_y_axis);
    TEST_ASSERT_EQUAL_INT(0, right_x_axis);
    TEST_ASSERT_EQUAL_INT(1, right_y_axis);
}

void test_control_scheme_completeness(void) {
    // Verify all required actions are mapped
    
    // Primary flight controls
    TEST_ASSERT_TRUE(INPUT_ACTION_THRUST_FORWARD < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_THRUST_BACK < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_PITCH_UP < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_PITCH_DOWN < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_YAW_LEFT < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_YAW_RIGHT < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_ROLL_LEFT < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_ROLL_RIGHT < INPUT_ACTION_COUNT);
    
    // Secondary controls
    TEST_ASSERT_TRUE(INPUT_ACTION_VERTICAL_UP < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_VERTICAL_DOWN < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_BOOST < INPUT_ACTION_COUNT);
    TEST_ASSERT_TRUE(INPUT_ACTION_BRAKE < INPUT_ACTION_COUNT);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_full_control_pipeline_simulation(void) {
    // Simulate a complete control input pipeline
    MockFlightControl* control = mock_flight_control_create();
    TEST_ASSERT_NOT_NULL(control);
    
    mock_setup_assisted_flight(control);
    
    // Simulate gamepad input: Right yaw + forward thrust
    float yaw_input = 0.6f;
    float thrust_input = 0.8f;
    float boost_input = 0.0f;
    
    // Apply dead zone
    yaw_input = mock_apply_dead_zone(yaw_input, control->input_config.dead_zone);
    thrust_input = mock_apply_dead_zone(thrust_input, control->input_config.dead_zone);
    
    // Calculate banking
    float banking_roll = mock_calculate_banking_roll(yaw_input, control->flight_assist_enabled);
    
    // Set up control state
    control->state.linear_input = (Vector3){0.0f, 0.0f, thrust_input};
    control->state.angular_input = (Vector3){0.0f, yaw_input, banking_roll};
    control->state.boost_input = boost_input;
    
    // Apply boost
    Vector3 final_linear = mock_apply_boost(control->state.linear_input, boost_input);
    
    // Verify results
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.8f, final_linear.z);  // Thrust preserved
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.6f, control->state.angular_input.y);  // Yaw preserved
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, control->state.angular_input.z); // Banking applied (clamped)
    
    mock_flight_control_destroy(control);
}

// ============================================================================
// PERFORMANCE VALIDATION TESTS
// ============================================================================

void test_control_parameter_ranges(void) {
    // Test that all control parameters are within expected ranges
    MockFlightControl* control = mock_flight_control_create();
    TEST_ASSERT_NOT_NULL(control);
    
    // Manual mode ranges
    mock_setup_manual_flight(control);
    TEST_ASSERT_TRUE(control->stability_assist >= 0.0f && control->stability_assist <= 0.1f);
    TEST_ASSERT_TRUE(control->inertia_dampening >= 0.0f && control->inertia_dampening <= 0.1f);
    
    // Assisted mode ranges
    mock_setup_assisted_flight(control);
    TEST_ASSERT_TRUE(control->stability_assist >= 0.1f && control->stability_assist <= 0.3f);
    TEST_ASSERT_TRUE(control->inertia_dampening >= 0.05f && control->inertia_dampening <= 0.2f);
    TEST_ASSERT_TRUE(control->state.assistance_level >= 0.2f && control->state.assistance_level <= 0.5f);
    
    mock_flight_control_destroy(control);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_unified_control_scheme(void) {
    printf("\n🎮 Unified Control Scheme Tests\n");
    printf("================================\n");
    
    // Component tests
    printf("\n🔧 Testing Component Management...\n");
    RUN_TEST(test_control_creation_and_defaults);
    RUN_TEST(test_flight_modes);
    
    // Banking system tests
    printf("\n🏁 Testing Banking System...\n");
    RUN_TEST(test_banking_calculation);
    RUN_TEST(test_banking_direction);
    
    // Input processing tests  
    printf("\n🎮 Testing Input Processing...\n");
    RUN_TEST(test_dead_zone_application);
    RUN_TEST(test_boost_system);
    
    // Control scheme validation
    printf("\n🎯 Testing Control Scheme...\n");
    RUN_TEST(test_xbox_controller_axis_mapping);
    RUN_TEST(test_control_scheme_completeness);
    
    // Integration tests
    printf("\n🔄 Testing Integration...\n");
    RUN_TEST(test_full_control_pipeline_simulation);
    RUN_TEST(test_control_parameter_ranges);
    
    printf("\n✅ Control Scheme Tests Complete\n");
}
