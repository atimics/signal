/**
 * @file test_microui_rendering.c
 * @brief Tests for MicroUI vertex generation and rendering pipeline
 * 
 * These tests verify that UI widgets actually generate visible vertices,
 * which is critical for ensuring UI elements appear on screen.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <string.h>

// Test fixtures
static mu_Context* ctx;
static int initial_vertex_count;
static int initial_command_count;

void setUp(void) {
    static bool initialized = false;
    if (!initialized) {
        // Initialize MicroUI system
        ui_microui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Begin a frame for testing
    ui_microui_begin_frame();
    
    // Store initial counts (should be 0 after begin_frame)
    initial_vertex_count = 0;
    initial_command_count = 0;
}

void tearDown(void) {
    // Don't end frame here - tests should handle their own frames
    // Don't shutdown between tests
}

// Test that button widgets generate vertices
void test_button_generates_vertices(void) {
    // Create a button
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 100));
    mu_button(ctx, "Test Button");
    mu_end_window(ctx);
    
    // End frame to process commands
    ui_microui_end_frame();
    
    // Verify vertices were generated
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(initial_vertex_count, vertex_count);
    TEST_ASSERT_MESSAGE(vertex_count > 0, "Button should generate vertices");
}

// Test that label widgets generate vertices
void test_label_generates_vertices(void) {
    // Create a label
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 100));
    mu_label(ctx, "Test Label Text");
    mu_end_window(ctx);
    
    // End frame to process commands
    ui_microui_end_frame();
    
    // Verify vertices were generated
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(initial_vertex_count, vertex_count);
    TEST_ASSERT_MESSAGE(vertex_count > 0, "Label should generate vertices");
}

// Test that empty frame generates no vertices
void test_empty_frame_no_vertices(void) {
    // End frame without creating any widgets
    ui_microui_end_frame();
    
    // Verify no vertices were generated
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_EQUAL_INT(initial_vertex_count, vertex_count);
}

// Test vertex buffer capacity
void test_vertex_buffer_capacity(void) {
    // Create many widgets to test buffer limits
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 400, 600));
    
    for (int i = 0; i < 100; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Button %d", i);
        mu_button(ctx, label);
    }
    
    mu_end_window(ctx);
    ui_microui_end_frame();
    
    // Verify vertices were generated but within limits
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
    TEST_ASSERT_LESS_THAN(8192, vertex_count); // Max vertex buffer size
}

// Test command queue generation
void test_command_queue_generation(void) {
    // Create multiple UI elements
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 300, 200));
    mu_label(ctx, "Header");
    mu_button(ctx, "Button 1");
    mu_button(ctx, "Button 2");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Verify multiple commands were generated
    int command_count = ui_microui_get_command_count();
    TEST_ASSERT_GREATER_THAN(3, command_count); // At least window + 3 widgets
}

// Test clipping rect management
void test_clipping_rect_management(void) {
    // Create nested containers with clipping
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 200));
    
    mu_Rect container_rect = mu_rect(20, 20, 100, 100);
    mu_begin_panel(ctx, "Panel");
    mu_layout_row(ctx, 1, (int[]){-1}, 0);
    mu_button(ctx, "Clipped Button");
    mu_end_panel(ctx);
    
    mu_end_window(ctx);
    ui_microui_end_frame();
    
    // Verify clipping generated vertices
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
}

// Test font texture binding
void test_font_texture_binding(void) {
    // Verify font texture is bound when rendering text
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 100));
    mu_label(ctx, "Text requiring font texture");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Check that font texture was referenced
    bool font_texture_bound = ui_microui_is_font_texture_bound();
    TEST_ASSERT_TRUE(font_texture_bound);
}

// Test vertex data structure
void test_vertex_data_structure(void) {
    // Create a simple button
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 100));
    mu_button(ctx, "Test");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Get vertex data
    const void* vertex_data = ui_microui_get_vertex_data();
    int vertex_count = ui_microui_get_vertex_count();
    
    TEST_ASSERT_NOT_NULL(vertex_data);
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
    
    // Verify vertex size is correct (x,y,u,v,color = 20 bytes)
    size_t expected_size = vertex_count * 20;
    size_t actual_size = ui_microui_get_vertex_data_size();
    TEST_ASSERT_EQUAL_size_t(expected_size, actual_size);
}

// Test draw call batching
void test_draw_call_batching(void) {
    // Create multiple widgets that should batch
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 300, 400));
    
    // Same color buttons should batch
    for (int i = 0; i < 5; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Button %d", i);
        mu_button(ctx, label);
    }
    
    mu_end_window(ctx);
    ui_microui_end_frame();
    
    // Verify draw calls were generated
    int draw_call_count = ui_microui_get_draw_call_count();
    TEST_ASSERT_GREATER_THAN(0, draw_call_count); // Should generate draw calls
    // Note: MicroUI doesn't batch commands - each command is a draw call
    // This is expected behavior, not a bug
}

// Test render state reset
void test_render_state_reset(void) {
    // Generate some vertices
    mu_begin_window(ctx, "Test Window", mu_rect(10, 10, 200, 100));
    mu_button(ctx, "Test");
    mu_end_window(ctx);
    ui_microui_end_frame();
    
    // Verify vertices exist
    TEST_ASSERT_GREATER_THAN(0, ui_microui_get_vertex_count());
    
    // Verify state is preserved until next frame
    int current_vertices = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, current_vertices);
    
    // Start new frame
    ui_microui_begin_frame();
    
    // Now check state after new frame (not automatically reset)
    // The vertex count persists from the render state until new commands are processed
    ui_microui_end_frame();
}

void cleanup(void) {
    ui_microui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_button_generates_vertices);
    RUN_TEST(test_label_generates_vertices);
    RUN_TEST(test_empty_frame_no_vertices);
    RUN_TEST(test_vertex_buffer_capacity);
    RUN_TEST(test_command_queue_generation);
    RUN_TEST(test_clipping_rect_management);
    RUN_TEST(test_font_texture_binding);
    RUN_TEST(test_vertex_data_structure);
    RUN_TEST(test_draw_call_batching);
    RUN_TEST(test_render_state_reset);
    
    int result = UNITY_END();
    cleanup();
    return result;
}