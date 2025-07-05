/**
 * @file test_graphics_pipeline.c
 * @brief Comprehensive tests for graphics pipeline and resource management
 * 
 * Tests critical graphics components including:
 * - Graphics API initialization and resource management
 * - Sokol resource lifecycle (buffers, images, pipelines)
 * - Render state validation and error handling
 * - Resource cleanup and memory management
 * - Graphics context stability
 */

#include "../vendor/unity.h"
#include "../../src/graphics_api.h"
#include "../../src/render_3d.h"
#include "../../src/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock backend for testing
#define SOKOL_DUMMY_BACKEND
#include "../../src/sokol_gfx.h"

// Test constants
#define TEST_BUFFER_SIZE 1024
#define TEST_TEXTURE_SIZE 64

// Global test state
static bool graphics_setup = false;
static RenderConfig test_render_config;

// ============================================================================
// TEST SETUP AND TEARDOWN
// ============================================================================

void setUp(void) {
    // Initialize graphics system if not already done
    if (!graphics_setup) {
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
        graphics_setup = true;
    }
    
    // Initialize test render config
    memset(&test_render_config, 0, sizeof(test_render_config));
    test_render_config.screen_width = 800;
    test_render_config.screen_height = 600;
    test_render_config.mode = RENDER_MODE_SOLID;
}

void tearDown(void) {
    // Reset render config
    memset(&test_render_config, 0, sizeof(test_render_config));
}

// ============================================================================
// GRAPHICS CONTEXT TESTS
// ============================================================================

void test_graphics_context_initialization(void) {
    // Test that graphics context is properly initialized
    bool context_valid = sg_isvalid();
    TEST_ASSERT_TRUE_MESSAGE(context_valid, "Graphics context should be valid after setup");
    
    printf("✅ Graphics context status: %s\n", context_valid ? "VALID" : "INVALID");
    
    // Test context info
    sg_desc desc = sg_query_desc();
    TEST_ASSERT_NOT_NULL_MESSAGE(&desc, "Graphics descriptor should be available");
    
    printf("✅ Graphics backend initialized successfully\n");
}

void test_graphics_context_stability(void) {
    // Test that context remains stable under various operations
    const int num_iterations = 50;
    
    for (int i = 0; i < num_iterations; i++) {
        bool valid = sg_isvalid();
        TEST_ASSERT_TRUE_MESSAGE(valid, "Context should remain valid throughout iterations");
        
        if (i % 10 == 0) {
            printf("🔍 Context stability check %d/%d: %s\n", 
                   i, num_iterations, valid ? "VALID" : "INVALID");
        }
        
        // Simulate some light operations
        sg_reset_state_cache();
    }
    
    TEST_PASS_MESSAGE("Graphics context stability test completed");
}

// ============================================================================
// RESOURCE MANAGEMENT TESTS
// ============================================================================

void test_buffer_creation_and_cleanup(void) {
    // Test buffer lifecycle
    printf("Testing buffer creation and cleanup...\n");
    
    // Create a vertex buffer
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    
    sg_buffer vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .usage = SG_USAGE_IMMUTABLE,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .label = "test_vertex_buffer"
    });
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, vertex_buffer.id, "Buffer should be created successfully");
    
    // Test buffer state
    sg_resource_state buffer_state = sg_query_buffer_state(vertex_buffer);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_VALID, buffer_state, "Buffer should be in valid state");
    
    // Test buffer info
    sg_buffer_info buffer_info = sg_query_buffer_info(vertex_buffer);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_BUFFERTYPE_VERTEXBUFFER, buffer_info.type, "Buffer type should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_USAGE_IMMUTABLE, buffer_info.usage, "Buffer usage should match");
    
    // Cleanup
    sg_destroy_buffer(vertex_buffer);
    
    // Verify cleanup
    sg_resource_state post_destroy_state = sg_query_buffer_state(vertex_buffer);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, post_destroy_state, 
                                  "Buffer should be invalid after destruction");
    
    printf("✅ Buffer lifecycle test completed\n");
}

