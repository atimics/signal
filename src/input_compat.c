/**
 * @file input_compat.c
 * @brief Implementation of compatibility layer for legacy input system
 */

#include "input_compat.h"
#include <stdio.h>

// Global service reference (temporary during migration)
static InputService* g_input_service = NULL;

// Legacy action ID mappings (from ui_navigation_menu_microui.c)
#define INPUT_ACTION_NAV_UP     59
#define INPUT_ACTION_NAV_DOWN   60
#define INPUT_ACTION_CONFIRM    57
#define INPUT_ACTION_CANCEL     58
#define INPUT_ACTION_BACK       61

// Initialize compatibility layer
void input_compat_init(InputService* service) {
    g_input_service = service;
    printf("✅ Input compatibility layer initialized\n");
}

void input_compat_shutdown(void) {
    g_input_service = NULL;
}

// Map legacy action IDs to new ones
InputActionID input_compat_map_legacy_action(uint32_t legacy_action) {
    switch (legacy_action) {
        case INPUT_ACTION_NAV_UP:
            return INPUT_ACTION_UI_UP;
            
        case INPUT_ACTION_NAV_DOWN:
            return INPUT_ACTION_UI_DOWN;
            
        case INPUT_ACTION_CONFIRM:
            return INPUT_ACTION_UI_CONFIRM;
            
        case INPUT_ACTION_CANCEL:
        case INPUT_ACTION_BACK:
            return INPUT_ACTION_UI_CANCEL;
            
        // Add more mappings as needed
        default:
            // If no mapping exists, return a safe default
            return INPUT_ACTION_COUNT;  // Invalid action
    }
}

// Legacy API implementations
bool input_mapping_just_pressed(uint32_t action) {
    if (!g_input_service) return false;
    
    InputActionID new_action = input_compat_map_legacy_action(action);
    if (new_action >= INPUT_ACTION_COUNT) return false;
    
    return g_input_service->is_action_just_pressed(g_input_service, new_action);
}

bool input_mapping_pressed(uint32_t action) {
    if (!g_input_service) return false;
    
    InputActionID new_action = input_compat_map_legacy_action(action);
    if (new_action >= INPUT_ACTION_COUNT) return false;
    
    return g_input_service->is_action_pressed(g_input_service, new_action);
}

bool input_mapping_just_released(uint32_t action) {
    if (!g_input_service) return false;
    
    InputActionID new_action = input_compat_map_legacy_action(action);
    if (new_action >= INPUT_ACTION_COUNT) return false;
    
    return g_input_service->is_action_just_released(g_input_service, new_action);
}

float input_mapping_value(uint32_t action) {
    if (!g_input_service) return 0.0f;
    
    InputActionID new_action = input_compat_map_legacy_action(action);
    if (new_action >= INPUT_ACTION_COUNT) return 0.0f;
    
    return g_input_service->get_action_value(g_input_service, new_action);
}

// Helper function to process frame (called from game loop)
void input_compat_process_frame(float delta_time) {
    if (g_input_service) {
        g_input_service->process_frame(g_input_service, delta_time);
    }
}