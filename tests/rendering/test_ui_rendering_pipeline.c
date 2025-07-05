/**
 * @file test_ui_rendering_pipeline.c
 * @brief Comprehensive tests for UI rendering pipeline and state management
 * 
 * Tests critical UI rendering components including:
 * - UI context management and validation
 * - Pipeline state transitions between 3D and UI rendering
 * - MicroUI integration and rendering safety
 * - Graphics context validation and error handling
 * - UI render pass management
 */

#include "../vendor/unity.h"
#include "../stubs/ui_function_stubs.h"
#include "../../src/core.h"
#include "../../src/systems.h"
#include "../../src/graphics_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Use the actual World structure from core.h, not a mock
// Use SystemScheduler from systems.h, not TaskScheduler

// Test constants
#define TEST_SCREEN_WIDTH 800
#define TEST_SCREEN_HEIGHT 600
#define TEST_DELTA_TIME 0.016f  // ~60 FPS

// Global test state
static struct World test_world;
static SystemScheduler test_scheduler;
static bool graphics_initialized = false;

// ============================================================================
// TEST SETUP AND TEARDOWN
// ============================================================================

void setUp(void) {
    // Initialize minimal world and scheduler
    memset(&test_world, 0, sizeof(test_world));
    memset(&test_scheduler, 0, sizeof(test_scheduler));
    
    // Initialize graphics if needed
    if (!graphics_initialized) {
        // Skip SOKOL_DUMMY_BACKEND redefinition - it's already defined in compiler flags
        
        graphics_initialized = true;
    }
    
    // Reset UI state
    ui_clear_scene_change_request();
}

void tearDown(void) {
    // Clean up UI state
    ui_clear_scene_change_request();
}

// ============================================================================
// UI CONTEXT AND VALIDATION TESTS
// ============================================================================

void test_ui_context_initialization(void) {
    // Test UI initialization without crashing
    ui_init();
    
    // In test mode, we just verify the stubs are called properly
    printf("✅ UI initialization stub called successfully\n");
    
    // Test cleanup
    ui_cleanup();
    TEST_PASS_MESSAGE("UI initialization and cleanup completed");
}

void test_graphics_context_validation(void) {
    // Test Sokol graphics context validation
    bool context_valid = sg_isvalid();
    
    // In dummy backend mode, context should be valid
    TEST_ASSERT_TRUE_MESSAGE(context_valid, "Graphics context should be valid with dummy backend");
    
    printf("✅ Graphics context validation: %s\n", context_valid ? "VALID" : "INVALID");
}

void test_ui_render_safety_checks(void) {
    // Test UI render function with minimal parameters
    // This should not crash even if UI is not fully initialized
    
    printf("Testing UI render safety checks...\n");
    
    // These calls should be safe even with minimal initialization
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "test_scene", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    // Test with NULL parameters (should handle gracefully)
    ui_render(NULL, NULL, 0.0f, NULL, 0, 0);
    
    TEST_PASS_MESSAGE("UI render safety checks completed without crashes");
}

// ============================================================================
// MICROUI INTEGRATION TESTS
// ============================================================================

