/**
 * @file ui_api.c
 * @brief Public UI API implementation
 */

#include "ui_api.h"
#include "ui_scene.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// SCENE TRANSITION STATE
// ============================================================================

static char g_requested_scene[64] = {0};
static bool g_scene_change_requested = false;

// ============================================================================
// DEBUG UI STATE
// ============================================================================

static bool g_debug_panel_visible = false;
static bool g_wireframe_enabled = false;
static bool g_hud_visible = true;

// ============================================================================
// SCENE TRANSITION API
// ============================================================================

void ui_request_scene_change(const char* scene_name)
{
    if (!scene_name) {
        return;
    }
    
    strncpy(g_requested_scene, scene_name, sizeof(g_requested_scene) - 1);
    g_requested_scene[sizeof(g_requested_scene) - 1] = '\0';
    g_scene_change_requested = true;
    
    printf("🎬 UI API: Scene change requested - %s\n", scene_name);
}

bool ui_has_scene_change_request(void)
{
    return g_scene_change_requested;
}

const char* ui_get_requested_scene(void)
{
    return g_scene_change_requested ? g_requested_scene : NULL;
}

void ui_clear_scene_change_request(void)
{
    g_scene_change_requested = false;
    g_requested_scene[0] = '\0';
}

// ============================================================================
// DEBUG UI CONTROL
// ============================================================================

void ui_toggle_debug_panel(void)
{
    g_debug_panel_visible = !g_debug_panel_visible;
    printf("🔧 Debug panel %s\n", g_debug_panel_visible ? "enabled" : "disabled");
}

void ui_set_debug_panel_visible(bool visible)
{
    g_debug_panel_visible = visible;
}

bool ui_is_debug_panel_visible(void)
{
    return g_debug_panel_visible;
}

// ============================================================================
// GLOBAL UI STATE
// ============================================================================

void ui_toggle_hud(void)
{
    g_hud_visible = !g_hud_visible;
    printf("📊 HUD %s\n", g_hud_visible ? "enabled" : "disabled");
}

void ui_set_hud_visible(bool visible)
{
    g_hud_visible = visible;
}

bool ui_is_hud_visible(void)
{
    return g_hud_visible;
}

void ui_toggle_wireframe(void)
{
    g_wireframe_enabled = !g_wireframe_enabled;
    printf("🔧 Wireframe mode %s\n", g_wireframe_enabled ? "enabled" : "disabled");
    
    // TODO: Apply to render config
}

bool ui_is_wireframe_enabled(void)
{
    return g_wireframe_enabled;
}
