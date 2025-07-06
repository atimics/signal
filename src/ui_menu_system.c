/**
 * @file ui_menu_system.c
 * @brief General purpose menu system implementation
 */

#include "ui_menu_system.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Helper function to keep selection within bounds
static inline void menu_clamp_selection(Menu* menu) {
    if (menu->selected_index < 0) menu->selected_index = 0;
    if (menu->selected_index >= menu->item_count) menu->selected_index = menu->item_count - 1;
}

void menu_init(Menu* menu, const char* title) {
    memset(menu, 0, sizeof(Menu));
    strncpy(menu->title, title, MENU_MAX_TEXT_LENGTH - 1);
    menu->title[MENU_MAX_TEXT_LENGTH - 1] = '\0';
    
    menu->selected_index = 0;
    menu->animation_timer = 0.0f;
    menu->show_descriptions = true;
    menu->show_cursor = true;
    menu->terminal_style = true;
}

void menu_add_item(Menu* menu, const char* text, const char* description, void* user_data) {
    if (menu->item_count >= MENU_MAX_ITEMS) {
        printf("⚠️ Menu: Cannot add item '%s' - menu is full (%d items)\n", text, MENU_MAX_ITEMS);
        return;
    }
    
    MenuItem* item = &menu->items[menu->item_count];
    strncpy(item->text, text, MENU_MAX_TEXT_LENGTH - 1);
    item->text[MENU_MAX_TEXT_LENGTH - 1] = '\0';
    
    if (description) {
        strncpy(item->description, description, MENU_MAX_DESC_LENGTH - 1);
        item->description[MENU_MAX_DESC_LENGTH - 1] = '\0';
    } else {
        item->description[0] = '\0';
    }
    
    item->user_data = user_data;
    item->enabled = true;
    
    menu->item_count++;
}

void menu_set_callbacks(Menu* menu, MenuSelectCallback on_select, MenuCancelCallback on_cancel, void* callback_data) {
    menu->on_select = on_select;
    menu->on_cancel = on_cancel;
    menu->callback_data = callback_data;
}

void menu_set_style(Menu* menu, bool show_descriptions, bool show_cursor, bool terminal_style) {
    menu->show_descriptions = show_descriptions;
    menu->show_cursor = show_cursor;
    menu->terminal_style = terminal_style;
}

void menu_update(Menu* menu, float delta_time) {
    menu->animation_timer += delta_time;
}

bool menu_handle_key(Menu* menu, int key) {
    // This will be integrated with the input system later
    // For now, just placeholder
    (void)menu;
    (void)key;
    return false;
}

