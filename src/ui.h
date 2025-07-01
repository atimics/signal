#ifndef UI_H
#define UI_H

#include "core.h"
#include "systems.h"

// UI State management
typedef struct
{
    bool show_debug_panel;
    bool show_hud;
    bool show_wireframe;
    float camera_speed;
    float time_scale;

    // FPS tracking
    float fps;
    int frame_count;
    float fps_timer;
} UIState;

void ui_init(void);
void ui_shutdown(void);
void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time);
bool ui_handle_event(const void* ev);  // Returns true if UI captured the event
void ui_toggle_debug_panel(void);
void ui_toggle_hud(void);

// UI visibility control
void ui_set_visible(bool visible);
void ui_set_debug_visible(bool visible);
bool ui_is_visible(void);
bool ui_is_debug_visible(void);

#endif  // UI_H