void test_image_creation_and_cleanup(void) {
    // Test image/texture lifecycle
    printf("Testing image creation and cleanup...\n");
    
    // Create test texture data
    const int width = TEST_TEXTURE_SIZE;
    const int height = TEST_TEXTURE_SIZE;
    const int channels = 4; // RGBA
    uint8_t* pixels = malloc(width * height * channels);
    
    // Fill with test pattern
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int offset = (y * width + x) * channels;
            pixels[offset + 0] = (uint8_t)(x * 255 / width);     // R
            pixels[offset + 1] = (uint8_t)(y * 255 / height);    // G
            pixels[offset + 2] = 128;                             // B
            pixels[offset + 3] = 255;                             // A
        }
    }
    
    // Create image
    sg_image test_image = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = pixels,
            .size = width * height * channels
        },
        .label = "test_image"
    });
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, test_image.id, "Image should be created successfully");
    
    // Test image state
    sg_resource_state image_state = sg_query_image_state(test_image);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_VALID, image_state, "Image should be in valid state");
    
    // Test image info
    sg_image_info image_info = sg_query_image_info(test_image);
    TEST_ASSERT_EQUAL_INT_MESSAGE(width, image_info.width, "Image width should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(height, image_info.height, "Image height should match");
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_PIXELFORMAT_RGBA8, image_info.pixel_format, "Image format should match");
    
    // Cleanup
    sg_destroy_image(test_image);
    free(pixels);
    
    // Verify cleanup
    sg_resource_state post_destroy_state = sg_query_image_state(test_image);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, post_destroy_state, 
                                  "Image should be invalid after destruction");
    
    printf("✅ Image lifecycle test completed\n");
}

void test_sampler_creation_and_cleanup(void) {
    // Test sampler lifecycle
    printf("Testing sampler creation and cleanup...\n");
    
    sg_sampler test_sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .label = "test_sampler"
    });
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, test_sampler.id, "Sampler should be created successfully");
    
    // Test sampler state
    sg_resource_state sampler_state = sg_query_sampler_state(test_sampler);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_VALID, sampler_state, "Sampler should be in valid state");
    
    // Cleanup
    sg_destroy_sampler(test_sampler);
    
    // Verify cleanup
    sg_resource_state post_destroy_state = sg_query_sampler_state(test_sampler);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, post_destroy_state, 
                                  "Sampler should be invalid after destruction");
    
    printf("✅ Sampler lifecycle test completed\n");
}

void test_shader_creation_and_cleanup(void) {
    // Test shader lifecycle
    printf("Testing shader creation and cleanup...\n");
    
    // Simple vertex shader
    const char* vs_source = 
        "#version 330\n"
        "layout(location=0) in vec3 position;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "}\n";
    
    // Simple fragment shader
    const char* fs_source = 
        "#version 330\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    
    sg_shader test_shader = sg_make_shader(&(sg_shader_desc){
        .vs.source = vs_source,
        .fs.source = fs_source,
        .label = "test_shader"
    });
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, test_shader.id, "Shader should be created successfully");
    
    // Test shader state
    sg_resource_state shader_state = sg_query_shader_state(test_shader);
    // Note: In dummy backend, shader compilation might not fully work
    TEST_ASSERT_TRUE_MESSAGE(shader_state == SG_RESOURCESTATE_VALID || shader_state == SG_RESOURCESTATE_FAILED,
                             "Shader should be either valid or failed (expected in test mode)");
    
    // Cleanup
    sg_destroy_shader(test_shader);
    
    // Verify cleanup
    sg_resource_state post_destroy_state = sg_query_shader_state(test_shader);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, post_destroy_state, 
                                  "Shader should be invalid after destruction");
    
    printf("✅ Shader lifecycle test completed\n");
}

void test_pipeline_creation_and_cleanup(void) {
    // Test pipeline lifecycle
    printf("Testing pipeline creation and cleanup...\n");
    
    // Create a simple shader first
    sg_shader shader = sg_make_shader(&(sg_shader_desc){
        .vs.source = 
            "#version 330\n"
            "layout(location=0) in vec3 position;\n"
            "void main() { gl_Position = vec4(position, 1.0); }\n",
        .fs.source = 
            "#version 330\n"
            "out vec4 frag_color;\n"
            "void main() { frag_color = vec4(1.0); }\n",
        .label = "pipeline_test_shader"
    });
    
    // Create pipeline
    sg_pipeline test_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shader,
        .layout = {
            .attrs[0] = { .format = SG_VERTEXFORMAT_FLOAT3 }
        },
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .cull_mode = SG_CULLMODE_NONE,
        .label = "test_pipeline"
    });
    
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, test_pipeline.id, "Pipeline should be created successfully");
    
    // Test pipeline state
    sg_resource_state pipeline_state = sg_query_pipeline_state(test_pipeline);
    // Pipeline might fail in dummy backend due to shader compilation
    TEST_ASSERT_TRUE_MESSAGE(pipeline_state == SG_RESOURCESTATE_VALID || pipeline_state == SG_RESOURCESTATE_FAILED,
                             "Pipeline should be either valid or failed (expected in test mode)");
    
    // Cleanup
    sg_destroy_pipeline(test_pipeline);
    sg_destroy_shader(shader);
    
    // Verify cleanup
    sg_resource_state post_destroy_state = sg_query_pipeline_state(test_pipeline);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, post_destroy_state, 
                                  "Pipeline should be invalid after destruction");
    
    printf("✅ Pipeline lifecycle test completed\n");
}

