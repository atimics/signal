// src/ui_navigation_menu_microui.c
// Navigation menu MicroUI rendering implementation

#include "ui_scene.h"
#include "ui_adaptive_controls.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "ui_menu_system.h"
#include "scene_state.h"
#include "microui/microui.h"
#include "sokol_gfx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Forward declarations for input system
bool input_mapping_just_pressed(uint32_t action);
#define INPUT_ACTION_NAV_UP 59
#define INPUT_ACTION_NAV_DOWN 60
#define INPUT_ACTION_CONFIRM 57

// Navigation menu data (must match ui_navigation_menu_impl.c)
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

// Menu selection callback
void navigation_menu_on_select(int item_index, void* user_data) {
    NavigationMenuData* data = (NavigationMenuData*)user_data;
    
    printf("🎮 Menu item selected: %s (index %d)\n", data->destinations[item_index], item_index);
    
    const char* scene_names[] = {
        "ship_launch_test",
        "flight_test", 
        "thruster_test"
    };
    
    if (item_index < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
        printf("🎮 Requesting scene transition to: %s\n", scene_names[item_index]);
        ui_request_scene_change(scene_names[item_index]);
    }
}

void navigation_menu_render_microui(NavigationMenuData* data, float delta_time) {
    // Get MicroUI context
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("❌ No MicroUI context available\n");
        return;
    }
    
    // Initialize menu system on first call
    if (!data->menu_initialized) {
        menu_init(&data->main_menu, "FTL NAVIGATION SYSTEM");
        
        // Add menu items from existing data
        for (int i = 0; i < data->destination_count; i++) {
            menu_add_item(&data->main_menu, data->destinations[i], data->descriptions[i], NULL);
        }
        
        // Set up callbacks and style
        menu_set_callbacks(&data->main_menu, navigation_menu_on_select, NULL, data);
        menu_set_style(&data->main_menu, true, true, true);  // descriptions, cursor, terminal style
        
        data->menu_initialized = true;
        printf("🎮 Navigation menu initialized with %d items\n", data->destination_count);
    }
    
    // Sync selection state
    data->main_menu.selected_index = data->selected_index;
    
    // Render using the general menu system
    menu_render(&data->main_menu, ctx, delta_time);
    
    // Sync back the selection state
    data->selected_index = data->main_menu.selected_index;
    data->animation_timer = data->main_menu.animation_timer;
}

// External render function called from navigation_menu_render
void navigation_menu_render_nuklear(struct nk_context* ctx, NavigationMenuData* data, float delta_time) {
    (void)ctx;  // Unused - we use MicroUI instead
    
    // Delegate to MicroUI implementation
    navigation_menu_render_microui(data, delta_time);
}