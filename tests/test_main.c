// tests/test_main.c
#include <stdio.h>

#include "vendor/unity.h"

// This file declares the test suites that the test runner will execute.
// The actual test runner is in `test_runner.c`.

// Forward declare the test suites to be run.
void suite_core_math(void);
void suite_assets(void);
void suite_rendering(void);

// New comprehensive rendering test suites
void suite_render_layers(void);
void suite_ui_rendering_pipeline(void);
void suite_graphics_pipeline(void);
void suite_render_3d_pipeline(void);
void suite_rendering_integration(void);

// Control scheme test suite
void suite_unified_control_scheme(void);

// These are required by Unity, but we can leave them empty if no
// global setup/teardown is needed for most tests.
void setUp(void)
{
}
void tearDown(void)
{
}
