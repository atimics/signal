/**
 * @file test_render_layers.c
 * @brief Comprehensive tests for the render layers system
 * 
 * Tests the offscreen rendering layer system including:
 * - Layer manager creation and destruction
 * - Layer creation, configuration, and management
 * - Layer ordering and compositing
 * - Resource management and cleanup
 * - Error handling and edge cases
 */

#include "../vendor/unity.h"
#include "../../src/render_layers.h"
#include "../../src/graphics_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test constants
#define TEST_SCREEN_WIDTH 800
#define TEST_SCREEN_HEIGHT 600

// Sokol backend for testing (defined in build system)
#include "../../src/sokol_gfx.h"

// Global test state
static LayerManager* test_manager = NULL;
static bool sokol_initialized = false;

// ============================================================================
// TEST SETUP AND TEARDOWN
// ============================================================================

void setUp(void) {
    // Initialize Sokol with dummy backend for testing
    if (!sokol_initialized) {
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
        sokol_initialized = true;
    }
    
    // Clean state for each test
    test_manager = NULL;
}

void tearDown(void) {
    // Clean up layer manager if created
    if (test_manager) {
        layer_manager_destroy(test_manager);
        test_manager = NULL;
    }
    
    // Don't shutdown Sokol between tests to avoid re-initialization issues
}

// ============================================================================
// LAYER MANAGER TESTS
// ============================================================================

void test_layer_manager_creation(void) {
    // Test successful creation
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    TEST_ASSERT_NOT_NULL_MESSAGE(test_manager, "Layer manager should be created successfully");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, test_manager->layer_count, "Initial layer count should be 0");
    TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_SCREEN_WIDTH, test_manager->screen_width, "Screen width should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_SCREEN_HEIGHT, test_manager->screen_height, "Screen height should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, test_manager->current_frame, "Initial frame should be 0");
}

