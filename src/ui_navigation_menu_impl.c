// src/ui_navigation_menu_impl.c
// Navigation menu implementation without direct nuklear inclusion

#include "ui_scene.h"
#include "ui_adaptive_controls.h"
#include "ui_api.h"
#include "scene_state.h"
#include "system/gamepad.h"
#include "system/gamepad_hotplug.h"
#include "system/input.h"  // For input_set_last_device_type and INPUT_DEVICE_KEYBOARD
#include "sokol_app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Navigation menu data
typedef struct {
    int selected_index;
    float animation_timer;
    bool gamepad_was_connected;
    
    // Menu items
    const char* destinations[9];
    const char* descriptions[9];
    int destination_count;
    
    // Control hints to display
    ControlHint nav_hints[3];
} NavigationMenuData;

static void navigation_menu_init(struct World* world) {
    (void)world;
    
    SceneUIModule* module = scene_ui_get_module("navigation_menu");
    if (!module) return;
    
    NavigationMenuData* data = calloc(1, sizeof(NavigationMenuData));
    module->data = data;
    
    // Initialize menu items
    data->destinations[0] = "🚀 Flight Test Range";
    data->descriptions[0] = "Test your ship's flight capabilities";
    
    data->destinations[1] = "🏁 Canyon Racing";
    data->descriptions[1] = "High-speed canyon racing challenge";
    
    data->destinations[2] = "🧲 Derelict Navigation";
    data->descriptions[2] = "Navigate the Aethelian command ship";
    
    data->destinations[3] = "🌟 System Overview";
    data->descriptions[3] = "View the star system map";
    
    data->destinations[4] = "🚀 Thruster Test";
    data->descriptions[4] = "Test gyroscopic stabilization system";
    
    data->destinations[5] = "🚀 Ship Launch Test";
    data->descriptions[5] = "Test ship launch with thruster visualization";
    
    data->destinations[6] = "🎬 Scene Selector";
    data->descriptions[6] = "Debug: Select any scene directly";
    
    data->destination_count = 7;
    data->selected_index = 0;
    
    // Set up control hints
    data->nav_hints[0] = UI_HINT_NAVIGATE;
    data->nav_hints[1] = UI_HINT_SELECT;
    data->nav_hints[2] = UI_HINT_BACK;
    
    // Check initial gamepad state
    data->gamepad_was_connected = (gamepad_get_primary() != NULL);
    
    printf("🧭 Navigation menu UI initialized\n");
}

static void navigation_menu_shutdown(struct World* world) {
    (void)world;
    
    SceneUIModule* module = scene_ui_get_module("navigation_menu");
    if (!module || !module->data) return;
    
    free(module->data);
    module->data = NULL;
    
    printf("🧭 Navigation menu UI shutdown\n");
}

// External render function that will be implemented in ui_navigation_menu.c
extern void navigation_menu_render_nuklear(struct nk_context* ctx, NavigationMenuData* data, float delta_time);

static void navigation_menu_render(struct nk_context* ctx, struct World* world, 
                                  struct SystemScheduler* scheduler, float delta_time) {
    (void)world;
    (void)scheduler;
    
    SceneUIModule* module = scene_ui_get_module("navigation_menu");
    if (!module || !module->data) return;
    
    NavigationMenuData* data = (NavigationMenuData*)module->data;
    
    // Update animation
    data->animation_timer += delta_time;
    
    // Check for gamepad connection changes
    bool gamepad_connected = (gamepad_get_primary() != NULL);
    if (gamepad_connected != data->gamepad_was_connected) {
        data->gamepad_was_connected = gamepad_connected;
        printf("🧭 Navigation menu: Gamepad %s\n", 
               gamepad_connected ? "connected" : "disconnected");
    }
    
    // Handle gamepad navigation
    ui_adaptive_menu_navigate(&data->selected_index, data->destination_count);
    
    // Delegate actual rendering to the nuklear implementation
    navigation_menu_render_nuklear(ctx, data, delta_time);
}

static bool navigation_menu_handle_event(const void* event, struct World* world) {
    (void)world;
    
    const sapp_event* ev = (const sapp_event*)event;
    SceneUIModule* module = scene_ui_get_module("navigation_menu");
    if (!module || !module->data) return false;
    
    NavigationMenuData* data = (NavigationMenuData*)module->data;
    
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch (ev->key_code) {
            case SAPP_KEYCODE_UP:
            case SAPP_KEYCODE_W:
                data->selected_index--;
                if (data->selected_index < 0) {
                    data->selected_index = data->destination_count - 1;
                }
                input_set_last_device_type(INPUT_DEVICE_KEYBOARD);
                return true;
                
            case SAPP_KEYCODE_DOWN:
            case SAPP_KEYCODE_S:
                data->selected_index++;
                if (data->selected_index >= data->destination_count) {
                    data->selected_index = 0;
                }
                input_set_last_device_type(INPUT_DEVICE_KEYBOARD);
                return true;
                
            case SAPP_KEYCODE_ENTER:
            case SAPP_KEYCODE_SPACE:
                // Trigger selection
                {
                    const char* scene_names[] = {
                        "flight_test",
                        "racing_canyon",
                        "derelict_alpha",
                        "system_overview",
                        "thruster_test",
                        "ship_launch_test",
                        "scene_selector"
                    };
                    
                    if (data->selected_index < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
                        printf("🧭 Navigation: Selected %s\n", scene_names[data->selected_index]);
                        scene_state_request_transition(NULL, scene_names[data->selected_index]);
                    }
                }
                input_set_last_device_type(INPUT_DEVICE_KEYBOARD);
                return true;
                
            case SAPP_KEYCODE_ESCAPE:
                // Could return to previous menu or quit
                printf("🧭 Navigation: ESC pressed\n");
                input_set_last_device_type(INPUT_DEVICE_KEYBOARD);
                return true;
                
            default:
                // Handle all other keys
                break;
        }
    }
    
    // Check for gamepad input
    if (ui_adaptive_menu_select()) {
        // Trigger selection with gamepad
        const char* scene_names[] = {
            "flight_test",
            "racing_canyon",
            "derelict_alpha",
            "system_overview",
            "thruster_test",
            "ship_launch_test",
            "scene_selector"
        };
        
        if (data->selected_index < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
            printf("🧭 Navigation: Gamepad selected %s\n", scene_names[data->selected_index]);
            scene_state_request_transition(NULL, scene_names[data->selected_index]);
        }
        return true;
    }
    
    return false;
}

// Create the module
SceneUIModule* create_navigation_menu_ui_module(void) {
    SceneUIModule* module = calloc(1, sizeof(SceneUIModule));
    
    module->scene_name = "navigation_menu";
    module->init = navigation_menu_init;
    module->shutdown = navigation_menu_shutdown;
    module->render = navigation_menu_render;
    module->handle_event = navigation_menu_handle_event;
    
    return module;
}