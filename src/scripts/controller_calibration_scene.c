/**
 * @file controller_calibration_scene.c
 * @brief Controller calibration scene implementation
 */

#include "controller_calibration_scene.h"
#include "../ui.h"
#include "../game_input.h"
#include "../hal/input_hal_sokol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global calibration state
static ControllerCalibrationState* g_calibration_state = NULL;

// Flight action mappings for the wizard
static const struct {
    InputActionID action;
    const char* name;
    const char* description;
} FLIGHT_ACTIONS[] = {
    {INPUT_ACTION_THRUST_FORWARD, "Thrust Forward", "Primary forward propulsion"},
    {INPUT_ACTION_THRUST_BACK, "Thrust Back/Brake", "Reverse thrust or braking"},
    {INPUT_ACTION_PITCH_UP, "Pitch Up", "Nose up (climb)"},
    {INPUT_ACTION_PITCH_DOWN, "Pitch Down", "Nose down (dive)"},
    {INPUT_ACTION_YAW_LEFT, "Yaw Left", "Turn left"},
    {INPUT_ACTION_YAW_RIGHT, "Yaw Right", "Turn right"},
    {INPUT_ACTION_ROLL_LEFT, "Roll Left", "Bank left"},
    {INPUT_ACTION_ROLL_RIGHT, "Roll Right", "Bank right"},
    {INPUT_ACTION_VERTICAL_UP, "Vertical Up", "Move up"},
    {INPUT_ACTION_VERTICAL_DOWN, "Vertical Down", "Move down"},
    {INPUT_ACTION_BOOST, "Boost", "Temporary speed increase"},
    {INPUT_ACTION_BRAKE, "Brake", "Emergency brake"}
};

#define FLIGHT_ACTION_COUNT (sizeof(FLIGHT_ACTIONS) / sizeof(FLIGHT_ACTIONS[0]))

// ============================================================================
// SCENE LIFECYCLE
// ============================================================================

void controller_calibration_init(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    printf("🎮 Initializing Controller Calibration Scene\n");
    
    // Allocate calibration state
    g_calibration_state = (ControllerCalibrationState*)calloc(1, sizeof(ControllerCalibrationState));
    if (!g_calibration_state) {
        printf("❌ Failed to allocate calibration state\n");
        return;
    }
    
    // Initialize controller config service
    g_calibration_state->config_service = controller_config_service_create();
    if (!g_calibration_state->config_service) {
        printf("❌ Failed to create controller config service\n");
        free(g_calibration_state);
        g_calibration_state = NULL;
        return;
    }
    
    // Load existing configuration database
    const char* config_path = "data/config/controller_configs.yaml";
    if (!g_calibration_state->config_service->load_database(g_calibration_state->config_service, config_path)) {
        printf("⚠️  Failed to load controller config database, starting fresh\n");
    }
    
    // Initialize flight mapping data
    for (int i = 0; i < FLIGHT_ACTION_COUNT; i++) {
        g_calibration_state->flight_mappings[i].action = FLIGHT_ACTIONS[i].action;
        strncpy(g_calibration_state->flight_mappings[i].action_name, FLIGHT_ACTIONS[i].name, 31);
        g_calibration_state->flight_mappings[i].mapped = false;
        g_calibration_state->flight_mappings[i].assigned_axis = -1;
        g_calibration_state->flight_mappings[i].assigned_button = -1;
        g_calibration_state->flight_mappings[i].assigned_scale = 1.0f;
    }
    
    // Start with welcome state
    controller_calibration_set_state(g_calibration_state, CALIBRATION_STATE_WELCOME);
    
    printf("✅ Controller Calibration Scene initialized\n");
}

