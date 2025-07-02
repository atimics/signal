/**
 * @file integration_test_stubs.c
 * @brief Integration test stubs that don't conflict with real physics system
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
// PERFORMANCE SYSTEM STUBS (only for tests that don't link performance lib)
// ============================================================================

// NOTE: performance_record_memory_usage is NOT included here since some tests
// will link with the real performance library

// ============================================================================
// CONFIG STUBS
// ============================================================================

bool config_get_auto_start(void) { return false; }
const char* config_get_startup_scene(void) { return "test_scene"; }
void config_save(void) {}
void config_set_auto_start(bool enabled) { (void)enabled; }
void config_set_startup_scene(const char* scene) { (void)scene; }

// ============================================================================
// APP/WINDOW STUBS
// ============================================================================

int sapp_width(void) { return 1920; }
int sapp_height(void) { return 1080; }

// ============================================================================
// GAMEPAD STUBS
// ============================================================================

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

// ============================================================================
// ASSETS SYSTEM STUBS
// ============================================================================

bool assets_init(AssetRegistry* registry, const char* asset_root) { 
    (void)registry; (void)asset_root; 
    return true; 
}
void assets_cleanup(AssetRegistry* registry) { (void)registry; }
void assets_list_loaded(AssetRegistry* registry) { (void)registry; }
bool assets_load_all_in_directory(AssetRegistry* registry) { 
    (void)registry; 
    return true; 
}

// ============================================================================
// DATA REGISTRY STUBS  
// ============================================================================

bool data_registry_init(void) { return true; }
void data_registry_cleanup(void) {}
void list_entity_templates(void) {}
void list_scene_templates(void) {}
void load_all_scene_templates(void) {}
void load_entity_templates(void) {}

// ============================================================================
// CAMERA STUBS
// ============================================================================

void camera_set_position(float x, float y, float z) { (void)x; (void)y; (void)z; }

// ============================================================================
// RENDER SYSTEM STUBS
// ============================================================================

bool render_init(void) { return true; }
void render_cleanup(void) {}
void set_render_config(void* config) { (void)config; }

// NOTE: physics_system_update is NOT included here since integration tests
// will link with the real physics library

#endif // TEST_MODE
