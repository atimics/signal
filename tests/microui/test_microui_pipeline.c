/**
 * @file test_microui_pipeline.c
 * @brief Tests for MicroUI command-to-vertex pipeline
 * 
 * These tests trace the flow from UI commands to vertex generation,
 * helping identify where the pipeline breaks.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <stdio.h>
#include <string.h>

static mu_Context* ctx;

// Helper to dump command details
static void dump_command(mu_Command* cmd) {
    switch (cmd->type) {
        case MU_COMMAND_RECT:
            printf("  RECT: pos(%d,%d) size(%dx%d) color(%d,%d,%d,%d)\n",
                   cmd->rect.rect.x, cmd->rect.rect.y,
                   cmd->rect.rect.w, cmd->rect.rect.h,
                   cmd->rect.color.r, cmd->rect.color.g,
                   cmd->rect.color.b, cmd->rect.color.a);
            break;
        case MU_COMMAND_TEXT:
            printf("  TEXT: '%s' at (%d,%d) color(%d,%d,%d,%d)\n",
                   cmd->text.str, cmd->text.pos.x, cmd->text.pos.y,
                   cmd->text.color.r, cmd->text.color.g,
                   cmd->text.color.b, cmd->text.color.a);
            break;
        case MU_COMMAND_CLIP:
            printf("  CLIP: rect(%d,%d,%dx%d)\n",
                   cmd->clip.rect.x, cmd->clip.rect.y,
                   cmd->clip.rect.w, cmd->clip.rect.h);
            break;
        case MU_COMMAND_ICON:
            printf("  ICON: id=%d at (%d,%d)\n",
                   cmd->icon.id, cmd->icon.rect.x, cmd->icon.rect.y);
            break;
        default:
            printf("  UNKNOWN: type=%d\n", cmd->type);
    }
}

void setUp(void) {
    static bool initialized = false;
    if (!initialized) {
        ui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
}

void tearDown(void) {
    // Don't shutdown between tests
}

// Test command generation for each widget type
void test_label_command_pipeline(void) {
    printf("\n🔧 Testing LABEL pipeline...\n");
    
    ui_begin_frame();
    
    // Clear commands
    ctx->command_list.idx = 0;
    
    // Create label
    mu_label(ctx, "Test Label");
    
    // Examine commands
    printf("Commands generated: %d\n", ctx->command_list.idx);
    
    mu_Command* cmd = NULL;
    int cmd_count = 0;
    while (mu_next_command(ctx, &cmd)) {
        printf("Command %d:\n", ++cmd_count);
        dump_command(cmd);
    }
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Final vertex count: %d\n", vertices);
    
    // Label should generate text command → vertices
    TEST_ASSERT_GREATER_THAN(0, cmd_count);
    if (vertices == 0) {
        printf("❌ Pipeline broken: Commands exist but no vertices\n");
    }
}

void test_button_command_pipeline(void) {
    printf("\n🔧 Testing BUTTON pipeline...\n");
    
    ui_begin_frame();
    ctx->command_list.idx = 0;
    
    // Create button with known position
    mu_layout_row(ctx, 1, (int[]){100}, 30);
    int result = mu_button(ctx, "Test Button");
    
    printf("Button result: %d\n", result);
    printf("Commands generated: %d\n", ctx->command_list.idx);
    
    mu_Command* cmd = NULL;
    int rect_cmds = 0, text_cmds = 0;
    while (mu_next_command(ctx, &cmd)) {
        dump_command(cmd);
        if (cmd->type == MU_COMMAND_RECT) rect_cmds++;
        if (cmd->type == MU_COMMAND_TEXT) text_cmds++;
    }
    
    ui_end_frame();
    
    printf("Summary: %d rect, %d text commands\n", rect_cmds, text_cmds);
    printf("Vertices: %d\n", ui_microui_get_vertex_count());
    
    // Button should have rect (background) + text
    TEST_ASSERT_GREATER_THAN(0, rect_cmds);
    TEST_ASSERT_GREATER_THAN(0, text_cmds);
}

void test_rect_direct_pipeline(void) {
    printf("\n🔧 Testing direct RECT pipeline...\n");
    
    ui_begin_frame();
    
    // Direct rect command
    mu_draw_rect(ctx, mu_rect(10, 10, 50, 50), mu_color(255, 0, 0, 255));
    
    // Should have generated commands (MicroUI may add internal commands)
    TEST_ASSERT_GREATER_THAN(0, ctx->command_list.idx);
    printf("Commands generated: %d\n", ctx->command_list.idx);
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Vertices from rect: %d (expected 6)\n", vertices);
    
    // Rect = 2 triangles = 6 vertices
    if (vertices != 6) {
        printf("❌ Rect vertex generation failed\n");
    }
}

void test_text_direct_pipeline(void) {
    printf("\n🔧 Testing direct TEXT pipeline...\n");
    
    ui_begin_frame();
    
    const char* text = "ABC";
    mu_draw_text(ctx, NULL, text, -1, mu_vec2(0, 0), mu_color(255, 255, 255, 255));
    
    TEST_ASSERT_GREATER_THAN(0, ctx->command_list.idx);
    printf("Commands generated: %d\n", ctx->command_list.idx);
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    int expected = strlen(text) * 6; // 6 vertices per char
    
    printf("Text: '%s' (%zu chars)\n", text, strlen(text));
    printf("Vertices: %d (expected %d)\n", vertices, expected);
    
    if (vertices != expected) {
        printf("❌ Text vertex generation failed\n");
    }
}

void test_window_container_pipeline(void) {
    printf("\n🔧 Testing WINDOW container pipeline...\n");
    
    ui_begin_frame();
    
    printf("Container stack before window: %d\n", ctx->container_stack.idx);
    
    int window_opened = mu_begin_window(ctx, "Test", mu_rect(0, 0, 200, 100));
    printf("Window opened: %d\n", window_opened);
    printf("Container stack after begin: %d\n", ctx->container_stack.idx);
    
    if (window_opened) {
        mu_label(ctx, "Inside window");
        mu_end_window(ctx);
    }
    
    printf("Container stack after end: %d\n", ctx->container_stack.idx);
    
    ui_end_frame();
    
    printf("Commands: %d\n", ui_microui_get_command_count());
    printf("Vertices: %d\n", ui_microui_get_vertex_count());
}

void test_layout_system_pipeline(void) {
    printf("\n🔧 Testing LAYOUT system pipeline...\n");
    
    ui_begin_frame();
    
    // Test layout row
    mu_layout_row(ctx, 2, (int[]){100, 100}, 30);
    
    // Get next layout positions
    mu_Rect r1 = mu_layout_next(ctx);
    mu_Rect r2 = mu_layout_next(ctx);
    
    printf("Layout slot 1: (%d,%d,%dx%d)\n", r1.x, r1.y, r1.w, r1.h);
    printf("Layout slot 2: (%d,%d,%dx%d)\n", r2.x, r2.y, r2.w, r2.h);
    
    // Draw in layout slots
    mu_draw_rect(ctx, r1, mu_color(255, 0, 0, 255));
    mu_draw_rect(ctx, r2, mu_color(0, 255, 0, 255));
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Vertices from 2 rects: %d (expected 12)\n", vertices);
}

void test_color_and_style_pipeline(void) {
    printf("\n🔧 Testing COLOR/STYLE pipeline...\n");
    
    ui_begin_frame();
    
    // Test different colors
    mu_draw_rect(ctx, mu_rect(0, 0, 10, 10), mu_color(255, 0, 0, 255));
    mu_draw_rect(ctx, mu_rect(20, 0, 10, 10), mu_color(0, 255, 0, 255));
    mu_draw_rect(ctx, mu_rect(40, 0, 10, 10), mu_color(0, 0, 255, 255));
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Vertices from 3 colored rects: %d\n", vertices);
    
    // Verify vertex data contains color info
    if (vertices > 0) {
        const void* vertex_data = ui_microui_get_vertex_data();
        TEST_ASSERT_NOT_NULL(vertex_data);
        printf("✓ Vertex data accessible\n");
    }
}

void test_clipping_pipeline(void) {
    printf("\n🔧 Testing CLIPPING pipeline...\n");
    
    ui_begin_frame();
    
    // Set clip rect
    mu_push_clip_rect(ctx, mu_rect(10, 10, 50, 50));
    
    // Draw rect partially outside clip
    mu_draw_rect(ctx, mu_rect(0, 0, 100, 100), mu_color(255, 0, 0, 255));
    
    mu_pop_clip_rect(ctx);
    
    ui_end_frame();
    
    int vertices = ui_microui_get_vertex_count();
    printf("Vertices with clipping: %d\n", vertices);
}

void test_empty_frame_pipeline(void) {
    printf("\n🔧 Testing EMPTY frame pipeline...\n");
    
    ui_begin_frame();
    // No content
    ui_end_frame();
    
    int commands = ui_microui_get_command_count();
    int vertices = ui_microui_get_vertex_count();
    
    printf("Empty frame - Commands: %d, Vertices: %d\n", commands, vertices);
    TEST_ASSERT_EQUAL_INT(0, commands);
    TEST_ASSERT_EQUAL_INT(0, vertices);
}

void test_complex_ui_pipeline(void) {
    printf("\n🔧 Testing COMPLEX UI pipeline...\n");
    
    ui_begin_frame();
    
    // Create complex UI
    if (mu_begin_window(ctx, "Complex", mu_rect(10, 10, 300, 200))) {
        mu_layout_row(ctx, 2, (int[]){100, -1}, 25);
        mu_label(ctx, "Name:");
        
        static char buf[128] = "Test";
        mu_textbox(ctx, buf, sizeof(buf));
        
        mu_layout_row(ctx, 3, (int[]){-1, -1, -1}, 25);
        if (mu_button(ctx, "OK")) printf("OK clicked\n");
        if (mu_button(ctx, "Cancel")) printf("Cancel clicked\n");
        if (mu_button(ctx, "Help")) printf("Help clicked\n");
        
        mu_end_window(ctx);
    }
    
    ui_end_frame();
    
    int commands = ui_microui_get_command_count();
    int vertices = ui_microui_get_vertex_count();
    
    printf("Complex UI - Commands: %d, Vertices: %d\n", commands, vertices);
    
    // Should have many commands
    TEST_ASSERT_GREATER_THAN(5, commands);
}

void cleanup(void) {
    ui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("\n🔧 MicroUI PIPELINE TEST SUITE 🔧\n");
    printf("===================================\n");
    printf("Tracing command-to-vertex pipeline\n\n");
    
    RUN_TEST(test_label_command_pipeline);
    RUN_TEST(test_button_command_pipeline);
    RUN_TEST(test_rect_direct_pipeline);
    RUN_TEST(test_text_direct_pipeline);
    RUN_TEST(test_window_container_pipeline);
    RUN_TEST(test_layout_system_pipeline);
    RUN_TEST(test_color_and_style_pipeline);
    RUN_TEST(test_clipping_pipeline);
    RUN_TEST(test_empty_frame_pipeline);
    RUN_TEST(test_complex_ui_pipeline);
    
    int result = UNITY_END();
    cleanup();
    return result;
}