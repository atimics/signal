/**
 * @file ui_test_stubs.h
 * @brief Test stubs for UI components that use nuklear
 * 
 * Provides no-op implementations of nuklear functions for testing
 * UI logic without requiring full graphics initialization.
 */

#ifndef UI_TEST_STUBS_H
#define UI_TEST_STUBS_H

#ifdef TEST_MODE

#include <stdbool.h>
#include <stdint.h>

// Mock nuklear context type - match the real nuklear struct
struct nk_context {
    int dummy;
};

// Mock nuklear rectangle type
struct nk_rect {
    float x, y, w, h;
};

// Mock nuklear flags
#define NK_WINDOW_BORDER         (1 << 0)
#define NK_WINDOW_TITLE          (1 << 1)
#define NK_WINDOW_MINIMIZABLE    (1 << 2)
#define NK_WINDOW_MOVABLE        (1 << 3)
#define NK_WINDOW_SCALABLE       (1 << 4)
#define NK_WINDOW_NO_INPUT       (1 << 5)
#define NK_WINDOW_BACKGROUND     (1 << 6)
#define NK_TEXT_LEFT             0
#define NK_TEXT_CENTERED         1
#define NK_TREE_TAB              0
#define NK_TREE_NODE             1
#define NK_MAXIMIZED             0
#define NK_MINIMIZED             1

// Mock nuklear function declarations
struct nk_rect nk_rect(float x, float y, float w, float h);
bool nk_begin(struct nk_context* ctx, const char* title, struct nk_rect bounds, uint32_t flags);
void nk_end(struct nk_context* ctx);

static inline void nk_end(struct nk_context* ctx) {
    (void)ctx;
}

// Nuklear function declarations (implemented in nuklear_test_stubs.c)
void nk_layout_row_dynamic(struct nk_context* ctx, float height, int cols);
bool nk_button_label(struct nk_context* ctx, const char* title);
void nk_label(struct nk_context* ctx, const char* text, int align);
void nk_labelf(struct nk_context* ctx, int align, const char* fmt, ...);
bool nk_checkbox_label(struct nk_context* ctx, const char* label, bool* active);
bool nk_group_begin(struct nk_context* ctx, const char* title, uint32_t flags);
void nk_group_end(struct nk_context* ctx);
bool nk_selectable_label(struct nk_context* ctx, const char* str, int align, bool* value);
void nk_spacing(struct nk_context* ctx, int cols);
int nk_strlen(const char* str);
bool nk_tree_push_hashed(struct nk_context* ctx, int type, const char* title, int state, const char* hash, int len, int line);
void nk_tree_pop(struct nk_context* ctx);

// Mock color type
struct nk_color {
    uint8_t r, g, b, a;
};

struct nk_color nk_rgb(int r, int g, int b);
void nk_style_push_color(struct nk_context* ctx, struct nk_color* style_color, struct nk_color color);
void nk_style_pop_color(struct nk_context* ctx);

// Helper macro for style color access in test mode
#define NK_STYLE_COLOR_PTR(ctx, member) ((void*)0)

// Style types
#define NK_COLOR_TEXT 0
#define NK_COLOR_BUTTON 1

#endif // TEST_MODE

#endif // UI_TEST_STUBS_H