void test_layer_manager_destruction(void) {
    // Create and destroy manager
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Add a layer to test proper cleanup
    RenderLayerConfig config = {
        .name = "test_layer",
        .width = 400,
        .height = 300,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
    TEST_ASSERT_NOT_NULL(layer);
    TEST_ASSERT_EQUAL_INT(1, test_manager->layer_count);
    
    // Destroy should clean up everything
    layer_manager_destroy(test_manager);
    test_manager = NULL;  // Prevent double cleanup in tearDown
    
    // Test passes if no crashes occur
    TEST_PASS_MESSAGE("Layer manager destruction completed successfully");
}

void test_layer_manager_resize(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Add a layer before resize
    RenderLayerConfig config = {
        .name = "resize_test",
        .width = TEST_SCREEN_WIDTH,
        .height = TEST_SCREEN_HEIGHT,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_STATIC
    };
    
    RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
    TEST_ASSERT_NOT_NULL(layer);
    
    // Test resize
    const int new_width = 1024;
    const int new_height = 768;
    layer_manager_resize(test_manager, new_width, new_height);
    
    TEST_ASSERT_EQUAL_INT(new_width, test_manager->screen_width);
    TEST_ASSERT_EQUAL_INT(new_height, test_manager->screen_height);
    TEST_ASSERT_EQUAL_INT(new_width, layer->width);
    TEST_ASSERT_EQUAL_INT(new_height, layer->height);
}

// ============================================================================
// LAYER MANAGEMENT TESTS
// ============================================================================

void test_layer_creation_and_configuration(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Test basic layer creation
    RenderLayerConfig config = {
        .name = "test_layer_basic",
        .width = 512,
        .height = 512,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
    
    TEST_ASSERT_NOT_NULL_MESSAGE(layer, "Layer should be created successfully");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("test_layer_basic", layer->name, "Layer name should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(512, layer->width, "Layer width should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(512, layer->height, "Layer height should match");
    TEST_ASSERT_TRUE_MESSAGE(layer->enabled, "Layer should be enabled by default");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(1.0f, layer->opacity, "Layer opacity should be 1.0 by default");
    TEST_ASSERT_EQUAL_INT_MESSAGE(BLEND_MODE_NORMAL, layer->blend_mode, "Layer blend mode should be normal by default");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UPDATE_DYNAMIC, layer->update_frequency, "Layer update frequency should match");
    TEST_ASSERT_TRUE_MESSAGE(layer->dirty, "Layer should be dirty initially");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, test_manager->layer_count, "Manager layer count should increase");
}

void test_layer_creation_without_depth(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    RenderLayerConfig config = {
        .name = "no_depth_layer",
        .width = 256,
        .height = 256,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_STATIC
    };
    
    RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
    
    TEST_ASSERT_NOT_NULL(layer);
    TEST_ASSERT_EQUAL_STRING("no_depth_layer", layer->name);
    TEST_ASSERT_EQUAL_INT(UPDATE_STATIC, layer->update_frequency);
}

void test_layer_lookup_by_name(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Create multiple layers
    const char* layer_names[] = {"layer1", "layer2", "layer3"};
    const int num_layers = 3;
    
    for (int i = 0; i < num_layers; i++) {
        RenderLayerConfig config = {
            .name = layer_names[i],
            .width = 100 + i * 50,
            .height = 100 + i * 50,
            .needs_depth = false,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = UPDATE_DYNAMIC
        };
        
        RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
        TEST_ASSERT_NOT_NULL(layer);
    }
    
    // Test lookup by name
    for (int i = 0; i < num_layers; i++) {
        RenderLayer* found = layer_manager_get_layer(test_manager, layer_names[i]);
        TEST_ASSERT_NOT_NULL_MESSAGE(found, "Layer should be found by name");
        TEST_ASSERT_EQUAL_STRING_MESSAGE(layer_names[i], found->name, "Found layer name should match");
        TEST_ASSERT_EQUAL_INT_MESSAGE(100 + i * 50, found->width, "Found layer width should match");
    }
    
    // Test lookup of non-existent layer
    RenderLayer* not_found = layer_manager_get_layer(test_manager, "nonexistent");
    TEST_ASSERT_NULL_MESSAGE(not_found, "Non-existent layer should return NULL");
}

void test_layer_removal(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Create layers
    const char* layer_names[] = {"remove1", "remove2", "remove3"};
    for (int i = 0; i < 3; i++) {
        RenderLayerConfig config = {
            .name = layer_names[i],
            .width = 200,
            .height = 200,
            .needs_depth = false,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = UPDATE_DYNAMIC
        };
        
        RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
        TEST_ASSERT_NOT_NULL(layer);
    }
    
    TEST_ASSERT_EQUAL_INT(3, test_manager->layer_count);
    
    // Remove middle layer
    layer_manager_remove_layer(test_manager, "remove2");
    TEST_ASSERT_EQUAL_INT(2, test_manager->layer_count);
    
    // Verify removed layer cannot be found
    RenderLayer* removed = layer_manager_get_layer(test_manager, "remove2");
    TEST_ASSERT_NULL(removed);
    
    // Verify other layers still exist
    RenderLayer* layer1 = layer_manager_get_layer(test_manager, "remove1");
    RenderLayer* layer3 = layer_manager_get_layer(test_manager, "remove3");
    TEST_ASSERT_NOT_NULL(layer1);
    TEST_ASSERT_NOT_NULL(layer3);
    
    // Remove non-existent layer (should not crash)
    layer_manager_remove_layer(test_manager, "nonexistent");
    TEST_ASSERT_EQUAL_INT(2, test_manager->layer_count);
}

// ============================================================================
// LAYER PROPERTIES TESTS
// ============================================================================

void test_layer_property_setters(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    RenderLayerConfig config = {
        .name = "properties_test",
        .width = 300,
        .height = 300,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
    TEST_ASSERT_NOT_NULL(layer);
    
    // Test enabled property
    layer_set_enabled(layer, false);
    TEST_ASSERT_FALSE(layer->enabled);
    layer_set_enabled(layer, true);
    TEST_ASSERT_TRUE(layer->enabled);
    
    // Test opacity property
    layer_set_opacity(layer, 0.5f);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, layer->opacity);
    
    // Test opacity clamping
    layer_set_opacity(layer, -0.5f);  // Below 0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, layer->opacity);
    
    layer_set_opacity(layer, 1.5f);   // Above 1
    TEST_ASSERT_EQUAL_FLOAT(1.0f, layer->opacity);
    
    // Test blend mode
    layer_set_blend_mode(layer, BLEND_MODE_ADDITIVE);
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_ADDITIVE, layer->blend_mode);
    
    layer_set_blend_mode(layer, BLEND_MODE_MULTIPLY);
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_MULTIPLY, layer->blend_mode);
    
    // Test order
    layer_set_order(layer, 5);
    TEST_ASSERT_EQUAL_INT(5, layer->order);
}

