/**
 * @file controller_calibration_scene_simple.c
 * @brief Simple controller calibration scene implementation
 */

#include "controller_calibration_scene.h"
#include "../ui.h"
#include "../ui_microui.h"
#include "../game_input.h"
#include "../microui/microui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple calibration state
static struct {
    bool initialized;
    float timer;
    char status[256];
} g_simple_calib = {0};

// ============================================================================
// SCENE LIFECYCLE
// ============================================================================

void controller_calibration_init(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    printf("🎮 Initializing Simple Controller Calibration Scene\n");
    
    g_simple_calib.initialized = true;
    g_simple_calib.timer = 0.0f;
    strcpy(g_simple_calib.status, "Controller calibration scene loaded successfully!");
    
    printf("✅ Simple Controller Calibration Scene initialized\n");
}

// Forward declaration for UI rendering function
void simple_controller_calibration_render_ui(void);

void controller_calibration_update(struct World* world, SceneStateManager* state, float delta_time) {
    (void)world;
    (void)state;
    
    if (!g_simple_calib.initialized) return;
    
    g_simple_calib.timer += delta_time;
    
    // Update status message
    if (g_simple_calib.timer > 1.0f) {
        InputService* service = game_input_get_service();
        if (service) {
            strcpy(g_simple_calib.status, "Controller system is working! Press ESC to return to menu.");
        } else {
            strcpy(g_simple_calib.status, "No input service available.");
        }
    }
    
    // Render UI - but do it through the UI system
    simple_controller_calibration_render_ui();
}

// Separate UI rendering function 
void simple_controller_calibration_render_ui(void) {
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("❌ No MicroUI context available for controller calibration\n");
        return;
    }
    
    printf("🎨 Rendering controller calibration UI (frame info)\n");
    
    if (mu_begin_window(ctx, "Controller Calibration", mu_rect(100, 100, 600, 400))) {
        
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        mu_label(ctx, "🎮 Controller Calibration System");
        
        mu_label(ctx, "");  // Spacer
        
        mu_label(ctx, g_simple_calib.status);
        
        mu_label(ctx, "");  // Spacer
        
        // Show current input values
        InputService* service = game_input_get_service();
        if (service) {
            char input_info[512];
            snprintf(input_info, sizeof(input_info), 
                "Current Input Values:\n"
                "Thrust Forward: %.2f\n"
                "Pitch: %.2f / %.2f\n"
                "Yaw: %.2f / %.2f\n"
                "Roll: %.2f / %.2f",
                service->get_action_value(service, INPUT_ACTION_THRUST_FORWARD),
                service->get_action_value(service, INPUT_ACTION_PITCH_UP),
                service->get_action_value(service, INPUT_ACTION_PITCH_DOWN),
                service->get_action_value(service, INPUT_ACTION_YAW_LEFT),
                service->get_action_value(service, INPUT_ACTION_YAW_RIGHT),
                service->get_action_value(service, INPUT_ACTION_ROLL_LEFT),
                service->get_action_value(service, INPUT_ACTION_ROLL_RIGHT)
            );
            
            mu_label(ctx, input_info);
        } else {
            mu_label(ctx, "Input service not available");
        }
        
        mu_label(ctx, "");  // Spacer
        mu_label(ctx, "Press ESC to return to the navigation menu");
        
        mu_end_window(ctx);
    } else {
        printf("❌ mu_begin_window failed for controller calibration\n");
    }
}

void controller_calibration_cleanup(struct World* world, SceneStateManager* state) {
    (void)world;
    (void)state;
    
    printf("🎮 Cleaning up Simple Controller Calibration Scene\n");
    
    g_simple_calib.initialized = false;
    
    printf("✅ Simple Controller Calibration Scene cleanup complete\n");
}

bool controller_calibration_input(struct World* world, SceneStateManager* state, const void* event) {
    (void)world;
    (void)event;
    
    if (!g_simple_calib.initialized) return false;
    
    // Use the input service for ESC handling
    InputService* service = game_input_get_service();
    if (service && service->is_action_just_pressed(service, INPUT_ACTION_UI_CANCEL)) {
        // Exit calibration
        scene_state_request_transition(state, "navigation_menu");
        return true;
    }
    
    return false;
}

// ============================================================================
// SCENE SCRIPT DEFINITION
// ============================================================================

const SceneScript controller_calibration_script = {
    .scene_name = "controller_calibration",
    .on_enter = controller_calibration_init,
    .on_update = controller_calibration_update,
    .on_exit = controller_calibration_cleanup,
    .on_input = controller_calibration_input
};