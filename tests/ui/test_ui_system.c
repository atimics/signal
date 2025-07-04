/**
 * @file test_ui_system.c
 * @brief Unit tests for modular UI system
 * 
 * Tests the new modular UI architecture including:
 * - UI API functionality
 * - Scene UI module registration and management
 * - UI component widgets
 * - Scene transition requests
 */

#include "../vendor/unity.h"
#include "../../src/ui_api.h"
#include "../../src/ui_scene.h"
#include "../../src/ui_components.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test setup and teardown functions (required by Unity framework)
void setUp(void)
{
    // Clear any UI state before each test
    ui_clear_scene_change_request();
}

void tearDown(void)
{
    // Clean up after each test
    ui_clear_scene_change_request();
}

// ============================================================================
// UI API TESTS
// ============================================================================

void test_ui_api_scene_transitions_basic(void)
{
    // Clear any existing state
    ui_clear_scene_change_request();
    
    // Initially no scene change request
    TEST_ASSERT_FALSE(ui_has_scene_change_request());
    TEST_ASSERT_NULL(ui_get_requested_scene());
    
    // Request a scene change
    ui_request_scene_change("test_scene");
    
    // Should now have a pending request
    TEST_ASSERT_TRUE(ui_has_scene_change_request());
    TEST_ASSERT_EQUAL_STRING("test_scene", ui_get_requested_scene());
    
    // Clear the request
    ui_clear_scene_change_request();
    
    // Should be back to no request
    TEST_ASSERT_FALSE(ui_has_scene_change_request());
    TEST_ASSERT_NULL(ui_get_requested_scene());
}

void test_ui_api_scene_transitions_overwrite(void)
{
    ui_clear_scene_change_request();
    
    // Request first scene
    ui_request_scene_change("scene_one");
    TEST_ASSERT_EQUAL_STRING("scene_one", ui_get_requested_scene());
    
    // Request second scene (should overwrite)
    ui_request_scene_change("scene_two");
    TEST_ASSERT_EQUAL_STRING("scene_two", ui_get_requested_scene());
    
    ui_clear_scene_change_request();
}

void test_ui_api_debug_panel_control(void)
{
    // Test debug panel visibility control
    ui_set_debug_panel_visible(false);
    TEST_ASSERT_FALSE(ui_is_debug_panel_visible());
    
    ui_set_debug_panel_visible(true);
    TEST_ASSERT_TRUE(ui_is_debug_panel_visible());
    
    // Test toggle functionality
    ui_toggle_debug_panel();
    TEST_ASSERT_FALSE(ui_is_debug_panel_visible());
    
    ui_toggle_debug_panel();
    TEST_ASSERT_TRUE(ui_is_debug_panel_visible());
}

void test_ui_api_hud_control(void)
{
    // Test HUD visibility control
    ui_set_hud_visible(true);
    TEST_ASSERT_TRUE(ui_is_hud_visible());
    
    ui_set_hud_visible(false);
    TEST_ASSERT_FALSE(ui_is_hud_visible());
    
    // Test toggle functionality
    ui_toggle_hud();
    TEST_ASSERT_TRUE(ui_is_hud_visible());
    
    ui_toggle_hud();
    TEST_ASSERT_FALSE(ui_is_hud_visible());
}

void test_ui_api_wireframe_control(void)
{
    // Test wireframe mode control
    bool initial_state = ui_is_wireframe_enabled();
    
    ui_toggle_wireframe();
    TEST_ASSERT_NOT_EQUAL(initial_state, ui_is_wireframe_enabled());
    
    ui_toggle_wireframe();
    TEST_ASSERT_EQUAL(initial_state, ui_is_wireframe_enabled());
}

// ============================================================================
// UI SCENE SYSTEM TESTS
// ============================================================================

void test_scene_ui_initialization(void)
{
    // Initialize the scene UI system
    scene_ui_init();
    
    // System should be ready for module registration
    // (Cannot test internal state without exposing it, but init should not crash)
    
    // Shutdown
    scene_ui_shutdown();
}

void test_scene_ui_module_registration(void)
{
    scene_ui_init();
    
    // Create a test module
    SceneUIModule* test_module = malloc(sizeof(SceneUIModule));
    test_module->scene_name = "test_scene";
    test_module->render = NULL;
    test_module->handle_event = NULL;
    test_module->shutdown = NULL;
    test_module->data = NULL;
    
    // Register the module
    scene_ui_register(test_module);
    
    // Should be able to get the module back
    SceneUIModule* retrieved = scene_ui_get_module("test_scene");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_STRING("test_scene", retrieved->scene_name);
    
    // Unregister the module
    scene_ui_unregister("test_scene");
    
    // Should no longer be available
    retrieved = scene_ui_get_module("test_scene");
    TEST_ASSERT_NULL(retrieved);
    
    scene_ui_shutdown();
}