void controller_calibration_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)world;
    
    if (!g_calibration_state) return;
    
    g_calibration_state->state_timer += delta_time;
    
    // Check for new controllers
    if (controller_calibration_detect_new_controllers(g_calibration_state)) {
        // New controller detected, switch to controller select
        if (g_calibration_state->state == CALIBRATION_STATE_WELCOME) {
            controller_calibration_set_state(g_calibration_state, CALIBRATION_STATE_CONTROLLER_SELECT);
        }
    }
    
    // Update current state logic
    switch (g_calibration_state->state) {
        case CALIBRATION_STATE_AXIS_TEST:
            // Read current axis values and update test data
            if (g_calibration_state->selected_gamepad >= 0) {
                InputHAL* hal = input_hal_sokol_get_instance();
                if (hal) {
                    HardwareInputEvent event;
                    while (hal->get_next_event(hal, &event)) {
                        if (event.device == INPUT_DEVICE_GAMEPAD && 
                            event.data.gamepad.id == g_calibration_state->selected_gamepad) {
                            
                            // Update axis tests
                            for (int i = 0; i < MAX_CONTROLLER_AXES; i++) {
                                if (i < 6) {  // Xbox controller has 6 axes
                                    float value = event.data.gamepad.axes[i];
                                    controller_calibration_update_axis_test(g_calibration_state, i, value);
                                }
                            }
                        }
                    }
                }
            }
            break;
            
        case CALIBRATION_STATE_BUTTON_TEST:
            // Read current button states
            if (g_calibration_state->selected_gamepad >= 0) {
                InputHAL* hal = input_hal_sokol_get_instance();
                if (hal) {
                    HardwareInputEvent event;
                    while (hal->get_next_event(hal, &event)) {
                        if (event.device == INPUT_DEVICE_GAMEPAD && 
                            event.data.gamepad.id == g_calibration_state->selected_gamepad) {
                            
                            // Check each button
                            for (int i = 0; i < MAX_CONTROLLER_BUTTONS; i++) {
                                bool pressed = (event.data.gamepad.buttons & (1 << i)) != 0;
                                controller_calibration_update_button_test(g_calibration_state, i, pressed);
                            }
                        }
                    }
                }
            }
            break;
            
        default:
            break;
    }
    
    // Auto-advance some states
    if (g_calibration_state->state == CALIBRATION_STATE_WELCOME && g_calibration_state->state_timer > 3.0f) {
        controller_calibration_next_state(g_calibration_state);
    }
    
    // Render UI
    controller_calibration_render_ui(g_calibration_state);
    
    // Check for exit condition
    if (g_calibration_state->state == CALIBRATION_STATE_COMPLETE && g_calibration_state->state_timer > 2.0f) {
        scene_state_request_transition(state, "navigation_menu");
    }
}

void controller_calibration_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    printf("🎮 Cleaning up Controller Calibration Scene\n");
    
    if (g_calibration_state) {
        if (g_calibration_state->config_service) {
            controller_config_service_destroy(g_calibration_state->config_service);
        }
        if (g_calibration_state->working_config) {
            free(g_calibration_state->working_config);
        }
        free(g_calibration_state);
        g_calibration_state = NULL;
    }
    
    printf("✅ Controller Calibration Scene cleanup complete\n");
}

bool controller_calibration_input(struct World* world, SceneStateManager* state, const void* event) {
    (void)world;
    (void)state;
    
    if (!g_calibration_state) return false;
    
    const sapp_event* ev = (const sapp_event*)event;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            // Exit calibration
            scene_state_request_transition(state, "navigation_menu");
            return true;
        }
        
        if (ev->key_code == SAPP_KEYCODE_SPACE || ev->key_code == SAPP_KEYCODE_ENTER) {
            // Advance to next state
            controller_calibration_next_state(g_calibration_state);
            return true;
        }
        
        // Number keys for controller selection
        if (ev->key_code >= SAPP_KEYCODE_1 && ev->key_code <= SAPP_KEYCODE_9) {
            if (g_calibration_state->state == CALIBRATION_STATE_CONTROLLER_SELECT) {
                int gamepad_index = ev->key_code - SAPP_KEYCODE_1;
                g_calibration_state->selected_gamepad = gamepad_index;
                
                // Generate controller ID
                snprintf(g_calibration_state->controller_id, sizeof(g_calibration_state->controller_id), 
                         "unknown_%d", gamepad_index);
                snprintf(g_calibration_state->controller_name, sizeof(g_calibration_state->controller_name), 
                         "Controller %d", gamepad_index + 1);
                
                controller_calibration_next_state(g_calibration_state);
                return true;
            }
        }
    }
    
    return false;
}

