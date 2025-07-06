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

// Include new input system headers
#include "game_input.h"
#include "services/input_service.h"

// Forward declaration
void navigation_menu_on_select(int item_index, void* user_data);

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

// Process input using the new event-based system
static void navigation_menu_process_input(NavigationMenuData* data) {
    if (!data || !game_input_is_new_system_enabled()) {
        return;
    }
    
    InputService* service = game_input_get_service();
    if (!service || !service->get_next_event) {
        printf("⚠️ Input service not available or invalid\n");
        return;
    }
    
    // Safety limit to prevent infinite loops
    int event_limit = 32;
    InputEvent event;
    while (event_limit-- > 0 && service->get_next_event(service, &event)) {
        printf("🔍 Processing input event: action=%d, just_pressed=%d, value=%.2f\n", 
               event.action, event.just_pressed, event.value);
        
        switch (event.action) {
            case INPUT_ACTION_UI_UP:
                if (event.just_pressed && data->selected_index > 0) {
                    data->selected_index--;
                    printf("🎮 Navigate UP: selected=%d\n", data->selected_index);
                }
                break;
                
            case INPUT_ACTION_UI_DOWN:
                if (event.just_pressed && data->selected_index < data->destination_count - 1) {
                    data->selected_index++;
                    printf("🎮 Navigate DOWN: selected=%d\n", data->selected_index);
                }
                break;
                
            case INPUT_ACTION_UI_CONFIRM:
                if (event.just_pressed) {
                    printf("🎮 CONFIRM pressed: selecting item %d\n", data->selected_index);
                    navigation_menu_on_select(data->selected_index, data);
                }
                break;
                
            case INPUT_ACTION_UI_CANCEL:
                if (event.just_pressed) {
                    printf("🎮 CANCEL pressed in navigation menu\n");
                    // Could implement back/exit behavior here
                }
                break;
                
            case INPUT_ACTION_UI_MENU:
                if (event.just_pressed) {
                    printf("🎮 MENU (Tab) pressed in navigation menu - ignoring (already in menu)\n");
                    // Tab pressed while already in navigation menu - ignore or implement toggle behavior
                }
                break;
                
            default:
                printf("🔍 Unhandled input action: %d\n", event.action);
                break;
        }
    }
}

// Guarded menu builder function
static void ensure_menu_built(NavigationMenuData* data) {
    if (data->menu_initialized) return;

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
    
    // Process input events from new input system
    navigation_menu_process_input(data);
    
    // Always ensure selected_index is valid before any rendering
    if (data->selected_index < 0 || data->selected_index >= data->destination_count) {
        data->selected_index = 0;
    }
    
    // Ensure menu is built (only once)
    ensure_menu_built(data);
    
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