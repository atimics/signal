/**
 * @file microui_test_stubs.c
 * @brief Test stubs for MicroUI functions used in unit tests
 */

// Don't include the real microui.h to avoid macro conflicts
// Instead, define what we need directly
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Sokol app function stubs (types defined in sokol_app_stub.h)
#ifdef SOKOL_APP_STUB_H
int sapp_width(void) { return 1280; }
int sapp_height(void) { return 720; }
#endif

// MicroUI type definitions for testing
typedef struct { int x, y, w, h; } mu_Rect;
typedef struct { int r, g, b, a; } mu_Color;
typedef struct {
    int idx;
    int items_size;
    void* items;
} mu_CommandList;

typedef struct {
    int idx;
} mu_LayoutStack;

typedef struct {
    int idx;
} mu_ContainerStack;

typedef struct {
    int idx;
} mu_RootList;

typedef struct mu_Context {
    int frame;
    mu_CommandList command_list;
    mu_RootList root_list;
    mu_ContainerStack container_stack;
    mu_LayoutStack layout_stack;
} mu_Context;

// Test state tracking
static int test_command_count = 0;
static int test_vertex_count = 0;
static int test_window_count = 0;
static int test_button_clicks = 0;

// Forward declarations
mu_Rect mu_rect(int x, int y, int w, int h);
int mu_begin_window_ex(mu_Context* ctx, const char* title, mu_Rect rect, int opt);
int mu_button_ex(mu_Context* ctx, const char* label, int icon, int opt);

// Reset test state
void microui_test_reset(void) {
    test_command_count = 0;
    test_vertex_count = 0;
    test_window_count = 0;
    test_button_clicks = 0;
}

// Get test metrics
int microui_test_get_command_count(void) { return test_command_count; }
int microui_test_get_vertex_count(void) { return test_vertex_count; }
int microui_test_get_window_count(void) { return test_window_count; }
int microui_test_get_button_clicks(void) { return test_button_clicks; }

// MicroUI stub implementations
mu_Context* mu_create_context(void) {
    mu_Context* ctx = calloc(1, sizeof(mu_Context));
    return ctx;
}

void mu_destroy_context(mu_Context* ctx) {
    if (ctx) free(ctx);
}

void mu_begin(mu_Context* ctx) {
    if (ctx) {
        ctx->frame++;
    }
}

void mu_end(mu_Context* ctx) {
    if (ctx) {
        // Reset command list for next frame
        ctx->command_list.idx = 0;
    }
}

int mu_begin_window(mu_Context* ctx, const char* title, mu_Rect rect) {
    return mu_begin_window_ex(ctx, title, rect, 0);
}

int mu_begin_window_ex(mu_Context* ctx, const char* title, mu_Rect rect, int opt) {
    (void)title;
    (void)rect;
    (void)opt;
    if (ctx) {
        test_window_count++;
        test_command_count += 3; // Window typically generates multiple commands
        return 1; // Always return success for tests
    }
    return 0;
}

void mu_end_window(mu_Context* ctx) {
    if (ctx) {
        test_command_count += 1;
    }
}

int mu_button(mu_Context* ctx, const char* label) {
    return mu_button_ex(ctx, label, 0, 0);
}

int mu_button_ex(mu_Context* ctx, const char* label, int icon, int opt) {
    (void)label;
    (void)icon;
    (void)opt;
    if (ctx) {
        test_command_count += 2; // Button generates rect and text commands
        test_vertex_count += 10; // Approximate vertices for a button
        
        // Simulate button click on every 3rd call
        if ((ctx->frame % 3) == 0) {
            test_button_clicks++;
            return 1;
        }
    }
    return 0;
}

void mu_label(mu_Context* ctx, const char* label) {
    (void)label;
    if (ctx) {
        test_command_count += 1;
        test_vertex_count += strlen(label) * 4; // Approximate vertices per character
    }
}

void mu_text(mu_Context* ctx, const char* text) {
    mu_label(ctx, text);
}

int mu_checkbox(mu_Context* ctx, const char* label, int* state) {
    (void)label;
    if (ctx && state) {
        test_command_count += 2;
        test_vertex_count += 8;
        
        // Toggle state on click
        if ((ctx->frame % 4) == 0) {
            *state = !*state;
            return 1;
        }
    }
    return 0;
}

int mu_textbox_ex(mu_Context* ctx, char* buf, int bufsz, int opt) {
    (void)buf;
    (void)bufsz;
    (void)opt;
    if (ctx) {
        test_command_count += 2;
        test_vertex_count += 12;
    }
    return 0;
}

void mu_layout_row(mu_Context* ctx, int items, const int* widths, int height) {
    (void)items;
    (void)widths;
    (void)height;
    if (ctx) {
        // Layout doesn't generate commands directly
    }
}

mu_Rect mu_layout_next(mu_Context* ctx) {
    if (ctx) {
        // Layout advancement - return a dummy rect
        return mu_rect(0, 0, 100, 30);
    }
    return mu_rect(0, 0, 0, 0);
}

mu_Rect mu_rect(int x, int y, int w, int h) {
    mu_Rect r = {x, y, w, h};
    return r;
}

mu_Color mu_color(int r, int g, int b, int a) {
    mu_Color c = {r, g, b, a};
    return c;
}

void mu_draw_rect(mu_Context* ctx, mu_Rect rect, mu_Color color) {
    (void)rect;
    (void)color;
    if (ctx) {
        test_command_count += 1;
        test_vertex_count += 4; // Rectangle = 4 vertices
    }
}

void mu_push_clip_rect(mu_Context* ctx, mu_Rect rect) {
    (void)rect;
    if (ctx) {
        test_command_count += 1;
    }
}

void mu_pop_clip_rect(mu_Context* ctx) {
    if (ctx) {
        test_command_count += 1;
    }
}