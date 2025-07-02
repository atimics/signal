/**
 * @file test_utilities.h
 * @brief Common test utilities and fixtures for the CGame test suite
 * 
 * This header provides shared utilities, fixtures, and helper functions
 * used across multiple test files in the CGame engine test suite.
 */

#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/systems.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ============================================================================
// TEST CONFIGURATION
// ============================================================================

// Common test limits
#define TEST_MAX_ENTITIES 100
#define TEST_TIMEOUT_MS 1000
#define TEST_EPSILON 0.001f
#define TEST_LARGE_EPSILON 0.01f

// Performance test thresholds
#define TEST_PERF_ENTITY_CREATION_MAX_MS 10.0
#define TEST_PERF_SYSTEM_UPDATE_MAX_MS 16.67  // 60 FPS target
#define TEST_PERF_MEMORY_ALLOC_MAX_MS 1.0

// ============================================================================
// COMMON TEST FIXTURES
// ============================================================================

/**
 * @brief Standard test world fixture
 * 
 * Creates a clean world for testing with reasonable limits.
 * Call test_world_setup() in setUp() and test_world_teardown() in tearDown().
 */
extern struct World* test_world;

/**
 * @brief Initialize a test world with standard configuration
 * @return true if successful, false otherwise
 */
bool test_world_setup(void);

/**
 * @brief Clean up the test world
 */
void test_world_teardown(void);

/**
 * @brief Create a basic entity with Transform and Physics components
 * @param world The world to create the entity in
 * @return EntityID of the created entity, or INVALID_ENTITY_ID on failure
 */
EntityID test_create_basic_entity(struct World* world);

/**
 * @brief Create a test entity with specified components
 * @param world The world to create the entity in
 * @param components Component mask for the entity
 * @return EntityID of the created entity, or INVALID_ENTITY_ID on failure
 */
EntityID test_create_entity_with_components(struct World* world, ComponentType components);

// ============================================================================
// TIMING UTILITIES
// ============================================================================

/**
 * @brief High-resolution timer for performance testing
 * @return Current time in milliseconds
 */
double test_get_time_ms(void);

/**
 * @brief Time a function call and return duration
 * @param description Description of what's being timed (for logging)
 * @param func Function to time
 * @return Duration in milliseconds
 */
double test_time_function(const char* description, void (*func)(void));

/**
 * @brief Macro to time a code block
 * Usage: TEST_TIME_BLOCK("my operation") { code }
 */
#define TEST_TIME_BLOCK(desc) \
    for (double _start = test_get_time_ms(), _duration = 0; \
         _duration == 0; \
         _duration = test_get_time_ms() - _start, \
         printf("⏱️  %s: %.2fms\n", desc, _duration))

// ============================================================================
// ASSERTION HELPERS
// ============================================================================

/**
 * @brief Assert that two Vector3 values are approximately equal
 */
#define TEST_ASSERT_VECTOR3_EQUAL(expected, actual) \
    do { \
        TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, expected.x, actual.x); \
        TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, expected.y, actual.y); \
        TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, expected.z, actual.z); \
    } while(0)

/**
 * @brief Assert that a value is within a reasonable range
 */
#define TEST_ASSERT_IN_RANGE(value, min, max) \
    do { \
        TEST_ASSERT_GREATER_OR_EQUAL(min, value); \
        TEST_ASSERT_LESS_OR_EQUAL(max, value); \
    } while(0)

/**
 * @brief Assert that a performance metric is within acceptable bounds
 */
#define TEST_ASSERT_PERFORMANCE(duration_ms, max_ms) \
    do { \
        if (duration_ms > max_ms) { \
            printf("❌ Performance regression: %.2fms > %.2fms\n", duration_ms, max_ms); \
        } \
        TEST_ASSERT_LESS_OR_EQUAL(max_ms, duration_ms); \
    } while(0)

/**
 * @brief Assert that an entity exists and has the expected components
 */
#define TEST_ASSERT_ENTITY_VALID(world, entity_id, expected_components) \
    do { \
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity_id); \
        struct Entity* entity = entity_get(world, entity_id); \
        TEST_ASSERT_NOT_NULL(entity); \
        TEST_ASSERT_EQUAL(expected_components, entity->component_mask & expected_components); \
    } while(0)

// ============================================================================
// MOCK AND STUB UTILITIES
// ============================================================================

