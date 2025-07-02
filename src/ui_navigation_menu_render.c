// src/ui_navigation_menu_render.c
// Navigation menu nuklear rendering

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

// Configure nuklear before including
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "nuklear.h"
#include "ui_adaptive_controls.h"
#include "scene_state.h"

// Navigation menu data (must match ui_navigation_menu_impl.c)
typedef struct {
    int selected_index;
    float animation_timer;
    bool gamepad_was_connected;
    
    const char* destinations[8];
    const char* descriptions[8];
    int destination_count;
    
    ControlHint nav_hints[3];
} NavigationMenuData;

void navigation_menu_render_nuklear(struct nk_context* ctx, NavigationMenuData* data, float delta_time) {
    (void)delta_time;
    
    // Main window
    struct nk_rect bounds = nk_rect(50, 50, 700, 500);
    if (nk_begin(ctx, "FTL Navigation Interface", bounds, 
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER)) {
        
        // Title
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label(ctx, "🧭 SELECT DESTINATION", NK_TEXT_CENTERED);
        
        // Show connection status
        if (ui_adaptive_should_show_gamepad()) {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "🎮 Gamepad Connected", NK_TEXT_CENTERED);
        }
        
        // Spacer
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_spacing(ctx, 1);
        
        // Menu items
        nk_layout_row_dynamic(ctx, 60, 1);
        
        for (int i = 0; i < data->destination_count; i++) {
            // Highlight selected item
            bool is_selected = (i == data->selected_index);
            
            if (is_selected) {
                // Animated selection indicator
                float pulse = sinf(data->animation_timer * 3.0f) * 0.5f + 0.5f;
                struct nk_color highlight = nk_rgb(
                    100 + (int)(pulse * 55),
                    150 + (int)(pulse * 55),
                    255
                );
                nk_style_push_color(ctx, &ctx->style.button.normal.data.color, highlight);
            }
            
            if (nk_button_label(ctx, data->destinations[i])) {
                // Handle selection
                const char* scene_names[] = {
                    "flight_test",
                    "racing_canyon",
                    "derelict_alpha",
                    "system_overview",
                    "scene_selector"
                };
                
                if (i < (int)(sizeof(scene_names) / sizeof(scene_names[0]))) {
                    printf("🧭 Navigation: Selected %s\n", scene_names[i]);
                    scene_state_request_transition(NULL, scene_names[i]);
                }
            }
            
            if (is_selected) {
                nk_style_pop_color(ctx);
                
                // Show description
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, data->descriptions[i], NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 60, 1);
            }
        }
        
        // Control hints at bottom
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_spacing(ctx, 1);
        
        // Render adaptive control hints
        nk_layout_row_dynamic(ctx, 20, 6); // 3 hints * 2 columns
        for (int i = 0; i < 3; i++) {
            nk_label(ctx, data->nav_hints[i].action_name, NK_TEXT_RIGHT);
            nk_label(ctx, ui_adaptive_get_hint_text(&data->nav_hints[i]), NK_TEXT_LEFT);
        }
    }
    nk_end(ctx);
}