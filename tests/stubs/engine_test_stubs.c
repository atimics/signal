/**
 * @file engine_test_stubs.c
 * @brief Test stubs for engine functions not needed for UI API testing
 */

#ifdef TEST_MODE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../src/system/gamepad.h"
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

// Gamepad stubs for testing
static GamepadState test_gamepad_state = {0};

bool gamepad_init(void) { return true; }
void gamepad_shutdown(void) {}
void gamepad_poll(void) {}
GamepadState* gamepad_get_state(int gamepad_index) { 
    (void)gamepad_index; 
    return &test_gamepad_state; 
}
bool gamepad_is_connected(int gamepad_index) { 
    (void)gamepad_index; 
    return false; 
}
bool gamepad_button_pressed(int gamepad_index, GamepadButton button) { 
    (void)gamepad_index; 
    (void)button; 
    return false; 
}
bool gamepad_button_just_pressed(int gamepad_index, GamepadButton button) { 
    (void)gamepad_index; 
    (void)button; 
    return false; 
}
bool gamepad_button_just_released(int gamepad_index, GamepadButton button) { 
    (void)gamepad_index; 
    (void)button; 
    return false; 
}
float gamepad_get_axis(int gamepad_index, const char* axis_name) { 
    (void)gamepad_index; 
    (void)axis_name; 
    return 0.0f; 
}
const char* gamepad_get_button_name(GamepadButton button) { 
    (void)button; 
    return "test_button"; 
}
void gamepad_set_deadzone(float deadzone) { 
    (void)deadzone; 
}

// System scheduler stub (minimal)
typedef struct {
    int frame_count;
    float total_time;
} SystemScheduler;

// ============================================================================
// PHYSICS SYSTEM STUBS
// ============================================================================

void physics_system_update(struct World* world, float delta_time) {
    (void)world;
    (void)delta_time;
}

#endif // TEST_MODE
