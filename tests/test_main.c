// tests/test_main.c
#include "vendor/unity.h"
#include <stdio.h>

// This file declares the test suites that the test runner will execute.
// The actual test runner is in `test_runner.c`.

// Forward declare the test suites to be run.
void suite_core_math(void);
void suite_assets(void);
void suite_rendering(void);

// These are required by Unity, but we can leave them empty if no
// global setup/teardown is needed for most tests.
void setUp(void) {}
void tearDown(void) {}

// The main function is in test_runner.c
