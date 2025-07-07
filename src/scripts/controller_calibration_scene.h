/**
 * @file controller_calibration_scene.h
 * @brief Controller calibration scene with UI wizard
 * 
 * Provides a comprehensive controller calibration wizard that:
 * - Detects all connected controllers
 * - Tests each axis and button
 * - Measures deadzones and ranges
 * - Creates YAML configuration files
 * - Integrates with the UI system
 */

#ifndef CONTROLLER_CALIBRATION_SCENE_H
#define CONTROLLER_CALIBRATION_SCENE_H

#include "../scene_script.h"
#include "../services/controller_config.h"

// Calibration wizard states
typedef enum {
    CALIBRATION_STATE_WELCOME = 0,
    CALIBRATION_STATE_CONTROLLER_SELECT,
    CALIBRATION_STATE_AXIS_TEST,
    CALIBRATION_STATE_DEADZONE_TEST,
    CALIBRATION_STATE_BUTTON_TEST,
    CALIBRATION_STATE_FLIGHT_MAPPING,
    CALIBRATION_STATE_VERIFICATION,
    CALIBRATION_STATE_SAVE_CONFIG,
    CALIBRATION_STATE_COMPLETE
} CalibrationState;

// Axis test data
typedef struct {
    int axis_index;
    float min_value;
    float max_value;
    float center_value;
    float deadzone;
    int sample_count;
    bool test_complete;
} AxisTestData;

// Button test data
typedef struct {
    int button_index;
    bool pressed;
    bool test_complete;
    char assigned_name[32];
} ButtonTestData;

// Flight mapping test data
typedef struct {
    InputActionID action;
    char action_name[32];
    bool mapped;
    int assigned_axis;
    int assigned_button;
    float assigned_scale;
} FlightMappingData;

// Calibration scene state
typedef struct {
    // Current state
    CalibrationState state;
    float state_timer;
    
    // Selected controller
    int selected_gamepad;
    char controller_id[MAX_CONTROLLER_ID];
    char controller_name[MAX_CONTROLLER_NAME];
    
    // Test data
    int current_axis;
    int current_button;
    int current_mapping;
    
    AxisTestData axis_tests[MAX_CONTROLLER_AXES];
    ButtonTestData button_tests[MAX_CONTROLLER_BUTTONS];
    FlightMappingData flight_mappings[16];  // All flight actions
    
    // Configuration being built
    ControllerConfig* working_config;
    
    // UI state
    bool show_instructions;
    bool show_progress;
    char status_message[256];
    char instruction_text[512];
    
    // Services
    ControllerConfigService* config_service;
} ControllerCalibrationState;

// Scene functions
void controller_calibration_init(struct World* world, SceneStateManager* state);
void controller_calibration_update(struct World* world, SceneStateManager* state, float delta_time);
void controller_calibration_cleanup(struct World* world, SceneStateManager* state);
bool controller_calibration_input(struct World* world, SceneStateManager* state, const void* event);

// UI functions
void controller_calibration_render_ui(ControllerCalibrationState* calib_state);
void controller_calibration_render_welcome(ControllerCalibrationState* calib_state);
void controller_calibration_render_controller_select(ControllerCalibrationState* calib_state);
void controller_calibration_render_axis_test(ControllerCalibrationState* calib_state);
void controller_calibration_render_deadzone_test(ControllerCalibrationState* calib_state);
void controller_calibration_render_button_test(ControllerCalibrationState* calib_state);
void controller_calibration_render_flight_mapping(ControllerCalibrationState* calib_state);
void controller_calibration_render_verification(ControllerCalibrationState* calib_state);

// State management
void controller_calibration_next_state(ControllerCalibrationState* calib_state);
void controller_calibration_set_state(ControllerCalibrationState* calib_state, CalibrationState new_state);
void controller_calibration_update_instructions(ControllerCalibrationState* calib_state);

// Test functions
void controller_calibration_start_axis_test(ControllerCalibrationState* calib_state, int axis);
void controller_calibration_update_axis_test(ControllerCalibrationState* calib_state, int axis, float value);
void controller_calibration_finish_axis_test(ControllerCalibrationState* calib_state, int axis);

void controller_calibration_start_button_test(ControllerCalibrationState* calib_state, int button);
void controller_calibration_update_button_test(ControllerCalibrationState* calib_state, int button, bool pressed);
void controller_calibration_finish_button_test(ControllerCalibrationState* calib_state, int button);

void controller_calibration_start_flight_mapping(ControllerCalibrationState* calib_state);
void controller_calibration_map_action(ControllerCalibrationState* calib_state, InputActionID action, int axis_or_button, bool is_axis, float scale);

// Utility functions
bool controller_calibration_detect_new_controllers(ControllerCalibrationState* calib_state);
void controller_calibration_auto_detect_deadzones(ControllerCalibrationState* calib_state);
bool controller_calibration_save_config(ControllerCalibrationState* calib_state);

// Scene script export
extern const SceneScript controller_calibration_script;

#endif // CONTROLLER_CALIBRATION_SCENE_H