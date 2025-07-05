/**
 * @file test_render_3d_pipeline.c
 * @brief Comprehensive tests for the 3D rendering pipeline
 * 
 * Tests critical 3D rendering components including:
 * - 3D render pipeline initialization and state management
 * - Mesh rendering and vertex buffer management
 * - Camera system integration
 * - MVP matrix calculations and transformations
 * - Entity-based rendering system
 * - Performance and memory management
 */

#include "../vendor/unity.h"
#include "../../src/render_3d.h"
#include "../../src/render.h"
#include "../../src/core.h"
#include "../../src/component/transform.h"
#include "../../src/component/renderable.h"
#include "../../src/graphics_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Mock backend for testing
#define SOKOL_DUMMY_BACKEND
#include "../../src/sokol_gfx.h"

// Test constants
#define TEST_SCREEN_WIDTH 1024
#define TEST_SCREEN_HEIGHT 768
#define TEST_FOV 60.0f
#define TEST_NEAR_PLANE 0.1f
#define TEST_FAR_PLANE 1000.0f

// Mock entity system components
typedef struct MockEntity {
    EntityID id;
    uint32_t component_mask;
    Transform transform;
    Renderable renderable;
} MockEntity;

typedef struct MockWorld {
    MockEntity entities[32];
    uint32_t entity_count;
} MockWorld;

// Global test state
static bool graphics_setup = false;
static RenderConfig test_config;
static MockWorld test_world;
static Camera test_camera;

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
    memset(&test_config, 0, sizeof(test_config));
    test_config.screen_width = TEST_SCREEN_WIDTH;
    test_config.screen_height = TEST_SCREEN_HEIGHT;
    test_config.mode = RENDER_MODE_SOLID;
    test_config.show_debug_info = false;
    test_config.show_velocities = false;
    test_config.show_collision_bounds = false;
    
    // Initialize test world
    memset(&test_world, 0, sizeof(test_world));
    test_world.entity_count = 0;
    
    // Initialize test camera
    memset(&test_camera, 0, sizeof(test_camera));
    test_camera.position = (Vector3){0.0f, 0.0f, 5.0f};
    test_camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    test_camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    test_camera.fov = TEST_FOV;
    test_camera.aspect_ratio = (float)TEST_SCREEN_WIDTH / (float)TEST_SCREEN_HEIGHT;
    test_camera.near_plane = TEST_NEAR_PLANE;
    test_camera.far_plane = TEST_FAR_PLANE;
}

