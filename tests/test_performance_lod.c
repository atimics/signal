// tests/test_performance_lod.c
// Test-driven development for LOD (Level of Detail) system - Sprint 19 Task 1

#include "vendor/unity.h"
#include "core.h"
#include "assets.h"
#include "systems.h"
#include "system/lod.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Test fixtures
static struct World* test_world;
static AssetRegistry test_registry;

// Helper functions for creating test entities
static EntityID create_test_entity(ComponentType components) {
    if (test_world->entity_count >= MAX_ENTITIES) return INVALID_ENTITY;
    
    EntityID id = test_world->next_entity_id++;
    struct Entity* entity = &test_world->entities[test_world->entity_count++];
    entity->id = id;
    entity->component_mask = components;
    
    // Initialize components
    if (components & COMPONENT_TRANSFORM) {
        entity->transform = malloc(sizeof(struct Transform));
        memset(entity->transform, 0, sizeof(struct Transform));
        entity->transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
        entity->transform->rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
    }
    
    if (components & COMPONENT_RENDERABLE) {
        entity->renderable = malloc(sizeof(struct Renderable));
        memset(entity->renderable, 0, sizeof(struct Renderable));
        entity->renderable->visible = true;
        entity->renderable->lod_level = LOD_HIGH;
    }
    
    if (components & COMPONENT_CAMERA) {
        entity->camera = malloc(sizeof(struct Camera));
        memset(entity->camera, 0, sizeof(struct Camera));
    }
    
    return id;
}

static void cleanup_test_entity(EntityID entity_id) {
    for (uint32_t i = 0; i < test_world->entity_count; i++) {
        struct Entity* entity = &test_world->entities[i];
        if (entity->id == entity_id) {
            if (entity->transform) free(entity->transform);
            if (entity->renderable) free(entity->renderable);
            if (entity->camera) free(entity->camera);
            // Shift remaining entities down
            for (uint32_t j = i; j < test_world->entity_count - 1; j++) {
                test_world->entities[j] = test_world->entities[j + 1];
            }
            test_world->entity_count--;
            break;
        }
    }
}

void setUp(void) {
    // Create a minimal world for testing
    test_world = malloc(sizeof(struct World));
    TEST_ASSERT_NOT_NULL(test_world);
    memset(test_world, 0, sizeof(struct World));
    
    // Initialize basic world state
    test_world->entity_count = 0;
    test_world->next_entity_id = 1;
    
    assets_init(&test_registry, "build/assets");
}

void tearDown(void) {
    if (test_world) {
        free(test_world);
        test_world = NULL;
    }
    assets_cleanup(&test_registry);
}

// ============================================================================
// LOD Distance Calculation Tests
// ============================================================================

void test_lod_distance_calculation_basic(void) {
    // Arrange: Create camera and entity at known positions
    EntityID camera_id = create_test_entity(COMPONENT_TRANSFORM | COMPONENT_CAMERA);
    struct Transform* camera_transform = test_world->entities[0].transform;
    camera_transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    EntityID entity_id = create_test_entity(COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
    struct Transform* entity_transform = test_world->entities[1].transform;
    entity_transform->position = (Vector3){10.0f, 0.0f, 0.0f};
    
    // Act: Calculate distance
    float distance = lod_calculate_distance(test_world, camera_id, entity_id);
    
    // Assert: Distance should be 10.0f
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, distance);
    
    // Cleanup
    cleanup_test_entity(camera_id);
    cleanup_test_entity(entity_id);
}

