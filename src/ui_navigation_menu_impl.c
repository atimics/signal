/**
 * @file ui_navigation_menu_impl.c
 * @brief Navigation menu UI module implementation
 */

#include "ui_scene.h"
#include "ui_adaptive_controls.h"
#include "ui_api.h"
#include "ui_menu_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Navigation menu data structure
typedef struct {
    int selected_index;
    float animation_timer;
    bool gamepad_was_connected;
    
    const char* destinations[9];
    const char* descriptions[9];
    int destination_count;
    
    ControlHint nav_hints[3];
    
    // New menu system
    Menu main_menu;
    bool menu_initialized;
} NavigationMenuData;

// Static module instance
static SceneUIModule nav_module = {0};
static NavigationMenuData nav_data = {0};

// ============================================================================
// NAVIGATION MENU UI MODULE IMPLEMENTATION
// ============================================================================

static void navigation_menu_init(struct World* world)
{
    (void)world;
    printf("🧭 Navigation Menu UI: Initializing\n");
    
    // Initialize navigation data
    nav_data.selected_index = 0;
    nav_data.animation_timer = 0.0f;
    nav_data.gamepad_was_connected = false;
    nav_data.menu_initialized = false;
    
    // Set up destinations
    nav_data.destinations[0] = "Flight Test";
    nav_data.descriptions[0] = "Free flight testing mode";
    
    nav_data.destinations[1] = "Template Scene";
    nav_data.descriptions[1] = "Development and testing template";
    
    nav_data.destination_count = 2;
    
    // Initialize control hints
    nav_data.nav_hints[0].action_name = "Navigate";
    nav_data.nav_hints[0].keyboard_hint = "↑↓ Arrow Keys";
    nav_data.nav_hints[0].gamepad_hint = "D-Pad / Left Stick";
    
    nav_data.nav_hints[1].action_name = "Select";
    nav_data.nav_hints[1].keyboard_hint = "Enter";
    nav_data.nav_hints[1].gamepad_hint = "A Button";
    
    nav_data.nav_hints[2].action_name = "Exit";
    nav_data.nav_hints[2].keyboard_hint = "Escape";
    nav_data.nav_hints[2].gamepad_hint = "B Button";
    
    // UI initialized with destinations
}

static void navigation_menu_shutdown(struct World* world)
{
    (void)world;
    // Shutting down navigation menu UI
    
    // Clean up any allocated resources
    memset(&nav_data, 0, sizeof(nav_data));
}

static void navigation_menu_update(struct World* world, float delta_time)
{
    (void)world;
    
    // Update animation timer
    nav_data.animation_timer += delta_time;
    
    // Update gamepad connection status
    bool gamepad_connected = ui_adaptive_should_show_gamepad();
    if (gamepad_connected != nav_data.gamepad_was_connected) {
        nav_data.gamepad_was_connected = gamepad_connected;
        // Gamepad connection state changed
    }
}

static bool navigation_menu_handle_event(const void* event, struct World* world)
{
    (void)event;
    (void)world;
    
    // Event handling is done through MicroUI
    return false;
}

// External render function (implemented in ui_navigation_menu_microui.c)
extern void navigation_menu_render_microui(NavigationMenuData* data, float delta_time);

static void navigation_menu_render(void* ctx, struct World* world, 
                                 struct SystemScheduler* scheduler, float delta_time)
{
    (void)ctx;  // We'll use the global MicroUI context
    (void)scheduler;
    
    // Update state before rendering
    navigation_menu_update(world, delta_time);
    
    // Call the MicroUI render function with our data
    navigation_menu_render_microui(&nav_data, delta_time);
}

// ============================================================================
// MODULE CREATION
// ============================================================================

SceneUIModule* create_navigation_menu_ui_module(void)
{
    // Creating navigation menu UI module
    
    // Set up the module
    nav_module.scene_name = "navigation_menu";
    nav_module.init = navigation_menu_init;
    nav_module.shutdown = navigation_menu_shutdown;
    nav_module.handle_event = navigation_menu_handle_event;
    nav_module.render = navigation_menu_render;
    nav_module.data = &nav_data;
    
    // Call the update function during render since there's no update field
    // The module will update its state when rendered
    
    return &nav_module;
}