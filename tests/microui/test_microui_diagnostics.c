/**
 * @file test_microui_diagnostics.c
 * @brief Diagnostic tests for MicroUI vertex generation issue
 * 
 * These tests are designed to help diagnose why MicroUI generates 0 vertices
 * despite processing commands. They provide detailed logging and step-by-step
 * verification of the rendering pipeline.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <stdio.h>
#include <string.h>

static mu_Context* ctx;

void setUp(void) {
    static bool initialized = false;
    printf("\n=== TEST SETUP ===\n");
    if (!initialized) {
        ui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
    printf("✓ MicroUI context initialized\n");
}

void tearDown(void) {
    printf("=== TEST TEARDOWN ===\n");
    // Don't shutdown between tests
}

// Test 1: Verify basic MicroUI state after initialization
void test_microui_initial_state(void) {
    printf("\n🔍 Testing MicroUI initial state...\n");
    
    // Check context state
    TEST_ASSERT_NOT_NULL(ctx->text_width);
    TEST_ASSERT_NOT_NULL(ctx->text_height);
    printf("✓ Text callbacks set\n");
    
    // Check clip stack
    TEST_ASSERT_EQUAL_INT(0, ctx->clip_stack.idx);
    printf("✓ Clip stack empty (idx=%d)\n", ctx->clip_stack.idx);
    
    // Check command list
    TEST_ASSERT_EQUAL_INT(0, ctx->command_list.idx);
    printf("✓ Command list empty (idx=%d)\n", ctx->command_list.idx);
    
    // Check style
    TEST_ASSERT_NOT_NULL(ctx->style);
    printf("✓ Style configured\n");
}

// Test 2: Trace a single frame with minimal content
void test_minimal_frame_trace(void) {
    printf("\n🔍 Tracing minimal frame...\n");
    
    // Begin frame
    printf("1. Begin frame\n");
    ui_begin_frame();
    
    // Check clip stack after begin
    printf("   - Clip stack idx after begin: %d\n", ctx->clip_stack.idx);
    TEST_ASSERT_GREATER_THAN(0, ctx->clip_stack.idx);
    
    // Add minimal content
    printf("2. Adding label\n");
    mu_label(ctx, "Test");
    
    // Check command generation
    printf("   - Commands after label: %d\n", ctx->command_list.idx);
    
    // End frame
    printf("3. End frame\n");
    ui_end_frame();
    
    // Check results
    int vertex_count = ui_microui_get_vertex_count();
    int command_count = ui_microui_get_command_count();
    
    printf("📊 Results:\n");
    printf("   - Commands generated: %d\n", command_count);
    printf("   - Vertices generated: %d\n", vertex_count);
    
    // This test documents the current behavior (0 vertices)
    // When fixed, this should be > 0
    if (vertex_count == 0) {
        printf("❌ BUG CONFIRMED: No vertices generated despite commands\n");
    }
}

// Test 3: Window creation and command generation
void test_window_command_generation(void) {
    printf("\n🔍 Testing window command generation...\n");
    
    ui_begin_frame();
    
    // Track command count at each step
    int cmd_start = ctx->command_list.idx;
    printf("Commands at start: %d\n", cmd_start);
    
    // Try to create window
    int window_result = mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 100));
    printf("Window begin result: %d\n", window_result);
    
    if (window_result) {
        printf("✓ Window created successfully\n");
        
        // Add content
        mu_label(ctx, "Window content");
        int cmd_after_label = ctx->command_list.idx;
        printf("Commands after label: %d (added %d)\n", cmd_after_label, cmd_after_label - cmd_start);
        
        mu_end_window(ctx);
        int cmd_after_window = ctx->command_list.idx;
        printf("Commands after window: %d (added %d)\n", cmd_after_window, cmd_after_window - cmd_start);
    } else {
        printf("❌ Window creation failed\n");
    }
    
    ui_end_frame();
    
    // Analyze commands
    printf("\n📊 Command Analysis:\n");
    mu_Command* cmd = NULL;
    int rect_cmds = 0, text_cmds = 0, clip_cmds = 0;
    
    // Reset command iterator
    ctx->command_list.idx = 0;
    
    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_RECT:
                rect_cmds++;
                printf("  - RECT: (%d,%d,%d,%d)\n", 
                       cmd->rect.rect.x, cmd->rect.rect.y, 
                       cmd->rect.rect.w, cmd->rect.rect.h);
                break;
            case MU_COMMAND_TEXT:
                text_cmds++;
                printf("  - TEXT: '%s' at (%d,%d)\n", 
                       cmd->text.str, cmd->text.pos.x, cmd->text.pos.y);
                break;
            case MU_COMMAND_CLIP:
                clip_cmds++;
                printf("  - CLIP: (%d,%d,%d,%d)\n",
                       cmd->clip.rect.x, cmd->clip.rect.y,
                       cmd->clip.rect.w, cmd->clip.rect.h);
                break;
        }
    }
    
    printf("Total: %d rect, %d text, %d clip commands\n", rect_cmds, text_cmds, clip_cmds);
    
    int vertex_count = ui_microui_get_vertex_count();
    printf("Vertices generated: %d\n", vertex_count);
    
    // Expected: text commands should generate vertices (6 per character)
    if (text_cmds > 0 && vertex_count == 0) {
        printf("❌ BUG: Text commands present but no vertices generated\n");
    }
}

// Test 4: Direct vertex generation test
void test_direct_vertex_generation(void) {
    printf("\n🔍 Testing direct vertex generation...\n");
    
    ui_begin_frame();
    
    // Create a simple rectangle directly
    mu_draw_rect(ctx, mu_rect(0, 0, 100, 100), mu_color(255, 0, 0, 255));
    
    ui_end_frame();
    
    int vertex_count = ui_microui_get_vertex_count();
    printf("Vertices after direct rect: %d\n", vertex_count);
    
    // A rectangle should generate 6 vertices (2 triangles)
    if (vertex_count == 0) {
        printf("❌ BUG: Direct rect drawing generates no vertices\n");
    } else {
        printf("✓ Direct rect generated %d vertices\n", vertex_count);
    }
}

// Test 5: Clip stack management
void test_clip_stack_management(void) {
    printf("\n🔍 Testing clip stack management...\n");
    
    // Test 1: Check clip stack in begin/end cycle
    printf("Before begin_frame: clip_stack.idx = %d\n", ctx->clip_stack.idx);
    
    ui_begin_frame();
    printf("After begin_frame: clip_stack.idx = %d\n", ctx->clip_stack.idx);
    TEST_ASSERT_EQUAL_INT(1, ctx->clip_stack.idx);
    
    // Get current clip rect
    mu_Rect clip = mu_get_clip_rect(ctx);
    printf("Current clip rect: (%d,%d,%d,%d)\n", clip.x, clip.y, clip.w, clip.h);
    
    ui_end_frame();
    printf("After end_frame: clip_stack.idx = %d\n", ctx->clip_stack.idx);
    TEST_ASSERT_EQUAL_INT(0, ctx->clip_stack.idx);
}

// Test 6: Text rendering diagnostics
void test_text_rendering_diagnostics(void) {
    printf("\n🔍 Testing text rendering...\n");
    
    ui_begin_frame();
    
    // Get text dimensions
    const char* test_text = "Hello";
    int width = ctx->text_width(NULL, test_text, -1);
    int height = ctx->text_height(NULL);
    printf("Text '%s' dimensions: %dx%d\n", test_text, width, height);
    
    // Draw text at specific position
    mu_draw_text(ctx, NULL, test_text, -1, mu_vec2(10, 10), mu_color(255, 255, 255, 255));
    
    ui_end_frame();
    
    int vertex_count = ui_microui_get_vertex_count();
    int expected_vertices = strlen(test_text) * 6; // 6 vertices per character
    
    printf("Text length: %zu characters\n", strlen(test_text));
    printf("Expected vertices: %d\n", expected_vertices);
    printf("Actual vertices: %d\n", vertex_count);
    
    if (vertex_count != expected_vertices) {
        printf("❌ Text vertex generation mismatch\n");
    }
}

// Test 7: Button interaction and rendering
void test_button_rendering_detailed(void) {
    printf("\n🔍 Testing button rendering in detail...\n");
    
    ui_begin_frame();
    
    // Create a simple window with a button
    if (mu_begin_window(ctx, "Button Test", mu_rect(50, 50, 200, 100))) {
        printf("✓ Window opened\n");
        
        // Check hover state before
        mu_Id hover_before = ctx->hover;
        
        // Create button
        int clicked = mu_button(ctx, "Click Me");
        
        // Check hover state after
        mu_Id hover_after = ctx->hover;
        
        printf("Button clicked: %d\n", clicked);
        printf("Hover changed: %s\n", hover_before != hover_after ? "yes" : "no");
        
        mu_end_window(ctx);
    }
    
    ui_end_frame();
    
    // Detailed vertex analysis
    int vertex_count = ui_microui_get_vertex_count();
    const void* vertex_data = ui_microui_get_vertex_data();
    size_t vertex_size = ui_microui_get_vertex_data_size();
    
    printf("\n📊 Vertex Data Analysis:\n");
    printf("  - Vertex count: %d\n", vertex_count);
    printf("  - Vertex data ptr: %p\n", vertex_data);
    printf("  - Total size: %zu bytes\n", vertex_size);
    
    if (vertex_count > 0) {
        printf("✓ Button generated vertices\n");
    } else {
        printf("❌ Button generated no vertices\n");
    }
}

// Test 8: Multi-frame consistency
void test_multi_frame_consistency(void) {
    printf("\n🔍 Testing multi-frame consistency...\n");
    
    int vertex_counts[3] = {0};
    
    for (int frame = 0; frame < 3; frame++) {
        printf("\nFrame %d:\n", frame + 1);
        
        ui_begin_frame();
        
        // Same content each frame
        mu_label(ctx, "Consistent content");
        
        ui_end_frame();
        
        vertex_counts[frame] = ui_microui_get_vertex_count();
        printf("  Vertices: %d\n", vertex_counts[frame]);
    }
    
    // All frames should generate same vertex count
    TEST_ASSERT_EQUAL_INT(vertex_counts[0], vertex_counts[1]);
    TEST_ASSERT_EQUAL_INT(vertex_counts[1], vertex_counts[2]);
    
    if (vertex_counts[0] == 0) {
        printf("❌ Consistent bug: No vertices across all frames\n");
    }
}

// Test 9: Memory and state verification
void test_render_state_integrity(void) {
    printf("\n🔍 Testing render state integrity...\n");
    
    // Generate some content
    ui_begin_frame();
    mu_label(ctx, "Test");
    ui_end_frame();
    
    // Check memory usage
    size_t memory = ui_microui_get_memory_usage();
    printf("Memory usage: %zu bytes\n", memory);
    TEST_ASSERT_GREATER_THAN(0, memory);
    
    // Verify we can get vertex data even if count is 0
    const void* vertex_data = ui_microui_get_vertex_data();
    TEST_ASSERT_NOT_NULL(vertex_data);
}

// Test 10: Root container investigation
void test_root_container_state(void) {
    printf("\n🔍 Testing root container state...\n");
    
    ui_begin_frame();
    
    // Check container stack
    printf("Container stack before content: %d\n", ctx->container_stack.idx);
    
    // Add content without window
    mu_label(ctx, "Direct label");
    
    printf("Container stack after content: %d\n", ctx->container_stack.idx);
    
    ui_end_frame();
    
    int vertex_count = ui_microui_get_vertex_count();
    printf("Vertices from direct label: %d\n", vertex_count);
}

void cleanup(void) {
    ui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("\n🔬 MicroUI DIAGNOSTIC TEST SUITE 🔬\n");
    printf("=====================================\n");
    printf("Purpose: Diagnose zero vertex generation issue\n\n");
    
    RUN_TEST(test_microui_initial_state);
    RUN_TEST(test_minimal_frame_trace);
    RUN_TEST(test_window_command_generation);
    RUN_TEST(test_direct_vertex_generation);
    RUN_TEST(test_clip_stack_management);
    RUN_TEST(test_text_rendering_diagnostics);
    RUN_TEST(test_button_rendering_detailed);
    RUN_TEST(test_multi_frame_consistency);
    RUN_TEST(test_render_state_integrity);
    RUN_TEST(test_root_container_state);
    
    int result = UNITY_END();
    cleanup();
    return result;
}