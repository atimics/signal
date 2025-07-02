/**
 * @file test_world.c
 * @brief Unit tests for ECS world management
 * 
 * Tests the core World structure and entity management functionality.
 */

#include "../support/test_utilities.h"
#include "../../src/core.h"

void setUp(void) {
    test_world_setup();
}

void tearDown(void) {
    test_world_teardown();
}

// ============================================================================
// WORLD INITIALIZATION TESTS
// ============================================================================

void test_world_initialization(void) {
    TEST_LOG_PROGRESS("Testing world initialization");
    
    TEST_ASSERT_NOT_NULL(test_world);
    TEST_ASSERT_EQUAL(0, test_world->entity_count);
    TEST_ASSERT_EQUAL(TEST_MAX_ENTITIES, test_world->max_entities);
    TEST_ASSERT_NOT_NULL(test_world->entities);
    
    TEST_LOG_SUCCESS("World initialized correctly");
}

void test_world_destruction(void) {
    TEST_LOG_PROGRESS("Testing world destruction");
    
    // Create some entities first
    EntityID entity1 = entity_create(test_world);
    EntityID entity2 = entity_create(test_world);
    
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity1);
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity2);
    TEST_ASSERT_EQUAL(2, test_world->entity_count);
    
    // Destroy world (done in tearDown)
    test_world_teardown();
    
    // Reinitialize for next test
    test_world_setup();
    
    TEST_LOG_SUCCESS("World destroyed without errors");
}

// ============================================================================
// ENTITY MANAGEMENT TESTS
// ============================================================================

void test_entity_creation_basic(void) {
    TEST_LOG_PROGRESS("Testing basic entity creation");
    
    EntityID entity = entity_create(test_world);
    
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity);
    TEST_ASSERT_EQUAL(1, test_world->entity_count);
    
    struct Entity* entity_ptr = entity_get(test_world, entity);
    TEST_ASSERT_NOT_NULL(entity_ptr);
    TEST_ASSERT_EQUAL(0, entity_ptr->component_mask);
    
    TEST_LOG_SUCCESS("Entity created successfully");
}

void test_entity_creation_multiple(void) {
    TEST_LOG_PROGRESS("Testing multiple entity creation");
    
    const int NUM_ENTITIES = 10;
    EntityID entities[NUM_ENTITIES];
    
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entities[i]);
    }
    
    TEST_ASSERT_EQUAL(NUM_ENTITIES, test_world->entity_count);
    
    // Verify all entities are unique
    for (int i = 0; i < NUM_ENTITIES; i++) {
        for (int j = i + 1; j < NUM_ENTITIES; j++) {
            TEST_ASSERT_NOT_EQUAL(entities[i], entities[j]);
        }
    }
    
    TEST_LOG_SUCCESS("Multiple entities created successfully");
}

void test_entity_creation_limit(void) {
    TEST_LOG_PROGRESS("Testing entity creation limit");
    
    // Create entities up to the limit
    for (uint32_t i = 0; i < test_world->max_entities; i++) {
        EntityID entity = entity_create(test_world);
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity);
    }
    
    TEST_ASSERT_EQUAL(test_world->max_entities, test_world->entity_count);
    
    // Try to create one more - should fail
    EntityID overflow_entity = entity_create(test_world);
    TEST_ASSERT_EQUAL(INVALID_ENTITY_ID, overflow_entity);
    
    TEST_LOG_SUCCESS("Entity limit enforced correctly");
}

void test_entity_removal(void) {
    TEST_LOG_PROGRESS("Testing entity removal");
    
    EntityID entity1 = entity_create(test_world);
    EntityID entity2 = entity_create(test_world);
    EntityID entity3 = entity_create(test_world);
    
    TEST_ASSERT_EQUAL(3, test_world->entity_count);
    
    // Remove middle entity
    entity_destroy(test_world, entity2);
    TEST_ASSERT_EQUAL(2, test_world->entity_count);
    
    // Verify entity is no longer accessible
    struct Entity* removed_entity = entity_get(test_world, entity2);
    TEST_ASSERT_NULL(removed_entity);
    
    // Verify other entities still exist
    TEST_ASSERT_NOT_NULL(entity_get(test_world, entity1));
    TEST_ASSERT_NOT_NULL(entity_get(test_world, entity3));
    
    TEST_LOG_SUCCESS("Entity removed successfully");
}

// ============================================================================
// WORLD CAPACITY TESTS
// ============================================================================

void test_world_capacity_management(void) {
    TEST_LOG_PROGRESS("Testing world capacity management");
    
    uint32_t initial_capacity = test_world->max_entities;
    
    // Fill world to capacity
    for (uint32_t i = 0; i < initial_capacity; i++) {
        EntityID entity = entity_create(test_world);
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity);
    }
    
    // Verify capacity is reached
    TEST_ASSERT_EQUAL(initial_capacity, test_world->entity_count);
    
    // Remove some entities
    uint32_t entities_to_remove = 5;
    for (uint32_t i = 0; i < entities_to_remove; i++) {
        entity_destroy(test_world, i);
    }
    
    TEST_ASSERT_EQUAL(initial_capacity - entities_to_remove, test_world->entity_count);
    
    // Should be able to create new entities now
    EntityID new_entity = entity_create(test_world);
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, new_entity);
    
    TEST_LOG_SUCCESS("World capacity managed correctly");
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_world_entity_creation_performance(void) {
    TEST_LOG_PROGRESS("Testing entity creation performance");
    
    const int NUM_ENTITIES = 100;
    
    TEST_TIME_BLOCK("Creating 100 entities") {
        for (int i = 0; i < NUM_ENTITIES; i++) {
            EntityID entity = entity_create(test_world);
            TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity);
        }
    }
    
    TEST_LOG_SUCCESS("Entity creation performance acceptable");
}

void test_world_entity_lookup_performance(void) {
    TEST_LOG_PROGRESS("Testing entity lookup performance");
    
    const int NUM_ENTITIES = 50;
    EntityID entities[NUM_ENTITIES];
    
    // Create entities
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
    }
    
    // Time entity lookups
    TEST_TIME_BLOCK("Looking up 50 entities") {
        for (int i = 0; i < NUM_ENTITIES; i++) {
            struct Entity* entity = entity_get(test_world, entities[i]);
            TEST_ASSERT_NOT_NULL(entity);
        }
    }
    
    TEST_LOG_SUCCESS("Entity lookup performance acceptable");
}

// ============================================================================
// TEST SUITE
// ============================================================================

void suite_world(void) {
    RUN_TEST(test_world_initialization);
    RUN_TEST(test_world_destruction);
    RUN_TEST(test_entity_creation_basic);
    RUN_TEST(test_entity_creation_multiple);
    RUN_TEST(test_entity_creation_limit);
    RUN_TEST(test_entity_removal);
    RUN_TEST(test_world_capacity_management);
    RUN_TEST(test_world_entity_creation_performance);
    RUN_TEST(test_world_entity_lookup_performance);
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_world();
    return UNITY_END();
}
#endif