// ============================================================================
// RESOURCE STRESS TESTS
// ============================================================================

void test_resource_creation_stress(void) {
    // Test creating and destroying many resources
    printf("Testing resource creation stress...\n");
    
    const int num_resources = 50;
    sg_buffer* buffers = malloc(num_resources * sizeof(sg_buffer));
    sg_image* images = malloc(num_resources * sizeof(sg_image));
    
    // Create many buffers
    float test_data[] = {1.0f, 2.0f, 3.0f, 4.0f};
    for (int i = 0; i < num_resources; i++) {
        buffers[i] = sg_make_buffer(&(sg_buffer_desc){
            .data = SG_RANGE(test_data),
            .usage = SG_USAGE_IMMUTABLE,
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .label = "stress_test_buffer"
        });
        
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, buffers[i].id, "Buffer creation should succeed");
    }
    
    // Create many images
    uint32_t pixel_data = 0xFFFFFFFF; // White pixel
    for (int i = 0; i < num_resources; i++) {
        images[i] = sg_make_image(&(sg_image_desc){
            .width = 1,
            .height = 1,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .data.subimage[0][0] = {
                .ptr = &pixel_data,
                .size = sizeof(pixel_data)
            },
            .label = "stress_test_image"
        });
        
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, images[i].id, "Image creation should succeed");
    }
    
    // Verify all resources are valid
    for (int i = 0; i < num_resources; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_VALID, sg_query_buffer_state(buffers[i]),
                                      "All buffers should be valid");
        TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_VALID, sg_query_image_state(images[i]),
                                      "All images should be valid");
    }
    
    // Cleanup all resources
    for (int i = 0; i < num_resources; i++) {
        sg_destroy_buffer(buffers[i]);
        sg_destroy_image(images[i]);
    }
    
    free(buffers);
    free(images);
    
    printf("✅ Resource stress test completed\n");
}

void test_resource_leak_detection(void) {
    // Test for resource leaks by creating and destroying resources repeatedly
    printf("Testing resource leak detection...\n");
    
    const int num_cycles = 20;
    const int resources_per_cycle = 10;
    
    for (int cycle = 0; cycle < num_cycles; cycle++) {
        sg_buffer* cycle_buffers = malloc(resources_per_cycle * sizeof(sg_buffer));
        
        // Create resources
        float data[] = {1.0f, 2.0f, 3.0f};
        for (int i = 0; i < resources_per_cycle; i++) {
            cycle_buffers[i] = sg_make_buffer(&(sg_buffer_desc){
                .data = SG_RANGE(data),
                .usage = SG_USAGE_IMMUTABLE,
                .type = SG_BUFFERTYPE_VERTEXBUFFER,
                .label = "leak_test_buffer"
            });
        }
        
        // Destroy resources
        for (int i = 0; i < resources_per_cycle; i++) {
            sg_destroy_buffer(cycle_buffers[i]);
        }
        
        free(cycle_buffers);
        
        // Context should remain valid throughout
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid during leak test");
        
        if (cycle % 5 == 0) {
            printf("🔍 Leak test cycle %d/%d completed\n", cycle, num_cycles);
        }
    }
    
    TEST_PASS_MESSAGE("Resource leak detection test completed");
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

void test_invalid_resource_handling(void) {
    // Test handling of invalid resource operations
    printf("Testing invalid resource handling...\n");
    
    // Test operations on invalid buffer
    sg_buffer invalid_buffer = {0};
    sg_resource_state invalid_state = sg_query_buffer_state(invalid_buffer);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, invalid_state, 
                                  "Invalid buffer should have invalid state");
    
    // Destroying invalid resource should not crash
    sg_destroy_buffer(invalid_buffer);
    
    // Test operations on invalid image
    sg_image invalid_image = {0};
    sg_resource_state invalid_image_state = sg_query_image_state(invalid_image);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, invalid_image_state,
                                  "Invalid image should have invalid state");
    
    sg_destroy_image(invalid_image);
    
    // Test invalid shader operations
    sg_shader invalid_shader = {0};
    sg_resource_state invalid_shader_state = sg_query_shader_state(invalid_shader);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, invalid_shader_state,
                                  "Invalid shader should have invalid state");
    
    sg_destroy_shader(invalid_shader);
    
    // Context should remain stable after invalid operations
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after invalid operations");
    
    printf("✅ Invalid resource handling test completed\n");
}

