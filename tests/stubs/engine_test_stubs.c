/**
 * @file engine_test_stubs.c
 * @brief Test stubs for engine functions not needed for UI API testing
 */

#ifdef TEST_MODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
// Legacy gamepad.h removed - no longer needed
#include "../../src/assets.h"

// ============================================================================
// GLOBAL VARIABLES STUBS
// ============================================================================

// Global asset registry (referenced by memory system)
AssetRegistry g_asset_registry = {0};

// ============================================================================
// SOKOL GRAPHICS API STUBS
// ============================================================================

// Sokol buffer/image handles (stub types)
typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;
typedef enum { 
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID
} sg_resource_state;

// Sokol buffer description (stub)
typedef struct {
    size_t size;
    const void* data;
    const char* label;
} sg_buffer_desc;

// Sokol image description (stub)
typedef struct {
    int width;
    int height;
    const void* data;
    const char* label;
} sg_image_desc;

// Sokol function stubs
sg_buffer sg_make_buffer(const sg_buffer_desc* desc) {
    (void)desc;
    return (sg_buffer){.id = 1};
}

sg_image sg_make_image(const sg_image_desc* desc) {
    (void)desc;
    return (sg_image){.id = 1};
}

void sg_destroy_buffer(sg_buffer buffer) {
    (void)buffer;
}

void sg_destroy_image(sg_image image) {
    (void)image;
}

sg_resource_state sg_query_buffer_state(sg_buffer buffer) {
    (void)buffer;
    return SG_RESOURCESTATE_VALID;
}

// ============================================================================
// PERFORMANCE SYSTEM STUBS
// ============================================================================

void performance_record_memory_usage(size_t bytes_allocated, size_t bytes_freed) {
    (void)bytes_allocated;
    (void)bytes_freed;
}

// Config stubs
bool config_get_auto_start(void) { return false; }
const char* config_get_startup_scene(void) { return "test_scene"; }
void config_save(void) {}
void config_set_auto_start(bool enabled) { (void)enabled; }
void config_set_startup_scene(const char* scene) { (void)scene; }

// App/window stubs
int sapp_width(void) { return 1920; }
int sapp_height(void) { return 1080; }

// Scene state stubs
#ifndef INCLUDE_REAL_SCENE_STATE
void scene_state_request_transition(const char* scene_name) {
    (void)scene_name;
    // Test stub - no-op
}
#endif

// UI adaptive control stubs
bool ui_adaptive_should_show_gamepad(void) { return false; }
const char* ui_adaptive_get_hint_text(const char* action) {
    (void)action;
    return "Press A";
}

// UI visibility stub
void ui_set_visible(bool visible) {
    (void)visible;
    // Test stub - no-op
}

// Gamepad stubs removed - legacy system deprecated

// System scheduler stub (minimal)
typedef struct {
    int frame_count;
    float total_time;
} SystemScheduler;

// ============================================================================
// UI PERFORMANCE WIDGET STUBS (only for tests that don't link ui_components)
// ============================================================================

// NOTE: performance_widget_* functions are NOT included here since some tests
// will link with the real ui_components library

// NOTE: physics_system_update is NOT included here since integration tests
// will link with the real physics library

// ============================================================================
// OFFSCREEN RENDERING STUBS
// ============================================================================

// Stub for tests that don't link render_3d.c
bool render_is_offscreen_mode(void) {
    return false;  // Tests always render to default
}

// ============================================================================
// SHADER LOADING STUBS
// ============================================================================

// Note: shader loading functions are implemented in assets.c

#endif // TEST_MODE
