// src/ui_navigation_menu_microui.c
// Navigation menu MicroUI rendering implementation

#include "ui_scene.h"
#include "ui_adaptive_controls.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "ui_menu_system.h"
#include "scene_state.h"
#include "microui/microui.h"
// #include "sokol_gfx.h" // Don't include Sokol directly in UI code
#include <stdio.h>
#include <string.h>
#include <math.h>

// Forward declarations for input system
bool input_mapping_just_pressed(uint32_t action);
#define INPUT_ACTION_NAV_UP 59
#define INPUT_ACTION_NAV_DOWN 60
#define INPUT_ACTION_CONFIRM 57

// Scene input system - disabled temporarily to prevent crashes
// TODO: Re-enable once proper event-based input system is implemented
typedef struct {
    bool up, down, left, right;
    bool confirm, cancel, back;
} MenuInput;

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
    
    // Bounds check before accessing arrays
    if (!data || item_index < 0 || item_index >= data->destination_count) {
        printf("❌ Menu selection out of bounds: index=%d, count=%d\n", 
               item_index, data ? data->destination_count : -1);
        return;
    }
    
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
    // Safety check
    if (!data) {
        printf("❌ ERROR: NavigationMenuData is NULL!\n");
        return;
    }
    
    // Validate data integrity
    if (data->destination_count < 0 || data->destination_count > 9) {
        printf("❌ ERROR: Invalid destination_count: %d\n", data->destination_count);
        return;
    }
    
    // Get MicroUI context
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("❌ No MicroUI context available\n");
        return;
    }
    
    // Skip input handling for now to avoid crashes
    // TODO: Implement proper event-based input once architecture is refactored
    
    // Always ensure selected_index is valid before any rendering
    if (data->selected_index < 0 || data->selected_index >= data->destination_count) {
        data->selected_index = 0;
    }
    
    // Initialize menu system on first call
    if (!data->menu_initialized) {
        // Debug: Log when initialization happens
        static int init_count = 0;
        printf("⚠️ Navigation menu initialization #%d (data=%p, initialized=%d)\n", 
               ++init_count, (void*)data, data->menu_initialized);
        
        // Safety check for destination data
        if (data->destination_count <= 0 || data->destination_count > 9) {
            printf("❌ ERROR: Invalid destination_count: %d\n", data->destination_count);
            // Initialize with safe defaults
            data->destination_count = 3;
            data->destinations[0] = "Ship Launch Test";
            data->destinations[1] = "Flight Test";
            data->destinations[2] = "Thruster Test";
            data->descriptions[0] = "Test ship launch sequence";
            data->descriptions[1] = "Free flight test mode";
            data->descriptions[2] = "Test thruster systems";
        }
        
        menu_init(&data->main_menu, "FTL NAVIGATION SYSTEM");
        
        // Add menu items from existing data
        for (int i = 0; i < data->destination_count; i++) {
            if (!data->destinations[i] || !data->descriptions[i]) {
                printf("❌ ERROR: NULL destination/description at index %d\n", i);
                continue;
            }
            menu_add_item(&data->main_menu, data->destinations[i], data->descriptions[i], NULL);
        }
        
        // Set up callbacks and style
        menu_set_callbacks(&data->main_menu, navigation_menu_on_select, NULL, data);
        menu_set_style(&data->main_menu, true, true, true);  // descriptions, cursor, terminal style
        
        data->menu_initialized = true;
        printf("🎮 Navigation menu initialized with %d items\n", data->destination_count);
    }
    
    // Sync selection state with bounds checking
    if (data->main_menu.item_count > 0) {
        // Ensure selected_index is within bounds before syncing
        if (data->selected_index >= data->main_menu.item_count) {
            data->selected_index = data->main_menu.item_count - 1;
        }
        if (data->selected_index < 0) {
            data->selected_index = 0;
        }
        data->main_menu.selected_index = data->selected_index;
    }
    
    // Render using the general menu system
    menu_render(&data->main_menu, ctx, delta_time);
    
    // Sync back the selection state with bounds checking
    if (data->main_menu.item_count > 0) {
        data->selected_index = data->main_menu.selected_index;
        // Double-check bounds after sync
        if (data->selected_index >= data->destination_count) {
            data->selected_index = data->destination_count - 1;
        }
        if (data->selected_index < 0) {
            data->selected_index = 0;
        }
    }
    data->animation_timer = data->main_menu.animation_timer;
}

// External render function called from navigation_menu_render
void navigation_menu_render(NavigationMenuData* data, float delta_time) {
    if (!data) {
        static int error_count = 0;
        if (error_count++ < 5) {
            printf("❌ ERROR: navigation_menu_render called with NULL data!\n");
        }
        return;
    }
    // Delegate to MicroUI implementation
    navigation_menu_render_microui(data, delta_time);
}