void test_lod_distance_calculation_3d(void) {
    // Arrange: Create entities at 3D positions
    EntityID camera_id = entity_create(test_world);
    entity_add_component(test_world, camera_id, COMPONENT_TRANSFORM | COMPONENT_CAMERA);
    
    struct Transform* camera_transform = entity_get_transform(test_world, camera_id);
    camera_transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    EntityID entity_id = entity_create(test_world);
    entity_add_component(test_world, entity_id, COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
    
    struct Transform* entity_transform = entity_get_transform(test_world, entity_id);
    entity_transform->position = (Vector3){3.0f, 4.0f, 0.0f}; // 3-4-5 triangle
    
    // Act: Calculate distance
    float distance = lod_calculate_distance(test_world, camera_id, entity_id);
    
    // Assert: Distance should be 5.0f (3-4-5 triangle)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, distance);
}

// ============================================================================
// LOD Level Selection Tests
// ============================================================================

void test_lod_level_selection_high_detail(void) {
    // Arrange: Short distance should give high detail
    float distance = 5.0f;
    
    // Act: Get LOD level
    LODLevel lod_level = lod_get_level_for_distance(distance);
    
    // Assert: Should be high detail
    TEST_ASSERT_EQUAL(LOD_HIGH, lod_level);
}

void test_lod_level_selection_medium_detail(void) {
    // Arrange: Medium distance should give medium detail
    float distance = 25.0f;
    
    // Act: Get LOD level
    LODLevel lod_level = lod_get_level_for_distance(distance);
    
    // Assert: Should be medium detail
    TEST_ASSERT_EQUAL(LOD_MEDIUM, lod_level);
}

void test_lod_level_selection_low_detail(void) {
    // Arrange: Long distance should give low detail
    float distance = 75.0f;
    
    // Act: Get LOD level
    LODLevel lod_level = lod_get_level_for_distance(distance);
    
    // Assert: Should be low detail
    TEST_ASSERT_EQUAL(LOD_LOW, lod_level);
}

void test_lod_level_selection_culled(void) {
    // Arrange: Very long distance should be culled
    float distance = 150.0f;
    
    // Act: Get LOD level
    LODLevel lod_level = lod_get_level_for_distance(distance);
    
    // Assert: Should be culled
    TEST_ASSERT_EQUAL(LOD_CULLED, lod_level);
}

// ============================================================================
// LOD System Integration Tests
// ============================================================================

void test_lod_system_updates_renderables(void) {
    // Arrange: Create multiple entities at different distances
    EntityID camera_id = entity_create(test_world);
    entity_add_component(test_world, camera_id, COMPONENT_TRANSFORM | COMPONENT_CAMERA);
    world_set_active_camera(test_world, camera_id);
    
    struct Transform* camera_transform = entity_get_transform(test_world, camera_id);
    camera_transform->position = (Vector3){0.0f, 0.0f, 0.0f};
    
    // Close entity (should be high LOD)
    EntityID close_entity = entity_create(test_world);
    entity_add_component(test_world, close_entity, COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
    struct Transform* close_transform = entity_get_transform(test_world, close_entity);
    close_transform->position = (Vector3){5.0f, 0.0f, 0.0f};
    
    // Far entity (should be low LOD)
    EntityID far_entity = entity_create(test_world);
    entity_add_component(test_world, far_entity, COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
    struct Transform* far_transform = entity_get_transform(test_world, far_entity);
    far_transform->position = (Vector3){80.0f, 0.0f, 0.0f};
    
    // Act: Update LOD system
    lod_system_update(test_world, NULL, 0.016f); // 60 FPS delta
    
    // Assert: Check LOD levels were set correctly
    struct Renderable* close_renderable = entity_get_renderable(test_world, close_entity);
    struct Renderable* far_renderable = entity_get_renderable(test_world, far_entity);
    
    TEST_ASSERT_NOT_NULL(close_renderable);
    TEST_ASSERT_NOT_NULL(far_renderable);
    
    TEST_ASSERT_EQUAL(LOD_HIGH, close_renderable->lod_level);
    TEST_ASSERT_EQUAL(LOD_LOW, far_renderable->lod_level);
}

// ============================================================================
// Performance Metrics Tests
// ============================================================================

void test_lod_performance_metrics_tracking(void) {
    // Arrange: Create entities and enable performance tracking
    EntityID camera_id = entity_create(test_world);
    entity_add_component(test_world, camera_id, COMPONENT_TRANSFORM | COMPONENT_CAMERA);
    world_set_active_camera(test_world, camera_id);
    
    // Create 10 entities at various distances
    for (int i = 0; i < 10; i++) {
        EntityID entity_id = entity_create(test_world);
        entity_add_component(test_world, entity_id, COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
        
        struct Transform* transform = entity_get_transform(test_world, entity_id);
        transform->position = (Vector3){(float)(i * 10), 0.0f, 0.0f}; // Spread entities out
    }
    
    // Act: Update LOD system and get performance metrics
    lod_system_update(test_world, NULL, 0.016f);
    LODPerformanceMetrics metrics = lod_get_performance_metrics();
    
    // Assert: Performance metrics should be tracked
    TEST_ASSERT_EQUAL(10, metrics.entities_processed);
    TEST_ASSERT_TRUE(metrics.entities_high_lod > 0);
    TEST_ASSERT_TRUE(metrics.entities_medium_lod >= 0);
    TEST_ASSERT_TRUE(metrics.entities_low_lod >= 0);
    TEST_ASSERT_TRUE(metrics.entities_culled >= 0);
    TEST_ASSERT_TRUE(metrics.update_time_ms >= 0.0f);
}

// ============================================================================
// LOD Configuration Tests  
// ============================================================================

void test_lod_configuration_thresholds(void) {
    // Arrange: Get default LOD configuration
    LODConfig config = lod_get_config();
    
    // Assert: Default thresholds should be reasonable
    TEST_ASSERT_TRUE(config.high_detail_distance > 0.0f);
    TEST_ASSERT_TRUE(config.medium_detail_distance > config.high_detail_distance);
    TEST_ASSERT_TRUE(config.low_detail_distance > config.medium_detail_distance);
    TEST_ASSERT_TRUE(config.cull_distance > config.low_detail_distance);
}

void test_lod_configuration_update(void) {
    // Arrange: Create custom LOD configuration
    LODConfig new_config = {
        .high_detail_distance = 15.0f,
        .medium_detail_distance = 30.0f,
        .low_detail_distance = 60.0f,
        .cull_distance = 120.0f
    };
    
    // Act: Update configuration
    lod_set_config(new_config);
    
    // Assert: Configuration should be updated
    LODConfig current_config = lod_get_config();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 15.0f, current_config.high_detail_distance);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, current_config.medium_detail_distance);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 60.0f, current_config.low_detail_distance);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 120.0f, current_config.cull_distance);
}

