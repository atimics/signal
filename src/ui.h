#ifndef UI_H
#define UI_H

#include "core.h"
#include "systems.h"

// Forward declarations
struct nk_context;

// ============================================================================
// CORE UI SYSTEM
// ============================================================================

void ui_init(void);
void ui_shutdown(void);
void ui_render(struct World* world, SystemScheduler* scheduler, float delta_time, const char* current_scene);
bool ui_handle_event(const void* ev);  // Returns true if UI captured the event

// UI visibility control
void ui_set_visible(bool visible);
void ui_set_debug_visible(bool visible);
bool ui_is_visible(void);
bool ui_is_debug_visible(void);

// NOTE: Flight HUD system moved to hud_system.h for modular camera mode support

#endif  // UI_H
