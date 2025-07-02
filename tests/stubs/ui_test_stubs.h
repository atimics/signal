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

// Mock nuklear function stubs
static inline struct nk_rect nk_rect(float x, float y, float w, float h) {
    struct nk_rect r = {x, y, w, h};
    return r;
}

static inline bool nk_begin(struct nk_context* ctx, const char* title, struct nk_rect bounds, uint32_t flags) {
    (void)ctx; (void)title; (void)bounds; (void)flags;
    return true;
}

static inline void nk_end(struct nk_context* ctx) {
    (void)ctx;
}

static inline void nk_layout_row_dynamic(struct nk_context* ctx, float height, int cols) {
    (void)ctx; (void)height; (void)cols;
}

static inline void nk_label(struct nk_context* ctx, const char* text, int align) {
    (void)ctx; (void)text; (void)align;
}

static inline void nk_labelf(struct nk_context* ctx, int align, const char* fmt, ...) {
    (void)ctx; (void)align; (void)fmt;
}

static inline bool nk_button_label(struct nk_context* ctx, const char* title) {
    (void)ctx; (void)title;
    return false; // Never clicked in test mode
}

static inline bool nk_group_begin(struct nk_context* ctx, const char* title, uint32_t flags) {
    (void)ctx; (void)title; (void)flags;
    return true;
}

static inline void nk_group_end(struct nk_context* ctx) {
    (void)ctx;
}

static inline bool nk_selectable_label(struct nk_context* ctx, const char* str, int align, bool* value) {
    (void)ctx; (void)str; (void)align; (void)value;
    return false;
}

static inline bool nk_checkbox_label(struct nk_context* ctx, const char* label, bool* active) {
    (void)ctx; (void)label; (void)active;
    return false;
}

static inline bool nk_tree_push(struct nk_context* ctx, int type, const char* title, int state) {
    (void)ctx; (void)type; (void)title; (void)state;
    return true;
}

static inline bool nk_tree_push_id(struct nk_context* ctx, int type, const char* title, int state, int id) {
    (void)ctx; (void)type; (void)title; (void)state; (void)id;
    return true;
}

static inline void nk_tree_pop(struct nk_context* ctx) {
    (void)ctx;
}

static inline void nk_spacing(struct nk_context* ctx, int cols) {
    (void)ctx; (void)cols;
}

// Color functions - these need to be more sophisticated to avoid style access
static inline void nk_style_push_color(struct nk_context* ctx, void* style_item, uint32_t color) {
    (void)ctx; (void)style_item; (void)color;
}

static inline void nk_style_pop_color(struct nk_context* ctx) {
    (void)ctx;
}

static inline uint32_t nk_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 0xFF;
}

// Helper macro for style color access in test mode
#define NK_STYLE_COLOR_PTR(ctx, member) ((void*)0)

// Style types
#define NK_COLOR_TEXT 0
#define NK_COLOR_BUTTON 1

#endif // TEST_MODE

#endif // UI_TEST_STUBS_H