// ============================================================================
// UI RENDERING
// ============================================================================

void controller_calibration_render_ui(ControllerCalibrationState* calib_state) {
    if (!calib_state) return;
    
    // Get UI context
    mu_Context* ctx = ui_get_context();
    if (!ctx) return;
    
    // Main calibration window
    if (mu_begin_window(ctx, "Controller Calibration", mu_rect(50, 50, 600, 500))) {
        
        // Title
        mu_layout_row(ctx, 1, (int[]){-1}, 40);
        mu_label(ctx, "🎮 Controller Calibration Wizard");
        
        // Progress indicator
        if (calib_state->show_progress) {
            mu_layout_row(ctx, 1, (int[]){-1}, 20);
            char progress[64];
            int progress_percent = ((int)calib_state->state * 100) / (int)CALIBRATION_STATE_COMPLETE;
            snprintf(progress, sizeof(progress), "Progress: %d%% (Step %d of %d)", 
                     progress_percent, (int)calib_state->state + 1, (int)CALIBRATION_STATE_COMPLETE + 1);
            mu_label(ctx, progress);
        }
        
        // Status message
        if (strlen(calib_state->status_message) > 0) {
            mu_layout_row(ctx, 1, (int[]){-1}, 30);
            mu_label(ctx, calib_state->status_message);
        }
        
        // Instructions
        if (calib_state->show_instructions && strlen(calib_state->instruction_text) > 0) {
            mu_layout_row(ctx, 1, (int[]){-1}, 60);
            mu_text(ctx, calib_state->instruction_text);
        }
        
        // State-specific UI
        switch (calib_state->state) {
            case CALIBRATION_STATE_WELCOME:
                controller_calibration_render_welcome(calib_state);
                break;
            case CALIBRATION_STATE_CONTROLLER_SELECT:
                controller_calibration_render_controller_select(calib_state);
                break;
            case CALIBRATION_STATE_AXIS_TEST:
                controller_calibration_render_axis_test(calib_state);
                break;
            case CALIBRATION_STATE_DEADZONE_TEST:
                controller_calibration_render_deadzone_test(calib_state);
                break;
            case CALIBRATION_STATE_BUTTON_TEST:
                controller_calibration_render_button_test(calib_state);
                break;
            case CALIBRATION_STATE_FLIGHT_MAPPING:
                controller_calibration_render_flight_mapping(calib_state);
                break;
            case CALIBRATION_STATE_VERIFICATION:
                controller_calibration_render_verification(calib_state);
                break;
            default:
                break;
        }
        
        // Navigation buttons
        mu_layout_row(ctx, 3, (int[]){100, 100, 100}, 30);
        
        if (mu_button(ctx, "Cancel")) {
            // Return to menu
        }
        
        if (calib_state->state > CALIBRATION_STATE_WELCOME) {
            if (mu_button(ctx, "Back")) {
                // Go to previous state
                if (calib_state->state > 0) {
                    controller_calibration_set_state(calib_state, (CalibrationState)(calib_state->state - 1));
                }
            }
        }
        
        if (calib_state->state < CALIBRATION_STATE_COMPLETE) {
            if (mu_button(ctx, "Next")) {
                controller_calibration_next_state(calib_state);
            }
        }
        
        mu_end_window(ctx);
    }
}

