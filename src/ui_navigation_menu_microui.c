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
} NavigationMenuData;

void navigation_menu_render_microui(NavigationMenuData* data, float delta_time) {
    (void)delta_time;
    
    // Get MicroUI context
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("❌ No MicroUI context available\n");
        return;
    }
    
    // TODO: Integrate with input mapping system
    // For now, keyboard navigation is handled through mouse hover on menu items
    
    // printf("🎨 Navigation menu: Attempting to create window (commands before: %d)\n", ctx->command_list.idx);
    
    // Main window - terminal style with no title bar
    int window_opts = MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOCLOSE;
    if (mu_begin_window_ex(ctx, "FTL Navigation Interface", mu_rect(50, 50, 700, 500), window_opts)) {
        // printf("🎨 Navigation menu: Window created successfully\n");
        
        // Terminal header with ASCII art
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "╔════════════════════════════════════════════════════════════════╗");
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "║            FTL NAVIGATION SYSTEM v3.14.159                     ║");
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "╠════════════════════════════════════════════════════════════════╣");
        mu_layout_row(ctx, 1, (int[]){-1}, 30);
        mu_label(ctx, "║ SELECT DESTINATION:                                            ║");
        
        // Show connection status
        if (ui_adaptive_should_show_gamepad()) {
            mu_layout_row(ctx, 1, (int[]){-1}, 20);
            mu_label(ctx, "Gamepad Connected");
        }
        
        // Spacer
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "");
        
        // Menu items with terminal style
        for (int i = 0; i < data->destination_count; i++) {
            mu_layout_row(ctx, 1, (int[]){-1}, 35);
            
            // Create terminal-style menu item with prefix
            char menu_text[256];
            if (i == data->selected_index) {
                // Selected item with arrow indicator
                snprintf(menu_text, sizeof(menu_text), " > [%d] %s", i + 1, data->destinations[i]);
                
                // Draw selection highlight
                mu_Rect item_rect = mu_layout_next(ctx);
                mu_draw_rect(ctx, item_rect, mu_color(0, 40, 0, 255));
            } else {
                // Normal item
                snprintf(menu_text, sizeof(menu_text), "   [%d] %s", i + 1, data->destinations[i]);
            }
            
            // Use label instead of button for terminal aesthetic
            mu_label(ctx, menu_text);
            
            // Also update mouse hover detection
            if (mu_mouse_over(ctx, mu_layout_next(ctx))) {
                data->selected_index = i;
                
                // Handle mouse clicks
                if (ctx->mouse_pressed == MU_MOUSE_LEFT) {
                    printf("🎮 Menu item clicked: %s (index %d)\n", data->destinations[i], i);
                    // Handle selection
                    const char* scene_names[] = {
                        "ship_launch_test",
                        "flight_test",
                        "thruster_test"
                    };
                    
                    if (i < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
                        printf("🎮 Requesting scene transition to: %s\n", scene_names[i]);
                        ui_request_scene_change(scene_names[i]);
                    }
                }
            }
            
            // Show description for selected item
            if (i == data->selected_index && data->descriptions[i]) {
                mu_layout_row(ctx, 1, (int[]){-1}, 20);
                mu_label(ctx, data->descriptions[i]);
            }
        }
        
        // Terminal footer
        mu_layout_row(ctx, 1, (int[]){-1}, 40);
        mu_label(ctx, "");  // Spacer
        
        // Control hints with terminal border
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "╠════════════════════════════════════════════════════════════════╣");
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "║ CONTROLS:                                                      ║");
        
        // Show controls in terminal format
        char control_line[256];
        snprintf(control_line, sizeof(control_line), "║ %-15s: %-45s ║", 
                 "Navigate", ui_adaptive_should_show_gamepad() ? "D-Pad / Left Stick" : "↑↓ Arrow Keys");
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, control_line);
        
        snprintf(control_line, sizeof(control_line), "║ %-15s: %-45s ║", 
                 "Select", ui_adaptive_should_show_gamepad() ? "A Button" : "Enter");
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, control_line);
        
        snprintf(control_line, sizeof(control_line), "║ %-15s: %-45s ║", 
                 "Exit", ui_adaptive_should_show_gamepad() ? "B Button" : "Escape");
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, control_line);
        
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "╚════════════════════════════════════════════════════════════════╝");
        
        mu_end_window(ctx);
    }
    
    // printf("🎨 Navigation menu: Commands after rendering: %d\n", ctx->command_list.idx);
}

// External render function called from navigation_menu_render
void navigation_menu_render_nuklear(struct nk_context* ctx, NavigationMenuData* data, float delta_time) {
    (void)ctx;  // Unused - we use MicroUI instead
    
    // Delegate to MicroUI implementation
    navigation_menu_render_microui(data, delta_time);
}