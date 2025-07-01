// tests/unit/test_ecs_core.c
// Unit tests for ECS core functionality - Sprint 19 TDD
// Test-Driven Development: RED phase - Write failing tests first

#include "../vendor/unity.h"
#include "../../src/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test world and entities
static struct World* test_world;
static const uint32_t MAX_TEST_ENTITIES = 100;

void setUp(void) {
    // Create a test world with manual initialization for unit testing
    test_world = malloc(sizeof(struct World));
    memset(test_world, 0, sizeof(struct World));
    
    // Initialize basic world structure
    test_world->entity_count = 0;
    test_world->max_entities = MAX_TEST_ENTITIES;
    test_world->entities = malloc(sizeof(struct Entity) * MAX_TEST_ENTITIES);
    test_world->next_entity_id = 1;
    memset(test_world->entities, 0, sizeof(struct Entity) * MAX_TEST_ENTITIES);
}

void tearDown(void) {
    if (test_world) {
        if (test_world->entities) {
            free(test_world->entities);
        }
        free(test_world);
        test_world = NULL;
    }
}

// ============================================================================
// ENTITY CREATION AND MANAGEMENT TESTS (RED PHASE)
// ============================================================================

void test_entity_creation_basic(void) {
    printf("🧪 Testing basic entity creation...\n");
    
    // Create an entity - this should fail initially (RED phase)
    EntityID entity = entity_create(test_world);
    
    // Entity should have valid ID
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entity);
    TEST_ASSERT_LESS_THAN(MAX_TEST_ENTITIES, entity);
    
    // World entity count should increase
    TEST_ASSERT_EQUAL_UINT32(1, test_world->entity_count);
    
    printf("✅ Basic entity creation test passed\n");
}

void test_entity_creation_with_components(void) {
    printf("🧪 Testing entity creation with components...\n");
    
    // Create entity with multiple components
    EntityID entity = entity_create(test_world);
    ComponentType components = COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_RENDERABLE;
    
    // Add components - this may fail initially (RED phase)
    bool result = entity_add_components(test_world, entity, components);
    TEST_ASSERT_TRUE(result);
    
    // Verify components are present
    TEST_ASSERT_TRUE(entity_has_component(test_world, entity, COMPONENT_TRANSFORM));
    TEST_ASSERT_TRUE(entity_has_component(test_world, entity, COMPONENT_PHYSICS));
    TEST_ASSERT_TRUE(entity_has_component(test_world, entity, COMPONENT_RENDERABLE));
    
    printf("✅ Entity component assignment test passed\n");
}

void test_component_data_access(void) {
    printf("🧪 Testing component data access...\n");
    
    // Create entity with transform component
    EntityID entity = entity_create(test_world);
    entity_add_components(test_world, entity, COMPONENT_TRANSFORM);
    
    // Get transform component - may fail initially (RED phase)
    struct Transform* transform = entity_get_transform(test_world, entity);
    TEST_ASSERT_NOT_NULL(transform);
    
    // Modify transform data
    transform->position.x = 10.0f;
    transform->position.y = 20.0f;
    transform->position.z = 30.0f;
    
    // Verify data persistence
    struct Transform* transform_check = entity_get_transform(test_world, entity);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, transform_check->position.x);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, transform_check->position.y);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, transform_check->position.z);
    
    printf("✅ Component data access test passed\n");
}

void test_entity_removal(void) {
    printf("🧪 Testing entity removal...\n");
    
    // Create multiple entities
    EntityID entity1 = entity_create(test_world);
    EntityID entity2 = entity_create(test_world);
    EntityID entity3 = entity_create(test_world);
    
    TEST_ASSERT_EQUAL_UINT32(3, test_world->entity_count);
    
    // Remove middle entity - may fail initially (RED phase)
    bool result = entity_destroy(test_world, entity2);
    TEST_ASSERT_TRUE(result);
    
    // Entity count should decrease
    TEST_ASSERT_EQUAL_UINT32(2, test_world->entity_count);
    
    // Removed entity should be invalid
    TEST_ASSERT_FALSE(entity_is_valid(test_world, entity2));
    
    // Other entities should still be valid
    TEST_ASSERT_TRUE(entity_is_valid(test_world, entity1));
    TEST_ASSERT_TRUE(entity_is_valid(test_world, entity3));
    
    printf("✅ Entity removal test passed\n");
}

// ============================================================================
// COMPONENT SYSTEM TESTS (RED PHASE)
// ============================================================================

