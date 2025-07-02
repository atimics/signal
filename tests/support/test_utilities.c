/**
 * @file test_utilities.c
 * @brief Implementation of common test utilities and fixtures
 */

#include "test_utilities.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

// ============================================================================
// GLOBAL TEST STATE
// ============================================================================

struct World* test_world = NULL;
static MockCallRecord mock_calls[100];
static int mock_call_count = 0;
static size_t memory_start_usage = 0;

// ============================================================================
// TEST WORLD FIXTURES
// ============================================================================

bool test_world_setup(void) {
    if (test_world != NULL) {
        test_world_teardown();
    }
    
    test_world = malloc(sizeof(struct World));
    if (!test_world) {
        return false;
    }
    
    if (!world_init(test_world)) {
        free(test_world);
        test_world = NULL;
        return false;
    }
    
    // Override max entities for testing efficiency
    test_world->max_entities = TEST_MAX_ENTITIES;
    
    return true;
}

void test_world_teardown(void) {
    if (test_world) {
        world_destroy(test_world);
        free(test_world);
        test_world = NULL;
    }
}

EntityID test_create_basic_entity(struct World* world) {
    if (!world) return INVALID_ENTITY_ID;
    
    EntityID entity = entity_create(world);
    if (entity == INVALID_ENTITY_ID) return entity;
    
    entity_add_component(world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    // Initialize with reasonable defaults
    struct Transform* transform = entity_get_transform(world, entity);
    if (transform) {
        transform->position = (Vector3){0.0f, 0.0f, 0.0f};
        transform->rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
        transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
        transform->dirty = true;
    }
    
    struct Physics* physics = entity_get_physics(world, entity);
    if (physics) {
        physics->velocity = (Vector3){0.0f, 0.0f, 0.0f};
        physics->angular_velocity = (Vector3){0.0f, 0.0f, 0.0f};
        physics->mass = 1.0f;
        physics->drag = 0.1f;
        physics->angular_drag = 0.1f;
        physics->enable_6dof = false;
    }
    
    return entity;
}

EntityID test_create_entity_with_components(struct World* world, ComponentType components) {
    if (!world) return INVALID_ENTITY_ID;
    
    EntityID entity = entity_create(world);
    if (entity == INVALID_ENTITY_ID) return entity;
    
    entity_add_component(world, entity, components);
    
    // Initialize components with test defaults
    if (components & COMPONENT_TRANSFORM) {
        struct Transform* transform = entity_get_transform(world, entity);
        if (transform) {
            test_populate_component(transform, COMPONENT_TRANSFORM);
        }
    }
    
    if (components & COMPONENT_PHYSICS) {
        struct Physics* physics = entity_get_physics(world, entity);
        if (physics) {
            test_populate_component(physics, COMPONENT_PHYSICS);
        }
    }
    
    return entity;
}

// ============================================================================
// TIMING UTILITIES
// ============================================================================

double test_get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

double test_time_function(const char* description, void (*func)(void)) {
    double start_time = test_get_time_ms();
    func();
    double duration = test_get_time_ms() - start_time;
    
    if (description) {
        printf("⏱️  %s: %.2fms\n", description, duration);
    }
    
    return duration;
}

// ============================================================================
// MOCK UTILITIES
// ============================================================================

void test_mock_init(void) {
    mock_call_count = 0;
    memset(mock_calls, 0, sizeof(mock_calls));
}

void test_mock_record_call(const char* function_name, void* parameters) {
    if (mock_call_count < 100) {
        mock_calls[mock_call_count].function_name = function_name;
        mock_calls[mock_call_count].call_count = 1;
        mock_calls[mock_call_count].last_parameters = parameters;
        
        // Check if this function was already called
        for (int i = 0; i < mock_call_count; i++) {
            if (strcmp(mock_calls[i].function_name, function_name) == 0) {
                mock_calls[i].call_count++;
                mock_calls[i].last_parameters = parameters;
                return;
            }
        }
        
        mock_call_count++;
    }
}

int test_mock_get_call_count(const char* function_name) {
    for (int i = 0; i < mock_call_count; i++) {
        if (strcmp(mock_calls[i].function_name, function_name) == 0) {
            return mock_calls[i].call_count;
        }
    }
    return 0;
}

void test_mock_reset(void) {
    test_mock_init();
}

// ============================================================================
// RANDOM DATA GENERATORS
// ============================================================================

void test_seed_random(unsigned int seed) {
    srand(seed);
}

float test_random_float(float min, float max) {
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

Vector3 test_random_vector3(float min, float max) {
    return (Vector3){
        test_random_float(min, max),
        test_random_float(min, max),
        test_random_float(min, max)
    };
}

Vector3 test_random_unit_vector3(void) {
    Vector3 v = test_random_vector3(-1.0f, 1.0f);
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length > TEST_EPSILON) {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    } else {
        v = (Vector3){1.0f, 0.0f, 0.0f};
    }
    return v;
}

// ============================================================================
// MEMORY TESTING UTILITIES
// ============================================================================

void test_memory_tracking_start(void) {
    // Simple memory usage tracking - could be enhanced with malloc hooks
    memory_start_usage = 0; // Placeholder - would need system-specific implementation
}

size_t test_memory_tracking_stop(void) {
    // Return memory delta - placeholder implementation
    return 0;
}

void test_check_memory_leaks(void) {
    // Placeholder for memory leak detection
    // In a full implementation, this would check for unfreed allocations
}

// ============================================================================
// FILE SYSTEM UTILITIES
// ============================================================================

bool test_create_temp_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) return false;
    
    if (content) {
        fputs(content, file);
    }
    
    fclose(file);
    return true;
}

