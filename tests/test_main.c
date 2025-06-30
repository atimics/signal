// tests/test_main.c
#include "vendor/unity.h"
#include <stdio.h>

// Include Sokol GFX for the headless rendering context
#define SOKOL_IMPL
#define SOKOL_DUMMY_BACKEND
#include "sokol_gfx.h"
#include "sokol_log.h"

// Forward declare the test suites to be run.
void suite_core_math(void);
void suite_assets(void);
void suite_rendering(void);

// setUp and tearDown are called before and after each test.
void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    // --- Phase 1 & 2 Tests (No GFX context needed) ---
    suite_core_math();
    RUN_TEST_SUITE(suite_assets);

    // --- Phase 3: Rendering Regression Test ---
    // This suite requires a valid (but headless) GFX context.
    printf("
--- Initializing Headless GFX Context for Rendering Tests ---
");
    sg_desc desc = {.logger.func = slog_func};
    sg_setup(&desc);
    
    RUN_TEST_SUITE(suite_rendering);
    
    sg_shutdown();
    printf("--- Headless GFX Context Shut Down ---
");

    return UNITY_END();
}
