/**
 * @file performance_test_stubs.c
 * @brief Test stubs for performance testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/render.h"
#include "../../src/core.h"

// Simple stub for performance tests - just validates basic functionality
void test_performance_stub(void) {
    printf("Performance tests are currently stubbed - basic validation only\n");
}

// Initialize a minimal render config for testing
void init_test_render_config(RenderConfig* config) {
    memset(config, 0, sizeof(RenderConfig));
    config->screen_width = 1920;
    config->screen_height = 1080;
    config->mode = RENDER_MODE_3D;
}
