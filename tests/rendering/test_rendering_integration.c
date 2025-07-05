/**
 * @file test_rendering_integration.c
 * @brief Integration tests for the complete rendering system
 * 
 * Tests the integration between all rendering components:
 * - Render layers + 3D pipeline + UI rendering
 * - Cross-system state management and isolation
 * - Performance under realistic usage scenarios
 * - Error handling across system boundaries
 * - Memory management and resource lifecycle
 */

#include "../vendor/unity.h"
#include "../../src/render_layers.h"
#include "../../src/render_3d.h"
#include "../../src/ui.h"
#include "../../src/ui_microui.h"
#include "../../src/graphics_api.h"
#include "../../src/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Mock backend for testing
#define SOKOL_DUMMY_BACKEND
#include "../../src/sokol_gfx.h"

// Test constants
#define TEST_SCREEN_WIDTH 1920
#define TEST_SCREEN_HEIGHT 1080
#define TEST_LAYER_WIDTH 1920
#define TEST_LAYER_HEIGHT 1080

// Mock world structure for integration testing
struct MockWorld {
    int entity_count;
    bool entities_valid;
};

struct MockScheduler {
    int task_count;
    bool scheduler_active;
};

// Global test state
static LayerManager* layer_manager = NULL;
static struct MockWorld test_world;
static struct MockScheduler test_scheduler;
static RenderConfig test_config;
static bool graphics_initialized = false;

// ============================================================================
// TEST SETUP AND TEARDOWN
// ============================================================================

void setUp(void) {
    // Initialize graphics system
    if (!graphics_initialized) {
        sg_setup(&(sg_desc){
            .environment = {
                .defaults = {
                    .color_format = SG_PIXELFORMAT_RGBA8,
                    .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
                    .sample_count = 1
                }
            },
            .logger.func = NULL // Suppress logging in tests
        });
        graphics_initialized = true;
    }
    
    // Initialize test world and scheduler
    memset(&test_world, 0, sizeof(test_world));
    test_world.entity_count = 5;
    test_world.entities_valid = true;
    
    memset(&test_scheduler, 0, sizeof(test_scheduler));
    test_scheduler.task_count = 3;
    test_scheduler.scheduler_active = true;
    
    // Initialize render config
    memset(&test_config, 0, sizeof(test_config));
    test_config.screen_width = TEST_SCREEN_WIDTH;
    test_config.screen_height = TEST_SCREEN_HEIGHT;
    test_config.mode = RENDER_MODE_SOLID;
    
    // Clean layer manager state
    layer_manager = NULL;
}

void tearDown(void) {
    // Clean up layer manager
    if (layer_manager) {
        layer_manager_destroy(layer_manager);
        layer_manager = NULL;
    }
    
    // Reset test state
    memset(&test_world, 0, sizeof(test_world));
    memset(&test_scheduler, 0, sizeof(test_scheduler));
    memset(&test_config, 0, sizeof(test_config));
}

// ============================================================================
// LAYER SYSTEM INTEGRATION TESTS
// ============================================================================

