#include "ui_function_stubs.h"
#include <stdio.h>

// Test state tracking
static bool scene_change_requested = false;

// UI function stubs implementation
void ui_init(void) {
    // No-op in test mode
    printf("🧪 ui_init() called in test mode\n");
}

void ui_cleanup(void) {
    // No-op in test mode
    printf("🧪 ui_cleanup() called in test mode\n");
}

void ui_shutdown(void) {
    // No-op in test mode  
    printf("🧪 ui_shutdown() called in test mode\n");
}

void ui_clear_scene_change_request(void) {
    // Reset scene change state in test mode
    scene_change_requested = false;
}

void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene, int screen_width, int screen_height) {
    // No-op in test mode
    (void)world; (void)scheduler; (void)delta_time; (void)current_scene; (void)screen_width; (void)screen_height;
    printf("🧪 ui_render() called in test mode\n");
}

void ui_request_scene_change(const char* scene_name) {
    // Set scene change state in test mode
    scene_change_requested = true;
    (void)scene_name;
    printf("🧪 ui_request_scene_change(%s) called in test mode\n", scene_name);
}

bool ui_has_scene_change_request(void) {
    // Return actual state in test mode
    return scene_change_requested;
}

// MicroUI function stubs implementation
void ui_microui_init(void) {
    // No-op in test mode
    printf("🧪 ui_microui_init() called in test mode\n");
}

void ui_microui_cleanup(void) {
    // No-op in test mode
    printf("🧪 ui_microui_cleanup() called in test mode\n");
}

void ui_microui_shutdown(void) {
    // No-op in test mode
    printf("🧪 ui_microui_shutdown() called in test mode\n");
}

void ui_microui_begin_frame(void) {
    // No-op in test mode
}

void ui_microui_end_frame(void) {
    // No-op in test mode
}

void ui_microui_render(int screen_width, int screen_height) {
    // No-op in test mode
    (void)screen_width; (void)screen_height;
    printf("🧪 ui_microui_render() called in test mode\n");
}

// Context access stubs
struct UIContext* ui_microui_get_context(void) {
    // Return NULL in test mode
    return NULL;
}

struct mu_Context* ui_microui_get_mu_context(void) {
    // Return NULL in test mode
    return NULL;
}

// MicroUI function stubs
struct mu_rect mu_rect(float x, float y, float w, float h) {
    struct mu_rect rect = {x, y, w, h};
    return rect;
}

bool mu_begin_window(struct mu_Context* ctx, const char* title, struct mu_rect rect) {
    (void)ctx; (void)title; (void)rect;
    return false; // No actual UI in test mode
}

void mu_end_window(struct mu_Context* ctx) {
    (void)ctx;
}

void mu_layout_row(struct mu_Context* ctx, int items, int* widths, int height) {
    (void)ctx; (void)items; (void)widths; (void)height;
}

void mu_label(struct mu_Context* ctx, const char* text) {
    (void)ctx; (void)text;
}

bool mu_button(struct mu_Context* ctx, const char* text) {
    (void)ctx; (void)text;
    return false; // No actual interaction in test mode
}
