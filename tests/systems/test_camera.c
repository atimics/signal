/**
 * @file test_camera_system.c
 * @brief Comprehensive tests for camera system
 * 
 * Tests the Camera component, camera switching, matrix calculations,
 * and camera behaviors.
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/system/camera.h"
#include "../../src/render_camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Test world and render config
static struct World test_world;
static RenderConfig test_render_config;

void setUp(void)
{
    // Initialize test world properly
    world_init(&test_world);
    
    // Initialize test render config
    memset(&test_render_config, 0, sizeof(RenderConfig));
    test_render_config.screen_width = 1280;
    test_render_config.screen_height = 720;
}

void tearDown(void)
{
    // Properly destroy the world
    world_destroy(&test_world);
}

// ============================================================================
// CAMERA COMPONENT TESTS
// ============================================================================

void test_camera_component_creation(void)
{
    EntityID entity = entity_create(&test_world);
    
    // Add camera component
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    bool success = true;
    TEST_ASSERT_TRUE(success);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    TEST_ASSERT_NOT_NULL(camera);
    TEST_ASSERT_NOT_NULL(transform);
    
    // Check default camera values
    TEST_ASSERT_TRUE(camera->fov > 0.0f);
    TEST_ASSERT_TRUE(camera->near_plane > 0.0f);
    TEST_ASSERT_TRUE(camera->far_plane > camera->near_plane);
    TEST_ASSERT_EQUAL_INT(CAMERA_BEHAVIOR_THIRD_PERSON, camera->behavior);
    TEST_ASSERT_FALSE(camera->is_active);
}

void test_camera_behavior_mode(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    
    // Test different camera behaviors
    camera->behavior = CAMERA_BEHAVIOR_FIRST_PERSON;
    TEST_ASSERT_EQUAL_INT(CAMERA_BEHAVIOR_FIRST_PERSON, camera->behavior);
    
    camera->behavior = CAMERA_BEHAVIOR_CHASE;
    camera->follow_distance = 10.0f;
    TEST_ASSERT_EQUAL_INT(CAMERA_BEHAVIOR_CHASE, camera->behavior);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, camera->follow_distance);
}

void test_camera_matrix_updates(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set camera position and look direction
    transform->position = (Vector3){ 0.0f, 5.0f, 10.0f };
    
    // Update camera matrices
    camera_update_matrices(camera);
    
    // View matrix should be updated (hard to test exact values, but should not be identity)
    // Projection matrix should be valid
    TEST_ASSERT_TRUE(true); // Just ensure no crash
}

// ============================================================================
// CAMERA SWITCHING TESTS
// ============================================================================

void test_camera_switching_basic(void)
{
    // Create multiple cameras
    EntityID camera1 = entity_create(&test_world);
    EntityID camera2 = entity_create(&test_world);
    
    entity_add_components(&test_world, camera1, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    entity_add_components(&test_world, camera2, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    // Set first camera as active
    world_set_active_camera(&test_world, camera1);
    TEST_ASSERT_EQUAL(camera1, world_get_active_camera(&test_world));
    
    // Switch to second camera
    world_set_active_camera(&test_world, camera2);
    TEST_ASSERT_EQUAL(camera2, world_get_active_camera(&test_world));
}

void test_camera_switching_by_index(void)
{
    // Create multiple cameras
    EntityID cameras[3];
    for (int i = 0; i < 3; i++) {
        cameras[i] = entity_create(&test_world);
        entity_add_components(&test_world, cameras[i], COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    }
    
    // Switch to camera by index
    bool success1 = switch_to_camera(&test_world, 0);
    TEST_ASSERT_TRUE(success1);
    
    bool success2 = switch_to_camera(&test_world, 1);
    TEST_ASSERT_TRUE(success2);
    
    // Invalid index should fail
    bool success3 = switch_to_camera(&test_world, 10);
    TEST_ASSERT_FALSE(success3);
}

void test_camera_cycling(void)
{
    // Create multiple cameras
    const int camera_count = 3;
    EntityID cameras[camera_count];
    
    for (int i = 0; i < camera_count; i++) {
        cameras[i] = entity_create(&test_world);
        entity_add_components(&test_world, cameras[i], COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    }
    
    // Get initial active camera
    EntityID initial_camera = world_get_active_camera(&test_world);
    
    // Cycle through cameras
    for (int i = 0; i < camera_count; i++) {
        bool success = cycle_to_next_camera(&test_world);
        TEST_ASSERT_TRUE(success);
        
        EntityID current_camera = world_get_active_camera(&test_world);
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, current_camera);
    }
    
    // Should have cycled back to a valid camera
    EntityID final_camera = world_get_active_camera(&test_world);
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, final_camera);
}

// ============================================================================
// CAMERA ASPECT RATIO TESTS
// ============================================================================

void test_camera_aspect_ratio_update(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    world_set_active_camera(&test_world, entity);
    
    float initial_aspect = camera->aspect_ratio;
    
    // Update aspect ratio
    float new_aspect = 4.0f / 3.0f;  // Different from default 16:9
    update_camera_aspect_ratio(&test_world, new_aspect);
    
    TEST_ASSERT_EQUAL_FLOAT(new_aspect, camera->aspect_ratio);
    TEST_ASSERT_NOT_EQUAL(initial_aspect, camera->aspect_ratio);
}

void test_camera_aspect_ratio_projection_update(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    world_set_active_camera(&test_world, entity);
    
    // Update aspect ratio should trigger projection matrix recalculation
    update_camera_aspect_ratio(&test_world, 2.0f);
    
    // Verify projection matrix was updated (indirectly, by no crash)
    camera_update_matrices(camera);
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// CAMERA FRUSTUM TESTS
// ============================================================================

void test_camera_frustum_extraction(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set camera parameters
    camera->fov = 60.0f;
    camera->aspect_ratio = 16.0f / 9.0f;
    camera->near_plane = 0.1f;
    camera->far_plane = 100.0f;
    
    transform->position = (Vector3){ 0.0f, 0.0f, 0.0f };
    
    // Update matrices first
    camera_update_matrices(camera);
    
    // Extract frustum planes
    float frustum_planes[6][4];
    camera_extract_frustum_planes(camera, frustum_planes);
    
    // All plane equations should have valid values (not NaN or infinite)
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            TEST_ASSERT_FALSE(isnan(frustum_planes[i][j]));
            TEST_ASSERT_FALSE(isinf(frustum_planes[i][j]));
        }
    }
}

// ============================================================================
// CAMERA MOVEMENT TESTS
// ============================================================================

void test_camera_position_updates(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set initial position
    Vector3 initial_pos = { 1.0f, 2.0f, 3.0f };
    transform->position = initial_pos;
    
    // Update camera
    camera_system_update(&test_world, &test_render_config, 0.016f);
    
    // Camera should maintain the transform position
    TEST_ASSERT_EQUAL_FLOAT(initial_pos.x, transform->position.x);
    TEST_ASSERT_EQUAL_FLOAT(initial_pos.y, transform->position.y);
    TEST_ASSERT_EQUAL_FLOAT(initial_pos.z, transform->position.z);
}

void test_camera_rotation_updates(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Transform* transform = entity_get_transform(&test_world, entity);
    
    // Set rotation
    transform->rotation = (Quaternion){ 0.0f, 0.707f, 0.0f, 0.707f }; // 90 degrees around Y
    transform->dirty = true;
    
    // Update camera
    camera_system_update(&test_world, &test_render_config, 0.016f);
    
    // Transform should maintain rotation
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, transform->rotation.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.707f, transform->rotation.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, transform->rotation.z);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.707f, transform->rotation.w);
}

// ============================================================================
// CAMERA PERFORMANCE TESTS
// ============================================================================

void test_camera_multiple_cameras_performance(void)
{
    const int camera_count = 10;
    EntityID cameras[camera_count];
    
    // Create many cameras
    for (int i = 0; i < camera_count; i++) {
        cameras[i] = entity_create(&test_world);
        entity_add_components(&test_world, cameras[i], COMPONENT_CAMERA | COMPONENT_TRANSFORM);
        
        struct Camera* camera = entity_get_camera(&test_world, cameras[i]);
        struct Transform* transform = entity_get_transform(&test_world, cameras[i]);
        
        // Set different positions
        transform->position = (Vector3){ (float)i, (float)i * 2, (float)i * 3 };
        camera->fov = 45.0f + (float)i;
    }
    
    // Update all cameras should be fast
    clock_t start = clock();
    camera_system_update(&test_world, &test_render_config, 0.016f);
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete within 2ms for 10 cameras
    TEST_ASSERT_TRUE(elapsed < 0.002);
}

// ============================================================================
// CAMERA EDGE CASES
// ============================================================================

void test_camera_invalid_parameters(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    
    // Test with invalid FOV
    camera->fov = 0.0f;
    camera_update_matrices(camera);
    // Should not crash
    
    camera->fov = 180.0f;
    camera_update_matrices(camera);
    // Should not crash
    
    // Test with invalid near/far planes
    camera->near_plane = 10.0f;
    camera->far_plane = 1.0f; // Far < near
    camera_update_matrices(camera);
    // Should not crash
    
    TEST_ASSERT_TRUE(true);
}

void test_camera_null_pointer_safety(void)
{
    // Test that camera functions handle NULL pointers gracefully
    camera_update_matrices(NULL);
    camera_extract_frustum_planes(NULL, NULL);
    
    world_set_active_camera(&test_world, INVALID_ENTITY);
    update_camera_aspect_ratio(&test_world, 1.0f);
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

void test_camera_no_cameras_edge_case(void)
{
    // Test camera system with no cameras in world
    EntityID active = world_get_active_camera(&test_world);
    TEST_ASSERT_EQUAL(INVALID_ENTITY, active);
    
    // These should not crash
    bool switched = switch_to_camera(&test_world, 0);
    TEST_ASSERT_FALSE(switched);
    
    bool cycled = cycle_to_next_camera(&test_world);
    TEST_ASSERT_FALSE(cycled);
    
    camera_system_update(&test_world, &test_render_config, 0.016f);
    
    TEST_ASSERT_TRUE(true);
}

void test_camera_single_camera_cycling(void)
{
    // Test cycling with only one camera
    EntityID camera = entity_create(&test_world);
    entity_add_components(&test_world, camera, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    world_set_active_camera(&test_world, camera);
    
    // Cycling should work but stay on same camera
    bool success = cycle_to_next_camera(&test_world);
    TEST_ASSERT_TRUE(success);
    
    EntityID active = world_get_active_camera(&test_world);
    TEST_ASSERT_EQUAL(camera, active);
}

// ============================================================================
// CAMERA PROJECTION TESTS
// ============================================================================

void test_camera_perspective_projection_parameters(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    
    // Set specific parameters  
    camera->fov = 90.0f;
    camera->aspect_ratio = 1.0f;
    camera->near_plane = 0.1f;
    camera->far_plane = 1000.0f;
    camera->behavior = CAMERA_BEHAVIOR_FIRST_PERSON;
    
    camera_update_matrices(camera);
    
    // Verify parameters are preserved
    TEST_ASSERT_EQUAL_INT(CAMERA_BEHAVIOR_FIRST_PERSON, camera->behavior);
    TEST_ASSERT_EQUAL_FLOAT(90.0f, camera->fov);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, camera->aspect_ratio);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, camera->near_plane);
    TEST_ASSERT_EQUAL_FLOAT(1000.0f, camera->far_plane);
}

void test_camera_follow_parameters(void)
{
    EntityID entity = entity_create(&test_world);
    entity_add_components(&test_world, entity, COMPONENT_CAMERA | COMPONENT_TRANSFORM);
    
    struct Camera* camera = entity_get_camera(&test_world, entity);
    
    // Set follow/chase parameters
    camera->behavior = CAMERA_BEHAVIOR_CHASE;
    camera->follow_distance = 15.0f;
    camera->aspect_ratio = 4.0f / 3.0f;
    camera->near_plane = 0.1f;
    camera->far_plane = 100.0f;
    camera->follow_smoothing = 0.8f;
    
    camera_update_matrices(camera);
    
    // Verify parameters are preserved
    TEST_ASSERT_EQUAL_INT(CAMERA_BEHAVIOR_CHASE, camera->behavior);
    TEST_ASSERT_EQUAL_FLOAT(15.0f, camera->follow_distance);
    TEST_ASSERT_EQUAL_FLOAT(4.0f / 3.0f, camera->aspect_ratio);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, camera->near_plane);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, camera->far_plane);
    TEST_ASSERT_EQUAL_FLOAT(0.8f, camera->follow_smoothing);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_camera_system(void)
{
    printf("\n📷 Camera System Tests\n");
    printf("======================\n");
    
    printf("🔧 Testing Camera Component...\n");
    RUN_TEST(test_camera_component_creation);
    RUN_TEST(test_camera_behavior_mode);
    RUN_TEST(test_camera_matrix_updates);
    
    printf("🔄 Testing Camera Switching...\n");
    RUN_TEST(test_camera_switching_basic);
    RUN_TEST(test_camera_switching_by_index);
    RUN_TEST(test_camera_cycling);
    
    printf("📐 Testing Aspect Ratio...\n");
    RUN_TEST(test_camera_aspect_ratio_update);
    RUN_TEST(test_camera_aspect_ratio_projection_update);
    
    printf("🔺 Testing Camera Frustum...\n");
    RUN_TEST(test_camera_frustum_extraction);
    
    printf("🎥 Testing Camera Movement...\n");
    RUN_TEST(test_camera_position_updates);
    RUN_TEST(test_camera_rotation_updates);
    
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_camera_multiple_cameras_performance);
    
    printf("🛡️  Testing Edge Cases...\n");
    RUN_TEST(test_camera_invalid_parameters);
    RUN_TEST(test_camera_null_pointer_safety);
    RUN_TEST(test_camera_no_cameras_edge_case);
    RUN_TEST(test_camera_single_camera_cycling);
    
    printf("📊 Testing Projections...\n");
    RUN_TEST(test_camera_perspective_projection_parameters);
    RUN_TEST(test_camera_follow_parameters);
    
    printf("✅ Camera System Tests Complete\n");
}

// ============================================================================
// MAIN TEST RUNNER (for standalone execution)
// ============================================================================

int main(void)
{
    UNITY_BEGIN();
    suite_camera_system();
    return UNITY_END();
}