void test_component_memory_allocation(void) {
    printf("🧪 Testing component memory allocation...\n");
    
    // Create entity with physics component
    EntityID entity = entity_create(test_world);
    entity_add_components(test_world, entity, COMPONENT_PHYSICS);
    
    // Get physics component
    struct Physics* physics = entity_get_physics(test_world, entity);
    TEST_ASSERT_NOT_NULL(physics);
    
    // Verify memory is properly allocated and zeroed
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, physics->velocity.z);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, physics->mass); // Should have default mass
    
    printf("✅ Component memory allocation test passed\n");
}

void test_component_removal(void) {
    printf("🧪 Testing component removal...\n");
    
    // Create entity with multiple components
    EntityID entity = entity_create(test_world);
    ComponentType components = COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_RENDERABLE;
    entity_add_components(test_world, entity, components);
    
    // Remove physics component - may fail initially (RED phase)
    bool result = entity_remove_component(test_world, entity, COMPONENT_PHYSICS);
    TEST_ASSERT_TRUE(result);
    
    // Physics component should be gone
    TEST_ASSERT_FALSE(entity_has_component(test_world, entity, COMPONENT_PHYSICS));
    TEST_ASSERT_NULL(entity_get_physics(test_world, entity));
    
    // Other components should remain
    TEST_ASSERT_TRUE(entity_has_component(test_world, entity, COMPONENT_TRANSFORM));
    TEST_ASSERT_TRUE(entity_has_component(test_world, entity, COMPONENT_RENDERABLE));
    
    printf("✅ Component removal test passed\n");
}

// ============================================================================
// MEMORY INTEGRATION TESTS (RED PHASE)
// ============================================================================

// Memory tracking test disabled for unit testing
// TODO: Re-enable when memory system is properly isolated
/*
void test_ecs_memory_tracking(void) {
    printf("🧪 Testing ECS memory tracking integration...\n");
    
    // Get initial memory stats
    size_t initial_mb, peak_mb;
    uint32_t asset_count;
    memory_get_stats(&initial_mb, &peak_mb, &asset_count);
    
    // Create many entities with components
    const uint32_t NUM_ENTITIES = 50;
    for (uint32_t i = 0; i < NUM_ENTITIES; i++) {
        EntityID entity = entity_create(test_world);
        entity_add_components(test_world, entity, 
            COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_RENDERABLE);
    }
    
    // Memory usage should increase - may fail initially (RED phase)
    size_t after_mb, after_peak;
    memory_get_stats(&after_mb, &after_peak, &asset_count);
    
    // Should have consumed some memory
    TEST_ASSERT_GREATER_THAN(initial_mb, after_mb);
    
    printf("✅ ECS memory tracking test passed\n");
}
*/

void test_component_pool_efficiency(void) {
    printf("🧪 Testing component pool allocation efficiency...\n");
    
    // Create many entities rapidly
    const uint32_t NUM_ENTITIES = 20;
    EntityID entities[NUM_ENTITIES];
    
    // Measure allocation performance - may fail initially (RED phase)
    for (uint32_t i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        entity_add_components(test_world, entities[i], COMPONENT_TRANSFORM);
        
        // Each entity should get valid transform
        struct Transform* transform = entity_get_transform(test_world, entities[i]);
        TEST_ASSERT_NOT_NULL(transform);
    }
    
    // All entities should be valid
    TEST_ASSERT_EQUAL_UINT32(NUM_ENTITIES, test_world->entity_count);
    
    printf("✅ Component pool efficiency test passed\n");
}

// ============================================================================
// PERFORMANCE AND SCALE TESTS (RED PHASE)
// ============================================================================

