/**
 * @file test_ship_assembly.c
 * @brief Comprehensive tests for ship design and assembly system
 */

#include "unity.h"
#include "../src/ship_assembly.h"
#include "../src/system/ode_physics.h"
#include "../support/test_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test fixtures
static ShipPartLibrary test_library;
static struct World test_world;
static ODEPhysicsSystem ode_system;

void setUp(void) {
    // Initialize test world
    world_init(&test_world);
    
    // Initialize part library
    ship_parts_init(&test_library);
    
    // Initialize ODE physics system
    ode_physics_init(&ode_system, 100);
    
    printf("🧪 Ship Assembly Test Setup Complete\n");
}

void tearDown(void) {
    // Cleanup
    ship_parts_cleanup(&test_library);
    ode_physics_shutdown(&ode_system);
    world_destroy(&test_world);
    
    printf("🧽 Ship Assembly Test Cleanup Complete\n");
}

// ============================================================================
// PART LIBRARY TESTS
// ============================================================================

void test_part_library_initialization(void) {
    ShipPartLibrary library;
    
    TEST_ASSERT_TRUE(ship_parts_init(&library));
    TEST_ASSERT_NOT_NULL(library.parts);
    TEST_ASSERT_EQUAL_INT(0, library.num_parts);
    TEST_ASSERT_EQUAL_INT(128, library.capacity);
    TEST_ASSERT_NOT_NULL(library.parts_by_category);
    TEST_ASSERT_NOT_NULL(library.counts_by_category);
    
    ship_parts_cleanup(&library);
}

void test_part_library_null_handling(void) {
    TEST_ASSERT_FALSE(ship_parts_init(NULL));
    
    ShipPartLibrary* null_lib = NULL;
    ship_parts_cleanup(null_lib); // Should not crash
}

