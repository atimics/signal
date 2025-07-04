// src/ui_navigation_menu_microui.c
// Navigation menu MicroUI rendering implementation

#include "ui_scene.h"
#include "ui_adaptive_controls.h"
#include "ui_api.h"
#include "ui_microui.h"
#include "scene_state.h"
#include "microui/microui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Navigation menu data (must match ui_navigation_menu_impl.c)
typedef struct {
    int selected_index;
    float animation_timer;
    bool gamepad_was_connected;
    
    const char* destinations[9];
    const char* descriptions[9];
    int destination_count;
    
    ControlHint nav_hints[3];
} NavigationMenuData;

void navigation_menu_render_microui(NavigationMenuData* data, float delta_time) {
    (void)delta_time;
    
    // Get MicroUI context
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("❌ No MicroUI context available\n");
        return;
    }
    
    // Main window
    if (mu_begin_window(ctx, "FTL Navigation Interface", mu_rect(50, 50, 700, 500))) {
        
        // Title
        mu_layout_row(ctx, 1, (int[]){-1}, 40);
        mu_label(ctx, "SELECT DESTINATION");
        
        // Show connection status
        if (ui_adaptive_should_show_gamepad()) {
            mu_layout_row(ctx, 1, (int[]){-1}, 20);
            mu_label(ctx, "Gamepad Connected");
        }
        
        // Spacer
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "");
        
        // Menu items
        for (int i = 0; i < data->destination_count; i++) {
            mu_layout_row(ctx, 1, (int[]){-1}, 50);
            
            // Highlight selected item with a different approach for MicroUI
            if (i == data->selected_index) {
                // Draw a colored background rect for selection
                mu_Rect item_rect = mu_layout_next(ctx);
                mu_draw_rect(ctx, item_rect, mu_color(100, 150, 255, 255));
            }
            
            if (mu_button(ctx, data->destinations[i])) {
                // Handle selection
                const char* scene_names[] = {
                    "ship_launch_test",
                    "flight_test",
                    "thruster_test"
                };
                
                if (i < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
                    scene_state_request_transition(NULL, scene_names[i]);
                }
            }
            
            // Show description for selected item
            if (i == data->selected_index && data->descriptions[i]) {
                mu_layout_row(ctx, 1, (int[]){-1}, 20);
                mu_label(ctx, data->descriptions[i]);
            }
        }
        
        // Control hints at bottom
        mu_layout_row(ctx, 1, (int[]){-1}, 40);
        mu_label(ctx, "");  // Spacer
        
        // Render adaptive control hints
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "Controls:");
        
        for (int i = 0; i < 3; i++) {
            mu_layout_row(ctx, 2, (int[]){200, -1}, 20);
            mu_label(ctx, data->nav_hints[i].action_name);
            mu_label(ctx, ui_adaptive_get_hint_text(&data->nav_hints[i]));
        }
        
        mu_end_window(ctx);
    }
}

// External render function called from navigation_menu_render
void navigation_menu_render_nuklear(struct nk_context* ctx, NavigationMenuData* data, float delta_time) {
    (void)ctx;  // Unused - we use MicroUI instead
    
    // Delegate to MicroUI implementation
    navigation_menu_render_microui(data, delta_time);
}