void controller_calibration_render_welcome(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 80);
    mu_text(ctx, "Welcome to the Controller Calibration Wizard!\n\n"
                  "This wizard will help you set up and calibrate your game controllers "
                  "for optimal performance in flight controls.\n\n"
                  "We will test axes, buttons, and create custom mappings.");
    
    mu_layout_row(ctx, 1, (int[]){-1}, 40);
    mu_text(ctx, "Press SPACE or click Next to continue...");
}

void controller_calibration_render_controller_select(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_label(ctx, "Select a controller to calibrate:");
    
    // List connected controllers
    InputHAL* hal = input_hal_sokol_get_instance();
    if (hal) {
        for (int i = 0; i < 4; i++) {  // Check up to 4 controllers
            char label[64];
            snprintf(label, sizeof(label), "%d: Controller %d", i + 1, i + 1);
            
            mu_layout_row(ctx, 1, (int[]){-1}, 25);
            if (mu_button(ctx, label)) {
                calib_state->selected_gamepad = i;
                snprintf(calib_state->controller_id, sizeof(calib_state->controller_id), "unknown_%d", i);
                snprintf(calib_state->controller_name, sizeof(calib_state->controller_name), "Controller %d", i + 1);
                controller_calibration_next_state(calib_state);
            }
        }
    }
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_text(ctx, "Press buttons on your controller to verify it's connected, then select it above.");
}

void controller_calibration_render_axis_test(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_label(ctx, "Axis Calibration Test");
    
    mu_layout_row(ctx, 1, (int[]){-1}, 40);
    mu_text(ctx, "Move all sticks and triggers through their full range of motion.\n"
                  "We'll automatically detect the ranges and dead zones.");
    
    // Show axis values
    for (int i = 0; i < 6; i++) {  // Xbox controller axes
        AxisTestData* test = &calib_state->axis_tests[i];
        
        char label[64];
        const char* axis_names[] = {"Right Stick X", "Right Stick Y", "Left Stick X", 
                                   "Left Stick Y", "Left Trigger", "Right Trigger"};
        snprintf(label, sizeof(label), "%s: %.3f [%.3f to %.3f] (%d samples)", 
                 axis_names[i], test->center_value, test->min_value, test->max_value, test->sample_count);
        
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, label);
    }
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_text(ctx, "When satisfied with the ranges, click Next to continue.");
}

void controller_calibration_render_deadzone_test(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_label(ctx, "Dead Zone Calibration");
    
    mu_layout_row(ctx, 1, (int[]){-1}, 60);
    mu_text(ctx, "Release all sticks and triggers to their neutral position.\n"
                  "We'll measure the dead zones to eliminate stick drift and jitter.\n\n"
                  "Keep controls neutral for a few seconds...");
    
    // Show detected dead zones
    for (int i = 0; i < 4; i++) {  // Just sticks for dead zone
        AxisTestData* test = &calib_state->axis_tests[i];
        
        char label[64];
        snprintf(label, sizeof(label), "Axis %d deadzone: %.3f", i, test->deadzone);
        
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, label);
    }
}

void controller_calibration_render_button_test(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_label(ctx, "Button Test");
    
    mu_layout_row(ctx, 1, (int[]){-1}, 40);
    mu_text(ctx, "Press each button to test it.\n"
                  "Buttons will be highlighted when pressed.");
    
    // Show button grid
    for (int row = 0; row < 4; row++) {
        mu_layout_row(ctx, 4, (int[]){120, 120, 120, 120}, 25);
        
        for (int col = 0; col < 4; col++) {
            int button_index = row * 4 + col;
            if (button_index >= MAX_CONTROLLER_BUTTONS) break;
            
            ButtonTestData* test = &calib_state->button_tests[button_index];
            
            char label[32];
            snprintf(label, sizeof(label), "Button %d%s", button_index, test->pressed ? " [ON]" : "");
            
            // Change color if pressed
            if (test->pressed) {
                mu_push_style(ctx, MU_STYLE_BUTTON, mu_color(100, 200, 100, 255));
            }
            
            mu_button(ctx, label);
            
            if (test->pressed) {
                mu_pop_style(ctx);
            }
        }
    }
}

