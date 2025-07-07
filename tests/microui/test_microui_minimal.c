/**
 * @file test_microui_minimal.c
 * @brief Minimal reproduction tests for MicroUI vertex generation
 * 
 * These tests create the simplest possible UI to isolate the vertex
 * generation issue.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <stdio.h>

static mu_Context* ctx;

void setUp(void) {
    static bool initialized = false;
    if (!initialized) {
        ui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Clear any state from previous test
    ui_begin_frame();
}

void tearDown(void) {
    // End frame if not already ended
    ui_end_frame();
    // Don't shutdown - we'll reuse the context
}

// Test 1: Absolute minimal - just begin/end
void test_absolute_minimal(void) {
    printf("\n📌 Test: Absolute minimal frame\n");
    
    // Already have begin_frame from setUp
    // Just end it
    ui_end_frame();
    
    printf("Result: %d vertices\n", ui_microui_get_vertex_count());
}

// Test 2: Single rect
void test_single_rect(void) {
    printf("\n📌 Test: Single rectangle\n");
    
    // Draw one rect at origin
    mu_draw_rect(ctx, mu_rect(0, 0, 10, 10), mu_color(255, 255, 255, 255));
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Result: %d vertices (expected 6)\n", vertices);
    
    if (vertices != 6) {
        printf("❌ FAIL: Rectangle should generate exactly 6 vertices\n");
    } else {
        printf("✅ PASS: Rectangle generated correct vertices\n");
    }
}

// Test 3: Single character
void test_single_character(void) {
    printf("\n📌 Test: Single character\n");
    
    // Draw one character
    mu_draw_text(ctx, NULL, "A", -1, mu_vec2(0, 0), mu_color(255, 255, 255, 255));
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Result: %d vertices (expected 6)\n", vertices);
    
    if (vertices != 6) {
        printf("❌ FAIL: Single character should generate exactly 6 vertices\n");
    } else {
        printf("✅ PASS: Character generated correct vertices\n");
    }
}

// Test 4: Check render function is called
void test_render_function_called(void) {
    printf("\n📌 Test: Render function execution\n");
    
    mu_draw_rect(ctx, mu_rect(0, 0, 1, 1), mu_color(255, 255, 255, 255));
    ui_end_frame();
    
    // Force render
    ui_render();
    
    int vertices = ui_microui_get_vertex_count();
    printf("After render: %d vertices\n", vertices);
}

// Test 5: Multiple frames
void test_multiple_frames(void) {
    printf("\n📌 Test: Multiple frames\n");
    
    // First frame already started in setUp
    mu_draw_rect(ctx, mu_rect(0, 0, 1, 1), mu_color(255, 255, 255, 255));
    ui_end_frame();
    printf("Frame 1: %d vertices\n", ui_microui_get_vertex_count());
    
    for (int i = 1; i < 3; i++) {
        ui_begin_frame();
        mu_draw_rect(ctx, mu_rect(0, 0, 1, 1), mu_color(255, 255, 255, 255));
        ui_end_frame();
        
        printf("Frame %d: %d vertices\n", i + 1, ui_microui_get_vertex_count());
    }
}

// Test 6: Verify command exists before vertex generation
void test_command_to_vertex_link(void) {
    printf("\n📌 Test: Command to vertex link\n");
    
    // Check commands before
    int cmds_before = ctx->command_list.idx;
    printf("Commands before: %d\n", cmds_before);
    
    // Add rect
    mu_draw_rect(ctx, mu_rect(0, 0, 1, 1), mu_color(255, 255, 255, 255));
    
    // Check commands after
    int cmds_after = ctx->command_list.idx;
    printf("Commands after: %d\n", cmds_after);
    printf("Commands added: %d\n", cmds_after - cmds_before);
    
    ui_end_frame();
    
    int final_commands = ui_microui_get_command_count();
    int vertices = ui_microui_get_vertex_count();
    
    printf("Final: %d commands, %d vertices\n", final_commands, vertices);
    
    if (final_commands > 0 && vertices == 0) {
        printf("❌ BUG CONFIRMED: Commands exist but no vertices generated!\n");
        
        // Dump command details
        printf("\nCommand details:\n");
        ctx->command_list.idx = 0;
        mu_Command* cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
            if (cmd->type == MU_COMMAND_RECT) {
                printf("  RECT at (%d,%d) size %dx%d\n",
                       cmd->rect.rect.x, cmd->rect.rect.y,
                       cmd->rect.rect.w, cmd->rect.rect.h);
            }
        }
    }
}

// Test 7: Direct vertex buffer access
void test_vertex_buffer_access(void) {
    printf("\n📌 Test: Vertex buffer access\n");
    
    mu_draw_rect(ctx, mu_rect(0, 0, 10, 10), mu_color(255, 0, 0, 255));
    ui_end_frame();
    
    const void* vdata = ui_microui_get_vertex_data();
    size_t vsize = ui_microui_get_vertex_data_size();
    
    printf("Vertex data ptr: %p\n", vdata);
    printf("Vertex data size: %zu bytes\n", vsize);
    
    TEST_ASSERT_NOT_NULL(vdata);
}

// Test 8: Manual command processing
void test_manual_command_processing(void) {
    printf("\n📌 Test: Manual command processing\n");
    
    // Add a rect command
    mu_draw_rect(ctx, mu_rect(5, 5, 20, 20), mu_color(128, 128, 128, 255));
    
    // Before end_frame, manually check commands
    printf("Commands in buffer: %d\n", ctx->command_list.idx);
    
    ui_end_frame();
    
    // After end_frame, check what happened
    int vertices = ui_microui_get_vertex_count();
    printf("Vertices after processing: %d\n", vertices);
    
    // Try to trace the issue
    if (vertices == 0) {
        printf("\n🔍 Investigating why no vertices...\n");
        
        // Check if commands still exist
        mu_Command* cmd = NULL;
        int cmd_count = 0;
        ctx->command_list.idx = 0;
        while (mu_next_command(ctx, &cmd)) {
            cmd_count++;
        }
        printf("  - Commands still in buffer: %d\n", cmd_count);
        
        // Check render state
        int render_cmds = ui_microui_get_rendered_command_count();
        printf("  - Rendered command count: %d\n", render_cmds);
    }
}

// Test 9: Simple label (most basic text widget)
void test_simple_label(void) {
    printf("\n📌 Test: Simple label\n");
    
    // Just a label, no window
    mu_label(ctx, "Hi");
    
    ui_end_frame();
    
    int commands = ui_microui_get_command_count();
    int vertices = ui_microui_get_vertex_count();
    
    printf("Label result: %d commands, %d vertices\n", commands, vertices);
    printf("Expected: 1 text command, 12 vertices (2 chars × 6)\n");
}

// Test 10: Trace exact vertex generation moment
void test_vertex_generation_trace(void) {
    printf("\n📌 Test: Vertex generation trace\n");
    
    printf("1. Start of test (after setUp begin_frame):\n");
    printf("   Vertices: %d\n", ui_microui_get_vertex_count());
    printf("   Commands: %d\n", ctx->command_list.idx);
    
    mu_draw_rect(ctx, mu_rect(0, 0, 5, 5), mu_color(255, 255, 255, 255));
    printf("3. After draw_rect:\n");
    printf("   Vertices: %d\n", ui_microui_get_vertex_count());
    printf("   Commands: %d\n", ctx->command_list.idx);
    
    ui_end_frame();
    printf("4. After end_frame:\n");
    printf("   Vertices: %d\n", ui_microui_get_vertex_count());
    printf("   Rendered commands: %d\n", ui_microui_get_rendered_command_count());
}

// Cleanup function to be called after all tests
void cleanup(void) {
    ui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("\n🔬 MicroUI MINIMAL REPRODUCTION TESTS 🔬\n");
    printf("========================================\n");
    printf("Isolating vertex generation issue\n\n");
    
    RUN_TEST(test_absolute_minimal);
    RUN_TEST(test_single_rect);
    RUN_TEST(test_single_character);
    RUN_TEST(test_render_function_called);
    RUN_TEST(test_multiple_frames);
    RUN_TEST(test_command_to_vertex_link);
    RUN_TEST(test_vertex_buffer_access);
    RUN_TEST(test_manual_command_processing);
    RUN_TEST(test_simple_label);
    RUN_TEST(test_vertex_generation_trace);
    
    int result = UNITY_END();
    cleanup();
    return result;
}