// tests/sprint_10_5/runner.c

// This file is the entry point for our test suite.
// It defines the Sokol implementation with a dummy backend
// to ensure we only build what's necessary for the tests,
// without pulling in the entire application framework.

#define SOKOL_IMPL
#define SOKOL_DUMMY_BACKEND
#include "sokol_gfx.h"

// Include the test C files directly.
// This is a simple way to build a test suite without a complex build system.
#include "test_task_1.c"
#include "test_task_2.c"
// Task 3 is tested separately due to its mocking requirements.

int main() {
    printf("===== Running Sprint 10.5 Test Suite (Tasks 1 & 2) =====\n");
    
    // Call tests from each task
    run_task_1_tests();
    run_task_2_tests();

    printf("===== All Sprint 10.5 integration tests passed! =====\n");
    printf("NOTE: Task 3 is tested in a separate, mocked executable.\n");
    return 0;
}
