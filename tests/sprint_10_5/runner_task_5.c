// tests/sprint_10_5/runner_task_5.c

#define SOKOL_IMPL
#define SOKOL_DUMMY_BACKEND
#include "sokol_wrapper.h"

#include "test_task_5.c"

int main() {
    printf("===== Running Sprint 10.5 Test Suite (Task 5) =====\n");
    run_task_5_tests();
    printf("=================================================\n");
    return 0;
}
