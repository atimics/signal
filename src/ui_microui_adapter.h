/**
 * @file ui_microui_adapter.h
 * @brief Adapter functions for scene UI to use Microui instead of Nuklear
 */

#ifndef UI_MICROUI_ADAPTER_H
#define UI_MICROUI_ADAPTER_H

#include "core.h"

// Forward declarations
struct mu_Context;
struct World;
struct SystemScheduler;

// Render scene-specific UI using Microui
void scene_ui_render_microui(struct mu_Context* ctx, const char* scene_name, 
                            struct World* world, struct SystemScheduler* scheduler, 
                            float delta_time, int screen_width, int screen_height);

#endif // UI_MICROUI_ADAPTER_H