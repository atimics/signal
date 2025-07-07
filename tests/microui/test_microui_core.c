/**
 * @file test_microui_core.c
 * @brief Core MicroUI functionality tests
 */

#include "../vendor/unity.h"
#include "../stubs/microui_test_stubs.h"
#include <stdio.h>
#include <string.h>

// MicroUI types from stubs
typedef struct { int x, y, w, h; } mu_Rect;
typedef struct { int r, g, b, a; } mu_Color;
typedef struct mu_Context mu_Context;

// External functions from stubs
extern mu_Context* mu_create_context(void);
extern void mu_destroy_context(mu_Context* ctx);
extern void mu_begin(mu_Context* ctx);
extern void mu_end(mu_Context* ctx);
extern int mu_begin_window(mu_Context* ctx, const char* title, mu_Rect rect);
extern int mu_begin_window_ex(mu_Context* ctx, const char* title, mu_Rect rect, int opt);
extern void mu_end_window(mu_Context* ctx);
extern int mu_button(mu_Context* ctx, const char* label);
extern void mu_text(mu_Context* ctx, const char* text);
extern mu_Rect mu_rect(int x, int y, int w, int h);

// Test setup and teardown
void setUp(void)
{
    microui_test_reset();
}

void tearDown(void)
{
    // Clean up after each test
}

// ============================================================================
// CONTEXT MANAGEMENT TESTS
// ============================================================================

void test_microui_context_initialization(void)
{
    mu_Context* ctx = mu_create_context();
    
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Verify initial state
    TEST_ASSERT_EQUAL(0, microui_test_get_command_count());
    TEST_ASSERT_EQUAL(0, microui_test_get_vertex_count());
    
    mu_destroy_context(ctx);
}

void test_microui_frame_lifecycle(void)
{
    mu_Context* ctx = mu_create_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Begin frame
    mu_begin(ctx);
    
    // Frame should be active
    // Note: In real implementation, we'd check ctx->frame or similar
    
    // End frame
    mu_end(ctx);
    
    // Verify frame completed
    // Command list should be reset for next frame
    
    mu_destroy_context(ctx);
}

// ============================================================================
// WIDGET TESTS
// ============================================================================

void test_microui_button_generates_commands(void)
{
    mu_Context* ctx = mu_create_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    mu_begin(ctx);
    
    // Create a window to contain the button
    if (mu_begin_window(ctx, "Test Window", mu_rect(0, 0, 200, 100))) {
        int initial_commands = microui_test_get_command_count();
        
        // Add a button
        mu_button(ctx, "Click Me");
        
        // Verify commands were generated
        TEST_ASSERT_GREATER_THAN(initial_commands, microui_test_get_command_count());
        
        mu_end_window(ctx);
    }
    
    mu_end(ctx);
    
    // Verify some vertices were generated
    TEST_ASSERT_GREATER_THAN(0, microui_test_get_vertex_count());
    
    mu_destroy_context(ctx);
}

void test_microui_text_generates_vertices(void)
{
    mu_Context* ctx = mu_create_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    mu_begin(ctx);
    
    if (mu_begin_window(ctx, "Text Test", mu_rect(0, 0, 300, 200))) {
        int initial_vertices = microui_test_get_vertex_count();
        
        // Add text
        mu_text(ctx, "Hello, MicroUI!");
        
        // Verify vertices were generated for text
        TEST_ASSERT_GREATER_THAN(initial_vertices, microui_test_get_vertex_count());
        
        mu_end_window(ctx);
    }
    
    mu_end(ctx);
    
    mu_destroy_context(ctx);
}

// ============================================================================
// COMMAND GENERATION TESTS
// ============================================================================

void test_microui_window_generates_commands(void)
{
    mu_Context* ctx = mu_create_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    mu_begin(ctx);
    
    int initial_commands = microui_test_get_command_count();
    int initial_windows = microui_test_get_window_count();
    
    // Create a window
    if (mu_begin_window(ctx, "Command Test", mu_rect(50, 50, 400, 300))) {
        mu_end_window(ctx);
    }
    
    // Verify window generated commands
    TEST_ASSERT_GREATER_THAN(initial_commands, microui_test_get_command_count());
    TEST_ASSERT_GREATER_THAN(initial_windows, microui_test_get_window_count());
    
    mu_end(ctx);
    
    mu_destroy_context(ctx);
}

void test_microui_empty_frame_generates_no_vertices(void)
{
    mu_Context* ctx = mu_create_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Empty frame
    mu_begin(ctx);
    mu_end(ctx);
    
    // No vertices should be generated
    TEST_ASSERT_EQUAL(0, microui_test_get_vertex_count());
    
    mu_destroy_context(ctx);
}

// ============================================================================
// INTERACTION TESTS
// ============================================================================

void test_microui_button_click_detection(void)
{
    mu_Context* ctx = mu_create_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    int initial_clicks = microui_test_get_button_clicks();
    
    // Simulate multiple frames (our stub clicks every 3rd frame)
    for (int i = 0; i < 5; i++) {
        mu_begin(ctx);
        
        if (mu_begin_window(ctx, "Click Test", mu_rect(0, 0, 200, 100))) {
            if (mu_button(ctx, "Test Button")) {
                // Button was clicked
            }
            mu_end_window(ctx);
        }
        
        mu_end(ctx);
    }
    
    // Verify button clicks were detected
    TEST_ASSERT_GREATER_THAN(initial_clicks, microui_test_get_button_clicks());
    
    mu_destroy_context(ctx);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    
    // Context tests
    RUN_TEST(test_microui_context_initialization);
    RUN_TEST(test_microui_frame_lifecycle);
    
    // Widget tests
    RUN_TEST(test_microui_button_generates_commands);
    RUN_TEST(test_microui_text_generates_vertices);
    
    // Command generation tests
    RUN_TEST(test_microui_window_generates_commands);
    RUN_TEST(test_microui_empty_frame_generates_no_vertices);
    
    // Interaction tests
    RUN_TEST(test_microui_button_click_detection);
    
    return UNITY_END();
}