void test_delete_temp_file(const char* filename) {
    unlink(filename);
}

bool test_file_exists(const char* filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

// ============================================================================
// LOGGING UTILITIES
// ============================================================================

void test_log(TestLogLevel level, const char* format, ...) {
    const char* prefixes[] = {"🔍", "ℹ️ ", "⚠️ ", "❌"};
    const char* prefix = (level < 4) ? prefixes[level] : "  ";
    
    printf("%s ", prefix);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

// ============================================================================
// COMPONENT TESTING UTILITIES
// ============================================================================

void test_verify_component_defaults(void* component, ComponentType type) {
    switch (type) {
        case COMPONENT_TRANSFORM: {
            struct Transform* t = (struct Transform*)component;
            TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, t->position.x);
            TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, t->position.y);
            TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, t->position.z);
            break;
        }
        case COMPONENT_PHYSICS: {
            struct Physics* p = (struct Physics*)component;
            TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, p->velocity.x);
            TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, p->velocity.y);
            TEST_ASSERT_FLOAT_WITHIN(TEST_EPSILON, 0.0f, p->velocity.z);
            TEST_ASSERT_GREATER_THAN(0.0f, p->mass);
            break;
        }
        default:
            // Unknown component type
            break;
    }
}

void test_populate_component(void* component, ComponentType type) {
    switch (type) {
        case COMPONENT_TRANSFORM: {
            struct Transform* t = (struct Transform*)component;
            t->position = (Vector3){0.0f, 0.0f, 0.0f};
            t->rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
            t->scale = (Vector3){1.0f, 1.0f, 1.0f};
            t->dirty = true;
            break;
        }
        case COMPONENT_PHYSICS: {
            struct Physics* p = (struct Physics*)component;
            p->velocity = (Vector3){0.0f, 0.0f, 0.0f};
            p->angular_velocity = (Vector3){0.0f, 0.0f, 0.0f};
            p->mass = 1.0f;
            p->drag = 0.1f;
            p->angular_drag = 0.1f;
            p->enable_6dof = false;
            break;
        }
        default:
            break;
    }
}

bool test_verify_component_integrity(void* component, ComponentType type) {
    if (!component) return false;
    
    switch (type) {
        case COMPONENT_TRANSFORM: {
            struct Transform* t = (struct Transform*)component;
            // Check for NaN values
            return !isnan(t->position.x) && !isnan(t->position.y) && !isnan(t->position.z) &&
                   !isnan(t->rotation.x) && !isnan(t->rotation.y) && !isnan(t->rotation.z) && !isnan(t->rotation.w) &&
                   !isnan(t->scale.x) && !isnan(t->scale.y) && !isnan(t->scale.z);
        }
        case COMPONENT_PHYSICS: {
            struct Physics* p = (struct Physics*)component;
            return !isnan(p->velocity.x) && !isnan(p->velocity.y) && !isnan(p->velocity.z) &&
                   !isnan(p->angular_velocity.x) && !isnan(p->angular_velocity.y) && !isnan(p->angular_velocity.z) &&
                   !isnan(p->mass) && p->mass > 0.0f;
        }
        default:
            return true;
    }
}

// ============================================================================
// SYSTEM TESTING UTILITIES
// ============================================================================

double test_run_system_timed(void (*system_func)(struct World*, float), 
                            struct World* world, float delta_time) {
    if (!system_func || !world) return -1.0;
    
    double start_time = test_get_time_ms();
    system_func(world, delta_time);
    return test_get_time_ms() - start_time;
}

void test_verify_system_processing(struct World* world, ComponentType components, 
                                 int expected_count) {
    if (!world) return;
    
    int actual_count = 0;
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if ((entity->component_mask & components) == components) {
            actual_count++;
        }
    }
    
    TEST_ASSERT_EQUAL(expected_count, actual_count);
}

// ============================================================================
// INTEGRATION TEST HELPERS
// ============================================================================

void test_create_integration_scene(struct World* world) {
    if (!world) return;
    
    // Create a player ship
    EntityID player = test_create_entity_with_components(world, 
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_CONTROL | COMPONENT_THRUSTERS);
    
    // Create some debris
    for (int i = 0; i < 5; i++) {
        EntityID debris = test_create_entity_with_components(world,
            COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
        
        struct Transform* transform = entity_get_transform(world, debris);
        if (transform) {
            transform->position = test_random_vector3(-50.0f, 50.0f);
        }
        
        struct Physics* physics = entity_get_physics(world, debris);
        if (physics) {
            physics->velocity = test_random_vector3(-5.0f, 5.0f);
        }
    }
    
    // Create a camera
    EntityID camera = test_create_entity_with_components(world,
        COMPONENT_TRANSFORM | COMPONENT_CAMERA);
}

void test_run_system_sequence(struct World* world, float delta_time, int iterations) {
    if (!world) return;
    
    for (int i = 0; i < iterations; i++) {
        // Run core systems in typical order
        physics_system_update(world, delta_time);
        // Add more systems as needed
    }
}

bool test_verify_pipeline_integrity(struct World* world) {
    if (!world) return false;
    
    // Verify all entities have valid component data
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        if (entity->component_mask & COMPONENT_TRANSFORM) {
            if (!test_verify_component_integrity(entity->transform, COMPONENT_TRANSFORM)) {
                return false;
            }
        }
        
        if (entity->component_mask & COMPONENT_PHYSICS) {
            if (!test_verify_component_integrity(entity->physics, COMPONENT_PHYSICS)) {
                return false;
            }
        }
    }
    
    return true;
}