void test_create_test_hull_part(void) {
    // Create a test hull part programmatically
    ShipPart* hull = calloc(1, sizeof(ShipPart));
    TEST_ASSERT_NOT_NULL(hull);
    
    hull->part_id = strdup("test_hull_basic");
    hull->display_name = strdup("Basic Test Hull");
    hull->category = PART_HULL;
    hull->mass = 500.0f;
    hull->center_of_mass = (Vector3){0.0f, 0.0f, 0.0f};
    hull->drag_coefficient = 0.2f;
    hull->mesh_name = strdup("hull_basic_mesh");
    hull->material_name = strdup("hull_metal");
    
    // Add attachment nodes
    hull->num_attachments = 4;
    hull->attachment_nodes = calloc(4, sizeof(AttachmentNode));
    
    // Top node
    hull->attachment_nodes[0] = (AttachmentNode){
        .position = {0.0f, 1.0f, 0.0f},
        .direction = {0.0f, 1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false,
        .connection_id = NULL
    };
    
    // Bottom node  
    hull->attachment_nodes[1] = (AttachmentNode){
        .position = {0.0f, -1.0f, 0.0f},
        .direction = {0.0f, -1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false,
        .connection_id = NULL
    };
    
    // Side nodes for radial attachment
    hull->attachment_nodes[2] = (AttachmentNode){
        .position = {1.0f, 0.0f, 0.0f},
        .direction = {1.0f, 0.0f, 0.0f},
        .type = ATTACH_RADIAL,
        .size = 0.5f,
        .occupied = false,
        .connection_id = NULL
    };
    
    hull->attachment_nodes[3] = (AttachmentNode){
        .position = {-1.0f, 0.0f, 0.0f},
        .direction = {-1.0f, 0.0f, 0.0f},
        .type = ATTACH_RADIAL,
        .size = 0.5f,
        .occupied = false,
        .connection_id = NULL
    };
    
    // Validate part creation
    TEST_ASSERT_EQUAL_STRING("test_hull_basic", hull->part_id);
    TEST_ASSERT_EQUAL_FLOAT(500.0f, hull->mass);
    TEST_ASSERT_EQUAL_INT(PART_HULL, hull->category);
    TEST_ASSERT_EQUAL_INT(4, hull->num_attachments);
    TEST_ASSERT_NOT_NULL(hull->attachment_nodes);
    
    // Test attachment nodes
    TEST_ASSERT_EQUAL_FLOAT(1.0f, hull->attachment_nodes[0].position.y);
    TEST_ASSERT_EQUAL_INT(ATTACH_STACK, hull->attachment_nodes[0].type);
    TEST_ASSERT_FALSE(hull->attachment_nodes[0].occupied);
    
    // Cleanup
    free(hull->part_id);
    free(hull->display_name);
    free(hull->mesh_name);
    free(hull->material_name);
    free(hull->attachment_nodes);
    free(hull);
}

void test_create_test_engine_part(void) {
    // Create a test engine part
    ShipPart* engine = calloc(1, sizeof(ShipPart));
    TEST_ASSERT_NOT_NULL(engine);
    
    engine->part_id = strdup("test_engine_basic");
    engine->display_name = strdup("Basic Test Engine");
    engine->category = PART_THRUSTER;
    engine->mass = 150.0f;
    engine->center_of_mass = (Vector3){0.0f, -0.5f, 0.0f};
    engine->drag_coefficient = 0.1f;
    engine->mesh_name = strdup("engine_basic_mesh");
    engine->material_name = strdup("engine_metal");
    
    // Single top attachment for stack mounting
    engine->num_attachments = 1;
    engine->attachment_nodes = calloc(1, sizeof(AttachmentNode));
    
    engine->attachment_nodes[0] = (AttachmentNode){
        .position = {0.0f, 0.5f, 0.0f},
        .direction = {0.0f, 1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false,
        .connection_id = NULL
    };
    
    // Validate engine creation
    TEST_ASSERT_EQUAL_STRING("test_engine_basic", engine->part_id);
    TEST_ASSERT_EQUAL_FLOAT(150.0f, engine->mass);
    TEST_ASSERT_EQUAL_INT(PART_THRUSTER, engine->category);
    TEST_ASSERT_EQUAL_INT(1, engine->num_attachments);
    TEST_ASSERT_EQUAL_INT(ATTACH_STACK, engine->attachment_nodes[0].type);
    
    // Cleanup
    free(engine->part_id);
    free(engine->display_name);
    free(engine->mesh_name);
    free(engine->material_name);
    free(engine->attachment_nodes);
    free(engine);
}

// ============================================================================
// SHIP ASSEMBLY TESTS
// ============================================================================

void test_ship_assembly_creation(void) {
    ShipAssembly* ship = ship_assembly_create("Test Ship Alpha");
    
    TEST_ASSERT_NOT_NULL(ship);
    TEST_ASSERT_EQUAL_STRING("Test Ship Alpha", ship->ship_name);
    TEST_ASSERT_EQUAL_INT(0, ship->num_parts);
    TEST_ASSERT_NULL(ship->root_part);
    TEST_ASSERT_NOT_NULL(ship->parts);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ship->total_mass);
    
    ship_assembly_destroy(ship);
}

void test_ship_assembly_null_handling(void) {
    ShipAssembly* null_ship = ship_assembly_create(NULL);
    TEST_ASSERT_NULL(null_ship);
    
    ship_assembly_destroy(NULL); // Should not crash
}

void test_simple_two_part_assembly(void) {
    printf("🔬 Testing simple two-part assembly...\n");
    
    // Create test parts
    ShipPart* hull = calloc(1, sizeof(ShipPart));
    hull->part_id = strdup("test_hull");
    hull->mass = 500.0f;
    hull->num_attachments = 2;
    hull->attachment_nodes = calloc(2, sizeof(AttachmentNode));
    hull->attachment_nodes[0] = (AttachmentNode){
        .position = {0.0f, 1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false
    };
    hull->attachment_nodes[1] = (AttachmentNode){
        .position = {0.0f, -1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false
    };
    
    ShipPart* engine = calloc(1, sizeof(ShipPart));
    engine->part_id = strdup("test_engine");
    engine->mass = 150.0f;
    engine->num_attachments = 1;
    engine->attachment_nodes = calloc(1, sizeof(AttachmentNode));
    engine->attachment_nodes[0] = (AttachmentNode){
        .position = {0.0f, 0.5f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false
    };
    
    // Create assembly
    ShipAssembly* ship = ship_assembly_create("Test Rocket");
    TEST_ASSERT_NOT_NULL(ship);
    
    // Add hull as root part
    Vector3 hull_pos = {0.0f, 0.0f, 0.0f};
    Quaternion hull_rot = {0.0f, 0.0f, 0.0f, 1.0f};
    bool hull_added = ship_assembly_add_part(ship, hull, NULL, 0, &hull_pos, &hull_rot);
    TEST_ASSERT_TRUE(hull_added);
    TEST_ASSERT_EQUAL_INT(1, ship->num_parts);
    TEST_ASSERT_NOT_NULL(ship->root_part);
    
    // Add engine to hull bottom
    Vector3 engine_pos = {0.0f, -1.5f, 0.0f};
    Quaternion engine_rot = {0.0f, 0.0f, 0.0f, 1.0f};
    bool engine_added = ship_assembly_add_part(ship, engine, ship->root_part, 1, &engine_pos, &engine_rot);
    TEST_ASSERT_TRUE(engine_added);
    TEST_ASSERT_EQUAL_INT(2, ship->num_parts);
    
    // Validate assembly
    TEST_ASSERT_TRUE(ship_assembly_validate(ship));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 650.0f, ship->total_mass);
    
    printf("✅ Two-part assembly test passed\n");
    
    // Cleanup
    ship_assembly_destroy(ship);
    free(hull->part_id);
    free(hull->attachment_nodes);
    free(hull);
    free(engine->part_id);
    free(engine->attachment_nodes);
    free(engine);
}

// ============================================================================
// ATTACHMENT VALIDATION TESTS
// ============================================================================

void test_attachment_node_compatibility(void) {
    // Create compatible nodes
    AttachmentNode stack_top = {
        .position = {0.0f, 1.0f, 0.0f},
        .direction = {0.0f, 1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false
    };
    
    AttachmentNode stack_bottom = {
        .position = {0.0f, -0.5f, 0.0f},
        .direction = {0.0f, -1.0f, 0.0f},
        .type = ATTACH_STACK,
        .size = 1.0f,
        .occupied = false
    };
    
    AttachmentNode radial_node = {
        .position = {1.0f, 0.0f, 0.0f},
        .direction = {1.0f, 0.0f, 0.0f},
        .type = ATTACH_RADIAL,
        .size = 0.5f,
        .occupied = false
    };
    
    // Test compatibility (would need implementation)
    // For now, just test data integrity
    TEST_ASSERT_EQUAL_INT(ATTACH_STACK, stack_top.type);
    TEST_ASSERT_EQUAL_INT(ATTACH_STACK, stack_bottom.type);
    TEST_ASSERT_EQUAL_INT(ATTACH_RADIAL, radial_node.type);
    TEST_ASSERT_FALSE(stack_top.occupied);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, stack_top.size);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, radial_node.size);
}

// ============================================================================
// MASS DISTRIBUTION TESTS
// ============================================================================

void test_single_part_mass_distribution(void) {
    ShipAssembly* ship = ship_assembly_create("Mass Test Ship");
    
    // Create single part
    ShipPart* part = calloc(1, sizeof(ShipPart));
    part->part_id = strdup("mass_test_part");
    part->mass = 100.0f;
    part->center_of_mass = (Vector3){1.0f, 2.0f, 3.0f};
    
    Vector3 pos = {5.0f, 6.0f, 7.0f};
    Quaternion rot = {0.0f, 0.0f, 0.0f, 1.0f};
    
    ship_assembly_add_part(ship, part, NULL, 0, &pos, &rot);
    
    // After adding part, total mass should equal part mass
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, ship->total_mass);
    
    // Center of mass should be part COM + part position
    // (Assuming simple addition - actual implementation may be more complex)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.0f, ship->center_of_mass.x); // 1 + 5
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 8.0f, ship->center_of_mass.y); // 2 + 6
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, ship->center_of_mass.z); // 3 + 7
    
    ship_assembly_destroy(ship);
    free(part->part_id);
    free(part);
}