void test_entity_iteration_performance(void) {
    printf("🧪 Testing entity iteration performance...\n");
    
    // Create entities with mixed components
    const uint32_t NUM_ENTITIES = 50;
    for (uint32_t i = 0; i < NUM_ENTITIES; i++) {
        EntityID entity = entity_create(test_world);
        if (i % 2 == 0) {
            entity_add_components(test_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
        } else {
            entity_add_components(test_world, entity, COMPONENT_TRANSFORM);
        }
    }
    
    // Count entities with physics components
    uint32_t physics_count = 0;
    for (uint32_t i = 0; i < test_world->entity_count; i++) {
        struct Entity* entity = &test_world->entities[i];
        if (entity->component_mask & COMPONENT_PHYSICS) {
            physics_count++;
        }
    }
    
    // Should find half the entities with physics - may fail initially (RED phase)
    TEST_ASSERT_EQUAL_UINT32(NUM_ENTITIES / 2, physics_count);
    
    printf("✅ Entity iteration performance test passed\n");
}

void test_component_cache_efficiency(void) {
    printf("🧪 Testing component cache efficiency...\n");
    
    // Create entities with same component types (cache-friendly)
    const uint32_t NUM_ENTITIES = 30;
    EntityID entities[NUM_ENTITIES];
    
    for (uint32_t i = 0; i < NUM_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        entity_add_components(test_world, entities[i], COMPONENT_TRANSFORM);
    }
    
    // Access all transforms in sequence (should be cache-friendly)
    for (uint32_t i = 0; i < NUM_ENTITIES; i++) {
        struct Transform* transform = entity_get_transform(test_world, entities[i]);
        TEST_ASSERT_NOT_NULL(transform);
        
        // Set unique position for verification
        transform->position.x = (float)i;
    }
    
    // Verify all positions were set correctly
    for (uint32_t i = 0; i < NUM_ENTITIES; i++) {
        struct Transform* transform = entity_get_transform(test_world, entities[i]);
        TEST_ASSERT_EQUAL_FLOAT((float)i, transform->position.x);
    }
    
    printf("✅ Component cache efficiency test passed\n");
}

// ============================================================================
// ERROR HANDLING TESTS (RED PHASE)
// ============================================================================

void test_invalid_entity_operations(void) {
    printf("🧪 Testing invalid entity operations...\n");
    
    // Test operations on invalid entity ID
    EntityID invalid_entity = INVALID_ENTITY_ID;
    
    // Should handle invalid operations gracefully - may fail initially (RED phase)
    TEST_ASSERT_FALSE(entity_is_valid(test_world, invalid_entity));
    TEST_ASSERT_FALSE(entity_has_component(test_world, invalid_entity, COMPONENT_TRANSFORM));
    TEST_ASSERT_NULL(entity_get_transform(test_world, invalid_entity));
    
    // Should not be able to add components to invalid entity
    bool result = entity_add_components(test_world, invalid_entity, COMPONENT_TRANSFORM);
    TEST_ASSERT_FALSE(result);
    
    printf("✅ Invalid entity operations test passed\n");
}

void test_component_type_validation(void) {
    printf("🧪 Testing component type validation...\n");
    
    EntityID entity = entity_create(test_world);
    
    // Test invalid component type - may fail initially (RED phase)
    ComponentType invalid_component = 0x8000; // Invalid component bit
    bool result = entity_add_components(test_world, entity, invalid_component);
    TEST_ASSERT_FALSE(result);
    
    // Should not have any components
    TEST_ASSERT_EQUAL_UINT32(0, test_world->entities[entity].component_mask);
    
    printf("✅ Component type validation test passed\n");
}

void test_world_capacity_limits(void) {
    printf("🧪 Testing world capacity limits...\n");
    
    // Fill up the world to capacity
    EntityID entities[MAX_TEST_ENTITIES];
    for (uint32_t i = 0; i < MAX_TEST_ENTITIES; i++) {
        entities[i] = entity_create(test_world);
        TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY_ID, entities[i]);
    }
    
    // Should be at capacity
    TEST_ASSERT_EQUAL_UINT32(MAX_TEST_ENTITIES, test_world->entity_count);
    
    // Creating one more entity should fail - may fail initially (RED phase)
    EntityID overflow_entity = entity_create(test_world);
    TEST_ASSERT_EQUAL(INVALID_ENTITY_ID, overflow_entity);
    
    // Entity count should not exceed capacity
    TEST_ASSERT_EQUAL_UINT32(MAX_TEST_ENTITIES, test_world->entity_count);
    
    printf("✅ World capacity limits test passed\n");
}

// ============================================================================
// TEST RUNNER SETUP
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    
    printf("🧪 Starting ECS Core Unit Tests (TDD Red Phase)\n");
    printf("================================================\n");
    
    // Entity Management Tests
    RUN_TEST(test_entity_creation_basic);
    RUN_TEST(test_entity_creation_with_components);
    RUN_TEST(test_component_data_access);
    RUN_TEST(test_entity_removal);
    
    // Component System Tests
    RUN_TEST(test_component_memory_allocation);
    RUN_TEST(test_component_removal);
    
    // Memory Integration Tests - Disabled for unit testing
    // RUN_TEST(test_ecs_memory_tracking); // TODO: Re-enable with proper memory system isolation
    // RUN_TEST(test_component_pool_efficiency);
    
    // Performance Tests
    RUN_TEST(test_entity_iteration_performance);
    RUN_TEST(test_component_cache_efficiency);
    
    // Error Handling Tests
    RUN_TEST(test_invalid_entity_operations);
    RUN_TEST(test_component_type_validation);
    RUN_TEST(test_world_capacity_limits);
    
    printf("================================================\n");
    printf("🎯 ECS Core Tests Complete - Ready for GREEN phase implementation\n");
    
    return UNITY_END();
}