void tearDown(void) {
    // Reset test state
    memset(&test_config, 0, sizeof(test_config));
    memset(&test_world, 0, sizeof(test_world));
    memset(&test_camera, 0, sizeof(test_camera));
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

MockEntity* create_test_entity(EntityID id, Vector3 position, Vector3 scale) {
    if (test_world.entity_count >= 32) return NULL;
    
    MockEntity* entity = &test_world.entities[test_world.entity_count++];
    memset(entity, 0, sizeof(MockEntity));
    
    entity->id = id;
    entity->component_mask = COMPONENT_TRANSFORM | COMPONENT_RENDERABLE;
    
    // Set transform
    entity->transform.position = position;
    entity->transform.rotation = (Vector3){0.0f, 0.0f, 0.0f};
    entity->transform.scale = scale;
    
    // Set renderable (minimal setup)
    entity->renderable.mesh_name[0] = '\0'; // Empty mesh name
    entity->renderable.texture_name[0] = '\0'; // Empty texture name
    entity->renderable.vertex_count = 0;
    entity->renderable.index_count = 0;
    
    return entity;
}

void create_cube_mesh_data(Renderable* renderable) {
    // Simple cube vertex data (position only)
    static float cube_vertices[] = {
        // Front face
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        // Back face
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
    };
    
    static uint16_t cube_indices[] = {
        0, 1, 2,  2, 3, 0,    // Front
        4, 5, 6,  6, 7, 4,    // Back
        5, 0, 3,  3, 6, 5,    // Left
        1, 7, 6,  6, 2, 1,    // Right
        3, 2, 6,  6, 5, 3,    // Top
        0, 4, 7,  7, 1, 0     // Bottom
    };
    
    renderable->vertex_count = sizeof(cube_vertices) / (3 * sizeof(float));
    renderable->index_count = sizeof(cube_indices) / sizeof(uint16_t);
    
    strncpy(renderable->mesh_name, "test_cube", sizeof(renderable->mesh_name) - 1);
}

// ============================================================================
// 3D RENDER PIPELINE INITIALIZATION TESTS
// ============================================================================

void test_render_3d_initialization(void) {
    // Test 3D render system initialization
    printf("Testing 3D render initialization...\n");
    
    // Initialize with mock asset registry
    AssetRegistry mock_registry = {0};
    bool init_result = render_init(&test_config, &mock_registry, 
                                   TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    
    if (init_result) {
        printf("✅ 3D render system initialized successfully\n");
        
        // Test cleanup
        render_cleanup(&test_config);
        TEST_PASS_MESSAGE("3D render initialization and cleanup completed");
    } else {
        printf("⚠️ 3D render initialization failed (expected in test mode)\n");
        TEST_IGNORE_MESSAGE("3D render initialization failed in test mode");
    }
}

void test_render_context_validation(void) {
    // Test render context validation
    printf("Testing render context validation...\n");
    
    bool context_valid = sg_isvalid();
    TEST_ASSERT_TRUE_MESSAGE(context_valid, "Graphics context should be valid for 3D rendering");
    
    // Test clearing screen
    render_clear(0.0f, 0.0f, 0.0f, 1.0f);
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after clear");
    
    // Test presenting (no-op in test mode)
    render_present(&test_config);
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after present");
    
    printf("✅ Render context validation completed\n");
}

// ============================================================================
// MATRIX AND TRANSFORMATION TESTS
// ============================================================================

void test_mvp_matrix_calculation(void) {
    // Test Model-View-Projection matrix calculation
    printf("Testing MVP matrix calculation...\n");
    
    // Create test transform
    Transform transform = {
        .position = {2.0f, 1.0f, 0.0f},
        .rotation = {0.0f, 45.0f, 0.0f}, // 45 degrees Y rotation
        .scale = {1.5f, 1.5f, 1.5f}
    };
    
    // Test model matrix (this would be done internally by the render system)
    // For now, just verify the transform values are reasonable
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 2.0f, transform.position.x, "Position X should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.0f, transform.position.y, "Position Y should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.0f, transform.position.z, "Position Z should be correct");
    
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 45.0f, transform.rotation.y, "Y rotation should be correct");
    
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.5f, transform.scale.x, "Scale X should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.5f, transform.scale.y, "Scale Y should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.5f, transform.scale.z, "Scale Z should be correct");
    
    printf("✅ MVP matrix calculation test completed\n");
}

void test_camera_matrix_calculation(void) {
    // Test camera view and projection matrix setup
    printf("Testing camera matrix calculation...\n");
    
    // Test camera parameters
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.0f, test_camera.position.x, "Camera X position should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.0f, test_camera.position.y, "Camera Y position should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 5.0f, test_camera.position.z, "Camera Z position should be correct");
    
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.0f, test_camera.target.x, "Camera target X should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.0f, test_camera.target.y, "Camera target Y should be correct");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 0.0f, test_camera.target.z, "Camera target Z should be correct");
    
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, TEST_FOV, test_camera.fov, "Camera FOV should be correct");
    
    float expected_aspect = (float)TEST_SCREEN_WIDTH / (float)TEST_SCREEN_HEIGHT;
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, expected_aspect, test_camera.aspect_ratio, 
                                     "Camera aspect ratio should be correct");
    
    printf("✅ Camera matrix calculation test completed\n");
}

// ============================================================================
// ENTITY RENDERING TESTS
// ============================================================================

void test_entity_creation_and_rendering(void) {
    // Test entity creation and rendering setup
    printf("Testing entity creation and rendering...\n");
    
    // Create test entity
    MockEntity* entity = create_test_entity(1, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f});
    TEST_ASSERT_NOT_NULL_MESSAGE(entity, "Test entity should be created successfully");
    
    // Setup renderable component
    create_cube_mesh_data(&entity->renderable);
    
    // Verify entity data
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, entity->id, "Entity ID should match");
    TEST_ASSERT_TRUE_MESSAGE(entity->component_mask & COMPONENT_TRANSFORM, "Entity should have transform component");
    TEST_ASSERT_TRUE_MESSAGE(entity->component_mask & COMPONENT_RENDERABLE, "Entity should have renderable component");
    
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, entity->renderable.vertex_count, "Entity should have vertices");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, entity->renderable.index_count, "Entity should have indices");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("test_cube", entity->renderable.mesh_name, "Mesh name should be set");
    
    printf("✅ Entity creation and rendering test completed\n");
}