void test_multi_part_mass_distribution(void) {
    ShipAssembly* ship = ship_assembly_create("Multi Mass Test");
    
    // Create two parts with different masses
    ShipPart* heavy_part = calloc(1, sizeof(ShipPart));
    heavy_part->part_id = strdup("heavy_part");
    heavy_part->mass = 200.0f;
    heavy_part->center_of_mass = (Vector3){0.0f, 0.0f, 0.0f};
    
    ShipPart* light_part = calloc(1, sizeof(ShipPart));
    light_part->part_id = strdup("light_part");
    light_part->mass = 50.0f;
    light_part->center_of_mass = (Vector3){0.0f, 0.0f, 0.0f};
    
    // Add parts at different positions
    Vector3 heavy_pos = {0.0f, 0.0f, 0.0f};
    Vector3 light_pos = {10.0f, 0.0f, 0.0f};
    Quaternion rot = {0.0f, 0.0f, 0.0f, 1.0f};
    
    ship_assembly_add_part(ship, heavy_part, NULL, 0, &heavy_pos, &rot);
    ship_assembly_add_part(ship, light_part, NULL, 0, &light_pos, &rot);
    
    // Total mass should be sum
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 250.0f, ship->total_mass);
    
    // Center of mass should be weighted average
    // Expected X: (200*0 + 50*10) / 250 = 500/250 = 2.0
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 2.0f, ship->center_of_mass.x);
    
    ship_assembly_destroy(ship);
    free(heavy_part->part_id);
    free(heavy_part);
    free(light_part->part_id);
    free(light_part);
}

