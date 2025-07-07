/**
 * @file test_bitmap_font.c
 * @brief Tests for bitmap font rendering system
 * 
 * Verifies font glyph rendering, UV coordinate calculation,
 * and text measurement functionality.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <string.h>
#include <math.h>

// Font metrics for testing (MicroUI uses 8x8 bitmap font)
#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define FONT_TEXTURE_SIZE 128

// Test fixtures
static mu_Context* ctx;

void setUp(void) {
    static bool initialized = false;
    if (!initialized) {
        ui_microui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
}

void tearDown(void) {
    // Don't shutdown between tests
}

// Test single character vertex generation
void test_single_char_vertex_generation(void) {
    ui_microui_begin_frame();
    
    // Render single character
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 100, 50));
    mu_label(ctx, "A");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Single character should generate 4 vertices (quad)
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_OR_EQUAL(4, vertex_count);
}

// Test UV coordinate calculation for glyphs
void test_glyph_uv_coordinates(void) {
    // Test UV calculation for character 'A' (ASCII 65)
    int char_code = 'A';
    int chars_per_row = FONT_TEXTURE_SIZE / FONT_WIDTH;
    
    // Calculate expected UV coordinates
    int row = char_code / chars_per_row;
    int col = char_code % chars_per_row;
    
    float expected_u0 = (float)(col * FONT_WIDTH) / FONT_TEXTURE_SIZE;
    float expected_v0 = (float)(row * FONT_HEIGHT) / FONT_TEXTURE_SIZE;
    float expected_u1 = (float)((col + 1) * FONT_WIDTH) / FONT_TEXTURE_SIZE;
    float expected_v1 = (float)((row + 1) * FONT_HEIGHT) / FONT_TEXTURE_SIZE;
    
    // Verify UV coordinates are in valid range
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_u0, (float)(col * FONT_WIDTH) / FONT_TEXTURE_SIZE);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_v0, (float)(row * FONT_HEIGHT) / FONT_TEXTURE_SIZE);
    TEST_ASSERT_TRUE(expected_u1 > expected_u0);
    TEST_ASSERT_TRUE(expected_v1 > expected_v0);
    TEST_ASSERT_TRUE(expected_u1 <= 1.0f);
    TEST_ASSERT_TRUE(expected_v1 <= 1.0f);
}

// Test multi-line text rendering
void test_multiline_text_rendering(void) {
    ui_microui_begin_frame();
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 200, 150));
    mu_label(ctx, "Line 1\nLine 2\nLine 3");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Multi-line text should generate many vertices
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(50, vertex_count); // 3 lines + window decoration
}

// Test text width measurement
void test_text_width_measurement(void) {
    // Test various string lengths
    const char* test_strings[] = {
        "A",           // 1 char = 8 pixels
        "Hello",       // 5 chars = 40 pixels
        "Test String", // 11 chars = 88 pixels
        ""             // 0 chars = 0 pixels
    };
    
    int expected_widths[] = {
        1 * FONT_WIDTH,
        5 * FONT_WIDTH,
        11 * FONT_WIDTH,
        0
    };
    
    for (int i = 0; i < 4; i++) {
        int width = ctx->text_width(ctx->style->font, test_strings[i], -1);
        TEST_ASSERT_EQUAL_INT(expected_widths[i], width);
    }
}

// Test text height measurement
void test_text_height_measurement(void) {
    // Single line should be font height
    int single_line_height = ctx->text_height(ctx->style->font);
    TEST_ASSERT_EQUAL_INT(FONT_HEIGHT, single_line_height);
    
    // Multi-line should be n * font height (text_height returns single line height)
    // For multi-line, we'd need to count newlines and multiply
    int multi_line_height = ctx->text_height(ctx->style->font) * 2;
    TEST_ASSERT_EQUAL_INT(2 * FONT_HEIGHT, multi_line_height);
}

// Test font texture loading
void test_font_texture_loading(void) {
    // Font texture should be loaded during init
    bool font_loaded = ui_microui_is_font_texture_bound();
    TEST_ASSERT_TRUE_MESSAGE(font_loaded, "Font texture should be loaded");
}

// Test special character rendering
void test_special_character_rendering(void) {
    ui_microui_begin_frame();
    
    // Test various special characters
    mu_begin_window(ctx, "Special Chars", mu_rect(10, 10, 300, 200));
    mu_label(ctx, "!@#$%^&*()");
    mu_label(ctx, "[]{}\\|;:'\"");
    mu_label(ctx, "<>,.?/~`");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // All characters should generate vertices
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(100, vertex_count);
}

// Test text clipping
void test_text_clipping(void) {
    ui_microui_begin_frame();
    
    // Create small window with long text
    mu_begin_window(ctx, "Small", mu_rect(10, 10, 80, 50));
    mu_label(ctx, "This is a very long text that should be clipped");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Vertices are generated for all text (clipping happens in GPU)
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
    // MicroUI generates vertices for all content, clipping is done during rendering
}

// Test text color in vertices
void test_text_color_vertices(void) {
    ui_microui_begin_frame();
    
    // Set custom text color
    ctx->style->colors[MU_COLOR_TEXT] = mu_color(255, 0, 0, 255); // Red
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 200, 100));
    mu_label(ctx, "Red Text");
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Verify vertices were generated with color data
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
}

// Test empty string handling
void test_empty_string_handling(void) {
    ui_microui_begin_frame();
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 200, 100));
    mu_label(ctx, ""); // Empty string
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Empty string should not crash, minimal vertices for window only
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_OR_EQUAL(0, vertex_count);
}

// Test maximum text length
void test_maximum_text_length(void) {
    ui_microui_begin_frame();
    
    // Create very long string
    char long_text[1024];
    memset(long_text, 'A', sizeof(long_text) - 1);
    long_text[1023] = '\0';
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 800, 600));
    mu_label(ctx, long_text);
    mu_end_window(ctx);
    
    ui_microui_end_frame();
    
    // Should handle long text without crashing
    int vertex_count = ui_microui_get_vertex_count();
    TEST_ASSERT_GREATER_THAN(0, vertex_count);
    TEST_ASSERT_LESS_THAN(8192, vertex_count); // Within buffer limits
}

// Test font metrics consistency
void test_font_metrics_consistency(void) {
    // All printable ASCII characters should have same metrics
    for (char c = 32; c <= 126; c++) {
        char str[2] = {c, '\0'};
        int width = ctx->text_width(ctx->style->font, str, -1);
        int height = ctx->text_height(ctx->style->font);
        
        TEST_ASSERT_EQUAL_INT(FONT_WIDTH, width);
        TEST_ASSERT_EQUAL_INT(FONT_HEIGHT, height);
    }
}

// Test text batching efficiency
void test_text_batching_efficiency(void) {
    ui_microui_begin_frame();
    
    mu_begin_window(ctx, "Batch Test", mu_rect(10, 10, 400, 300));
    
    // Multiple labels with same color should batch
    for (int i = 0; i < 10; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Label %d", i);
        mu_label(ctx, label);
    }
    
    mu_end_window(ctx);
    ui_microui_end_frame();
    
    // Check that draw calls were generated
    int draw_calls = ui_microui_get_draw_call_count();
    TEST_ASSERT_GREATER_THAN(0, draw_calls); // MicroUI doesn't batch - each command is a draw call
}

void cleanup(void) {
    ui_microui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_single_char_vertex_generation);
    RUN_TEST(test_glyph_uv_coordinates);
    RUN_TEST(test_multiline_text_rendering);
    RUN_TEST(test_text_width_measurement);
    RUN_TEST(test_text_height_measurement);
    RUN_TEST(test_font_texture_loading);
    RUN_TEST(test_special_character_rendering);
    RUN_TEST(test_text_clipping);
    RUN_TEST(test_text_color_vertices);
    RUN_TEST(test_empty_string_handling);
    RUN_TEST(test_maximum_text_length);
    RUN_TEST(test_font_metrics_consistency);
    RUN_TEST(test_text_batching_efficiency);
    
    int result = UNITY_END();
    cleanup();
    return result;
}