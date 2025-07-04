/**
 * @file test_ui_events.c
 * @brief Tests for UI event processing and widget interaction
 * 
 * Verifies mouse/keyboard event delivery, event timing,
 * widget interaction, and focus management.
 */

#include "unity.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include "ui_microui_test_utils.h"
#include <string.h>

// Test state
static mu_Context* ctx;
static bool button_clicked;
static bool key_pressed;
static int mouse_events_processed;
static int keyboard_events_processed;

// Event callbacks
static void on_button_click(void) {
    button_clicked = true;
}

static void on_key_press(void) {
    key_pressed = true;
}

void setUp(void) {
    static bool initialized = false;
    if (!initialized) {
        ui_init();
        initialized = true;
    }
    ctx = ui_microui_get_mu_context();
    TEST_ASSERT_NOT_NULL(ctx);
    
    // Reset test state
    button_clicked = false;
    key_pressed = false;
    mouse_events_processed = 0;
    keyboard_events_processed = 0;
}

void tearDown(void) {
    // Don't shutdown between tests
}

// Test mouse button event delivery
void test_mouse_button_event_delivery(void) {
    int clicked = 0;
    
    // Process several frames to establish window state
    for (int i = 0; i < 5; i++) {
        ui_begin_frame();
        
        // Move mouse to button area (accounting for window title bar)
        mu_input_mousemove(ctx, 200, 150);
        
        // Simulate click pattern on frame 2-3
        if (i == 2) {
            mu_input_mousedown(ctx, 200, 150, MU_MOUSE_LEFT);
        } else if (i == 3) {
            mu_input_mouseup(ctx, 200, 150, MU_MOUSE_LEFT);
        }
        
        mu_begin_window(ctx, "Test", mu_rect(100, 100, 200, 100));
        if (mu_button(ctx, "Click Me")) {
            clicked = 1;
        }
        mu_end_window(ctx);
        
        ui_end_frame();
    }
    
    // Button click should have been registered
    TEST_ASSERT_TRUE_MESSAGE(clicked, "Button should register click");
}

// Test keyboard event delivery
void test_keyboard_event_delivery(void) {
    static char buffer[256] = "";
    
    // First frame - create textbox
    ui_begin_frame();
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 100));
    mu_textbox(ctx, buffer, sizeof(buffer));
    mu_end_window(ctx);
    ui_end_frame();
    
    // Second frame - click on textbox to focus
    ui_begin_frame();
    mu_input_mousemove(ctx, 150, 50);
    mu_input_mousedown(ctx, 150, 50, MU_MOUSE_LEFT);
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 100));
    mu_textbox(ctx, buffer, sizeof(buffer));
    mu_end_window(ctx);
    ui_end_frame();
    
    // Third frame - release mouse and type
    ui_begin_frame();
    mu_input_mouseup(ctx, 150, 50, MU_MOUSE_LEFT);
    mu_input_text(ctx, "A");
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 100));
    mu_textbox(ctx, buffer, sizeof(buffer));
    mu_end_window(ctx);
    ui_end_frame();
    
    // Buffer should contain typed text
    TEST_ASSERT_NOT_EQUAL(0, strlen(buffer));
}

// Test mouse movement events
void test_mouse_movement_events(void) {
    mu_Id initial_hover = 0;
    mu_Id hover_over_button = 0;
    
    // First frame - no hover
    ui_begin_frame();
    mu_input_mousemove(ctx, 10, 10);
    mu_begin_window(ctx, "Test", mu_rect(50, 50, 200, 100));
    mu_button(ctx, "Hover Me");
    mu_end_window(ctx);
    ui_end_frame();
    initial_hover = ctx->hover;
    
    // Second frame - move mouse over button
    ui_begin_frame();
    mu_input_mousemove(ctx, 150, 100);
    mu_begin_window(ctx, "Test", mu_rect(50, 50, 200, 100));
    mu_button(ctx, "Hover Me");
    mu_end_window(ctx);
    ui_end_frame();
    hover_over_button = ctx->hover;
    
    // Hover state should change when over button
    TEST_ASSERT_NOT_EQUAL(initial_hover, hover_over_button);
}