void menu_render(Menu* menu, mu_Context* ctx, float delta_time) {
    if (!ctx || !menu) return;
    
    menu_update(menu, delta_time);
    
    // Create main window
    int window_opts = MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOCLOSE | MU_OPT_NOFRAME;
    if (mu_begin_window_ex(ctx, menu->title, mu_rect(100, 150, 800, 400), window_opts)) {
        
        // Title
        mu_layout_row(ctx, 1, (int[]){-1}, 40);
        mu_label(ctx, menu->title);
        
        if (menu->show_descriptions) {
            // Two-panel layout: menu on left, description on right
            // Calculate proper height for panels based on number of items
            int panel_height = menu->item_count * 25 + 80;  // 25px per item + header/spacing
            mu_layout_row(ctx, 3, (int[]){350, 50, 350}, panel_height);
            
            // Left panel - menu items
            mu_begin_panel(ctx, "menu_items");
            {
                mu_layout_row(ctx, 1, (int[]){-1}, 30);
                mu_label(ctx, "SELECT OPTION:");
                
                mu_layout_row(ctx, 1, (int[]){-1}, 10);
                mu_label(ctx, "");  // Spacer
                
                // Render menu items
                for (int i = 0; i < menu->item_count; i++) {
                    mu_layout_row(ctx, 1, (int[]){-1}, 25);
                    
                    static char display_text[MENU_MAX_TEXT_LENGTH + 16];
                    if (menu->terminal_style) {
                        if (i == menu->selected_index && menu->show_cursor) {
                            // Flashing cursor effect
                            const char* cursor = ((int)(menu->animation_timer * 2) % 2) ? ">" : " ";
                            snprintf(display_text, sizeof(display_text), " %s [%d] %s", 
                                   cursor, i + 1, menu->items[i].text);
                        } else {
                            snprintf(display_text, sizeof(display_text), "   [%d] %s", 
                                   i + 1, menu->items[i].text);
                        }
                    } else {
                        // Standard style
                        if (i == menu->selected_index) {
                            snprintf(display_text, sizeof(display_text), "> %s", menu->items[i].text);
                        } else {
                            snprintf(display_text, sizeof(display_text), "  %s", menu->items[i].text);
                        }
                    }
                    
                    // Get the rect that mu_label() would use
                    mu_Rect item_rect = mu_layout_next(ctx);
                    
                    
                    // Draw label inside rect  
                    mu_draw_control_text(ctx, display_text, item_rect, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
                    
                    // Handle mouse interaction
                    if (mu_mouse_over(ctx, item_rect)) {
                        menu->selected_index = i;
                        menu_clamp_selection(menu);
                        
                        if (ctx->mouse_pressed == MU_MOUSE_LEFT) {
                            if (menu->on_select && menu->items[i].enabled) {
                                menu->on_select(i, menu->callback_data);
                            }
                        }
                    }
                }
            }
            mu_end_panel(ctx);
            
            // Middle panel - spacer/divider
            mu_begin_panel(ctx, "divider");
            {
                if (menu->terminal_style) {
                    // Simple vertical divider
                    for (int y = 0; y < 8; y++) {
                        mu_layout_row(ctx, 1, (int[]){-1}, 25);
                        mu_label(ctx, "│");
                    }
                } else {
                    // Empty spacer
                    mu_layout_row(ctx, 1, (int[]){-1}, 200);
                    mu_label(ctx, "");
                }
            }
            mu_end_panel(ctx);
            
            // Right panel - description
            mu_begin_panel(ctx, "description");
            {
                mu_layout_row(ctx, 1, (int[]){-1}, 30);
                mu_label(ctx, "INFORMATION:");
                
                mu_layout_row(ctx, 1, (int[]){-1}, 10);
                mu_label(ctx, "");  // Spacer
                
                // Show description for selected item  
                if (menu->selected_index >= 0 && menu->selected_index < menu->item_count) {
                    // Extra safety check
                    if (menu->selected_index >= MENU_MAX_ITEMS) {
                        menu->selected_index = menu->item_count - 1;
                    }
                    const char* desc = menu->items[menu->selected_index].description;
                    if (desc && strlen(desc) > 0) {
                        // Word wrap description
                        int chars_per_line = 35;
                        int desc_len = strlen(desc);
                        
                        for (int start = 0; start < desc_len; start += chars_per_line) {
                            mu_layout_row(ctx, 1, (int[]){-1}, 20);
                            
                            char line[64];
                            int len = desc_len - start;
                            if (len > chars_per_line) len = chars_per_line;
                            
                            strncpy(line, desc + start, len);
                            line[len] = '\0';
                            
                            mu_label(ctx, line);
                        }
                    } else {
                        mu_layout_row(ctx, 1, (int[]){-1}, 20);
                        mu_label(ctx, "No description available.");
                    }
                }
            }
            mu_end_panel(ctx);
            
        } else {
            // Single panel layout - just menu items
            // Calculate proper height for panel based on number of items
            int panel_height = menu->item_count * 25 + 80;  // 25px per item + header/spacing
            mu_layout_row(ctx, 1, (int[]){-1}, panel_height);
            
            mu_begin_panel(ctx, "menu_items");
            {
                mu_layout_row(ctx, 1, (int[]){-1}, 30);
                mu_label(ctx, "SELECT OPTION:");
                
                mu_layout_row(ctx, 1, (int[]){-1}, 10);
                mu_label(ctx, "");  // Spacer
                
                // Render menu items
                for (int i = 0; i < menu->item_count; i++) {
                    mu_layout_row(ctx, 1, (int[]){-1}, 25);
                    
                    static char display_text[MENU_MAX_TEXT_LENGTH + 16];
                    if (menu->terminal_style) {
                        if (i == menu->selected_index && menu->show_cursor) {
                            // Flashing cursor effect
                            const char* cursor = ((int)(menu->animation_timer * 2) % 2) ? ">" : " ";
                            snprintf(display_text, sizeof(display_text), " %s [%d] %s", 
                                   cursor, i + 1, menu->items[i].text);
                        } else {
                            snprintf(display_text, sizeof(display_text), "   [%d] %s", 
                                   i + 1, menu->items[i].text);
                        }
                    } else {
                        // Standard style
                        if (i == menu->selected_index) {
                            snprintf(display_text, sizeof(display_text), "> %s", menu->items[i].text);
                        } else {
                            snprintf(display_text, sizeof(display_text), "  %s", menu->items[i].text);
                        }
                    }
                    
                    // Get the rect that mu_label() would use
                    mu_Rect item_rect = mu_layout_next(ctx);
                    
                    
                    // Draw label inside rect  
                    mu_draw_control_text(ctx, display_text, item_rect, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
                    
                    // Handle mouse interaction
                    if (mu_mouse_over(ctx, item_rect)) {
                        menu->selected_index = i;
                        menu_clamp_selection(menu);
                        
                        if (ctx->mouse_pressed == MU_MOUSE_LEFT) {
                            if (menu->on_select && menu->items[i].enabled) {
                                menu->on_select(i, menu->callback_data);
                            }
                        }
                    }
                }
            }
            mu_end_panel(ctx);
        }
        
        // Footer with controls
        mu_layout_row(ctx, 1, (int[]){-1}, 0);  // Reset to full width
        
        mu_layout_row(ctx, 1, (int[]){-1}, 30);
        mu_label(ctx, "");  // Spacer
        
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        mu_label(ctx, "CONTROLS:");
        
        mu_layout_row(ctx, 1, (int[]){-1}, 20);
        if (menu->terminal_style) {
            mu_label(ctx, "MOUSE: Navigate and Select    ESC: Cancel");
        } else {
            mu_label(ctx, "Use mouse to navigate and select items");
        }
        
        mu_end_window(ctx);
    }
}