/**
 * @file test_performance_critical.c
 * @brief Performance tests for critical game systems
 * 
 * Tests performance characteristics of systems critical for gameplay:
 * - ECS performance with high entity counts
 * - Physics system performance
 * - Rendering pipeline performance
 * - Memory allocation patterns
 * - UI system responsiveness
 */

#include "../vendor/unity.h"
#include "../../src/core.h"
#include "../../src/systems.h"
#include "../../src/system/physics.h"
#include "../../src/render.h"
#include "../../src/ui_api.h"
#include "../../src/ui_scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Performance test configuration
#define PERFORMANCE_ENTITY_COUNT 1000
#define PERFORMANCE_FRAME_COUNT 60
#define PERFORMANCE_TARGET_FPS 60.0f
#define PERFORMANCE_MAX_FRAME_TIME_MS 16.67f // 60 FPS target

// Test world for performance tests
static struct World* perf_world;
static RenderConfig perf_render_config;

void setUp(void)
{
    // Initialize large world for performance testing
    perf_world = malloc(sizeof(struct World));
    memset(perf_world, 0, sizeof(struct World));
    
    // Use proper world initialization
    if (!world_init(perf_world)) {
        printf("Failed to initialize test world\n");
        TEST_FAIL_MESSAGE("World initialization failed");
    }
    
    // Initialize render config
    memset(&perf_render_config, 0, sizeof(RenderConfig));
    perf_render_config.screen_width = 1920;
    perf_render_config.screen_height = 1080;
}

void tearDown(void)
{
    if (perf_world) {
        world_destroy(perf_world);
        free(perf_world);
        perf_world = NULL;
    }
}

// ============================================================================
// ECS PERFORMANCE TESTS
// ============================================================================