// ============================================================================
// ODE PHYSICS INTEGRATION TESTS
// ============================================================================

void test_assembly_physics_creation(void) {
    printf("🔬 Testing assembly physics creation...\n");
    
    ShipAssembly* ship = ship_assembly_create("Physics Test Ship");
    
    // Create a simple part
    ShipPart* part = calloc(1, sizeof(ShipPart));
    part->part_id = strdup("physics_test_part");
    part->mass = 100.0f;
    part->center_of_mass = (Vector3){0.0f, 0.0f, 0.0f};
    
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Quaternion rot = {0.0f, 0.0f, 0.0f, 1.0f};
    ship_assembly_add_part(ship, part, NULL, 0, &pos, &rot);
    
    // Create physics representation
    bool physics_created = ship_assembly_create_physics(ship, &test_world);
    TEST_ASSERT_TRUE(physics_created);
    
    // Verify ship has entity in world
    TEST_ASSERT_NOT_EQUAL(INVALID_ENTITY, ship->entity_id);
    
    printf("✅ Assembly physics creation test passed\n");
    
    ship_assembly_destroy(ship);
    free(part->part_id);
    free(part);
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_assembly_performance_small(void) {
    printf("🔬 Testing small assembly performance...\n");
    
    const int part_count = 10;
    ShipAssembly* ship = ship_assembly_create("Performance Test Ship");
    
    // Measure assembly time
    clock_t start = clock();
    
    for (int i = 0; i < part_count; i++) {
        ShipPart* part = calloc(1, sizeof(ShipPart));
        part->part_id = calloc(32, sizeof(char));
        snprintf(part->part_id, 32, "perf_part_%d", i);
        part->mass = 10.0f + i;
        part->center_of_mass = (Vector3){0.0f, 0.0f, 0.0f};
        
        Vector3 pos = {(float)i, 0.0f, 0.0f};
        Quaternion rot = {0.0f, 0.0f, 0.0f, 1.0f};
        
        bool added = ship_assembly_add_part(ship, part, NULL, 0, &pos, &rot);
        TEST_ASSERT_TRUE(added);
        
        free(part->part_id);
        free(part);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("⏱️  Added %d parts in %.4f seconds\n", part_count, elapsed);
    TEST_ASSERT_TRUE(elapsed < 0.1); // Should be fast for small assemblies
    TEST_ASSERT_EQUAL_INT(part_count, ship->num_parts);
    
    ship_assembly_destroy(ship);
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

void test_assembly_invalid_operations(void) {
    ShipAssembly* ship = ship_assembly_create("Error Test Ship");
    
    // Try to add NULL part
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Quaternion rot = {0.0f, 0.0f, 0.0f, 1.0f};
    bool result = ship_assembly_add_part(ship, NULL, NULL, 0, &pos, &rot);
    TEST_ASSERT_FALSE(result);
    
    // Try to add part to NULL assembly
    ShipPart* part = calloc(1, sizeof(ShipPart));
    part->part_id = strdup("error_test_part");
    result = ship_assembly_add_part(NULL, part, NULL, 0, &pos, &rot);
    TEST_ASSERT_FALSE(result);
    
    // Try to validate NULL assembly
    result = ship_assembly_validate(NULL);
    TEST_ASSERT_FALSE(result);
    
    ship_assembly_destroy(ship);
    free(part->part_id);
    free(part);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

void test_assembly_stress_medium(void) {
    printf("🔬 Testing medium assembly stress...\n");
    
    const int part_count = 50;
    ShipAssembly* ship = ship_assembly_create("Stress Test Ship");
    
    // Add many parts
    for (int i = 0; i < part_count; i++) {
        ShipPart* part = calloc(1, sizeof(ShipPart));
        part->part_id = calloc(32, sizeof(char));
        snprintf(part->part_id, 32, "stress_part_%d", i);
        part->mass = 1.0f + (i % 10);
        part->center_of_mass = (Vector3){0.0f, 0.0f, 0.0f};
        
        Vector3 pos = {(float)(i % 10), (float)(i / 10), 0.0f};
        Quaternion rot = {0.0f, 0.0f, 0.0f, 1.0f};
        
        bool added = ship_assembly_add_part(ship, part, NULL, 0, &pos, &rot);
        TEST_ASSERT_TRUE(added);
        
        free(part->part_id);
        free(part);
    }
    
    // Validate assembly
    TEST_ASSERT_TRUE(ship_assembly_validate(ship));
    TEST_ASSERT_EQUAL_INT(part_count, ship->num_parts);
    
    // Mass should be reasonable sum
    float expected_mass = 0.0f;
    for (int i = 0; i < part_count; i++) {
        expected_mass += 1.0f + (i % 10);
    }
    TEST_ASSERT_FLOAT_WITHIN(1.0f, expected_mass, ship->total_mass);
    
    printf("✅ Medium stress test passed with %d parts\n", part_count);
    
    ship_assembly_destroy(ship);
}

// ============================================================================
// TEST RUNNER
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    
    printf("🚀 Starting Ship Assembly Comprehensive Test Suite\n");
    printf("==================================================\n");
    
    // Part Library Tests
    printf("📚 Testing Part Library...\n");
    RUN_TEST(test_part_library_initialization);
    RUN_TEST(test_part_library_null_handling);
    RUN_TEST(test_create_test_hull_part);
    RUN_TEST(test_create_test_engine_part);
    
    // Ship Assembly Tests
    printf("🚢 Testing Ship Assembly...\n");
    RUN_TEST(test_ship_assembly_creation);
    RUN_TEST(test_ship_assembly_null_handling);
    RUN_TEST(test_simple_two_part_assembly);
    
    // Attachment Tests
    printf("🔗 Testing Attachment System...\n");
    RUN_TEST(test_attachment_node_compatibility);
    
    // Mass Distribution Tests
    printf("⚖️  Testing Mass Distribution...\n");
    RUN_TEST(test_single_part_mass_distribution);
    RUN_TEST(test_multi_part_mass_distribution);
    
    // Physics Integration Tests
    printf("🌐 Testing Physics Integration...\n");
    RUN_TEST(test_assembly_physics_creation);
    
    // Performance Tests
    printf("⚡ Testing Performance...\n");
    RUN_TEST(test_assembly_performance_small);
    
    // Error Handling Tests
    printf("❌ Testing Error Handling...\n");
    RUN_TEST(test_assembly_invalid_operations);
    
    // Stress Tests
    printf("💪 Testing Stress Cases...\n");
    RUN_TEST(test_assembly_stress_medium);
    
    printf("✅ Ship Assembly Test Suite Complete\n");
    
    return UNITY_END();
}