void test_complete_layer_system_setup(void) {
    // Test setting up a complete layer system like a real game would
    printf("Testing complete layer system setup...\n");
    
    // Create layer manager
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL_MESSAGE(layer_manager, "Layer manager should be created");
    
    // Create background layer (static)
    RenderLayerConfig bg_config = {
        .name = "background",
        .width = TEST_LAYER_WIDTH,
        .height = TEST_LAYER_HEIGHT,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_STATIC
    };
    
    RenderLayer* bg_layer = layer_manager_add_layer(layer_manager, &bg_config);
    TEST_ASSERT_NOT_NULL_MESSAGE(bg_layer, "Background layer should be created");
    layer_set_order(bg_layer, 0);
    
    // Create 3D scene layer (dynamic with depth)
    RenderLayerConfig scene_config = {
        .name = "3d_scene",
        .width = TEST_LAYER_WIDTH,
        .height = TEST_LAYER_HEIGHT,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* scene_layer = layer_manager_add_layer(layer_manager, &scene_config);
    TEST_ASSERT_NOT_NULL_MESSAGE(scene_layer, "3D scene layer should be created");
    layer_set_order(scene_layer, 1);
    
    // Create UI layer (on-demand)
    RenderLayerConfig ui_config = {
        .name = "ui",
        .width = TEST_LAYER_WIDTH,
        .height = TEST_LAYER_HEIGHT,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_ON_DEMAND
    };
    
    RenderLayer* ui_layer = layer_manager_add_layer(layer_manager, &ui_config);
    TEST_ASSERT_NOT_NULL_MESSAGE(ui_layer, "UI layer should be created");
    layer_set_order(ui_layer, 2);
    layer_set_blend_mode(ui_layer, BLEND_MODE_NORMAL);
    
    // Create effects layer (additive)
    RenderLayerConfig fx_config = {
        .name = "effects",
        .width = TEST_LAYER_WIDTH,
        .height = TEST_LAYER_HEIGHT,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* fx_layer = layer_manager_add_layer(layer_manager, &fx_config);
    TEST_ASSERT_NOT_NULL_MESSAGE(fx_layer, "Effects layer should be created");
    layer_set_order(fx_layer, 3);
    layer_set_blend_mode(fx_layer, BLEND_MODE_ADDITIVE);
    layer_set_opacity(fx_layer, 0.75f);
    
    // Verify complete setup
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, layer_manager->layer_count, "All layers should be created");
    
    // Test layer lookup
    TEST_ASSERT_NOT_NULL(layer_manager_get_layer(layer_manager, "background"));
    TEST_ASSERT_NOT_NULL(layer_manager_get_layer(layer_manager, "3d_scene"));
    TEST_ASSERT_NOT_NULL(layer_manager_get_layer(layer_manager, "ui"));
    TEST_ASSERT_NOT_NULL(layer_manager_get_layer(layer_manager, "effects"));
    
    printf("✅ Complete layer system setup completed\n");
}

void test_layer_isolation_and_compositing(void) {
    // Test that layers properly isolate rendering and composite correctly
    printf("Testing layer isolation and compositing...\n");
    
    // Setup layer system
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Create two layers with different properties
    RenderLayerConfig layer1_config = {
        .name = "layer1",
        .width = 800, .height = 600,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayerConfig layer2_config = {
        .name = "layer2",
        .width = 800, .height = 600,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_STATIC
    };
    
    RenderLayer* layer1 = layer_manager_add_layer(layer_manager, &layer1_config);
    RenderLayer* layer2 = layer_manager_add_layer(layer_manager, &layer2_config);
    
    TEST_ASSERT_NOT_NULL(layer1);
    TEST_ASSERT_NOT_NULL(layer2);
    
    // Set different properties to test isolation
    layer_set_order(layer1, 0);
    layer_set_opacity(layer1, 1.0f);
    layer_set_blend_mode(layer1, BLEND_MODE_NORMAL);
    
    layer_set_order(layer2, 1);
    layer_set_opacity(layer2, 0.6f);
    layer_set_blend_mode(layer2, BLEND_MODE_ADDITIVE);
    
    // Verify properties are isolated
    TEST_ASSERT_EQUAL_INT(0, layer1->order);
    TEST_ASSERT_EQUAL_INT(1, layer2->order);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, layer1->opacity);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.6f, layer2->opacity);
    
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_NORMAL, layer1->blend_mode);
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_ADDITIVE, layer2->blend_mode);
    
    // Test layer rendering (begin/end)
    layer_begin_render(layer1);
    // In real code: render 3D content here
    layer_end_render();
    
    layer_begin_render(layer2);
    // In real code: render UI content here
    layer_end_render();
    
    // Test compositing (this would blend all layers to screen)
    layer_manager_composite(layer_manager);
    
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after layer operations");
    
    printf("✅ Layer isolation and compositing test completed\n");
}

// ============================================================================
// 3D + UI INTEGRATION TESTS
// ============================================================================

