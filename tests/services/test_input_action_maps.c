/**
 * @file test_input_action_maps.c
 * @brief Tests for input action mapping system (Sprint 25)
 */

#include "../vendor/unity.h"
#include "../../src/services/input_action_maps.h"
#include "../../src/services/input_service.h"
#include "../../src/hal/input_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

void test_action_map_collection_creation(void) {
    printf("🧪 Testing action map collection creation...\n");
    
    ActionMapCollection* collection = action_map_collection_create();
    TEST_ASSERT_NOT_NULL(collection);
    TEST_ASSERT_EQUAL_INT(0, collection->map_count);
    
    action_map_collection_destroy(collection);
    
    printf("✅ Action map collection creation test passed\n");
}

void test_action_map_manager_creation(void) {
    printf("🧪 Testing action map manager creation...\n");
    
    ActionMapManager* manager = action_map_manager_create();
    TEST_ASSERT_NOT_NULL(manager);
    TEST_ASSERT_NOT_NULL(manager->load_from_file);
    TEST_ASSERT_NOT_NULL(manager->save_to_file);
    TEST_ASSERT_NOT_NULL(manager->add_map);
    TEST_ASSERT_NOT_NULL(manager->add_binding);
    
    action_map_manager_destroy(manager);
    
    printf("✅ Action map manager creation test passed\n");
}

void test_action_map_add_map(void) {
    printf("🧪 Testing action map addition...\n");
    
    ActionMapManager* manager = action_map_manager_create();
    TEST_ASSERT_NOT_NULL(manager);
    
    // Add a map
    bool success = manager->add_map(manager, "test_map");
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL_INT(1, manager->collection->map_count);
    TEST_ASSERT_EQUAL_STRING("test_map", manager->collection->maps[0].name);
    TEST_ASSERT_TRUE(manager->collection->maps[0].is_active);
    TEST_ASSERT_EQUAL_INT(0, manager->collection->maps[0].binding_count);
    
    action_map_manager_destroy(manager);
    
    printf("✅ Action map addition test passed\n");
}

void test_action_map_add_binding(void) {
    printf("🧪 Testing action map binding addition...\n");
    
    ActionMapManager* manager = action_map_manager_create();
    TEST_ASSERT_NOT_NULL(manager);
    
    // Add a map first
    manager->add_map(manager, "flight_test");
    
    // Add a binding
    bool success = manager->add_binding(manager, "flight_test", 
                                       INPUT_ACTION_THRUST_FORWARD, 
                                       INPUT_DEVICE_KEYBOARD, 
                                       87, 0); // W key
    TEST_ASSERT_TRUE(success);
    
    // Check the binding was added
    ActionMap* map = &manager->collection->maps[0];
    TEST_ASSERT_EQUAL_INT(1, map->binding_count);
    TEST_ASSERT_EQUAL_INT(INPUT_ACTION_THRUST_FORWARD, map->bindings[0].action);
    TEST_ASSERT_EQUAL_INT(INPUT_DEVICE_KEYBOARD, map->bindings[0].device_type);
    TEST_ASSERT_EQUAL_INT(87, map->bindings[0].key_or_button);
    TEST_ASSERT_TRUE(map->bindings[0].is_active);
    
    action_map_manager_destroy(manager);
    
    printf("✅ Action map binding addition test passed\n");
}

void test_action_map_load_from_file_stub(void) {
    printf("🧪 Testing action map file loading (stub)...\n");
    
    ActionMapManager* manager = action_map_manager_create();
    TEST_ASSERT_NOT_NULL(manager);
    
    // Test loading from a file (stub implementation)
    bool loaded = manager->load_from_file(manager, "test_config.json");
    TEST_ASSERT_TRUE(loaded);
    
    // The stub implementation should add a default flight map
    TEST_ASSERT_EQUAL_INT(1, manager->collection->map_count);
    TEST_ASSERT_EQUAL_STRING("flight", manager->collection->maps[0].name);
    
    action_map_manager_destroy(manager);
    
    printf("✅ Action map file loading test passed\n");
}

void test_action_maps_load_and_apply_with_null_service(void) {
    printf("🧪 Testing action_maps_load_and_apply with null service...\n");
    
    bool result = action_maps_load_and_apply(NULL, "test_config.json");
    TEST_ASSERT_FALSE(result);
    
    printf("✅ Null service test passed\n");
}

void test_action_maps_load_and_apply_integration(void) {
    printf("🧪 Testing action_maps_load_and_apply integration...\n");
    
    // Create a mock input service
    InputService* service = input_service_create();
    TEST_ASSERT_NOT_NULL(service);
    
    // Test the legacy compatibility function
    bool result = action_maps_load_and_apply(service, "assets/config/action_maps.json");
    TEST_ASSERT_TRUE(result);
    
    input_service_destroy(service);
    
    printf("✅ Integration test passed\n");
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🚀 Running Action Maps Tests (Sprint 25)\n");
    printf("=========================================\n");
    
    RUN_TEST(test_action_map_collection_creation);
    RUN_TEST(test_action_map_manager_creation);
    RUN_TEST(test_action_map_add_map);
    RUN_TEST(test_action_map_add_binding);
    RUN_TEST(test_action_map_load_from_file_stub);
    RUN_TEST(test_action_maps_load_and_apply_with_null_service);
    RUN_TEST(test_action_maps_load_and_apply_integration);
    
    printf("=========================================\n");
    
    return UNITY_END();
}