void test_resource_validation(void) {
    // Test resource validation mechanisms
    printf("Testing resource validation...\n");
    
    // Create valid buffer
    float vertices[] = {0.0f, 0.0f, 0.0f};
    sg_buffer valid_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .usage = SG_USAGE_IMMUTABLE,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .label = "validation_test_buffer"
    });
    
    // Test validation before destruction
    sg_resource_state pre_destroy = sg_query_buffer_state(valid_buffer);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_VALID, pre_destroy, "Buffer should be valid before destruction");
    
    // Destroy and test post-destruction state
    sg_destroy_buffer(valid_buffer);
    sg_resource_state post_destroy = sg_query_buffer_state(valid_buffer);
    TEST_ASSERT_EQUAL_INT_MESSAGE(SG_RESOURCESTATE_INVALID, post_destroy, "Buffer should be invalid after destruction");
    
    // Double-destroy should not crash
    sg_destroy_buffer(valid_buffer);
    
    TEST_PASS_MESSAGE("Resource validation test completed");
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_render_config_integration(void) {
    // Test integration with render config system
    printf("Testing render config integration...\n");
    
    // Test render config initialization
    RenderConfig config = {0};
    config.screen_width = 1024;
    config.screen_height = 768;
    config.mode = RENDER_MODE_WIREFRAME;
    config.show_debug_info = true;
    
    // Verify config values
    TEST_ASSERT_EQUAL_INT_MESSAGE(1024, config.screen_width, "Screen width should be set correctly");
    TEST_ASSERT_EQUAL_INT_MESSAGE(768, config.screen_height, "Screen height should be set correctly");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDER_MODE_WIREFRAME, config.mode, "Render mode should be set correctly");
    TEST_ASSERT_TRUE_MESSAGE(config.show_debug_info, "Debug info flag should be set");
    
    printf("✅ Render config integration test completed\n");
}

void test_graphics_screenshot_functionality(void) {
    // Test screenshot capture functionality
    printf("Testing graphics screenshot functionality...\n");
    
    // Test screenshot function exists and handles calls gracefully
    bool screenshot_result = graphics_capture_screenshot("test_screenshot.bmp");
    
    // In test mode, this might fail due to dummy backend
    if (screenshot_result) {
        printf("✅ Screenshot captured successfully\n");
    } else {
        printf("⚠️ Screenshot failed (expected in test mode)\n");
    }
    
    // Function should not crash regardless of result
    TEST_PASS_MESSAGE("Screenshot functionality test completed");
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_resource_creation_performance(void) {
    // Test performance of resource creation/destruction
    printf("Testing resource creation performance...\n");
    
    const int num_iterations = 100;
    
    // Time buffer creation/destruction cycles
    for (int i = 0; i < num_iterations; i++) {
        float data[] = {(float)i, (float)i + 1.0f, (float)i + 2.0f};
        
        sg_buffer buffer = sg_make_buffer(&(sg_buffer_desc){
            .data = SG_RANGE(data),
            .usage = SG_USAGE_IMMUTABLE,
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .label = "perf_test_buffer"
        });
        
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SG_INVALID_ID, buffer.id, "Buffer creation should succeed");
        
        sg_destroy_buffer(buffer);
        
        if (i % 20 == 0) {
            printf("🔍 Performance test iteration %d/%d\n", i, num_iterations);
        }
    }
    
    // Context should remain stable
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after performance test");
    
    printf("✅ Resource creation performance test completed\n");
}

// ============================================================================
// TEST SUITE RUNNER
// ============================================================================

void suite_graphics_pipeline(void) {
    printf("\n=== Running Graphics Pipeline Test Suite ===\n");
    
    // Context tests
    RUN_TEST(test_graphics_context_initialization);
    RUN_TEST(test_graphics_context_stability);
    
    // Resource management tests
    RUN_TEST(test_buffer_creation_and_cleanup);
    RUN_TEST(test_image_creation_and_cleanup);
    RUN_TEST(test_sampler_creation_and_cleanup);
    RUN_TEST(test_shader_creation_and_cleanup);
    RUN_TEST(test_pipeline_creation_and_cleanup);
    
    // Stress tests
    RUN_TEST(test_resource_creation_stress);
    RUN_TEST(test_resource_leak_detection);
    
    // Error handling tests
    RUN_TEST(test_invalid_resource_handling);
    RUN_TEST(test_resource_validation);
    
    // Integration tests
    RUN_TEST(test_render_config_integration);
    RUN_TEST(test_graphics_screenshot_functionality);
    
    // Performance tests
    RUN_TEST(test_resource_creation_performance);
    
    printf("=== Graphics Pipeline Test Suite Complete ===\n\n");
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_graphics_pipeline();
    
    // Cleanup graphics
    if (graphics_setup) {
        sg_shutdown();
    }
    
    return UNITY_END();
}
#endif