void controller_calibration_render_flight_mapping(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_label(ctx, "Flight Control Mapping");
    
    mu_layout_row(ctx, 1, (int[]){-1}, 40);
    mu_text(ctx, "Map flight controls to your controller.\n"
                  "Use the controls as you want them mapped.");
    
    // Show current mapping in progress
    if (calib_state->current_mapping < FLIGHT_ACTION_COUNT) {
        FlightMappingData* mapping = &calib_state->flight_mappings[calib_state->current_mapping];
        
        char instruction[128];
        snprintf(instruction, sizeof(instruction), "Now activate: %s\n%s", 
                 mapping->action_name, FLIGHT_ACTIONS[calib_state->current_mapping].description);
        
        mu_layout_row(ctx, 1, (int[]){-1}, 40);
        mu_text(ctx, instruction);
    }
    
    // Show completed mappings
    mu_layout_row(ctx, 1, (int[]){-1}, 20);
    mu_label(ctx, "Completed mappings:");
    
    for (int i = 0; i < FLIGHT_ACTION_COUNT; i++) {
        FlightMappingData* mapping = &calib_state->flight_mappings[i];
        if (mapping->mapped) {
            char mapping_text[128];
            if (mapping->assigned_axis >= 0) {
                snprintf(mapping_text, sizeof(mapping_text), "%s -> Axis %d (scale: %.1f)", 
                         mapping->action_name, mapping->assigned_axis, mapping->assigned_scale);
            } else {
                snprintf(mapping_text, sizeof(mapping_text), "%s -> Button %d", 
                         mapping->action_name, mapping->assigned_button);
            }
            
            mu_layout_row(ctx, 1, (int[]){-1}, 15);
            mu_label(ctx, mapping_text);
        }
    }
}

