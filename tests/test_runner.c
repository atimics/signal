// tests/test_runner.c
#include "vendor/unity.h"
#include <stdio.h>

// Define the Sokol implementation for our headless tests.
// This must only be done in one place.
#define SOKOL_IMPL
#define SOKOL_DUMMY_BACKEND
#include "sokol_gfx.h"
#include "sokol_log.h"

// Forward declare the test suites - defined in their respective test files
void suite_core_math(void);
void suite_assets(void);
void suite_rendering(void);

// These are required by Unity for global setup/teardown
void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    printf("\n--- Running Core & Asset Tests ---\n");
    suite_core_math();
    suite_assets();

    // The rendering test suite requires a valid (but headless) GFX context.
    // We set it up and tear it down just for this suite.
    printf("\n--- Running Rendering Regression Tests ---\n");
    sg_desc desc = {.logger.func = slog_func};
    sg_setup(&desc);
    
    suite_rendering();
    
    sg_shutdown();
    printf("--- Headless GFX Context Shut Down ---\n");

    return UNITY_END();
}