void test_3d_ui_integration_scenario(void) {
    // Test realistic 3D + UI integration scenario
    printf("Testing 3D + UI integration scenario...\n");
    
    // Setup layer system for 3D + UI
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Create 3D scene layer
    RenderLayerConfig scene_config = {
        .name = "game_scene",
        .width = TEST_SCREEN_WIDTH,
        .height = TEST_SCREEN_HEIGHT,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* scene_layer = layer_manager_add_layer(layer_manager, &scene_config);
    TEST_ASSERT_NOT_NULL(scene_layer);
    layer_set_order(scene_layer, 0);
    
    // Create UI overlay layer
    RenderLayerConfig ui_config = {
        .name = "game_ui",
        .width = TEST_SCREEN_WIDTH,
        .height = TEST_SCREEN_HEIGHT,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_ON_DEMAND
    };
    
    RenderLayer* ui_layer = layer_manager_add_layer(layer_manager, &ui_config);
    TEST_ASSERT_NOT_NULL(ui_layer);
    layer_set_order(ui_layer, 1);
    
    // Simulate game frame rendering
    const int num_frames = 3;
    
    for (int frame = 0; frame < num_frames; frame++) {
        printf("🎨 Simulating integration frame %d/%d\n", frame + 1, num_frames);
        
        // Frame 1: Render 3D scene
        layer_begin_render(scene_layer);
        
        // Simulate 3D rendering
        render_clear(0.2f, 0.3f, 0.5f, 1.0f); // Sky blue background
        
        // In real code: render all 3D entities here
        // render_frame(&test_world, &camera, &test_config);
        
        layer_end_render();
        
        // Frame 2: Render UI
        if (frame % 2 == 0) { // Update UI every other frame (on-demand)
            layer_manager_mark_dirty(layer_manager, "game_ui");
        }
        
        if (layer_should_update(layer_manager, ui_layer)) {
            layer_begin_render(ui_layer);
            
            // Simulate UI rendering
            ui_render(&test_world, &test_scheduler, 0.016f, "test_scene", 
                      TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
            
            layer_end_render();
        }
        
        // Frame 3: Composite all layers
        layer_manager_composite(layer_manager);
        
        // Verify stability throughout
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable during integration");
    }
    
    printf("✅ 3D + UI integration scenario completed\n");
}

void test_cross_system_state_management(void) {
    // Test that different rendering systems don't interfere with each other's state
    printf("Testing cross-system state management...\n");
    
    // Initialize multiple systems
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Create layers for different systems
    RenderLayerConfig render_config = {
        .name = "render_system",
        .width = 1024, .height = 768,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayerConfig ui_config = {
        .name = "ui_system",
        .width = 1024, .height = 768,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_ON_DEMAND
    };
    
    RenderLayer* render_layer = layer_manager_add_layer(layer_manager, &render_config);
    RenderLayer* ui_layer = layer_manager_add_layer(layer_manager, &ui_config);
    
    TEST_ASSERT_NOT_NULL(render_layer);
    TEST_ASSERT_NOT_NULL(ui_layer);
    
    // Test state isolation by modifying one system's settings
    layer_set_opacity(render_layer, 0.8f);
    layer_set_blend_mode(render_layer, BLEND_MODE_MULTIPLY);
    layer_set_enabled(render_layer, true);
    
    layer_set_opacity(ui_layer, 0.9f);
    layer_set_blend_mode(ui_layer, BLEND_MODE_NORMAL);
    layer_set_enabled(ui_layer, false);
    
    // Verify that changes to one system don't affect the other
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.8f, render_layer->opacity);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.9f, ui_layer->opacity);
    
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_MULTIPLY, render_layer->blend_mode);
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_NORMAL, ui_layer->blend_mode);
    
    TEST_ASSERT_TRUE(render_layer->enabled);
    TEST_ASSERT_FALSE(ui_layer->enabled);
    
    // Test that graphics context remains stable across operations
    bool context_before = sg_isvalid();
    
    // Simulate operations from both systems
    layer_begin_render(render_layer);
    render_clear(1.0f, 0.0f, 0.0f, 1.0f);
    layer_end_render();
    
    ui_render(&test_world, &test_scheduler, 0.016f, "state_test", 
              TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    bool context_after = sg_isvalid();
    
    TEST_ASSERT_EQUAL_MESSAGE(context_before, context_after, 
                             "Context validity should be preserved across systems");
    
    printf("✅ Cross-system state management test completed\n");
}

// ============================================================================
// PERFORMANCE INTEGRATION TESTS
// ============================================================================

