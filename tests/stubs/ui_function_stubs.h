#ifndef UI_FUNCTION_STUBS_H
#define UI_FUNCTION_STUBS_H

#include "../../src/core.h"
#include "../../src/systems.h"

// Test scheduler type that's compatible with SystemScheduler
typedef struct TestScheduler {
    SystemScheduler base_scheduler;
    int test_field;
} TestScheduler;

// UI function stubs for testing
void ui_init(void);
void ui_cleanup(void);
void ui_shutdown(void);
void ui_clear_scene_change_request(void);
void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene, int screen_width, int screen_height);
void ui_request_scene_change(const char* scene_name);
bool ui_has_scene_change_request(void);

// MicroUI function stubs for testing  
void ui_microui_init(void);
void ui_microui_cleanup(void);
void ui_microui_shutdown(void);
void ui_microui_begin_frame(void);
void ui_microui_end_frame(void);
void ui_microui_render(int screen_width, int screen_height);

// Forward declarations
struct UIContext;
struct mu_Context;

// MicroUI function stubs
struct mu_rect { float x, y, w, h; };
struct mu_rect mu_rect(float x, float y, float w, float h);
bool mu_begin_window(struct mu_Context* ctx, const char* title, struct mu_rect rect);
void mu_end_window(struct mu_Context* ctx);
void mu_layout_row(struct mu_Context* ctx, int items, int* widths, int height);
void mu_label(struct mu_Context* ctx, const char* text);
bool mu_button(struct mu_Context* ctx, const char* text);

// Context access stubs
struct UIContext* ui_microui_get_context(void);
struct mu_Context* ui_microui_get_mu_context(void);

#endif // UI_FUNCTION_STUBS_H
