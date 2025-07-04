/**
 * @file ui_microui_test_utils.h
 * @brief Test utilities for MicroUI testing
 * 
 * Provides additional functions for inspecting MicroUI state during tests.
 */

#ifndef UI_MICROUI_TEST_UTILS_H
#define UI_MICROUI_TEST_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Vertex and command inspection functions
int ui_microui_get_vertex_count(void);
int ui_microui_get_command_count(void);
int ui_microui_get_draw_call_count(void);

// Vertex data access
const void* ui_microui_get_vertex_data(void);
size_t ui_microui_get_vertex_data_size(void);

// Font texture state
bool ui_microui_is_font_texture_bound(void);

// Memory usage tracking
size_t ui_microui_get_memory_usage(void);

// UI state queries
bool ui_wants_mouse_input(void);
bool ui_wants_keyboard_input(void);

// UI visibility control
void ui_set_visible(bool visible);
bool ui_is_visible(void);

// UI system lifecycle
void ui_init(void);
void ui_shutdown(void);

// Frame management
void ui_begin_frame(void);
void ui_end_frame(void);
void ui_render(void);

// Event handling
typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_MOUSE_BUTTON,
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_KEY,
    UI_EVENT_CHAR,
    UI_EVENT_SCROLL
} ui_event_type_t;

typedef struct {
    int button;
    int action; // 0 = release, 1 = press
    float x, y;
} ui_mouse_button_event_t;

typedef struct {
    float x, y;
} ui_mouse_move_event_t;

typedef struct {
    int key;
    int action; // 0 = release, 1 = press
    int mods;
} ui_key_event_t;

typedef struct {
    float x, y;
} ui_scroll_event_t;

typedef struct {
    ui_event_type_t type;
    union {
        ui_mouse_button_event_t mouse_button;
        ui_mouse_move_event_t mouse_move;
        ui_key_event_t key;
        ui_scroll_event_t scroll;
    };
} ui_event_t;

void ui_handle_event(ui_event_t* event);

// Modifier key flags
#define UI_MOD_SHIFT 0x01
#define UI_MOD_CTRL  0x02
#define UI_MOD_ALT   0x04

#ifdef __cplusplus
}
#endif

#endif // UI_MICROUI_TEST_UTILS_H