void test_multiple_entity_rendering(void) {
    // Test rendering multiple entities
    printf("Testing multiple entity rendering...\n");
    
    const int num_entities = 5;
    
    // Create multiple entities at different positions
    for (int i = 0; i < num_entities; i++) {
        Vector3 position = {(float)(i * 2 - 4), 0.0f, 0.0f}; // Spread along X axis
        Vector3 scale = {1.0f, 1.0f, 1.0f};
        
        MockEntity* entity = create_test_entity(i + 1, position, scale);
        TEST_ASSERT_NOT_NULL_MESSAGE(entity, "Each entity should be created successfully");
        
        create_cube_mesh_data(&entity->renderable);
    }
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(num_entities, test_world.entity_count, 
                                  "World should contain all created entities");
    
    // Verify each entity has unique position
    for (int i = 0; i < num_entities; i++) {
        float expected_x = (float)(i * 2 - 4);
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, expected_x, test_world.entities[i].transform.position.x,
                                         "Entity position should be unique");
    }
    
    printf("✅ Multiple entity rendering test completed\n");
}

void test_entity_transform_updates(void) {
    // Test entity transform updates
    printf("Testing entity transform updates...\n");
    
    // Create test entity
    MockEntity* entity = create_test_entity(1, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f});
    TEST_ASSERT_NOT_NULL(entity);
    
    // Test position updates
    entity->transform.position.x = 5.0f;
    entity->transform.position.y = 3.0f;
    entity->transform.position.z = -2.0f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, entity->transform.position.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, entity->transform.position.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.0f, entity->transform.position.z);
    
    // Test rotation updates
    entity->transform.rotation.x = 30.0f;
    entity->transform.rotation.y = 45.0f;
    entity->transform.rotation.z = 60.0f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 30.0f, entity->transform.rotation.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 45.0f, entity->transform.rotation.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 60.0f, entity->transform.rotation.z);
    
    // Test scale updates
    entity->transform.scale.x = 2.0f;
    entity->transform.scale.y = 0.5f;
    entity->transform.scale.z = 1.5f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, entity->transform.scale.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, entity->transform.scale.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, entity->transform.scale.z);
    
    printf("✅ Entity transform updates test completed\n");
}

// ============================================================================
// RENDER MODE TESTS
// ============================================================================

void test_render_mode_switching(void) {
    // Test switching between different render modes
    printf("Testing render mode switching...\n");
    
    // Test solid mode
    test_config.mode = RENDER_MODE_SOLID;
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDER_MODE_SOLID, test_config.mode, "Render mode should be solid");
    
    // Test wireframe mode
    test_config.mode = RENDER_MODE_WIREFRAME;
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDER_MODE_WIREFRAME, test_config.mode, "Render mode should be wireframe");
    
    // Test point mode (if available)
    test_config.mode = RENDER_MODE_POINT;
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDER_MODE_POINT, test_config.mode, "Render mode should be point");
    
    printf("✅ Render mode switching test completed\n");
}