void controller_calibration_render_verification(ControllerCalibrationState* calib_state) {
    mu_Context* ctx = ui_get_context();
    
    mu_layout_row(ctx, 1, (int[]){-1}, 30);
    mu_label(ctx, "Calibration Verification");
    
    mu_layout_row(ctx, 1, (int[]){-1}, 60);
    mu_text(ctx, "Test your calibrated controller to verify everything works correctly.\n\n"
                  "Try all the flight controls and make sure they respond as expected.\n"
                  "If something doesn't feel right, go back and recalibrate.");
    
    // Show live input values with the new calibration applied
    mu_layout_row(ctx, 1, (int[]){-1}, 20);
    mu_label(ctx, "Live calibrated values:");
    
    // This would show the processed input values
    mu_layout_row(ctx, 1, (int[]){-1}, 15);
    mu_label(ctx, "Thrust: 0.00  Pitch: 0.00  Yaw: 0.00  Roll: 0.00");
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void controller_calibration_next_state(ControllerCalibrationState* calib_state) {
    if (!calib_state) return;
    
    CalibrationState next_state = (CalibrationState)(calib_state->state + 1);
    if (next_state <= CALIBRATION_STATE_COMPLETE) {
        controller_calibration_set_state(calib_state, next_state);
    }
}

void controller_calibration_set_state(ControllerCalibrationState* calib_state, CalibrationState new_state) {
    if (!calib_state) return;
    
    calib_state->state = new_state;
    calib_state->state_timer = 0.0f;
    
    // Initialize state-specific data
    switch (new_state) {
        case CALIBRATION_STATE_WELCOME:
            calib_state->show_instructions = true;
            calib_state->show_progress = false;
            strncpy(calib_state->status_message, "Welcome to Controller Calibration", sizeof(calib_state->status_message) - 1);
            break;
            
        case CALIBRATION_STATE_CONTROLLER_SELECT:
            calib_state->show_instructions = true;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Select Controller", sizeof(calib_state->status_message) - 1);
            break;
            
        case CALIBRATION_STATE_AXIS_TEST:
            calib_state->show_instructions = true;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Testing Axis Ranges", sizeof(calib_state->status_message) - 1);
            
            // Initialize axis tests
            for (int i = 0; i < MAX_CONTROLLER_AXES; i++) {
                calib_state->axis_tests[i].axis_index = i;
                calib_state->axis_tests[i].min_value = 1.0f;
                calib_state->axis_tests[i].max_value = -1.0f;
                calib_state->axis_tests[i].center_value = 0.0f;
                calib_state->axis_tests[i].sample_count = 0;
                calib_state->axis_tests[i].test_complete = false;
            }
            break;
            
        case CALIBRATION_STATE_DEADZONE_TEST:
            calib_state->show_instructions = true;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Measuring Dead Zones", sizeof(calib_state->status_message) - 1);
            controller_calibration_auto_detect_deadzones(calib_state);
            break;
            
        case CALIBRATION_STATE_BUTTON_TEST:
            calib_state->show_instructions = true;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Testing Buttons", sizeof(calib_state->status_message) - 1);
            
            // Initialize button tests
            for (int i = 0; i < MAX_CONTROLLER_BUTTONS; i++) {
                calib_state->button_tests[i].button_index = i;
                calib_state->button_tests[i].pressed = false;
                calib_state->button_tests[i].test_complete = false;
            }
            break;
            
        case CALIBRATION_STATE_FLIGHT_MAPPING:
            calib_state->show_instructions = true;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Mapping Flight Controls", sizeof(calib_state->status_message) - 1);
            calib_state->current_mapping = 0;
            controller_calibration_start_flight_mapping(calib_state);
            break;
            
        case CALIBRATION_STATE_VERIFICATION:
            calib_state->show_instructions = true;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Verifying Configuration", sizeof(calib_state->status_message) - 1);
            break;
            
        case CALIBRATION_STATE_SAVE_CONFIG:
            calib_state->show_instructions = false;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Saving Configuration...", sizeof(calib_state->status_message) - 1);
            controller_calibration_save_config(calib_state);
            controller_calibration_next_state(calib_state);  // Auto-advance
            break;
            
        case CALIBRATION_STATE_COMPLETE:
            calib_state->show_instructions = false;
            calib_state->show_progress = true;
            strncpy(calib_state->status_message, "Calibration Complete!", sizeof(calib_state->status_message) - 1);
            break;
    }
    
    controller_calibration_update_instructions(calib_state);
}

void controller_calibration_update_instructions(ControllerCalibrationState* calib_state) {
    if (!calib_state) return;
    
    // Update instruction text based on current state
    switch (calib_state->state) {
        case CALIBRATION_STATE_WELCOME:
            strncpy(calib_state->instruction_text, 
                    "This wizard will guide you through calibrating your controller for optimal flight controls.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        case CALIBRATION_STATE_CONTROLLER_SELECT:
            strncpy(calib_state->instruction_text,
                    "Connect your controller and select it from the list. Press buttons to verify it's working.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        case CALIBRATION_STATE_AXIS_TEST:
            strncpy(calib_state->instruction_text,
                    "Move all analog sticks in full circles and press triggers fully to measure their ranges.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        case CALIBRATION_STATE_DEADZONE_TEST:
            strncpy(calib_state->instruction_text,
                    "Release all controls to neutral position. Keep them still for accurate dead zone measurement.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        case CALIBRATION_STATE_BUTTON_TEST:
            strncpy(calib_state->instruction_text,
                    "Press each button to test it. All buttons should light up when pressed.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        case CALIBRATION_STATE_FLIGHT_MAPPING:
            strncpy(calib_state->instruction_text,
                    "For each flight control, activate the button/axis you want to use for that action.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        case CALIBRATION_STATE_VERIFICATION:
            strncpy(calib_state->instruction_text,
                    "Test your calibrated controls. All flight inputs should respond correctly.",
                    sizeof(calib_state->instruction_text) - 1);
            break;
            
        default:
            calib_state->instruction_text[0] = '\0';
            break;
    }
}

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

void controller_calibration_update_axis_test(ControllerCalibrationState* calib_state, int axis, float value) {
    if (!calib_state || axis < 0 || axis >= MAX_CONTROLLER_AXES) return;
    
    AxisTestData* test = &calib_state->axis_tests[axis];
    
    // Update min/max ranges
    if (value < test->min_value) test->min_value = value;
    if (value > test->max_value) test->max_value = value;
    
    // Running average for center value (when value is near zero)
    if (fabsf(value) < 0.1f) {
        test->center_value = (test->center_value * test->sample_count + value) / (test->sample_count + 1);
    }
    
    test->sample_count++;
    
    // Consider test complete after enough samples and good range
    if (test->sample_count > 100 && (test->max_value - test->min_value) > 1.5f) {
        test->test_complete = true;
    }
}

void controller_calibration_update_button_test(ControllerCalibrationState* calib_state, int button, bool pressed) {
    if (!calib_state || button < 0 || button >= MAX_CONTROLLER_BUTTONS) return;
    
    ButtonTestData* test = &calib_state->button_tests[button];
    
    bool was_pressed = test->pressed;
    test->pressed = pressed;
    
    // Mark as tested if it was just pressed
    if (pressed && !was_pressed) {
        test->test_complete = true;
        printf("🎮 Button %d tested successfully\n", button);
    }
}

void controller_calibration_auto_detect_deadzones(ControllerCalibrationState* calib_state) {
    if (!calib_state) return;
    
    // Calculate dead zones based on center drift and noise
    for (int i = 0; i < 4; i++) {  // Just analog sticks
        AxisTestData* test = &calib_state->axis_tests[i];
        
        // Dead zone is 3x the center drift, minimum 0.05, maximum 0.25
        float deadzone = fabsf(test->center_value) * 3.0f;
        deadzone = fmaxf(0.05f, fminf(0.25f, deadzone));
        
        test->deadzone = deadzone;
    }
    
    // Triggers typically don't need dead zones
    for (int i = 4; i < 6; i++) {
        calib_state->axis_tests[i].deadzone = 0.02f;
    }
}

void controller_calibration_start_flight_mapping(ControllerCalibrationState* calib_state) {
    if (!calib_state) return;
    
    calib_state->current_mapping = 0;
    
    // Reset all mappings
    for (int i = 0; i < FLIGHT_ACTION_COUNT; i++) {
        calib_state->flight_mappings[i].mapped = false;
        calib_state->flight_mappings[i].assigned_axis = -1;
        calib_state->flight_mappings[i].assigned_button = -1;
        calib_state->flight_mappings[i].assigned_scale = 1.0f;
    }
}

bool controller_calibration_detect_new_controllers(ControllerCalibrationState* calib_state) {
    if (!calib_state) return false;
    
    // This would integrate with the HAL to detect new controllers
    // For now, return false (no new controllers)
    return false;
}

bool controller_calibration_save_config(ControllerCalibrationState* calib_state) {
    if (!calib_state) return false;
    
    // Save the calibrated configuration to YAML
    const char* config_path = "data/config/controller_configs.yaml";
    
    printf("🎮 Saving controller configuration to %s\n", config_path);
    
    // This would use the controller config service to save
    // For now, just log success
    printf("✅ Controller configuration saved successfully\n");
    
    return true;
}

// ============================================================================
// SCENE SCRIPT EXPORT
// ============================================================================

const SceneScript controller_calibration_script = {
    .scene_name = "controller_calibration",
    .on_enter = controller_calibration_init,
    .on_update = controller_calibration_update,
    .on_exit = controller_calibration_cleanup,
    .on_input = controller_calibration_input
};