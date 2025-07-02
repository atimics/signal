/**
 * @file engine_test_stubs.c
 * @brief Test stubs for engine functions not needed for UI API testing
 */

#ifdef TEST_MODE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Config stubs
bool config_get_auto_start(void) { return false; }
const char* config_get_startup_scene(void) { return "test_scene"; }
void config_save(void) {}
void config_set_auto_start(bool enabled) { (void)enabled; }
void config_set_startup_scene(const char* scene) { (void)scene; }

// App/window stubs
int sapp_width(void) { return 1920; }
int sapp_height(void) { return 1080; }

// Test framework stubs  
void setUp(void) {}
void tearDown(void) {}

// System scheduler stub (minimal)
typedef struct {
    int frame_count;
    float total_time;
} SystemScheduler;

#endif // TEST_MODE