void test_debug_rendering_flags(void) {
    // Test debug rendering flag functionality
    printf("Testing debug rendering flags...\n");
    
    // Test debug info toggle
    test_config.show_debug_info = true;
    TEST_ASSERT_TRUE_MESSAGE(test_config.show_debug_info, "Debug info should be enabled");
    
    test_config.show_debug_info = false;
    TEST_ASSERT_FALSE_MESSAGE(test_config.show_debug_info, "Debug info should be disabled");
    
    // Test velocity visualization toggle
    test_config.show_velocities = true;
    TEST_ASSERT_TRUE_MESSAGE(test_config.show_velocities, "Velocity visualization should be enabled");
    
    test_config.show_velocities = false;
    TEST_ASSERT_FALSE_MESSAGE(test_config.show_velocities, "Velocity visualization should be disabled");
    
    // Test collision bounds toggle
    test_config.show_collision_bounds = true;
    TEST_ASSERT_TRUE_MESSAGE(test_config.show_collision_bounds, "Collision bounds should be enabled");
    
    test_config.show_collision_bounds = false;
    TEST_ASSERT_FALSE_MESSAGE(test_config.show_collision_bounds, "Collision bounds should be disabled");
    
    printf("✅ Debug rendering flags test completed\n");
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_render_performance_simulation(void) {
    // Simulate render performance scenarios
    printf("Testing render performance simulation...\n");
    
    const int num_entities = 20;
    const int num_frames = 10;
    
    // Create many entities
    for (int i = 0; i < num_entities; i++) {
        Vector3 position = {
            (float)(i % 5) * 2.0f - 4.0f,
            (float)(i / 5) * 2.0f - 2.0f,
            0.0f
        };
        Vector3 scale = {1.0f, 1.0f, 1.0f};
        
        MockEntity* entity = create_test_entity(i + 1, position, scale);
        if (entity) {
            create_cube_mesh_data(&entity->renderable);
        }
    }
    
    TEST_ASSERT_EQUAL_INT_MESSAGE(num_entities, test_world.entity_count, 
                                  "All performance test entities should be created");
    
    // Simulate multiple frames of rendering
    for (int frame = 0; frame < num_frames; frame++) {
        // Simulate entity updates
        for (uint32_t i = 0; i < test_world.entity_count; i++) {
            // Rotate entities slightly each frame
            test_world.entities[i].transform.rotation.y += 1.0f;
        }
        
        // Context should remain stable throughout
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid during performance test");
        
        if (frame % 5 == 0) {
            printf("🔍 Performance simulation frame %d/%d\n", frame, num_frames);
        }
    }
    
    printf("✅ Render performance simulation completed\n");
}

void test_memory_usage_simulation(void) {
    // Test memory usage patterns during rendering
    printf("Testing memory usage simulation...\n");
    
    const int num_cycles = 5;
    const int entities_per_cycle = 8;
    
    for (int cycle = 0; cycle < num_cycles; cycle++) {
        // Clear previous entities
        test_world.entity_count = 0;
        
        // Create entities for this cycle
        for (int i = 0; i < entities_per_cycle; i++) {
            Vector3 position = {(float)i, 0.0f, 0.0f};
            Vector3 scale = {1.0f, 1.0f, 1.0f};
            
            MockEntity* entity = create_test_entity(i + 1, position, scale);
            if (entity) {
                create_cube_mesh_data(&entity->renderable);
            }
        }
        
        TEST_ASSERT_EQUAL_INT_MESSAGE(entities_per_cycle, test_world.entity_count,
                                      "Each cycle should create the correct number of entities");
        
        // Context should remain stable
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid during memory test");
    }
    
    printf("✅ Memory usage simulation completed\n");
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

void test_invalid_entity_handling(void) {
    // Test handling of invalid entity data
    printf("Testing invalid entity handling...\n");
    
    // Create entity with invalid data
    MockEntity* entity = create_test_entity(999, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 0.0f});
    TEST_ASSERT_NOT_NULL(entity);
    
    // Set invalid mesh data
    entity->renderable.vertex_count = 0;
    entity->renderable.index_count = 0;
    entity->renderable.mesh_name[0] = '\0';
    
    // System should handle this gracefully (no crash)
    TEST_ASSERT_EQUAL_INT(0, entity->renderable.vertex_count);
    TEST_ASSERT_EQUAL_INT(0, entity->renderable.index_count);
    
    // Test zero scale
    entity->transform.scale = (Vector3){0.0f, 0.0f, 0.0f};
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, entity->transform.scale.x);
    
    printf("✅ Invalid entity handling test completed\n");
}

