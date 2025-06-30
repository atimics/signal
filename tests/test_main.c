// tests/test_main.c
#include "vendor/unity.h"
#include <stdio.h>

// Forward declare the test suites to be run.
void suite_core_math(void);

// setUp and tearDown are called before and after each test.
// We can leave them empty for now.
void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    // Run the test suites
    suite_core_math();

    return UNITY_END();
}