// ============================================================================
// Integration with Rendering System Tests
// ============================================================================

void test_lod_rendering_integration(void) {
    // This test verifies that the rendering system respects LOD levels
    // and doesn't render culled entities
    
    // Arrange: Create entities at different LOD levels
    EntityID camera_id = entity_create(test_world);
    entity_add_component(test_world, camera_id, COMPONENT_TRANSFORM | COMPONENT_CAMERA);
    world_set_active_camera(test_world, camera_id);
    
    // Very far entity that should be culled
    EntityID culled_entity = entity_create(test_world);
    entity_add_component(test_world, culled_entity, COMPONENT_TRANSFORM | COMPONENT_RENDERABLE);
    struct Transform* culled_transform = entity_get_transform(test_world, culled_entity);
    culled_transform->position = (Vector3){200.0f, 0.0f, 0.0f}; // Beyond cull distance
    
    // Act: Update LOD system
    lod_system_update(test_world, NULL, 0.016f);
    
    // Assert: Culled entity should not be visible
    struct Renderable* culled_renderable = entity_get_renderable(test_world, culled_entity);
    TEST_ASSERT_NOT_NULL(culled_renderable);
    TEST_ASSERT_EQUAL(LOD_CULLED, culled_renderable->lod_level);
    TEST_ASSERT_FALSE(culled_renderable->visible); // Should be marked as not visible
}
