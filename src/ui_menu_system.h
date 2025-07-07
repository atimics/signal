/**
 * @file ui_menu_system.h
 * @brief General purpose menu system for MicroUI
 */

#ifndef UI_MENU_SYSTEM_H
#define UI_MENU_SYSTEM_H

#include "microui/microui.h"
#include <stdbool.h>

// Maximum menu items and text lengths
#define MENU_MAX_ITEMS 16
#define MENU_MAX_TEXT_LENGTH 128
#define MENU_MAX_DESC_LENGTH 256

// Menu item structure
typedef struct {
    char text[MENU_MAX_TEXT_LENGTH];
    char description[MENU_MAX_DESC_LENGTH];
    void* user_data;  // Custom data for each item
    bool enabled;
} MenuItem;

// Menu callback function types
typedef void (*MenuSelectCallback)(int item_index, void* user_data);
typedef void (*MenuCancelCallback)(void* user_data);

// Menu configuration
typedef struct {
    char title[MENU_MAX_TEXT_LENGTH];
    MenuItem items[MENU_MAX_ITEMS];
    int item_count;
    
    // Callbacks
    MenuSelectCallback on_select;
    MenuCancelCallback on_cancel;
    void* callback_data;
    
    // Visual settings
    bool show_descriptions;
    bool show_cursor;
    bool terminal_style;
    
    // State
    int selected_index;
    float animation_timer;
} Menu;

// Menu system functions
void menu_init(Menu* menu, const char* title);
void menu_add_item(Menu* menu, const char* text, const char* description, void* user_data);
void menu_set_callbacks(Menu* menu, MenuSelectCallback on_select, MenuCancelCallback on_cancel, void* callback_data);
void menu_set_style(Menu* menu, bool show_descriptions, bool show_cursor, bool terminal_style);

// Rendering function
void menu_render(Menu* menu, mu_Context* ctx, float delta_time);

// Input handling
bool menu_handle_key(Menu* menu, int key);
void menu_update(Menu* menu, float delta_time);

#endif // UI_MENU_SYSTEM_H