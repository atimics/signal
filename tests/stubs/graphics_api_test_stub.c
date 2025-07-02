/**
 * @file graphics_api_test_stub.c
 * @brief Test stub for graphics API that avoids sokol implementation
 */

#ifdef TEST_MODE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Define minimum required types for nuklear in test mode
struct nk_context;
typedef struct nk_context nk_context;

// Stub implementation for get_nuklear_context
nk_context* get_nuklear_context(void) {
    static nk_context dummy_ctx;
    return &dummy_ctx;
}

// Stub implementations for any other graphics functions that might be called
void graphics_begin_frame(void) {
    // No-op in test mode
}

void graphics_end_frame(void) {
    // No-op in test mode
}

bool graphics_init(int width, int height) {
    (void)width; (void)height;
    return true; // Always succeed in test mode
}

void graphics_cleanup(void) {
    // No-op in test mode
}

#endif // TEST_MODE
