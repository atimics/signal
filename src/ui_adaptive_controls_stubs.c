/**
 * @file ui_adaptive_controls_stubs.c
 * @brief Stub implementations for adaptive controls to allow building without Nuklear
 */

#include "ui_adaptive_controls.h"
#include <stdio.h>

// Initialize the adaptive controls system
void ui_adaptive_controls_init(void)
{
    printf("✅ Adaptive controls initialized (stub)\n");
}

// Shutdown the adaptive controls system
void ui_adaptive_controls_shutdown(void)
{
    printf("✅ Adaptive controls shut down (stub)\n");
}

// Update adaptive controls based on current context
void ui_adaptive_controls_update(float delta_time)
{
    (void)delta_time;
}

// Render adaptive control overlays
void ui_adaptive_controls_render(void* ctx, void* world)
{
    (void)ctx;
    (void)world;
}

// Check if adaptive controls are currently active
bool ui_adaptive_controls_active(void)
{
    return false;
}

// Set adaptive controls enabled/disabled
void ui_adaptive_controls_set_enabled(bool enabled)
{
    (void)enabled;
}

// Handle input events for adaptive controls
bool ui_adaptive_controls_handle_event(const void* event)
{
    (void)event;
    return false;
}