/**
 * @brief Mock function call counter
 */
typedef struct {
    const char* function_name;
    int call_count;
    void* last_parameters;
} MockCallRecord;

/**
 * @brief Initialize mock call tracking
 */
void test_mock_init(void);

/**
 * @brief Record a mock function call
 */
void test_mock_record_call(const char* function_name, void* parameters);

/**
 * @brief Get the number of times a mock function was called
 */
int test_mock_get_call_count(const char* function_name);

/**
 * @brief Reset all mock call counters
 */
void test_mock_reset(void);

// ============================================================================
// RANDOM DATA GENERATORS
// ============================================================================

/**
 * @brief Generate a random float between min and max
 */
float test_random_float(float min, float max);

/**
 * @brief Generate a random Vector3 with components in the given range
 */
Vector3 test_random_vector3(float min, float max);

/**
 * @brief Generate a random unit vector (normalized)
 */
Vector3 test_random_unit_vector3(void);

/**
 * @brief Seed the random number generator for reproducible tests
 */
void test_seed_random(unsigned int seed);

// ============================================================================
// MEMORY TESTING UTILITIES
// ============================================================================

/**
 * @brief Track memory allocations during test execution
 */
void test_memory_tracking_start(void);

/**
 * @brief Stop memory tracking and return stats
 */
size_t test_memory_tracking_stop(void);

/**
 * @brief Check for memory leaks (call in tearDown)
 */
void test_check_memory_leaks(void);

// ============================================================================
// FILE SYSTEM UTILITIES
// ============================================================================

/**
 * @brief Create a temporary test file with specified content
 * @param filename Name of the temporary file
 * @param content Content to write to the file
 * @return true if successful, false otherwise
 */
bool test_create_temp_file(const char* filename, const char* content);

/**
 * @brief Delete a temporary test file
 * @param filename Name of the file to delete
 */
void test_delete_temp_file(const char* filename);

/**
 * @brief Check if a file exists
 * @param filename Path to the file
 * @return true if file exists, false otherwise
 */
bool test_file_exists(const char* filename);

// ============================================================================
// LOGGING AND DEBUG UTILITIES
// ============================================================================

/**
 * @brief Log levels for test output
 */
typedef enum {
    TEST_LOG_DEBUG,
    TEST_LOG_INFO,
    TEST_LOG_WARNING,
    TEST_LOG_ERROR
} TestLogLevel;

/**
 * @brief Log a message with the specified level
 */
void test_log(TestLogLevel level, const char* format, ...);

/**
 * @brief Log test progress
 */
#define TEST_LOG_PROGRESS(msg) test_log(TEST_LOG_INFO, "🔄 %s", msg)

/**
 * @brief Log test success
 */
#define TEST_LOG_SUCCESS(msg) test_log(TEST_LOG_INFO, "✅ %s", msg)

/**
 * @brief Log test failure
 */
#define TEST_LOG_FAILURE(msg) test_log(TEST_LOG_ERROR, "❌ %s", msg)

// ============================================================================
// COMPONENT TESTING UTILITIES
// ============================================================================

/**
 * @brief Verify that a component has expected default values
 */
void test_verify_component_defaults(void* component, ComponentType type);

/**
 * @brief Create a component with test data
 */
void test_populate_component(void* component, ComponentType type);

/**
 * @brief Verify component data integrity
 */
bool test_verify_component_integrity(void* component, ComponentType type);

// ============================================================================
// SYSTEM TESTING UTILITIES
// ============================================================================

/**
 * @brief Run a system update with timing
 */
double test_run_system_timed(void (*system_func)(struct World*, float), 
                            struct World* world, float delta_time);

/**
 * @brief Verify system has processed expected number of entities
 */
void test_verify_system_processing(struct World* world, ComponentType components, 
                                 int expected_count);

// ============================================================================
// INTEGRATION TEST HELPERS
// ============================================================================

/**
 * @brief Create a complete test scene with multiple entities
 */
void test_create_integration_scene(struct World* world);

/**
 * @brief Run multiple system updates in sequence
 */
void test_run_system_sequence(struct World* world, float delta_time, int iterations);

/**
 * @brief Verify full pipeline functionality
 */
bool test_verify_pipeline_integrity(struct World* world);

#endif // TEST_UTILITIES_H