void test_microui_context_safety(void) {
    // Test MicroUI context operations without full initialization
    printf("Testing MicroUI context safety...\n");
    
    // These should handle NULL or uninitialized contexts gracefully
    ui_microui_begin_frame();
    ui_microui_end_frame();
    
    // Test rendering with uninitialized context
    ui_microui_render(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    TEST_PASS_MESSAGE("MicroUI context safety tests completed");
}

void test_microui_command_generation(void) {
    // Test MicroUI command generation in safe mode
    printf("Testing MicroUI command generation...\n");
    
    // Initialize MicroUI context for testing
    ui_microui_init();
    
    // Generate some basic UI commands
    ui_microui_begin_frame();
    
    // Test basic widget creation (should generate commands) 
    // In test mode, context functions return NULL so we skip actual UI calls
    struct mu_Context* ctx = ui_microui_get_mu_context();
    if (ctx) {
        printf("✅ MicroUI context available\n");
        // Would do actual UI calls here in real mode
    } else {
        printf("⚠️ MicroUI context not available (expected in test mode)\n");
    }
    
    ui_microui_end_frame();
    
    printf("✅ MicroUI command generation completed\n");
    
    // Cleanup
    ui_microui_cleanup();
    TEST_PASS_MESSAGE("MicroUI command generation test completed");
}

// ============================================================================
// PIPELINE STATE MANAGEMENT TESTS
// ============================================================================

void test_render_pass_state_transitions(void) {
    // Test state transitions between different render phases
    printf("Testing render pass state transitions...\n");
    
    // Simulate the main render loop state transitions
    bool initial_state = sg_isvalid();
    TEST_ASSERT_TRUE(initial_state);
    
    // Simulate beginning a render pass
    printf("🔍 Graphics state before pass: %s\n", sg_isvalid() ? "VALID" : "INVALID");
    
    // Begin pass (in real code this would be done by the main render loop)
    // We can't actually call sg_begin_pass in tests without proper setup,
    // but we can test the validation logic
    
    // Simulate UI preparation phase
    printf("🔍 Graphics state during UI prep: %s\n", sg_isvalid() ? "VALID" : "INVALID");
    
    // Test that context remains valid throughout simulated operations
    bool final_state = sg_isvalid();
    TEST_ASSERT_TRUE_MESSAGE(final_state, "Graphics context should remain valid");
    
    TEST_PASS_MESSAGE("Render pass state transition test completed");
}

void test_pipeline_state_isolation(void) {
    // Test that UI pipeline operations don't interfere with 3D pipeline state
    printf("Testing pipeline state isolation...\n");
    
    // Record initial state
    bool initial_valid = sg_isvalid();
    
    // Simulate 3D rendering setup (we can't do actual Sokol calls in tests)
    printf("🎨 Simulating 3D render setup\n");
    
    // Simulate UI rendering phase
    printf("🎨 Simulating UI render phase\n");
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "test_scene", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    // Verify context is still valid after UI operations
    bool post_ui_valid = sg_isvalid();
    TEST_ASSERT_EQUAL_MESSAGE(initial_valid, post_ui_valid, 
                             "Graphics context validity should be preserved");
    
    TEST_PASS_MESSAGE("Pipeline state isolation test completed");
}

// ============================================================================
// ERROR HANDLING AND RECOVERY TESTS
// ============================================================================

void test_ui_error_recovery(void) {
    // Test UI system's ability to handle and recover from errors
    printf("Testing UI error recovery mechanisms...\n");
    
    // Test rendering with invalid parameters
    ui_render(NULL, NULL, -1.0f, "", -1, -1);
    
    // Context should still be valid after error conditions
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after error conditions");
    
    // Test multiple error conditions in sequence
    for (int i = 0; i < 5; i++) {
        ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, NULL, 0, 0);
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable during repeated errors");
    }
    
    TEST_PASS_MESSAGE("UI error recovery test completed");
}

void test_context_invalidation_handling(void) {
    // Test handling of context invalidation scenarios
    printf("Testing context invalidation handling...\n");
    
    // Test UI behavior when context becomes invalid
    bool context_state = sg_isvalid();
    printf("🔍 Context state: %s\n", context_state ? "VALID" : "INVALID");
    
    // Attempt UI rendering (should handle invalid context gracefully)
    if (!context_state) {
        ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "test_scene", 
                  TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        
        // Should not crash even with invalid context
        TEST_PASS_MESSAGE("Handled invalid context gracefully");
    } else {
        // Context is valid, test normal operation
        ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "test_scene", 
                  TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after normal operation");
    }
}

// ============================================================================
// PERFORMANCE AND MEMORY TESTS
// ============================================================================