void test_layer_dirty_marking(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    RenderLayerConfig config = {
        .name = "dirty_test",
        .width = 200,
        .height = 200,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_ON_DEMAND
    };
    
    RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
    TEST_ASSERT_NOT_NULL(layer);
    
    // Initially dirty
    TEST_ASSERT_TRUE(layer->dirty);
    
    // Mark as clean (simulating render completion)
    layer->dirty = false;
    TEST_ASSERT_FALSE(layer->dirty);
    
    // Mark dirty by name
    layer_manager_mark_dirty(test_manager, "dirty_test");
    TEST_ASSERT_TRUE(layer->dirty);
    
    // Mark non-existent layer (should not crash)
    layer_manager_mark_dirty(test_manager, "nonexistent");
    // Test passes if no crash occurs
}

// ============================================================================
// UPDATE FREQUENCY TESTS
// ============================================================================

void test_layer_should_update_logic(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Test UPDATE_STATIC layer
    RenderLayerConfig static_config = {
        .name = "static_layer",
        .width = 100, .height = 100,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_STATIC
    };
    
    RenderLayer* static_layer = layer_manager_add_layer(test_manager, &static_config);
    TEST_ASSERT_NOT_NULL(static_layer);
    
    // Static layer should update only when dirty
    static_layer->dirty = true;
    TEST_ASSERT_TRUE(layer_should_update(test_manager, static_layer));
    
    static_layer->dirty = false;
    TEST_ASSERT_FALSE(layer_should_update(test_manager, static_layer));
    
    // Test UPDATE_DYNAMIC layer
    RenderLayerConfig dynamic_config = {
        .name = "dynamic_layer",
        .width = 100, .height = 100,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* dynamic_layer = layer_manager_add_layer(test_manager, &dynamic_config);
    TEST_ASSERT_NOT_NULL(dynamic_layer);
    
    // Dynamic layer should always update
    dynamic_layer->dirty = true;
    TEST_ASSERT_TRUE(layer_should_update(test_manager, dynamic_layer));
    
    dynamic_layer->dirty = false;
    TEST_ASSERT_TRUE(layer_should_update(test_manager, dynamic_layer));
    
    // Test UPDATE_ON_DEMAND layer
    RenderLayerConfig on_demand_config = {
        .name = "on_demand_layer",
        .width = 100, .height = 100,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_ON_DEMAND
    };
    
    RenderLayer* on_demand_layer = layer_manager_add_layer(test_manager, &on_demand_config);
    TEST_ASSERT_NOT_NULL(on_demand_layer);
    
    // On-demand layer should update only when dirty
    on_demand_layer->dirty = true;
    TEST_ASSERT_TRUE(layer_should_update(test_manager, on_demand_layer));
    
    on_demand_layer->dirty = false;
    TEST_ASSERT_FALSE(layer_should_update(test_manager, on_demand_layer));
    
    // Test disabled layer
    static_layer->enabled = false;
    TEST_ASSERT_FALSE(layer_should_update(test_manager, static_layer));
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

void test_null_parameter_handling(void) {
    // Test NULL manager creation parameters
    LayerManager* null_manager = layer_manager_create(0, 0);
    TEST_ASSERT_NOT_NULL(null_manager);  // Should still create with zero dimensions
    if (null_manager) {
        layer_manager_destroy(null_manager);
    }
    
    // Test NULL manager operations
    layer_manager_destroy(NULL);  // Should not crash
    layer_manager_resize(NULL, 100, 100);  // Should not crash
    
    RenderLayer* null_layer = layer_manager_get_layer(NULL, "test");
    TEST_ASSERT_NULL(null_layer);
    
    layer_manager_remove_layer(NULL, "test");  // Should not crash
    layer_manager_mark_dirty(NULL, "test");  // Should not crash
    
    // Test layer operations with NULL parameters
    layer_set_enabled(NULL, true);  // Should not crash
    layer_set_opacity(NULL, 0.5f);  // Should not crash
    layer_set_blend_mode(NULL, BLEND_MODE_ADDITIVE);  // Should not crash
    layer_set_order(NULL, 1);  // Should not crash
    
    sg_image null_texture = layer_get_color_texture(NULL);
    TEST_ASSERT_EQUAL_INT(0, null_texture.id);
    
    bool should_update = layer_should_update(NULL, NULL);
    TEST_ASSERT_FALSE(should_update);
}

void test_max_layers_limit(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Create layers up to the maximum
    for (int i = 0; i < MAX_RENDER_LAYERS; i++) {
        char layer_name[32];
        snprintf(layer_name, sizeof(layer_name), "layer_%d", i);
        
        RenderLayerConfig config = {
            .name = layer_name,
            .width = 100, .height = 100,
            .needs_depth = false,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = UPDATE_DYNAMIC
        };
        
        RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
        TEST_ASSERT_NOT_NULL_MESSAGE(layer, "Layer creation should succeed up to maximum");
    }
    
    TEST_ASSERT_EQUAL_INT(MAX_RENDER_LAYERS, test_manager->layer_count);
    
    // Try to create one more layer (should fail)
    RenderLayerConfig overflow_config = {
        .name = "overflow_layer",
        .width = 100, .height = 100,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* overflow_layer = layer_manager_add_layer(test_manager, &overflow_config);
    TEST_ASSERT_NULL_MESSAGE(overflow_layer, "Layer creation should fail when exceeding maximum");
    TEST_ASSERT_EQUAL_INT(MAX_RENDER_LAYERS, test_manager->layer_count);
}

void test_invalid_layer_config(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Test NULL config
    RenderLayer* null_config_layer = layer_manager_add_layer(test_manager, NULL);
    TEST_ASSERT_NULL(null_config_layer);
    
    // Test config with NULL name
    RenderLayerConfig null_name_config = {
        .name = NULL,
        .width = 100, .height = 100,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    
    RenderLayer* null_name_layer = layer_manager_add_layer(test_manager, &null_name_config);
    (void)null_name_layer; // Suppress unused variable warning
    // Implementation should handle this gracefully (may or may not create layer)
    // Just test that it doesn't crash
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_layer_ordering_and_sorting(void) {
    test_manager = layer_manager_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Create layers with different orders
    const char* layer_names[] = {"background", "scene", "ui", "effects"};
    const int orders[] = {0, 1, 3, 2};  // Intentionally out of order
    const int num_layers = 4;
    
    for (int i = 0; i < num_layers; i++) {
        RenderLayerConfig config = {
            .name = layer_names[i],
            .width = 200, .height = 200,
            .needs_depth = false,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 1,
            .update_frequency = UPDATE_DYNAMIC
        };
        
        RenderLayer* layer = layer_manager_add_layer(test_manager, &config);
        TEST_ASSERT_NOT_NULL(layer);
        layer_set_order(layer, orders[i]);
    }
    
    // Verify layers are created in insertion order initially
    for (int i = 0; i < num_layers; i++) {
        TEST_ASSERT_EQUAL_STRING(layer_names[i], test_manager->layers[i].name);
        TEST_ASSERT_EQUAL_INT(orders[i], test_manager->layers[i].order);
    }
}

void test_typical_usage_scenario(void) {
    // Simulate a typical game rendering setup
    test_manager = layer_manager_create(1920, 1080);
    TEST_ASSERT_NOT_NULL(test_manager);
    
    // Create background layer (static)
    RenderLayerConfig bg_config = {
        .name = "background",
        .width = 1920, .height = 1080,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_STATIC
    };
    RenderLayer* bg_layer = layer_manager_add_layer(test_manager, &bg_config);
    TEST_ASSERT_NOT_NULL(bg_layer);
    layer_set_order(bg_layer, 0);
    
    // Create 3D scene layer (dynamic with depth)
    RenderLayerConfig scene_config = {
        .name = "3d_scene",
        .width = 1920, .height = 1080,
        .needs_depth = true,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    RenderLayer* scene_layer = layer_manager_add_layer(test_manager, &scene_config);
    TEST_ASSERT_NOT_NULL(scene_layer);
    layer_set_order(scene_layer, 1);
    
    // Create UI layer (on-demand)
    RenderLayerConfig ui_config = {
        .name = "ui",
        .width = 1920, .height = 1080,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_ON_DEMAND
    };
    RenderLayer* ui_layer = layer_manager_add_layer(test_manager, &ui_config);
    TEST_ASSERT_NOT_NULL(ui_layer);
    layer_set_order(ui_layer, 2);
    layer_set_blend_mode(ui_layer, BLEND_MODE_NORMAL);
    
    // Create effects layer (additive blending)
    RenderLayerConfig fx_config = {
        .name = "effects",
        .width = 1920, .height = 1080,
        .needs_depth = false,
        .color_format = SG_PIXELFORMAT_RGBA8,
        .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
        .sample_count = 1,
        .update_frequency = UPDATE_DYNAMIC
    };
    RenderLayer* fx_layer = layer_manager_add_layer(test_manager, &fx_config);
    TEST_ASSERT_NOT_NULL(fx_layer);
    layer_set_order(fx_layer, 3);
    layer_set_blend_mode(fx_layer, BLEND_MODE_ADDITIVE);
    layer_set_opacity(fx_layer, 0.8f);
    
    // Verify all layers are properly configured
    TEST_ASSERT_EQUAL_INT(4, test_manager->layer_count);
    
    // Test update logic for each layer type
    TEST_ASSERT_FALSE(layer_should_update(test_manager, bg_layer));  // Static, not dirty
    TEST_ASSERT_TRUE(layer_should_update(test_manager, scene_layer)); // Dynamic
    TEST_ASSERT_FALSE(layer_should_update(test_manager, ui_layer));   // On-demand, not dirty
    TEST_ASSERT_TRUE(layer_should_update(test_manager, fx_layer));    // Dynamic
    
    // Mark UI as dirty and test again
    layer_manager_mark_dirty(test_manager, "ui");
    TEST_ASSERT_TRUE(layer_should_update(test_manager, ui_layer));
    
    // Test layer lookup
    RenderLayer* found_scene = layer_manager_get_layer(test_manager, "3d_scene");
    TEST_ASSERT_NOT_NULL(found_scene);
    // Scene layer should have depth target (indicates depth was requested)
    TEST_ASSERT_NOT_EQUAL(0, found_scene->depth_target.id);
    
    RenderLayer* found_fx = layer_manager_get_layer(test_manager, "effects");
    TEST_ASSERT_NOT_NULL(found_fx);
    TEST_ASSERT_EQUAL_INT(BLEND_MODE_ADDITIVE, found_fx->blend_mode);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, found_fx->opacity);
}

// ============================================================================
// TEST SUITE RUNNER
// ============================================================================

void suite_render_layers(void) {
    printf("\n=== Running Render Layers Test Suite ===\n");
    
    // Manager tests
    RUN_TEST(test_layer_manager_creation);
    RUN_TEST(test_layer_manager_destruction);
    RUN_TEST(test_layer_manager_resize);
    
    // Layer management tests
    RUN_TEST(test_layer_creation_and_configuration);
    RUN_TEST(test_layer_creation_without_depth);
    RUN_TEST(test_layer_lookup_by_name);
    RUN_TEST(test_layer_removal);
    
    // Property tests
    RUN_TEST(test_layer_property_setters);
    RUN_TEST(test_layer_dirty_marking);
    
    // Update frequency tests
    RUN_TEST(test_layer_should_update_logic);
    
    // Error handling tests
    RUN_TEST(test_null_parameter_handling);
    RUN_TEST(test_max_layers_limit);
    RUN_TEST(test_invalid_layer_config);
    
    // Integration tests
    RUN_TEST(test_layer_ordering_and_sorting);
    RUN_TEST(test_typical_usage_scenario);
    
    printf("=== Render Layers Test Suite Complete ===\n\n");
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_render_layers();
    
    // Cleanup Sokol
    if (sokol_initialized) {
        sg_shutdown();
    }
    
    return UNITY_END();
}
#endif
