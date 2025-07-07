// tests/test_performance_lod_simple.c
// Simplified LOD system tests - Sprint 19 Task 1

#include "vendor/unity.h"
#include "system/lod.h"
#include <math.h>

void setUp(void) {
    lod_system_init();
}

void tearDown(void) {
    lod_system_cleanup();
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
    float distance = 35.0f;
    
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
    float distance = 250.0f;
    
    // Act: Get LOD level
    LODLevel lod_level = lod_get_level_for_distance(distance);
    
    // Assert: Should be culled
    TEST_ASSERT_EQUAL(LOD_CULLED, lod_level);
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
    
    // Reset to defaults for other tests
    lod_reset_config();
}

// ============================================================================
// Performance Metrics Tests
// ============================================================================

void test_lod_performance_metrics_initialization(void) {
    // Act: Get initial performance metrics
    LODPerformanceMetrics metrics = lod_get_performance_metrics();
    
    // Assert: Initial metrics should be zero
    TEST_ASSERT_EQUAL(0, metrics.entities_processed);
    TEST_ASSERT_EQUAL(0, metrics.entities_high_lod);
    TEST_ASSERT_EQUAL(0, metrics.entities_medium_lod);
    TEST_ASSERT_EQUAL(0, metrics.entities_low_lod);
    TEST_ASSERT_EQUAL(0, metrics.entities_culled);
    TEST_ASSERT_TRUE(metrics.update_time_ms >= 0.0f);
}

// Test runner main function
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_lod_level_selection_high_detail);
    RUN_TEST(test_lod_level_selection_medium_detail);
    RUN_TEST(test_lod_level_selection_low_detail);
    RUN_TEST(test_lod_level_selection_culled);
    RUN_TEST(test_lod_configuration_thresholds);
    RUN_TEST(test_lod_configuration_update);
    RUN_TEST(test_lod_performance_metrics_initialization);
    
    return UNITY_END();
}