void test_multi_layer_performance(void) {
    // Test performance with multiple active layers
    printf("Testing multi-layer performance...\n");
    
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Create maximum number of layers
    const int num_layers = MAX_RENDER_LAYERS;
    
    for (int i = 0; i < num_layers; i++) {
        char layer_name[32];
        snprintf(layer_name, sizeof(layer_name), "perf_layer_%d", i);
        
        RenderLayerConfig config = {
            .name = layer_name,
            .width = 512, .height = 512, // Smaller resolution for performance
            .needs_depth = (i % 2 == 0), // Alternate depth requirements
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = (i % 3 == 0) ? UPDATE_STATIC : UPDATE_DYNAMIC
        };
        
        RenderLayer* layer = layer_manager_add_layer(layer_manager, &config);
        TEST_ASSERT_NOT_NULL_MESSAGE(layer, "Performance test layer should be created");
        
        layer_set_order(layer, i);
        layer_set_opacity(layer, 0.8f + (float)i * 0.02f); // Slightly different opacities
    }
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(num_layers, layer_manager->layer_count, 
                                  "All performance test layers should be created");
    
    // Simulate multiple frames with all layers active
    const int num_frames = 10;
    
    for (int frame = 0; frame < num_frames; frame++) {
        // Update some layers (simulate dynamic content)
        for (int i = 0; i < num_layers; i++) {
            RenderLayer* layer = &layer_manager->layers[i];
            
            if (layer_should_update(layer_manager, layer)) {
                layer_begin_render(layer);
                
                // Simulate light rendering work
                render_clear((float)i / num_layers, 0.5f, 0.8f, 1.0f);
                
                layer_end_render();
            }
        }
        
        // Composite all layers
        layer_manager_composite(layer_manager);
        
        // Verify stability under load
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable under multi-layer load");
        
        if (frame % 3 == 0) {
            printf("🔍 Multi-layer performance frame %d/%d\n", frame + 1, num_frames);
        }
    }
    
    printf("✅ Multi-layer performance test completed\n");
}

void test_memory_pressure_simulation(void) {
    // Test system behavior under memory pressure
    printf("Testing memory pressure simulation...\n");
    
    const int num_cycles = 5;
    const int layers_per_cycle = 4;
    
    for (int cycle = 0; cycle < num_cycles; cycle++) {
        // Create layer manager
        layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
        TEST_ASSERT_NOT_NULL(layer_manager);
        
        // Create multiple layers
        for (int i = 0; i < layers_per_cycle; i++) {
            char layer_name[32];
            snprintf(layer_name, sizeof(layer_name), "memory_test_%d_%d", cycle, i);
            
            RenderLayerConfig config = {
                .name = layer_name,
                .width = 1024, .height = 1024, // Large textures for memory pressure
                .needs_depth = true,
                .color_format = SG_PIXELFORMAT_RGBA8,
                .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
                .sample_count = 1,
                .update_frequency = UPDATE_DYNAMIC
            };
            
            RenderLayer* layer = layer_manager_add_layer(layer_manager, &config);
            TEST_ASSERT_NOT_NULL(layer);
            
            // Render to each layer
            layer_begin_render(layer);
            render_clear((float)i / layers_per_cycle, 0.3f, 0.7f, 1.0f);
            layer_end_render();
        }
        
        // Simulate some rendering work
        layer_manager_composite(layer_manager);
        
        // Clean up this cycle
        layer_manager_destroy(layer_manager);
        layer_manager = NULL;
        
        // Verify context remains stable
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable under memory pressure");
        
        printf("🔍 Memory pressure cycle %d/%d completed\n", cycle + 1, num_cycles);
    }
    
    printf("✅ Memory pressure simulation completed\n");
}

// ============================================================================
// ERROR HANDLING INTEGRATION TESTS
// ============================================================================