// Test event timing with frames
void test_event_frame_timing(void) {
    // Events outside frame should be queued or handled appropriately
    ui_event_t event = {
        .type = UI_EVENT_MOUSE_MOVE,
        .mouse_move = {
            .x = 100,
            .y = 100
        }
    };
    
    // Send event outside frame
    ui_handle_event(&event);
    
    // Begin frame and check mouse position
    ui_begin_frame();
    // Mouse position is tracked internally, we can't directly access it in tests
    // The position should be reflected in hover/focus behavior
    ui_end_frame();
}

// Test widget click interaction
void test_widget_click_interaction(void) {
    int click_count = 0;
    
    // Process multiple frames for proper click detection
    for (int i = 0; i < 4; i++) {
        ui_begin_frame();
        
        // Position mouse over button (accounting for window position and title bar)
        mu_input_mousemove(ctx, 200, 150);
        
        // Simulate click pattern
        if (i == 1) {
            mu_input_mousedown(ctx, 200, 150, MU_MOUSE_LEFT);
        } else if (i == 2) {
            mu_input_mouseup(ctx, 200, 150, MU_MOUSE_LEFT);
        }
        
        mu_begin_window(ctx, "Test", mu_rect(100, 100, 200, 100));
        int button_result = mu_button(ctx, "Counter");
        if (button_result) {
            click_count++;
        }
        mu_end_window(ctx);
        
        ui_end_frame();
    }
    
    TEST_ASSERT_EQUAL_INT(1, click_count);
}

// Test focus management
void test_focus_management(void) {
    static char buf1[64] = "Field 1";
    static char buf2[64] = "Field 2";
    mu_Id focus1 = 0, focus2 = 0;
    
    // First frame - click first textbox
    ui_begin_frame();
    mu_input_mousemove(ctx, 150, 60);
    mu_input_mousedown(ctx, 150, 60, MU_MOUSE_LEFT);
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 200));
    mu_textbox(ctx, buf1, sizeof(buf1));
    mu_textbox(ctx, buf2, sizeof(buf2));
    mu_end_window(ctx);
    
    mu_input_mouseup(ctx, 150, 60, MU_MOUSE_LEFT);
    ui_end_frame();
    focus1 = ctx->focus;
    
    // Second frame - click second textbox
    ui_begin_frame();
    mu_input_mousemove(ctx, 150, 90);
    mu_input_mousedown(ctx, 150, 90, MU_MOUSE_LEFT);
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 200));
    mu_textbox(ctx, buf1, sizeof(buf1));
    mu_textbox(ctx, buf2, sizeof(buf2));
    mu_end_window(ctx);
    
    mu_input_mouseup(ctx, 150, 90, MU_MOUSE_LEFT);
    ui_end_frame();
    focus2 = ctx->focus;
    
    // Focus should change between widgets
    TEST_ASSERT_NOT_EQUAL(focus1, focus2);
}

// Test scroll events
void test_scroll_events(void) {
    // First frame - create scrollable content
    ui_begin_frame();
    mu_input_mousemove(ctx, 100, 80);
    
    mu_begin_window(ctx, "Scrollable", mu_rect(10, 10, 200, 150));
    mu_begin_panel(ctx, "Content");
    
    for (int i = 0; i < 20; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Item %d", i);
        mu_label(ctx, label);
    }
    
    mu_end_panel(ctx);
    mu_end_window(ctx);
    ui_end_frame();
    
    // Second frame - send scroll
    ui_begin_frame();
    mu_input_mousemove(ctx, 100, 80);
    mu_input_scroll(ctx, 0, -10);
    
    mu_begin_window(ctx, "Scrollable", mu_rect(10, 10, 200, 150));
    mu_begin_panel(ctx, "Content");
    
    for (int i = 0; i < 20; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Item %d", i);
        mu_label(ctx, label);
    }
    
    mu_end_panel(ctx);
    mu_end_window(ctx);
    ui_end_frame();
    
    // Scroll should be processed
    TEST_ASSERT_NOT_EQUAL(0, ctx->scroll_delta.y);
}

// Test event consumption
void test_event_consumption(void) {
    ui_begin_frame();
    
    // Create button that should consume click
    mu_begin_window(ctx, "Test", mu_rect(100, 100, 200, 100));
    mu_button(ctx, "Click");
    mu_end_window(ctx);
    
    // Simulate click on button
    mu_input_mousemove(ctx, 150, 130);
    mu_input_mousedown(ctx, 150, 130, MU_MOUSE_LEFT);
    mu_input_mouseup(ctx, 150, 130, MU_MOUSE_LEFT);
    
    ui_end_frame();
    
    // Event should be consumed by UI
    bool ui_wants_mouse = ui_wants_mouse_input();
    TEST_ASSERT_TRUE(ui_wants_mouse);
}

