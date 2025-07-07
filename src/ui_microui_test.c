/**
 * @file ui_microui_test.c
 * @brief Simple test to verify MicroUI functionality
 */

#include "ui_microui.h"
#include <stdio.h>

void test_microui_basic(void) {
    printf("🧪 Testing basic MicroUI functionality...\n");
    
    // Initialize
    ui_microui_init();
    
    // Test basic frame operations
    mu_Context* ctx = ui_microui_begin_frame();
    
    // Try to create a simple window
    if (mu_begin_window(ctx, "Test", mu_rect(10, 10, 200, 100))) {
        mu_label(ctx, "Hello MicroUI!");
        mu_end_window(ctx);
    }
    
    ui_microui_end_frame();
    
    // Cleanup
    ui_microui_shutdown();
    
    printf("✅ MicroUI test completed successfully\n");
}