void test_cascade_error_handling(void) {
    // Test error handling across multiple systems
    printf("Testing cascade error handling...\n");
    
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Create a layer
    RenderLayerConfig config = {
        .name = "error_test",
        .width = 512, .height = 512,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* layer = layer_manager_add_layer(layer_manager, &config);
    TEST_ASSERT_NOT_NULL(layer);
    
    // Test error scenarios
    
    // 1. Invalid render operations
    layer_begin_render(NULL); // Should handle gracefully
    layer_end_render(); // Should handle unmatched end
    
    // 2. Invalid UI operations with NULL parameters
    ui_render(NULL, NULL, -1.0f, NULL, -1, -1);
    
    // 3. Invalid layer operations
    layer_manager_mark_dirty(layer_manager, "nonexistent_layer");
    layer_set_opacity(NULL, 0.5f);
    layer_set_enabled(NULL, true);
    
    // 4. Mixed operations that might cause state corruption
    layer_begin_render(layer);
    ui_render(&test_world, &test_scheduler, 0.016f, "error_test", 512, 512);
    layer_end_render();
    
    // Verify that context remains stable despite errors
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable despite error conditions");
    
    // Verify layer system still works
    RenderLayer* found_layer = layer_manager_get_layer(layer_manager, "error_test");
    TEST_ASSERT_NOT_NULL_MESSAGE(found_layer, "Layer should still be accessible after errors");
    
    printf("✅ Cascade error handling test completed\n");
}

void test_resource_exhaustion_recovery(void) {
    // Test recovery from resource exhaustion scenarios
    printf("Testing resource exhaustion recovery...\n");
    
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Attempt to create more layers than maximum
    const int attempt_layers = MAX_RENDER_LAYERS + 3;
    int successful_layers = 0;
    
    for (int i = 0; i < attempt_layers; i++) {
        char layer_name[32];
        snprintf(layer_name, sizeof(layer_name), "exhaust_test_%d", i);
        
        RenderLayerConfig config = {
            .name = layer_name,
            .width = 256, .height = 256,
            .needs_depth = false,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = UPDATE_DYNAMIC
        };
        
        RenderLayer* layer = layer_manager_add_layer(layer_manager, &config);
        if (layer) {
            successful_layers++;
        }
    }
    
    // Should have created exactly MAX_RENDER_LAYERS
    TEST_ASSERT_EQUAL_INT_MESSAGE(MAX_RENDER_LAYERS, successful_layers,
                                  "Should create exactly maximum number of layers");
    TEST_ASSERT_EQUAL_INT_MESSAGE(MAX_RENDER_LAYERS, layer_manager->layer_count,
                                  "Layer manager should report correct count");
    
    // System should still function normally with maximum layers
    for (int i = 0; i < MAX_RENDER_LAYERS; i++) {
        RenderLayer* layer = &layer_manager->layers[i];
        layer_begin_render(layer);
        render_clear(0.1f, 0.2f, 0.3f, 1.0f);
        layer_end_render();
    }
    
    layer_manager_composite(layer_manager);
    
    // Context should remain stable
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable after resource exhaustion");
    
    printf("✅ Resource exhaustion recovery test completed\n");
}

// ============================================================================
// REAL-WORLD SCENARIO TESTS
// ============================================================================

void test_game_frame_simulation(void) {
    // Simulate a realistic game frame with all systems working together
    printf("Testing complete game frame simulation...\n");
    
    // Setup complete rendering system
    layer_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(layer_manager);
    
    // Create game layers
    const char* layer_names[] = {"background", "world", "entities", "effects", "ui", "debug"};
    const UpdateFrequency frequencies[] = {
        UPDATE_STATIC, UPDATE_DYNAMIC, UPDATE_DYNAMIC, 
        UPDATE_DYNAMIC, UPDATE_ON_DEMAND, UPDATE_ON_DEMAND
    };
    const BlendMode blend_modes[] = {
        BLEND_MODE_NORMAL, BLEND_MODE_NORMAL, BLEND_MODE_NORMAL,
        BLEND_MODE_ADDITIVE, BLEND_MODE_NORMAL, BLEND_MODE_NORMAL
    };
    const int num_game_layers = 6;
    
    for (int i = 0; i < num_game_layers; i++) {
        RenderLayerConfig config = {
            .name = layer_names[i],
            .width = TEST_SCREEN_WIDTH,
            .height = TEST_SCREEN_HEIGHT,
            .needs_depth = (i >= 1 && i <= 2), // World and entities need depth
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = frequencies[i]
        };
        
        RenderLayer* layer = layer_manager_add_layer(layer_manager, &config);
        TEST_ASSERT_NOT_NULL(layer);
        
        layer_set_order(layer, i);
        layer_set_blend_mode(layer, blend_modes[i]);
        
        if (i == 3) { // Effects layer
            layer_set_opacity(layer, 0.8f);
        }
    }
    
    // Simulate multiple game frames
    const int num_frames = 8;
    
    for (int frame = 0; frame < num_frames; frame++) {
        printf("🎮 Simulating game frame %d/%d\n", frame + 1, num_frames);
        
        // Update frame counter
        layer_manager->current_frame = frame;
        
        // 1. Background layer (static - only render if dirty)
        RenderLayer* bg_layer = layer_manager_get_layer(layer_manager, "background");
        if (frame == 0) { // Only render background on first frame
            layer_manager_mark_dirty(layer_manager, "background");
        }
        
        if (layer_should_update(layer_manager, bg_layer)) {
            layer_begin_render(bg_layer);
            render_clear(0.1f, 0.2f, 0.4f, 1.0f); // Sky blue
            layer_end_render();
        }
        
        // 2. World layer (dynamic - always update)
        RenderLayer* world_layer = layer_manager_get_layer(layer_manager, "world");
        if (layer_should_update(layer_manager, world_layer)) {
            layer_begin_render(world_layer);
            render_clear(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
            // In real game: render world geometry
            layer_end_render();
        }
        
        // 3. Entities layer (dynamic - always update)
        RenderLayer* entities_layer = layer_manager_get_layer(layer_manager, "entities");
        if (layer_should_update(layer_manager, entities_layer)) {
            layer_begin_render(entities_layer);
            render_clear(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
            // In real game: render all entities
            layer_end_render();
        }
        
        // 4. Effects layer (dynamic - always update)
        RenderLayer* effects_layer = layer_manager_get_layer(layer_manager, "effects");
        if (layer_should_update(layer_manager, effects_layer)) {
            layer_begin_render(effects_layer);
            render_clear(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
            // In real game: render particle effects, explosions, etc.
            layer_end_render();
        }
        
        // 5. UI layer (on-demand - update when UI changes)
        RenderLayer* ui_layer = layer_manager_get_layer(layer_manager, "ui");
        if (frame % 3 == 0) { // Simulate UI updates every 3 frames
            layer_manager_mark_dirty(layer_manager, "ui");
        }
        
        if (layer_should_update(layer_manager, ui_layer)) {
            layer_begin_render(ui_layer);
            render_clear(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
            
            // Render UI
            ui_render(&test_world, &test_scheduler, 0.016f, "game_scene", 
                      TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
            
            layer_end_render();
        }
        
        // 6. Debug layer (on-demand - only when debug mode is active)
        RenderLayer* debug_layer = layer_manager_get_layer(layer_manager, "debug");
        if (frame == 2 || frame == 5) { // Simulate debug mode toggle
            layer_manager_mark_dirty(layer_manager, "debug");
        }
        
        if (layer_should_update(layer_manager, debug_layer)) {
            layer_begin_render(debug_layer);
            render_clear(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
            // In real game: render debug information, wireframes, etc.
            layer_end_render();
        }
        
        // 7. Composite all layers to final image
        layer_manager_composite(layer_manager);
        
        // 8. Present frame (in real game: sg_commit())
        // TEST: Just verify context stability
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain stable throughout game frame");
    }
    
    printf("✅ Complete game frame simulation completed\n");
}

// ============================================================================
// TEST SUITE RUNNER
// ============================================================================

void suite_rendering_integration(void) {
    printf("\n=== Running Rendering Integration Test Suite ===\n");
    
    // Layer system integration tests
    RUN_TEST(test_complete_layer_system_setup);
    RUN_TEST(test_layer_isolation_and_compositing);
    
    // 3D + UI integration tests
    RUN_TEST(test_3d_ui_integration_scenario);
    RUN_TEST(test_cross_system_state_management);
    
    // Performance integration tests
    RUN_TEST(test_multi_layer_performance);
    RUN_TEST(test_memory_pressure_simulation);
    
    // Error handling integration tests
    RUN_TEST(test_cascade_error_handling);
    RUN_TEST(test_resource_exhaustion_recovery);
    
    // Real-world scenario tests
    RUN_TEST(test_game_frame_simulation);
    
    printf("=== Rendering Integration Test Suite Complete ===\n\n");
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_rendering_integration();
    
    // Cleanup graphics
    if (graphics_initialized) {
        sg_shutdown();
    }
    
    return UNITY_END();
}
#endif
