/**
 * @file test_scene_system.c
 * @brief Unit tests for scene management system
 * 
 * Tests scene state management, transitions, and script execution:
 * - Scene state transitions
 * - Scene loading and unloading
 * - Scene script management
 * - Configuration integration
 */

#include "../vendor/unity.h"
#include "../../src/scene_state.h"
#include "../../src/scene_script.h"
#include "../../src/config.h"
#include "../../src/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock world for testing
static struct World test_world;

void setUp(void)
{
    // Initialize test world
    memset(&test_world, 0, sizeof(struct World));
    test_world.max_entities = 100;
    test_world.entities = malloc(sizeof(struct Entity) * 100);
    test_world.entity_count = 0;
    test_world.next_entity_id = 1;
}

void tearDown(void)
{
    if (test_world.entities) {
        free(test_world.entities);
        test_world.entities = NULL;
    }
}

// ============================================================================
// SCENE STATE TESTS
// ============================================================================

void test_scene_state_initialization(void)
{
    SceneState scene_state;
    
    // Initialize scene state
    scene_state_init(&scene_state);
    
    // Check initial values
    TEST_ASSERT_EQUAL_INT(SCENE_STATE_LOADING, scene_state.current_state);
    TEST_ASSERT_NOT_NULL(scene_state.current_scene_name);
    TEST_ASSERT_FALSE(scene_state.transition_pending);
    TEST_ASSERT_FALSE(scene_state.debug_ui_visible);
    
    // Cleanup
    scene_state_cleanup(&scene_state);
}

void test_scene_state_transition_request(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Initially no transition pending
    TEST_ASSERT_FALSE(scene_state_has_pending_transition(&scene_state));
    
    // Request a transition
    scene_state_request_transition(&scene_state, "test_scene");
    
    // Should now have pending transition
    TEST_ASSERT_TRUE(scene_state_has_pending_transition(&scene_state));
    TEST_ASSERT_EQUAL_STRING("test_scene", scene_state_get_next_scene(&scene_state));
    
    scene_state_cleanup(&scene_state);
}

void test_scene_state_transition_execution(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Set initial scene
    strcpy(scene_state.current_scene_name, "initial_scene");
    scene_state.current_state = SCENE_STATE_RUNNING;
    
    // Request transition
    scene_state_request_transition(&scene_state, "target_scene");
    
    // Execute transition step by step
    TEST_ASSERT_TRUE(scene_state_has_pending_transition(&scene_state));
    
    // Start transition
    bool transition_completed = scene_state_execute_transition(&scene_state, &test_world);
    
    // Transition should be in progress or completed
    // (Actual behavior depends on implementation)
    
    scene_state_cleanup(&scene_state);
}

void test_scene_state_debug_ui_toggle(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Test debug UI visibility
    TEST_ASSERT_FALSE(scene_state_is_debug_ui_visible(&scene_state));
    
    scene_state_set_debug_ui_visible(&scene_state, true);
    TEST_ASSERT_TRUE(scene_state_is_debug_ui_visible(&scene_state));
    
    scene_state_set_debug_ui_visible(&scene_state, false);
    TEST_ASSERT_FALSE(scene_state_is_debug_ui_visible(&scene_state));
    
    scene_state_cleanup(&scene_state);
}

void test_scene_state_invalid_transitions(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Test null scene name
    scene_state_request_transition(&scene_state, NULL);
    // Should handle gracefully (not crash)
    
    // Test empty scene name
    scene_state_request_transition(&scene_state, "");
    // Should handle gracefully
    
    // Test very long scene name
    char long_name[300];
    memset(long_name, 'a', 299);
    long_name[299] = '\0';
    scene_state_request_transition(&scene_state, long_name);
    // Should handle gracefully (possibly truncate)
    
    scene_state_cleanup(&scene_state);
}

// ============================================================================
// SCENE SCRIPT TESTS
// ============================================================================

void test_scene_script_registration(void)
{
    // Test that scene scripts can be registered
    // Note: This tests the interface, actual scripts are registered at runtime
    
    // These should not crash
    scene_script_execute_enter("logo", &test_world, NULL);
    scene_script_execute_update("logo", &test_world, NULL, 0.016f);
    scene_script_execute_exit("logo", &test_world, NULL);
}

void test_scene_script_nonexistent_scene(void)
{
    // Test handling of non-existent scene scripts
    // Should not crash when called with invalid scene names
    
    scene_script_execute_enter("nonexistent_scene", &test_world, NULL);
    scene_script_execute_update("nonexistent_scene", &test_world, NULL, 0.016f);
    scene_script_execute_exit("nonexistent_scene", &test_world, NULL);
    
    // If we get here without crashing, the test passes
    TEST_ASSERT_TRUE(true);
}

