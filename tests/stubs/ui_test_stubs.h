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

// Mock nuklear context type
typedef struct {
    int dummy;
} nk_context;

// Mock nuklear rectangle type
typedef struct {
    float x, y, w, h;
} nk_rect;

// Mock nuklear flags
#define NK_WINDOW_BORDER      (1 << 0)
#define NK_WINDOW_TITLE       (1 << 1)
#define NK_WINDOW_MINIMIZABLE (1 << 2)
#define NK_WINDOW_MOVABLE     (1 << 3)
#define NK_WINDOW_SCALABLE    (1 << 4)
#define NK_TEXT_LEFT          0
#define NK_TEXT_CENTERED      1

// Mock nuklear function stubs
static inline nk_rect nk_rect(float x, float y, float w, float h) {
    nk_rect r = {x, y, w, h};
    return r;
}

static inline bool nk_begin(nk_context* ctx, const char* title, nk_rect bounds, uint32_t flags) {
    (void)ctx; (void)title; (void)bounds; (void)flags;
    return true;
}

static inline void nk_end(nk_context* ctx) {
    (void)ctx;
}

static inline void nk_layout_row_dynamic(nk_context* ctx, float height, int cols) {
    (void)ctx; (void)height; (void)cols;
}

static inline void nk_label(nk_context* ctx, const char* text, int align) {
    (void)ctx; (void)text; (void)align;
}

static inline void nk_labelf(nk_context* ctx, int align, const char* fmt, ...) {
    (void)ctx; (void)align; (void)fmt;
}

static inline bool nk_button_label(nk_context* ctx, const char* title) {
    (void)ctx; (void)title;
    return false; // Never clicked in test mode
}

static inline bool nk_group_begin(nk_context* ctx, const char* title, uint32_t flags) {
    (void)ctx; (void)title; (void)flags;
    return true;
}

static inline void nk_group_end(nk_context* ctx) {
    (void)ctx;
}

static inline bool nk_selectable_label(nk_context* ctx, const char* str, int align, bool* value) {
    (void)ctx; (void)str; (void)align; (void)value;
    return false;
}

static inline bool nk_checkbox_label(nk_context* ctx, const char* label, bool* active) {
    (void)ctx; (void)label; (void)active;
    return false;
}

// Color functions
static inline void nk_style_push_color(nk_context* ctx, int type, uint32_t color) {
    (void)ctx; (void)type; (void)color;
}

static inline void nk_style_pop_color(nk_context* ctx) {
    (void)ctx;
}

// Style types
#define NK_COLOR_TEXT 0
#define NK_COLOR_BUTTON 1

#endif // TEST_MODE

#endif // UI_TEST_STUBS_H