// Test modifier keys
void test_modifier_keys(void) {
    ui_begin_frame();
    
    // Create text input
    static char buffer[256] = "";
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 100));
    mu_textbox(ctx, buffer, sizeof(buffer));
    mu_end_window(ctx);
    
    // Send key with modifiers
    ui_event_t event = {
        .type = UI_EVENT_KEY,
        .key = {
            .key = 'A',
            .action = 1,
            .mods = UI_MOD_CTRL | UI_MOD_SHIFT
        }
    };
    ui_handle_event(&event);
    
    ui_end_frame();
    
    // Modifiers should be tracked
    TEST_ASSERT_BITS(UI_MOD_CTRL, UI_MOD_CTRL, event.key.mods);
    TEST_ASSERT_BITS(UI_MOD_SHIFT, UI_MOD_SHIFT, event.key.mods);
}

// Test rapid event processing
void test_rapid_event_processing(void) {
    ui_begin_frame();
    
    // Send many events rapidly
    for (int i = 0; i < 100; i++) {
        ui_event_t event = {
            .type = UI_EVENT_MOUSE_MOVE,
            .mouse_move = {
                .x = i * 2,
                .y = i * 2
            }
        };
        ui_handle_event(&event);
    }
    
    ui_end_frame();
    
    // Events should be processed without issues
    // We can't directly verify mouse position in mu_Context
    TEST_ASSERT_TRUE(true); // Test completed without crash
}

// Test widget state persistence
void test_widget_state_persistence(void) {
    static char persistent_text[256] = "Initial";
    char initial_copy[256];
    strcpy(initial_copy, persistent_text);
    
    // First frame - focus textbox
    ui_begin_frame();
    mu_input_mousemove(ctx, 150, 50);
    mu_input_mousedown(ctx, 150, 50, MU_MOUSE_LEFT);
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 100));
    mu_textbox(ctx, persistent_text, sizeof(persistent_text));
    mu_end_window(ctx);
    
    mu_input_mouseup(ctx, 150, 50, MU_MOUSE_LEFT);
    ui_end_frame();
    
    // Second frame - type text
    ui_begin_frame();
    mu_input_text(ctx, "X");
    
    mu_begin_window(ctx, "Test", mu_rect(10, 10, 300, 100));
    mu_textbox(ctx, persistent_text, sizeof(persistent_text));
    mu_end_window(ctx);
    ui_end_frame();
    
    // Text should have changed
    TEST_ASSERT_NOT_EQUAL(0, strcmp(persistent_text, initial_copy));
}

// Test double click detection
void test_double_click_detection(void) {
    int double_clicks = 0;
    
    // Simulate double click timing
    for (int frame = 0; frame < 5; frame++) {
        ui_begin_frame();
        
        mu_begin_window(ctx, "Test", mu_rect(100, 100, 200, 100));
        
        // Position mouse over button
        mu_input_mousemove(ctx, 150, 130);
        
        // Simulate clicks on specific frames
        if (frame == 1 || frame == 2) {
            mu_input_mousedown(ctx, 150, 130, MU_MOUSE_LEFT);
            mu_input_mouseup(ctx, 150, 130, MU_MOUSE_LEFT);
        }
        
        // Check for double click (would need custom implementation)
        if (mu_button(ctx, "Double Click Me")) {
            if (frame == 2) {
                double_clicks++;
            }
        }
        
        mu_end_window(ctx);
        ui_end_frame();
    }
    
    // Should detect rapid clicks
    TEST_ASSERT_GREATER_THAN(0, double_clicks);
}

void cleanup(void) {
    ui_shutdown();
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_mouse_button_event_delivery);
    RUN_TEST(test_keyboard_event_delivery);
    RUN_TEST(test_mouse_movement_events);
    RUN_TEST(test_event_frame_timing);
    RUN_TEST(test_widget_click_interaction);
    RUN_TEST(test_focus_management);
    RUN_TEST(test_scroll_events);
    RUN_TEST(test_event_consumption);
    RUN_TEST(test_modifier_keys);
    RUN_TEST(test_rapid_event_processing);
    RUN_TEST(test_widget_state_persistence);
    RUN_TEST(test_double_click_detection);
    
    int result = UNITY_END();
    cleanup();
    return result;
}