void test_ecs_entity_creation_performance(void)
{
    printf("  📊 Testing entity creation performance...\n");
    
    clock_t start_time = clock();
    
    // Create many entities
    for (int i = 0; i < PERFORMANCE_ENTITY_COUNT; i++) {
        EntityID entity = entity_create(perf_world);
        TEST_ASSERT_NOT_EQUAL(0, entity);
        
        // Add components to make it realistic
        entity_add_component(perf_world, entity, 
                           COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("    Created %d entities in %.2f ms (%.2f μs per entity)\n", 
           PERFORMANCE_ENTITY_COUNT, elapsed_ms, 
           (elapsed_ms * 1000.0) / PERFORMANCE_ENTITY_COUNT);
    
    // Should complete within reasonable time (< 100ms)
    TEST_ASSERT_LESS_THAN(100.0, elapsed_ms);
    
    // Should maintain entity count
    TEST_ASSERT_EQUAL_INT(PERFORMANCE_ENTITY_COUNT, perf_world->entity_count);
}

void test_ecs_component_access_performance(void)
{
    printf("  📊 Testing component access performance...\n");
    
    // First create entities
    test_ecs_entity_creation_performance();
    
    clock_t start_time = clock();
    
    // Access components many times
    int access_count = 0;
    for (int frame = 0; frame < 100; frame++) {
        for (uint32_t i = 0; i < perf_world->entity_count; i++) {
            struct Entity* entity = &perf_world->entities[i];
            
            if (entity->component_mask & COMPONENT_TRANSFORM) {
                struct Transform* transform = entity_get_transform(perf_world, entity->id);
                if (transform) {
                    // Simple operation to prevent optimization
                    transform->position.x += 0.001f;
                    access_count++;
                }
            }
            
            if (entity->component_mask & COMPONENT_PHYSICS) {
                struct Physics* physics = entity_get_physics(perf_world, entity->id);
                if (physics) {
                    physics->velocity.x += 0.001f;
                    access_count++;
                }
            }
        }
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("    Performed %d component accesses in %.2f ms (%.2f ns per access)\n", 
           access_count, elapsed_ms, (elapsed_ms * 1000000.0) / access_count);
    
    // Should complete within reasonable time for high-frequency operations
    TEST_ASSERT_LESS_THAN(500.0, elapsed_ms);
}

void test_ecs_system_update_performance(void)
{
    printf("  📊 Testing system update performance...\n");
    
    // Create entities with physics components
    for (int i = 0; i < PERFORMANCE_ENTITY_COUNT; i++) {
        EntityID entity = entity_create(perf_world);
        entity_add_component(perf_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
        
        // Set some initial values for realistic computation
        struct Transform* transform = entity_get_transform(perf_world, entity);
        struct Physics* physics = entity_get_physics(perf_world, entity);
        
        if (transform && physics) {
            transform->position = (Vector3){
                (float)(rand() % 100), 
                (float)(rand() % 100), 
                (float)(rand() % 100)
            };
            physics->velocity = (Vector3){
                (float)(rand() % 10) - 5.0f,
                (float)(rand() % 10) - 5.0f,
                (float)(rand() % 10) - 5.0f
            };
            physics->mass = 1.0f + (float)(rand() % 100) / 100.0f;
        }
    }
    
    clock_t start_time = clock();
    
    // Update physics system multiple times
    float delta_time = 1.0f / PERFORMANCE_TARGET_FPS;
    for (int frame = 0; frame < PERFORMANCE_FRAME_COUNT; frame++) {
        physics_system_update(perf_world, &perf_render_config, delta_time);
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    double ms_per_frame = elapsed_ms / PERFORMANCE_FRAME_COUNT;
    
    printf("    Updated %d entities for %d frames in %.2f ms (%.2f ms per frame)\n", 
           PERFORMANCE_ENTITY_COUNT, PERFORMANCE_FRAME_COUNT, elapsed_ms, ms_per_frame);
    
    // Should maintain target frame rate
    TEST_ASSERT_LESS_THAN(PERFORMANCE_MAX_FRAME_TIME_MS, ms_per_frame);
    
    printf("    Performance: %.1f FPS equivalent\n", 1000.0f / ms_per_frame);
}

// ============================================================================
// MEMORY PERFORMANCE TESTS
// ============================================================================

void test_memory_allocation_performance(void)
{
    printf("  📊 Testing memory allocation performance...\n");
    
    const int allocation_count = 1000;
    void* allocations[allocation_count];
    
    clock_t start_time = clock();
    
    // Allocate many small blocks
    for (int i = 0; i < allocation_count; i++) {
        allocations[i] = malloc(64); // Small allocations like component data
        TEST_ASSERT_NOT_NULL(allocations[i]);
    }
    
    clock_t mid_time = clock();
    
    // Free all blocks
    for (int i = 0; i < allocation_count; i++) {
        free(allocations[i]);
        allocations[i] = NULL;
    }
    
    clock_t end_time = clock();
    
    double alloc_ms = ((double)(mid_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    double free_ms = ((double)(end_time - mid_time)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("    Allocated %d blocks in %.2f ms, freed in %.2f ms\n", 
           allocation_count, alloc_ms, free_ms);
    
    // Should complete within reasonable time
    TEST_ASSERT_LESS_THAN(50.0, alloc_ms);
    TEST_ASSERT_LESS_THAN(50.0, free_ms);
}

void test_memory_fragmentation_resistance(void)
{
    printf("  📊 Testing memory fragmentation resistance...\n");
    
    const int cycle_count = 100;
    const int allocs_per_cycle = 50;
    
    clock_t start_time = clock();
    
    for (int cycle = 0; cycle < cycle_count; cycle++) {
        void* allocations[allocs_per_cycle];
        
        // Allocate varying sizes
        for (int i = 0; i < allocs_per_cycle; i++) {
            size_t size = 32 + (rand() % 256); // Variable sizes 32-288 bytes
            allocations[i] = malloc(size);
            TEST_ASSERT_NOT_NULL(allocations[i]);
        }
        
        // Free in random order to create fragmentation
        for (int i = 0; i < allocs_per_cycle; i++) {
            int index = rand() % allocs_per_cycle;
            if (allocations[index]) {
                free(allocations[index]);
                allocations[index] = NULL;
            }
        }
        
        // Free any remaining
        for (int i = 0; i < allocs_per_cycle; i++) {
            if (allocations[i]) {
                free(allocations[i]);
            }
        }
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("    Completed %d allocation/fragmentation cycles in %.2f ms\n", 
           cycle_count, elapsed_ms);
    
    // Should handle fragmentation without severe performance degradation
    TEST_ASSERT_LESS_THAN(1000.0, elapsed_ms);
}

// ============================================================================
// UI SYSTEM PERFORMANCE TESTS
// ============================================================================

void test_ui_scene_transition_performance(void)
{
    printf("  📊 Testing UI scene transition performance...\n");
    
    // Initialize UI systems
    scene_ui_init();
    
    const int transition_count = 100;
    clock_t start_time = clock();
    
    // Perform rapid scene transitions
    for (int i = 0; i < transition_count; i++) {
        char scene_name[32];
        snprintf(scene_name, sizeof(scene_name), "test_scene_%d", i % 10);
        
        ui_request_scene_change(scene_name);
        
        // Verify request was registered
        TEST_ASSERT_TRUE(ui_has_scene_change_request());
        TEST_ASSERT_EQUAL_STRING(scene_name, ui_get_requested_scene());
        
        ui_clear_scene_change_request();
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("    Performed %d scene transitions in %.2f ms (%.2f μs per transition)\n", 
           transition_count, elapsed_ms, (elapsed_ms * 1000.0) / transition_count);
    
    // UI operations should be very fast
    TEST_ASSERT_LESS_THAN(50.0, elapsed_ms);
    
    scene_ui_shutdown();
}

void test_ui_module_registration_performance(void)
{
    printf("  📊 Testing UI module registration performance...\n");
    
    scene_ui_init();
    
    const int module_count = 50;
    SceneUIModule* modules[module_count];
    
    clock_t start_time = clock();
    
    // Register many modules
    for (int i = 0; i < module_count; i++) {
        modules[i] = malloc(sizeof(SceneUIModule));
        char* scene_name = malloc(32);
        snprintf(scene_name, 32, "test_module_%d", i);
        
        modules[i]->scene_name = scene_name;
        modules[i]->render = NULL;
        modules[i]->handle_event = NULL;
        modules[i]->shutdown = NULL;
        modules[i]->data = NULL;
        
        scene_ui_register(modules[i]);
    }
    
    clock_t mid_time = clock();
    
    // Look up modules
    int found_count = 0;
    for (int i = 0; i < module_count; i++) {
        char scene_name[32];
        snprintf(scene_name, sizeof(scene_name), "test_module_%d", i);
        
        SceneUIModule* found = scene_ui_get_module(scene_name);
        if (found) {
            found_count++;
        }
    }
    
    clock_t end_time = clock();
    
    double register_ms = ((double)(mid_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    double lookup_ms = ((double)(end_time - mid_time)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("    Registered %d modules in %.2f ms, looked up %d in %.2f ms\n", 
           module_count, register_ms, found_count, lookup_ms);
    
    // Clean up
    for (int i = 0; i < module_count; i++) {
        char scene_name[32];
        snprintf(scene_name, sizeof(scene_name), "test_module_%d", i);
        scene_ui_unregister(scene_name);
    }
    
    // Should be efficient operations
    TEST_ASSERT_LESS_THAN(20.0, register_ms);
    TEST_ASSERT_LESS_THAN(10.0, lookup_ms);
    TEST_ASSERT_EQUAL_INT(module_count, found_count);
    
    scene_ui_shutdown();
}

// ============================================================================
// INTEGRATION PERFORMANCE TESTS
// ============================================================================

void test_full_frame_performance(void)
{
    printf("  📊 Testing full frame update performance...\n");
    
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &perf_render_config);
    
    // Create realistic entity mix
    const int physics_entities = PERFORMANCE_ENTITY_COUNT / 2;
    const int camera_entities = 5;
    const int other_entities = PERFORMANCE_ENTITY_COUNT - physics_entities - camera_entities;
    
    // Physics entities
    for (int i = 0; i < physics_entities; i++) {
        EntityID entity = entity_create(perf_world);
        entity_add_component(perf_world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    }
    
    // Camera entities
    for (int i = 0; i < camera_entities; i++) {
        EntityID entity = entity_create(perf_world);
        entity_add_component(perf_world, entity, COMPONENT_TRANSFORM | COMPONENT_CAMERA);
    }
    
    // Other entities
    for (int i = 0; i < other_entities; i++) {
        EntityID entity = entity_create(perf_world);
        entity_add_component(perf_world, entity, COMPONENT_TRANSFORM);
    }
    
    printf("    Created %d entities (%d physics, %d cameras, %d other)\n", 
           perf_world->entity_count, physics_entities, camera_entities, other_entities);
    
    clock_t start_time = clock();
    
    // Simulate full game frames
    float delta_time = 1.0f / PERFORMANCE_TARGET_FPS;
    for (int frame = 0; frame < PERFORMANCE_FRAME_COUNT; frame++) {
        scheduler_update(&scheduler, perf_world, &perf_render_config, delta_time);
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    double ms_per_frame = elapsed_ms / PERFORMANCE_FRAME_COUNT;
    
    printf("    Updated %d frames in %.2f ms (%.2f ms per frame)\n", 
           PERFORMANCE_FRAME_COUNT, elapsed_ms, ms_per_frame);
    printf("    Performance: %.1f FPS equivalent\n", 1000.0f / ms_per_frame);
    
    // Should maintain target frame rate with margin
    TEST_ASSERT_LESS_THAN(PERFORMANCE_MAX_FRAME_TIME_MS * 0.8f, ms_per_frame);
    
    scheduler_destroy(&scheduler, &perf_render_config);
}

void test_stress_test_many_entities(void)
{
    printf("  📊 Testing stress conditions with many entities...\n");
    
    SystemScheduler scheduler;
    scheduler_init(&scheduler, &perf_render_config);
    
    const int stress_entity_count = PERFORMANCE_ENTITY_COUNT * 2;
    
    // Create stress load
    for (int i = 0; i < stress_entity_count; i++) {
        EntityID entity = entity_create(perf_world);
        
        // Vary component combinations
        uint32_t mask = COMPONENT_TRANSFORM;
        if (i % 3 == 0) mask |= COMPONENT_PHYSICS;
        if (i % 20 == 0) mask |= COMPONENT_CAMERA;
        
        entity_add_component(perf_world, entity, mask);
    }
    
    printf("    Created %d entities for stress test\n", perf_world->entity_count);
    
    clock_t start_time = clock();
    
    // Run fewer frames but with much higher load
    float delta_time = 1.0f / PERFORMANCE_TARGET_FPS;
    for (int frame = 0; frame < 10; frame++) {
        scheduler_update(&scheduler, perf_world, &perf_render_config, delta_time);
    }
    
    clock_t end_time = clock();
    double elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    double ms_per_frame = elapsed_ms / 10;
    
    printf("    Stress test: %.2f ms per frame with %d entities\n", 
           ms_per_frame, perf_world->entity_count);
    
    // Should still be somewhat reasonable even under stress
    TEST_ASSERT_LESS_THAN(50.0, ms_per_frame);
    
    scheduler_destroy(&scheduler, &perf_render_config);
}

// ============================================================================
// TEST SUITE REGISTRATION
// ============================================================================

void suite_performance_critical(void)
{
    printf("\n⚡ Critical Performance Tests\n");
    printf("=============================\n");
    
    printf("🏃 Testing ECS Performance...\n");
    RUN_TEST(test_ecs_entity_creation_performance);
    RUN_TEST(test_ecs_component_access_performance);
    RUN_TEST(test_ecs_system_update_performance);
    
    printf("💾 Testing Memory Performance...\n");
    RUN_TEST(test_memory_allocation_performance);
    RUN_TEST(test_memory_fragmentation_resistance);
    
    printf("🎨 Testing UI Performance...\n");
    RUN_TEST(test_ui_scene_transition_performance);
    RUN_TEST(test_ui_module_registration_performance);
    
    printf("🎮 Testing Integration Performance...\n");
    RUN_TEST(test_full_frame_performance);
    RUN_TEST(test_stress_test_many_entities);
    
    printf("✅ Critical Performance Tests Complete\n");
}

int main(void) {
    UNITY_BEGIN();
    suite_performance_critical();
    return UNITY_END();
}
