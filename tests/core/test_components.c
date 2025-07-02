/**
 * @file test_components.c
 * @brief Unit tests for ECS component system
 * 
 * Tests component addition, removal, and data management.
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
// COMPONENT ADDITION TESTS
// ============================================================================

void test_component_addition_single(void) {
    TEST_LOG_PROGRESS("Testing single component addition");
    
    EntityID entity = entity_create(test_world);
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity);
    
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM);
    
    struct Entity* entity_ptr = entity_get(test_world, entity);
    TEST_ASSERT_NOT_NULL(entity_ptr);
    TEST_ASSERT_EQUAL(COMPONENT_TRANSFORM, entity_ptr->component_mask);
    
    struct Transform* transform = entity_get_transform(test_world, entity);
    TEST_ASSERT_NOT_NULL(transform);
    
    TEST_LOG_SUCCESS("Single component added successfully");
}

void test_component_addition_multiple(void) {
    TEST_LOG_PROGRESS("Testing multiple component addition");
    
    EntityID entity = entity_create(test_world);
    ComponentType components = COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_CAMERA;
    
    entity_add_component(test_world, entity, components);
    
    struct Entity* entity_ptr = entity_get(test_world, entity);
    TEST_ASSERT_EQUAL(components, entity_ptr->component_mask);
    
    // Verify all components are accessible
    TEST_ASSERT_NOT_NULL(entity_get_transform(test_world, entity));
    TEST_ASSERT_NOT_NULL(entity_get_physics(test_world, entity));
    TEST_ASSERT_NOT_NULL(entity_get_camera(test_world, entity));
    
    TEST_LOG_SUCCESS("Multiple components added successfully");
}

void test_component_addition_incremental(void) {
    TEST_LOG_PROGRESS("Testing incremental component addition");
    
    EntityID entity = entity_create(test_world);
    
    // Add components one by one
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM);
    struct Entity* entity_ptr = entity_get(test_world, entity);
    TEST_ASSERT_EQUAL(COMPONENT_TRANSFORM, entity_ptr->component_mask);
    
    entity_add_component(test_world, entity, COMPONENT_PHYSICS);
    TEST_ASSERT_EQUAL(COMPONENT_TRANSFORM | COMPONENT_PHYSICS, entity_ptr->component_mask);
    
    entity_add_component(test_world, entity, COMPONENT_CONTROL_AUTHORITY);
    TEST_ASSERT_EQUAL(COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_CONTROL_AUTHORITY, 
                     entity_ptr->component_mask);
    
    TEST_LOG_SUCCESS("Incremental component addition works");
}

// ============================================================================
// COMPONENT REMOVAL TESTS
// ============================================================================

void test_component_removal_single(void) {
    TEST_LOG_PROGRESS("Testing single component removal");
    
    EntityID entity = entity_create(test_world);
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Entity* entity_ptr = entity_get(test_world, entity);
    TEST_ASSERT_EQUAL(COMPONENT_TRANSFORM | COMPONENT_PHYSICS, entity_ptr->component_mask);
    
    entity_remove_component(test_world, entity, COMPONENT_PHYSICS);
    TEST_ASSERT_EQUAL(COMPONENT_TRANSFORM, entity_ptr->component_mask);
    
    // Verify component is no longer accessible
    TEST_ASSERT_NOT_NULL(entity_get_transform(test_world, entity));
    TEST_ASSERT_NULL(entity_get_physics(test_world, entity));
    
    TEST_LOG_SUCCESS("Single component removed successfully");
}

void test_component_removal_multiple(void) {
    TEST_LOG_PROGRESS("Testing multiple component removal");
    
    EntityID entity = entity_create(test_world);
    ComponentType initial_components = COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_CAMERA | COMPONENT_CONTROL_AUTHORITY;
    entity_add_component(test_world, entity, initial_components);
    
    ComponentType components_to_remove = COMPONENT_PHYSICS | COMPONENT_CAMERA;
    entity_remove_component(test_world, entity, components_to_remove);
    
    struct Entity* entity_ptr = entity_get(test_world, entity);
    ComponentType expected_mask = initial_components & ~components_to_remove;
    TEST_ASSERT_EQUAL(expected_mask, entity_ptr->component_mask);
    
    // Verify correct components remain
    TEST_ASSERT_NOT_NULL(entity_get_transform(test_world, entity));
    TEST_ASSERT_NOT_NULL(entity_get_control_authority(test_world, entity));
    TEST_ASSERT_NULL(entity_get_physics(test_world, entity));
    TEST_ASSERT_NULL(entity_get_camera(test_world, entity));
    
    TEST_LOG_SUCCESS("Multiple components removed successfully");
}

// ============================================================================
// COMPONENT DATA TESTS
// ============================================================================

void test_component_data_initialization(void) {
    TEST_LOG_PROGRESS("Testing component data initialization");
    
    EntityID entity = entity_create(test_world);
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Transform* transform = entity_get_transform(test_world, entity);
    struct Physics* physics = entity_get_physics(test_world, entity);
    
    TEST_ASSERT_NOT_NULL(transform);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Verify default values (these might be zero-initialized)
    TEST_ASSERT(test_verify_component_integrity(transform, COMPONENT_TRANSFORM));
    TEST_ASSERT(test_verify_component_integrity(physics, COMPONENT_PHYSICS));
    
    TEST_LOG_SUCCESS("Component data initialized correctly");
}

void test_component_data_modification(void) {
    TEST_LOG_PROGRESS("Testing component data modification");
    
    EntityID entity = entity_create(test_world);
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Transform* transform = entity_get_transform(test_world, entity);
    struct Physics* physics = entity_get_physics(test_world, entity);
    
    // Modify component data
    transform->position = (Vector3){10.0f, 20.0f, 30.0f};
    transform->rotation = (Quaternion){0.0f, 0.0f, 0.707f, 0.707f};
    
    physics->velocity = (Vector3){5.0f, 0.0f, -5.0f};
    physics->mass = 2.5f;
    
    // Verify modifications persist
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 10.0f, transform->position.x);
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 20.0f, transform->position.y);
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 30.0f, transform->position.z);
    
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 5.0f, physics->velocity.x);
    TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 2.5f, physics->mass);
    
    TEST_LOG_SUCCESS("Component data modified correctly");
}

// ============================================================================
// COMPONENT ACCESS TESTS
// ============================================================================

void test_component_access_invalid_entity(void) {
    TEST_LOG_PROGRESS("Testing component access for invalid entity");
    
    EntityID invalid_entity = INVALID_ENTITY_ID;
    
    TEST_ASSERT_NULL(entity_get_transform(test_world, invalid_entity));
    TEST_ASSERT_NULL(entity_get_physics(test_world, invalid_entity));
    TEST_ASSERT_NULL(entity_get_camera(test_world, invalid_entity));
    
    TEST_LOG_SUCCESS("Invalid entity access handled correctly");
}

void test_component_access_nonexistent_component(void) {
    TEST_LOG_PROGRESS("Testing access to non-existent component");
    
    EntityID entity = entity_create(test_world);
    entity_add_component(test_world, entity, COMPONENT_TRANSFORM);
    
    // Try to access component that wasn't added
    TEST_ASSERT_NOT_NULL(entity_get_transform(test_world, entity));
    TEST_ASSERT_NULL(entity_get_physics(test_world, entity));
    TEST_ASSERT_NULL(entity_get_camera(test_world, entity));
    
    TEST_LOG_SUCCESS("Non-existent component access handled correctly");
}

// ============================================================================
// COMPONENT ITERATION TESTS
// ============================================================================

void test_component_iteration(void) {
    TEST_LOG_PROGRESS("Testing component iteration");
    
    const int NUM_ENTITIES = 10;
    EntityID entities[NUM_ENTITIES];
    
    // Create entities with different component combinations
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        
        if (i % 2 == 0) {
            entity_add_component(test_world, entities[i], COMPONENT_TRANSFORM);
        }
        if (i % 3 == 0) {
            entity_add_component(test_world, entities[i], COMPONENT_PHYSICS);
        }
    }
    
    // Count entities with transform component
    int transform_count = 0;
    for (uint32_t i = 0; i < test_world->entity_count; i++) {
        struct Entity* entity = &test_world->entities[i];
        if (entity->component_mask & COMPONENT_TRANSFORM) {
            transform_count++;
        }
    }
    
    // Should have 5 entities with transform (indices 0, 2, 4, 6, 8)
    TEST_ASSERT_EQUAL(5, transform_count);
    
    TEST_LOG_SUCCESS("Component iteration works correctly");
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_component_access_performance(void) {
    TEST_LOG_PROGRESS("Testing component access performance");
    
    const int NUM_ENTITIES = 50;
    EntityID entities[NUM_ENTITIES];
    
    // Create entities with components
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        entity_add_component(test_world, entities[i], COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    }
    
    // Time component access
    TEST_TIME_BLOCK("Accessing components for 50 entities") {
        for (int i = 0; i < NUM_ENTITIES; i++) {
            struct Transform* transform = entity_get_transform(test_world, entities[i]);
            struct Physics* physics = entity_get_physics(test_world, entities[i]);
            
            TEST_ASSERT_NOT_NULL(transform);
            TEST_ASSERT_NOT_NULL(physics);
            
            // Simple operation to ensure access
            transform->dirty = true;
            physics->mass += 0.1f;
        }
    }
    
    TEST_LOG_SUCCESS("Component access performance acceptable");
}

// ============================================================================
// TEST SUITE
// ============================================================================

void suite_components(void) {
    RUN_TEST(test_component_addition_single);
    RUN_TEST(test_component_addition_multiple);
    RUN_TEST(test_component_addition_incremental);
    RUN_TEST(test_component_removal_single);
    RUN_TEST(test_component_removal_multiple);
    RUN_TEST(test_component_data_initialization);
    RUN_TEST(test_component_data_modification);
    RUN_TEST(test_component_access_invalid_entity);
    RUN_TEST(test_component_access_nonexistent_component);
    RUN_TEST(test_component_iteration);
    RUN_TEST(test_component_access_performance);
}

// Stand-alone test runner
#ifdef TEST_STANDALONE
int main(void) {
    UNITY_BEGIN();
    suite_components();
    return UNITY_END();
}
#endif