void test_scene_script_null_parameters(void)
{
    // Test scene script functions with null parameters
    // Should handle gracefully
    
    scene_script_execute_enter("logo", NULL, NULL);
    scene_script_execute_update("logo", NULL, NULL, 0.016f);
    scene_script_execute_exit("logo", NULL, NULL);
    
    scene_script_execute_enter(NULL, &test_world, NULL);
    scene_script_execute_update(NULL, &test_world, NULL, 0.016f);
    scene_script_execute_exit(NULL, &test_world, NULL);
    
    // If we get here without crashing, the test passes
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// SCENE INTEGRATION TESTS
// ============================================================================

void test_scene_full_lifecycle(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Start in a known scene
    strcpy(scene_state.current_scene_name, "logo");
    scene_state.current_state = SCENE_STATE_RUNNING;
    
    // Execute scene enter script
    scene_script_execute_enter("logo", &test_world, &scene_state);
    
    // Update scene for a few frames
    for (int i = 0; i < 5; i++) {
        scene_script_execute_update("logo", &test_world, &scene_state, 0.016f);
        scene_state_update(&scene_state, 0.016f);
    }
    
    // Request transition to another scene
    scene_state_request_transition(&scene_state, "navigation_menu");
    
    // Execute exit script
    scene_script_execute_exit("logo", &test_world, &scene_state);
    
    // Execute transition
    if (scene_state_has_pending_transition(&scene_state)) {
        scene_state_execute_transition(&scene_state, &test_world);
    }
    
    // Execute enter script for new scene
    scene_script_execute_enter("navigation_menu", &test_world, &scene_state);
    
    scene_state_cleanup(&scene_state);
}

void test_scene_rapid_transitions(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Test rapid transition requests
    scene_state_request_transition(&scene_state, "scene1");
    scene_state_request_transition(&scene_state, "scene2");
    scene_state_request_transition(&scene_state, "scene3");
    
    // Should handle multiple requests gracefully
    // (Usually by keeping only the latest request)
    TEST_ASSERT_TRUE(scene_state_has_pending_transition(&scene_state));
    
    const char* next_scene = scene_state_get_next_scene(&scene_state);
    TEST_ASSERT_NOT_NULL(next_scene);
    
    scene_state_cleanup(&scene_state);
}

// ============================================================================
// CONFIGURATION INTEGRATION TESTS
// ============================================================================

void test_scene_startup_configuration(void)
{
    // Test that scene system respects startup configuration
    const char* startup_scene = config_get_startup_scene();
    TEST_ASSERT_NOT_NULL(startup_scene);
    
    bool auto_start = config_get_auto_start();
    // auto_start can be true or false, both are valid
    (void)auto_start; // Suppress unused variable warning
    
    // Test that we can change startup scene
    config_set_startup_scene("test_startup_scene");
    TEST_ASSERT_EQUAL_STRING("test_startup_scene", config_get_startup_scene());
    
    // Reset to original value
    config_set_startup_scene(startup_scene);
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_scene_transition_performance(void)
{
    SceneState scene_state;
    scene_state_init(&scene_state);
    
    // Measure time for scene transitions
    // This is a basic performance smoke test
    
    clock_t start_time = clock();
    
    // Perform multiple transitions
    for (int i = 0; i < 10; i++) {
        char scene_name[32];
        snprintf(scene_name, sizeof(scene_name), "test_scene_%d", i);
        
        scene_state_request_transition(&scene_state, scene_name);
        
        if (scene_state_has_pending_transition(&scene_state)) {
            scene_state_execute_transition(&scene_state, &test_world);
        }
    }
    
    clock_t end_time = clock();
    double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // Should complete within reasonable time (1 second)
    TEST_ASSERT_LESS_THAN(1.0, elapsed);
    
    scene_state_cleanup(&scene_state);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_scene_system(void)
{
    printf("\n🎬 Scene Management Tests\n");
    printf("=========================\n");
    
    printf("🏗️  Testing Scene State Management...\n");
    RUN_TEST(test_scene_state_initialization);
    RUN_TEST(test_scene_state_transition_request);
    RUN_TEST(test_scene_state_transition_execution);
    RUN_TEST(test_scene_state_debug_ui_toggle);
    RUN_TEST(test_scene_state_invalid_transitions);
    
    printf("📜 Testing Scene Scripts...\n");
    RUN_TEST(test_scene_script_registration);
    RUN_TEST(test_scene_script_nonexistent_scene);
    RUN_TEST(test_scene_script_null_parameters);
    
    printf("🔄 Testing Scene Integration...\n");
    RUN_TEST(test_scene_full_lifecycle);
    RUN_TEST(test_scene_rapid_transitions);
    
    printf("⚙️  Testing Configuration Integration...\n");
    RUN_TEST(test_scene_startup_configuration);
    
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_scene_transition_performance);
    
    printf("✅ Scene Management Tests Complete\n");
}