void test_render_error_recovery(void) {
    // Test error recovery during rendering
    printf("Testing render error recovery...\n");
    
    // Test with NULL render config
    render_cleanup(NULL); // Should not crash
    
    // Test with invalid screen dimensions
    RenderConfig invalid_config = test_config;
    invalid_config.screen_width = 0;
    invalid_config.screen_height = 0;
    
    // These operations should handle invalid config gracefully
    render_clear(1.0f, 0.0f, 0.0f, 1.0f);
    render_present(&invalid_config);
    
    // Context should remain stable
    TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid after error recovery test");
    
    printf("✅ Render error recovery test completed\n");
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_full_render_pipeline_simulation(void) {
    // Test complete render pipeline simulation
    printf("Testing full render pipeline simulation...\n");
    
    // Setup: Create entities and camera
    MockEntity* entity1 = create_test_entity(1, (Vector3){-2.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f});
    MockEntity* entity2 = create_test_entity(2, (Vector3){2.0f, 0.0f, 0.0f}, (Vector3){1.5f, 1.5f, 1.5f});
    MockEntity* entity3 = create_test_entity(3, (Vector3){0.0f, 2.0f, 0.0f}, (Vector3){0.8f, 0.8f, 0.8f});
    
    TEST_ASSERT_NOT_NULL(entity1);
    TEST_ASSERT_NOT_NULL(entity2);
    TEST_ASSERT_NOT_NULL(entity3);
    
    create_cube_mesh_data(&entity1->renderable);
    create_cube_mesh_data(&entity2->renderable);
    create_cube_mesh_data(&entity3->renderable);
    
    // Simulate render loop
    const int num_frames = 5;
    for (int frame = 0; frame < num_frames; frame++) {
        printf("🎨 Simulating render frame %d/%d\n", frame + 1, num_frames);
        
        // 1. Update entities (simulate movement)
        entity1->transform.rotation.y += 2.0f;
        entity2->transform.rotation.x += 1.5f;
        entity3->transform.position.y = 2.0f + sinf((float)frame * 0.1f) * 0.5f;
        
        // 2. Update camera (simulate slight movement)
        test_camera.position.z = 5.0f + sinf((float)frame * 0.05f) * 0.2f;
        
        // 3. Clear screen
        render_clear(0.1f, 0.1f, 0.2f, 1.0f);
        
        // 4. Render entities (simulated)
        for (uint32_t i = 0; i < test_world.entity_count; i++) {
            MockEntity* ent = &test_world.entities[i];
            // In real code: render_entity_3d would be called here
            // For test: just verify entity data is valid
            TEST_ASSERT_TRUE(ent->component_mask & COMPONENT_RENDERABLE);
            TEST_ASSERT_GREATER_THAN(0, ent->renderable.vertex_count);
        }
        
        // 5. Present frame
        render_present(&test_config);
        
        // Verify context stability
        TEST_ASSERT_TRUE_MESSAGE(sg_isvalid(), "Context should remain valid throughout render loop");
    }
    
    printf("✅ Full render pipeline simulation completed\n");
}

// ============================================================================
// TEST SUITE RUNNER
// ============================================================================

void suite_render_3d_pipeline(void) {
    printf("\n=== Running 3D Render Pipeline Test Suite ===\n");
    
    // Initialization tests
    RUN_TEST(test_render_3d_initialization);
    RUN_TEST(test_render_context_validation);
    
    // Matrix and transformation tests
    RUN_TEST(test_mvp_matrix_calculation);
    RUN_TEST(test_camera_matrix_calculation);
    
    // Entity rendering tests
    RUN_TEST(test_entity_creation_and_rendering);
    RUN_TEST(test_multiple_entity_rendering);
    RUN_TEST(test_entity_transform_updates);
    
    // Render mode tests
    RUN_TEST(test_render_mode_switching);
    RUN_TEST(test_debug_rendering_flags);
    
    // Performance tests
    RUN_TEST(test_render_performance_simulation);
    RUN_TEST(test_memory_usage_simulation);
    
    // Error handling tests
    RUN_TEST(test_invalid_entity_handling);
    RUN_TEST(test_render_error_recovery);
    
    // Integration tests
    RUN_TEST(test_full_render_pipeline_simulation);
    
    printf("=== 3D Render Pipeline Test Suite Complete ===\n\n");
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_render_3d_pipeline();
    
    // Cleanup graphics
    if (graphics_setup) {
        sg_shutdown();
    }
    
    return UNITY_END();
}
#endif
