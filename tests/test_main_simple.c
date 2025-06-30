// tests/test_main_simple.c - Phase 1 Math Tests Only
#include <stdio.h>

#include "vendor/unity.h"

// Forward declare the test suites to be run.
void suite_core_math(void);

// setUp and tearDown are called before and after each test.
void setUp(void)
{
}
void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();

    // Phase 1: Core math unit tests
    suite_core_math();

    return UNITY_END();
}
