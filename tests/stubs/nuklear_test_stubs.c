/**
 * @file nuklear_test_stubs.c
 * @brief Test stub implementations for Nuklear UI functions
 */

#ifdef TEST_MODE

#include "ui_test_stubs.h"
#include <stdarg.h>

// Nuklear function implementations (non-inline)
struct nk_rect nk_rect(float x, float y, float w, float h) {
    struct nk_rect r = {x, y, w, h};
    return r;
}

bool nk_begin(struct nk_context* ctx, const char* title, struct nk_rect bounds, uint32_t flags) {
    (void)ctx; (void)title; (void)bounds; (void)flags;
    return true;
}

void nk_end(struct nk_context* ctx) {
    (void)ctx;
}

void nk_layout_row_dynamic(struct nk_context *ctx, float height, int cols) {
    (void)ctx; (void)height; (void)cols;
}

bool nk_button_label(struct nk_context *ctx, const char *title) {
    (void)ctx; (void)title;
    return false;
}

void nk_label(struct nk_context *ctx, const char *str, int flags) {
    (void)ctx; (void)str; (void)flags;
}

void nk_labelf(struct nk_context *ctx, int flags, const char *fmt, ...) {
    (void)ctx; (void)flags; (void)fmt;
}

bool nk_checkbox_label(struct nk_context *ctx, const char *label, bool *active) {
    (void)ctx; (void)label; (void)active;
    return false;
}

bool nk_group_begin(struct nk_context *ctx, const char *title, uint32_t flags) {
    (void)ctx; (void)title; (void)flags;
    return true;
}

void nk_group_end(struct nk_context *ctx) {
    (void)ctx;
}

bool nk_selectable_label(struct nk_context *ctx, const char *str, int flags, bool *value) {
    (void)ctx; (void)str; (void)flags; (void)value;
    return false;
}

void nk_spacing(struct nk_context *ctx, int cols) {
    (void)ctx; (void)cols;
}

int nk_strlen(const char *str) {
    int len = 0;
    while (str && str[len]) len++;
    return len;
}

bool nk_tree_push_hashed(struct nk_context *ctx, int type, const char *title, int state, const char *hash, int len, int line) {
    (void)ctx; (void)type; (void)title; (void)state; (void)hash; (void)len; (void)line;
    return true;
}

void nk_tree_pop(struct nk_context *ctx) {
    (void)ctx;
}

struct nk_color nk_rgb(int r, int g, int b) {
    struct nk_color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b, 255};
    return color;
}

void nk_style_push_color(struct nk_context *ctx, struct nk_color *style_color, struct nk_color color) {
    (void)ctx; (void)style_color; (void)color;
}

void nk_style_pop_color(struct nk_context *ctx) {
    (void)ctx;
}

#endif // TEST_MODE
