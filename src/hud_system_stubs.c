/**
 * @file hud_system_stubs.c
 * @brief Temporary stubs for HUD system to allow building without Nuklear
 */

#include "hud_system.h"
#include <stdio.h>

// Forward declaration to avoid Nuklear dependency
struct nk_context;

// Stub implementations for HUD functions

void hud_system_init(void) {
    printf("✅ HUD system initialized (stub)\n");
}

void hud_system_shutdown(void) {
    printf("✅ HUD system shut down (stub)\n");
}

void hud_system_update(float delta_time) {
    (void)delta_time;
}

void hud_system_render(void* ctx, struct World* world) {
    (void)ctx;
    (void)world;
}

void hud_system_set_camera_mode(HUDCameraMode mode) {
    (void)mode;
}

HUDCameraMode hud_system_get_camera_mode(void) {
    return HUD_CAMERA_MODE_CHASE_NEAR;
}

void hud_system_configure_mode(HUDCameraMode mode, HUDComponentType* components, int component_count) {
    (void)mode;
    (void)components;
    (void)component_count;
}

void hud_system_set_component_position(HUDComponentType type, Vector2 position) {
    (void)type;
    (void)position;
}

void hud_system_set_component_size(HUDComponentType type, Vector2 size) {
    (void)type;
    (void)size;
}

void hud_system_set_component_opacity(HUDComponentType type, float opacity) {
    (void)type;
    (void)opacity;
}

// Individual component render stubs
void hud_render_targeting_reticle(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_speedometer(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_thrust_indicator(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_instrument_panel(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_communications(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_trade_interface(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_waypoint_display(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_tactical_overlay(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_proximity_warnings(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_weapon_systems(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}

void hud_render_exploration_scanner(void* ctx, struct World* world, Vector2 pos, Vector2 size, float opacity) {
    (void)ctx; (void)world; (void)pos; (void)size; (void)opacity;
}