void test_scene_ui_module_null_handling(void)
{
    scene_ui_init();
    
    // Test null module registration
    scene_ui_register(NULL);  // Should not crash
    
    // Test getting non-existent module
    SceneUIModule* retrieved = scene_ui_get_module("nonexistent");
    TEST_ASSERT_NULL(retrieved);
    
    // Test unregistering non-existent module
    scene_ui_unregister("nonexistent");  // Should not crash
    
    scene_ui_shutdown();
}

// ============================================================================
// UI COMPONENTS TESTS
// ============================================================================

void test_scene_list_widget_initialization(void)
{
    SceneListWidget widget;
    
    // Initialize widget
    scene_list_widget_init(&widget);
    
    // Check initial state
    TEST_ASSERT_NULL(widget.scene_names);
    TEST_ASSERT_NULL(widget.scene_descriptions);
    TEST_ASSERT_EQUAL_INT(0, widget.scene_count);
    TEST_ASSERT_EQUAL_INT(-1, widget.selected_index);
    TEST_ASSERT_FALSE(widget.scenes_loaded);
    
    // Cleanup
    scene_list_widget_shutdown(&widget);
}

void test_config_widget_initialization(void)
{
    ConfigWidget widget;
    
    // Initialize widget (this will sync from actual config)
    config_widget_init(&widget);
    
    // Widget should have some sensible defaults
    TEST_ASSERT_NOT_NULL(widget.startup_scene);
    // auto_start can be true or false, both are valid
    
    // Test sync functionality
    config_widget_sync_from_config(&widget);
    // Should not crash and should maintain consistency
}

void test_performance_widget_initialization(void)
{
    PerformanceWidget widget;
    
    // Initialize widget
    performance_widget_init(&widget);
    
    // Check initial state
    TEST_ASSERT_EQUAL_FLOAT(0.0f, widget.fps);
    TEST_ASSERT_EQUAL_INT(0, widget.frame_count);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, widget.update_timer);
}

void test_performance_widget_update(void)
{
    PerformanceWidget widget;
    performance_widget_init(&widget);
    
    // Update with some time
    performance_widget_update(&widget, 0.016f);  // ~60 FPS frame time
    
    // Should have updated internal state
    TEST_ASSERT_EQUAL_INT(1, widget.frame_count);
    TEST_ASSERT_EQUAL_FLOAT(0.016f, widget.update_timer);
    
    // FPS should still be 0 until 1 second has passed
    TEST_ASSERT_EQUAL_FLOAT(0.0f, widget.fps);
    
    // Update for more than a full second
    // We already have 0.016f from the first update, so we need 61 more updates
    // to get 0.016f + (61 * 0.016f) = 0.016f + 0.976f = 0.992f (still < 1.0f)
    // So we need 62 more updates: 0.016f + (62 * 0.016f) = 0.016f + 0.992f = 1.008f (> 1.0f)
    for (int i = 0; i < 62; i++) {
        performance_widget_update(&widget, 0.016f);
    }
    
    // Now FPS should be calculated (approximately 60)
    // Total frames: 1 + 62 = 63, Total time: ~1.008f, FPS: 63/1.008 ≈ 62.5
    TEST_ASSERT_GREATER_THAN(50.0f, widget.fps);
    TEST_ASSERT_LESS_THAN(70.0f, widget.fps);
}

void test_entity_browser_widget_initialization(void)
{
    EntityBrowserWidget widget;
    
    // Initialize widget
    entity_browser_widget_init(&widget);
    
    // Check initial state
    TEST_ASSERT_EQUAL_INT(-1, widget.selected_entity);
    TEST_ASSERT_FALSE(widget.show_components);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_ui_system(void)
{
    printf("\n🧪 UI System Tests\n");
    printf("==================\n");
    
    printf("📡 Testing UI API...\n");
    RUN_TEST(test_ui_api_scene_transitions_basic);
    RUN_TEST(test_ui_api_scene_transitions_overwrite);
    RUN_TEST(test_ui_api_debug_panel_control);
    RUN_TEST(test_ui_api_hud_control);
    RUN_TEST(test_ui_api_wireframe_control);
    
    printf("🎨 Testing Scene UI System...\n");
    RUN_TEST(test_scene_ui_initialization);
    RUN_TEST(test_scene_ui_module_registration);
    RUN_TEST(test_scene_ui_module_null_handling);
    
    printf("🧩 Testing UI Components...\n");
    RUN_TEST(test_scene_list_widget_initialization);
    RUN_TEST(test_config_widget_initialization);
    RUN_TEST(test_performance_widget_initialization);
    RUN_TEST(test_performance_widget_update);
    RUN_TEST(test_entity_browser_widget_initialization);
    
    printf("✅ UI System Tests Complete\n");
}

int main(void)
{
    UNITY_BEGIN();
    
    // Run UI system test suite
    suite_ui_system();
    
    return UNITY_END();
}