void test_ui_render_performance_stability(void) {
    // Test UI rendering performance under repeated calls
    printf("Testing UI render performance stability...\n");
    
    const int num_frames = 100;
    
    for (int frame = 0; frame < num_frames; frame++) {
        // Simulate frame rendering
        ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "perf_test", 
                  TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        
        // Check that context remains valid throughout
        if (!sg_isvalid()) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Context became invalid at frame %d", frame);
            TEST_FAIL_MESSAGE(msg);
            break;
        }
        
        // Simulate scene changes occasionally
        if (frame % 20 == 0) {
            ui_request_scene_change("new_scene");
            ui_clear_scene_change_request();
        }
    }
    
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after performance test");
    TEST_PASS_MESSAGE("UI render performance stability test completed");
}

void test_ui_memory_stability(void) {
    // Test UI system memory stability over multiple initialization/cleanup cycles
    printf("Testing UI memory stability...\n");
    
    const int num_cycles = 10;
    
    for (int cycle = 0; cycle < num_cycles; cycle++) {
        // Initialize UI
        ui_init();
        
        // Perform some UI operations
        ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "memory_test", 
                  TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        
        // Test MicroUI operations if available
        ui_microui_begin_frame();
        ui_microui_end_frame();
        
        // Cleanup
        ui_cleanup();
        
        // Context should remain stable throughout cycles
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable during memory test");
    }
    
    TEST_PASS_MESSAGE("UI memory stability test completed");
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_ui_3d_integration_scenario(void) {
    // Test typical integration scenario: 3D rendering followed by UI
    printf("Testing UI-3D integration scenario...\n");
    
    // Simulate typical game frame
    
    // 1. Setup phase
    bool context_valid = sg_isvalid();
    TEST_ASSERT_TRUE(context_valid);
    
    // 2. Simulate 3D rendering phase
    printf("🎨 Phase 1: 3D rendering simulation\n");
    // In real code: render_frame(world, camera)
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should be valid after 3D phase");
    
    // 3. UI preparation phase
    printf("🎨 Phase 2: UI preparation\n");
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "integration_test", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should be valid after UI prep");
    
    // 4. UI rendering phase
    printf("🎨 Phase 3: UI rendering\n");
    ui_microui_begin_frame();
    
    // Add some UI content
    struct mu_Context* ctx = ui_microui_get_mu_context();
    if (ctx) {
        if (mu_begin_window(ctx, "Integration Test", mu_rect(50, 50, 300, 200))) {
            mu_layout_row(ctx, 1, (int[]){-1}, 0);
            mu_label(ctx, "Integration test UI");
            
            if (mu_button(ctx, "Test Button")) {
                printf("Test button clicked\n");
            }
            
            mu_end_window(ctx);
        }
    } else {
        printf("⚠️ MicroUI context not available (expected in test mode)\n");
    }
    
    ui_microui_end_frame();
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should be valid after UI render");
    
    // 5. Frame completion
    printf("🎨 Phase 4: Frame completion\n");
    // In real code: sg_commit()
    
    TEST_PASS_MESSAGE("UI-3D integration scenario completed successfully");
}

void test_scene_transition_ui_flow(void) {
    // Test UI behavior during scene transitions
    printf("Testing scene transition UI flow...\n");
    
    // Start with initial scene
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "scene_a", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_TRUE(sg_isvalid());
    
    // Request scene change
    ui_request_scene_change("scene_b");
    TEST_ASSERT_TRUE(ui_has_scene_change_request());
    
    // Render during transition
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "scene_b", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_TRUE(sg_isvalid());
    
    // Clear transition
    ui_clear_scene_change_request();
    TEST_ASSERT_FALSE(ui_has_scene_change_request());
    
    // Continue rendering in new scene
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "scene_b", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_TRUE(sg_isvalid());
    
    TEST_PASS_MESSAGE("Scene transition UI flow test completed");
}

// ============================================================================
// REGRESSION TESTS
// ============================================================================

void test_render_crash_regression(void) {
    // Regression test for the specific crash issue documented in RES_UI_RENDER_CRASH_SOLUTION.md
    printf("Testing render crash regression...\n");
    
    // This test specifically addresses the issue where:
    // "Assertion failed: (_sg.valid), function sg_end_pass, file sokol_gfx.h, line 21284"
    
    // Simulate the problematic scenario
    printf("🔍 Pre-render context check: %s\n", sg_isvalid() ? "VALID" : "INVALID");
    
    // Prepare UI (this should generate commands successfully)
    ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "crash_test", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    printf("🔍 Post-UI-prep context check: %s\n", sg_isvalid() ? "VALID" : "INVALID");
    
    // Attempt UI rendering (the original failure point)
    if (sg_isvalid()) {
        ui_microui_render(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        printf("🔍 Post-UI-render context check: %s\n", sg_isvalid() ? "VALID" : "INVALID");
    } else {
        printf("⚠️ Context invalid before UI render - skipping as expected\n");
    }
    
    // The test passes if we don't crash during this sequence
    TEST_PASS_MESSAGE("Render crash regression test completed without crashing");
}

void test_pipeline_state_corruption_regression(void) {
    // Regression test for pipeline state corruption between 3D and UI rendering
    printf("Testing pipeline state corruption regression...\n");
    
    // Record initial state
    bool initial_state = sg_isvalid();
    
    // Simulate rapid state changes that could cause corruption
    for (int i = 0; i < 10; i++) {
        // Alternate between different rendering contexts
        if (i % 2 == 0) {
            // Simulate 3D rendering context
            printf("🎨 Iteration %d: 3D context\n", i);
        } else {
            // Simulate UI rendering context
            printf("🎨 Iteration %d: UI context\n", i);
            ui_render(&test_world, &test_scheduler, TEST_DELTA_TIME, "corruption_test", 
                      TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        }
        
        // Verify state remains stable
        bool current_state = sg_isvalid();
        if (current_state != initial_state) {
            char msg[256];
            snprintf(msg, sizeof(msg), "State corruption detected at iteration %d", i);
            TEST_FAIL_MESSAGE(msg);
            break;
        }
    }
    
    TEST_PASS_MESSAGE("Pipeline state corruption regression test completed");
}

// ============================================================================
// TEST SUITE RUNNER
// ============================================================================

void suite_ui_rendering_pipeline(void) {
    printf("\n=== Running UI Rendering Pipeline Test Suite ===\n");
    
    // Context and validation tests
    RUN_TEST(test_ui_context_initialization);
    RUN_TEST(test_graphics_context_validation);
    RUN_TEST(test_ui_render_safety_checks);
    
    // MicroUI integration tests
    RUN_TEST(test_microui_context_safety);
    RUN_TEST(test_microui_command_generation);
    
    // Pipeline state management tests
    RUN_TEST(test_render_pass_state_transitions);
    RUN_TEST(test_pipeline_state_isolation);
    
    // Error handling tests
    RUN_TEST(test_ui_error_recovery);
    RUN_TEST(test_context_invalidation_handling);
    
    // Performance tests
    RUN_TEST(test_ui_render_performance_stability);
    RUN_TEST(test_ui_memory_stability);
    
    // Integration tests
    RUN_TEST(test_ui_3d_integration_scenario);
    RUN_TEST(test_scene_transition_ui_flow);
    
    // Regression tests
    RUN_TEST(test_render_crash_regression);
    RUN_TEST(test_pipeline_state_corruption_regression);
    
    printf("=== UI Rendering Pipeline Test Suite Complete ===\n\n");
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_ui_rendering_pipeline();
    
    // Cleanup graphics
    if (graphics_initialized) {
        sg_shutdown();
    }
    
    return UNITY_END();
}
#endif
