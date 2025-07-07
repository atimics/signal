/**
 * @file test_simplified_ship_design.c
 * @brief Comprehensive tests for the simplified single-body ship design system
 */

#include "unity.h"
#include "../src/core.h"
#include "../src/system/physics.h"
#include "../stubs/graphics_api_test_stub.h"
#include <math.h>
#include <string.h>

// Test world for ship testing
static struct World test_world;

// Forward declarations for test ship system
typedef enum {
    COMPONENT_THRUSTER,
    COMPONENT_TANK, 
    COMPONENT_WEAPON,
    COMPONENT_SYSTEM,
    COMPONENT_COUNT
} ComponentType;

typedef struct {
    float max_thrust;         // Maximum thrust force (N)
    Vector3 thrust_direction; // Local thrust vector (usually -Z)
    float specific_impulse;   // Fuel efficiency
    float gimbal_range;       // Degrees of gimbal movement
    
    // Current state
    float current_thrust;     // 0.0 to 1.0 throttle
    Vector3 gimbal_angle;     // Current gimbal orientation
    float fuel_flow;          // Current fuel consumption
} ThrusterData;

typedef struct {
    float capacity;           // Fuel capacity (kg)
    float current_fuel;       // Current fuel (kg)
    float flow_rate;          // Max fuel flow rate (kg/s)
} TankData;

typedef struct {
    ComponentType type;        // THRUSTER, TANK, WEAPON, etc.
    
    // Mesh attachment (compile-time validated)
    char* hull_mesh;          // Which hull mesh this attaches to
    Vector3 attach_point;     // Exact mesh vertex/point (0.5, 1.0, 0.0)
    Vector3 attach_normal;    // Surface normal at attachment
    
    // Physical properties
    float mass;               // Component mass
    Vector3 local_com;        // Component center of mass offset
    
    // Functional properties (union for different types)
    union {
        ThrusterData thruster;
        TankData tank;
    } data;
    
    // Runtime state
    bool active;
    float health;
    float temperature;
} ShipComponent;

typedef struct {
    // Core physics body (single entity)
    EntityID ship_entity;
    struct Physics* physics;
    struct Transform* transform;
    
    // Ship hull/mesh
    char* hull_mesh;           // Base mesh asset
    Vector3 hull_com;          // Center of mass
    float hull_mass;           // Base mass
    
    // Attached components
    ShipComponent* components;  // Array of components
    int component_count;
    int max_components;
    
    // Cached physics properties
    float total_mass;          // Hull + all components
    Vector3 total_com;         // Combined center of mass
    
} Ship;

// Mock mesh data for testing
typedef struct {
    char* mesh_name;
    Vector3* vertices;
    int vertex_count;
} MockMesh;

// Test mesh data
static Vector3 pod_basic_vertices[] = {
    {0.0f, -1.2f, 0.0f},      // Bottom attachment point
    {0.0f, 1.0f, 0.0f},       // Top
    {1.0f, 0.0f, 0.0f},       // Right side
    {-1.0f, 0.0f, 0.0f},      // Left side
    {0.0f, 0.0f, 1.0f},       // Front
    {0.0f, 0.0f, -1.0f},      // Back
};

static MockMesh pod_basic_mesh = {
    .mesh_name = "pod_basic.obj",
    .vertices = pod_basic_vertices,
    .vertex_count = 6
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static Ship* create_test_ship(void) {
    Ship* ship = calloc(1, sizeof(Ship));
    
    // Create ship entity
    ship->ship_entity = entity_create(&test_world);
    entity_add_component(&test_world, ship->ship_entity, COMPONENT_PHYSICS | COMPONENT_TRANSFORM);
    
    ship->physics = entity_get_physics(&test_world, ship->ship_entity);
    ship->transform = entity_get_transform(&test_world, ship->ship_entity);
    
    // Set hull properties
    ship->hull_mesh = strdup("pod_basic.obj");
    ship->hull_com = (Vector3){0.0f, 0.0f, 0.0f};
    ship->hull_mass = 500.0f; // 500kg hull
    
    // Initialize component array
    ship->max_components = 10;
    ship->components = calloc(ship->max_components, sizeof(ShipComponent));
    ship->component_count = 0;
    
    // Set initial physics properties
    ship->total_mass = ship->hull_mass;
    ship->total_com = ship->hull_com;
    ship->physics->mass = ship->hull_mass;
    
    return ship;
}

static void destroy_test_ship(Ship* ship) {
    if (!ship) return;
    
    free(ship->hull_mesh);
    free(ship->components);
    free(ship);
}

static bool validate_attachment_point(const char* mesh_name, Vector3 attach_point) {
    // Simple validation - check if point exists in our mock mesh
    if (strcmp(mesh_name, "pod_basic.obj") == 0) {
        const float tolerance = 0.1f;
        
        for (int i = 0; i < pod_basic_mesh.vertex_count; i++) {
            Vector3 vertex = pod_basic_mesh.vertices[i];
            float distance = sqrtf(
                (vertex.x - attach_point.x) * (vertex.x - attach_point.x) +
                (vertex.y - attach_point.y) * (vertex.y - attach_point.y) +
                (vertex.z - attach_point.z) * (vertex.z - attach_point.z)
            );
            
            if (distance < tolerance) {
                return true;
            }
        }
    }
    
    return false;
}

static bool ship_add_component(Ship* ship, ComponentType type, Vector3 attach_point, float mass) {
    if (ship->component_count >= ship->max_components) {
        return false;
    }
    
    // Validate attachment point
    if (!validate_attachment_point(ship->hull_mesh, attach_point)) {
        printf("ERROR: Invalid attachment point (%.2f,%.2f,%.2f) for mesh %s\n",
               attach_point.x, attach_point.y, attach_point.z, ship->hull_mesh);
        return false;
    }
    
    ShipComponent* comp = &ship->components[ship->component_count];
    comp->type = type;
    comp->hull_mesh = strdup(ship->hull_mesh);
    comp->attach_point = attach_point;
    comp->attach_normal = (Vector3){0, -1, 0}; // Default down
    comp->mass = mass;
    comp->local_com = (Vector3){0, 0, 0};
    comp->active = true;
    comp->health = 1.0f;
    comp->temperature = 20.0f; // Room temperature
    
    ship->component_count++;
    return true;
}

static void ship_recalculate_mass_properties(Ship* ship) {
    float total_mass = ship->hull_mass;
    Vector3 weighted_com = vector3_multiply(ship->hull_com, ship->hull_mass);
    
    // Add component contributions
    for (int i = 0; i < ship->component_count; i++) {
        ShipComponent* comp = &ship->components[i];
        total_mass += comp->mass;
        
        Vector3 comp_world_com = vector3_add(comp->attach_point, comp->local_com);
        Vector3 comp_weighted = vector3_multiply(comp_world_com, comp->mass);
        weighted_com = vector3_add(weighted_com, comp_weighted);
    }
    
    ship->total_mass = total_mass;
    ship->total_com = vector3_divide(weighted_com, total_mass);
    ship->physics->mass = total_mass;
}

static void ship_apply_thruster_forces(Ship* ship) {
    Vector3 total_force = {0, 0, 0};
    Vector3 total_torque = {0, 0, 0};
    
    for (int i = 0; i < ship->component_count; i++) {
        ShipComponent* comp = &ship->components[i];
        
        if (comp->type == COMPONENT_THRUSTER && comp->active) {
            ThrusterData* thruster = &comp->data.thruster;
            
            // Calculate thrust force in local space
            Vector3 local_thrust = vector3_multiply(
                thruster->thrust_direction, 
                thruster->max_thrust * thruster->current_thrust
            );
            
            // Transform to world space using ship orientation
            Vector3 world_thrust = quaternion_rotate_vector(ship->transform->rotation, local_thrust);
            
            // Apply force to total
            total_force = vector3_add(total_force, world_thrust);
            
            // Calculate torque from offset thrust application
            Vector3 force_offset = vector3_subtract(comp->attach_point, ship->total_com);
            Vector3 component_torque = vector3_cross_product(force_offset, world_thrust);
            total_torque = vector3_add(total_torque, component_torque);
        }
    }
    
    // Apply combined forces to ship physics
    physics_add_force(ship->physics, total_force);
    physics_add_torque(ship->physics, total_torque);
}

// ============================================================================
// SETUP AND TEARDOWN
// ============================================================================

void setUp(void) {
    // Initialize test world
    memset(&test_world, 0, sizeof(test_world));
    test_world.entity_capacity = 100;
    test_world.entities = calloc(test_world.entity_capacity, sizeof(struct Entity));
    test_world.entity_count = 0;
    test_world.next_entity_id = 1;
}

void tearDown(void) {
    // Cleanup test world
    if (test_world.entities) {
        free(test_world.entities);
        test_world.entities = NULL;
    }
    memset(&test_world, 0, sizeof(test_world));
}

// ============================================================================
// BASIC COMPONENT SYSTEM TESTS
// ============================================================================

void test_ship_creation(void) {
    Ship* ship = create_test_ship();
    
    TEST_ASSERT_NOT_NULL(ship);
    TEST_ASSERT_NOT_NULL(ship->physics);
    TEST_ASSERT_NOT_NULL(ship->transform);
    TEST_ASSERT_EQUAL_STRING("pod_basic.obj", ship->hull_mesh);
    TEST_ASSERT_EQUAL_FLOAT(500.0f, ship->hull_mass);
    TEST_ASSERT_EQUAL_FLOAT(500.0f, ship->total_mass);
    TEST_ASSERT_EQUAL_INT(0, ship->component_count);
    
    destroy_test_ship(ship);
}

void test_attachment_point_validation(void) {
    // Valid attachment points (exist in pod_basic_mesh)
    TEST_ASSERT_TRUE(validate_attachment_point("pod_basic.obj", (Vector3){0.0f, -1.2f, 0.0f}));
    TEST_ASSERT_TRUE(validate_attachment_point("pod_basic.obj", (Vector3){0.0f, 1.0f, 0.0f}));
    TEST_ASSERT_TRUE(validate_attachment_point("pod_basic.obj", (Vector3){1.0f, 0.0f, 0.0f}));
    
    // Invalid attachment points (don't exist in mesh)
    TEST_ASSERT_FALSE(validate_attachment_point("pod_basic.obj", (Vector3){5.0f, 5.0f, 5.0f}));
    TEST_ASSERT_FALSE(validate_attachment_point("pod_basic.obj", (Vector3){0.0f, -2.0f, 0.0f}));
    
    // Invalid mesh
    TEST_ASSERT_FALSE(validate_attachment_point("nonexistent.obj", (Vector3){0.0f, 0.0f, 0.0f}));
}

void test_component_addition(void) {
    Ship* ship = create_test_ship();
    
    // Add thruster at valid attachment point
    bool result = ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(1, ship->component_count);
    
    // Check component properties
    ShipComponent* comp = &ship->components[0];
    TEST_ASSERT_EQUAL_INT(COMPONENT_THRUSTER, comp->type);
    TEST_ASSERT_EQUAL_FLOAT(167.0f, comp->mass);
    TEST_ASSERT_TRUE(comp->active);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, comp->health);
    
    // Try to add component at invalid attachment point
    result = ship_add_component(ship, COMPONENT_TANK, (Vector3){10.0f, 10.0f, 10.0f}, 50.0f);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(1, ship->component_count); // Should still be 1
    
    destroy_test_ship(ship);
}

void test_mass_calculation(void) {
    Ship* ship = create_test_ship();
    
    // Initial mass should be hull only
    TEST_ASSERT_EQUAL_FLOAT(500.0f, ship->total_mass);
    
    // Add thruster (167kg)
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    ship_recalculate_mass_properties(ship);
    
    TEST_ASSERT_EQUAL_FLOAT(667.0f, ship->total_mass); // 500 + 167
    TEST_ASSERT_EQUAL_FLOAT(667.0f, ship->physics->mass);
    
    // Add fuel tank (200kg)
    ship_add_component(ship, COMPONENT_TANK, (Vector3){0.0f, 1.0f, 0.0f}, 200.0f);
    ship_recalculate_mass_properties(ship);
    
    TEST_ASSERT_EQUAL_FLOAT(867.0f, ship->total_mass); // 500 + 167 + 200
    
    destroy_test_ship(ship);
}

void test_center_of_mass_calculation(void) {
    Ship* ship = create_test_ship();
    
    // Initial COM should be hull COM
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ship->total_com.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ship->total_com.y);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ship->total_com.z);
    
    // Add thruster at bottom (should shift COM down)
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    ship_recalculate_mass_properties(ship);
    
    // Expected COM: (500*0 + 167*(-1.2)) / (500+167) = -200.4/667 ≈ -0.3
    float expected_y = (500.0f * 0.0f + 167.0f * (-1.2f)) / (500.0f + 167.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expected_y, ship->total_com.y);
    
    // Add tank at top (should shift COM back up)
    ship_add_component(ship, COMPONENT_TANK, (Vector3){0.0f, 1.0f, 0.0f}, 200.0f);
    ship_recalculate_mass_properties(ship);
    
    // Expected COM: (500*0 + 167*(-1.2) + 200*1.0) / (500+167+200)
    expected_y = (500.0f * 0.0f + 167.0f * (-1.2f) + 200.0f * 1.0f) / (500.0f + 167.0f + 200.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expected_y, ship->total_com.y);
    
    destroy_test_ship(ship);
}

// ============================================================================
// THRUSTER SYSTEM TESTS
// ============================================================================

void test_thruster_configuration(void) {
    Ship* ship = create_test_ship();
    
    // Add thruster
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    
    // Configure thruster data
    ShipComponent* thruster = &ship->components[0];
    thruster->data.thruster.max_thrust = 25000.0f;      // 25 kN
    thruster->data.thruster.thrust_direction = (Vector3){0, 0, 1}; // Forward
    thruster->data.thruster.specific_impulse = 445.0f;
    thruster->data.thruster.gimbal_range = 8.5f;
    thruster->data.thruster.current_thrust = 0.0f;      // Off initially
    
    TEST_ASSERT_EQUAL_FLOAT(25000.0f, thruster->data.thruster.max_thrust);
    TEST_ASSERT_EQUAL_FLOAT(445.0f, thruster->data.thruster.specific_impulse);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, thruster->data.thruster.current_thrust);
    
    destroy_test_ship(ship);
}

void test_thruster_force_application(void) {
    Ship* ship = create_test_ship();
    
    // Add thruster pointing up (positive Y)
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    ship_recalculate_mass_properties(ship);
    
    // Configure thruster
    ShipComponent* thruster = &ship->components[0];
    thruster->data.thruster.max_thrust = 25000.0f;
    thruster->data.thruster.thrust_direction = (Vector3){0, 1, 0}; // Up
    thruster->data.thruster.current_thrust = 1.0f; // Full throttle
    
    // Clear any existing forces
    ship->physics->force_accumulator = (Vector3){0, 0, 0};
    ship->physics->torque_accumulator = (Vector3){0, 0, 0};
    
    // Apply thruster forces
    ship_apply_thruster_forces(ship);
    
    // Should have 25kN upward force
    TEST_ASSERT_FLOAT_WITHIN(100.0f, 25000.0f, ship->physics->force_accumulator.y);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 0.0f, ship->physics->force_accumulator.x);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 0.0f, ship->physics->force_accumulator.z);
    
    // Should also have torque due to offset from COM
    float expected_torque_magnitude = 25000.0f * fabsf(ship->total_com.y - (-1.2f));
    float actual_torque_magnitude = sqrtf(
        ship->physics->torque_accumulator.x * ship->physics->torque_accumulator.x +
        ship->physics->torque_accumulator.y * ship->physics->torque_accumulator.y +
        ship->physics->torque_accumulator.z * ship->physics->torque_accumulator.z
    );
    
    TEST_ASSERT_GREATER_THAN_FLOAT(1000.0f, actual_torque_magnitude); // Should have some torque
    
    destroy_test_ship(ship);
}

void test_multiple_thruster_forces(void) {
    Ship* ship = create_test_ship();
    
    // Add two thrusters on opposite sides
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){1.0f, 0.0f, 0.0f}, 100.0f);  // Right
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){-1.0f, 0.0f, 0.0f}, 100.0f); // Left
    ship_recalculate_mass_properties(ship);
    
    // Configure thrusters pointing in opposite directions
    ship->components[0].data.thruster.max_thrust = 10000.0f;
    ship->components[0].data.thruster.thrust_direction = (Vector3){-1, 0, 0}; // Left
    ship->components[0].data.thruster.current_thrust = 1.0f;
    
    ship->components[1].data.thruster.max_thrust = 10000.0f;
    ship->components[1].data.thruster.thrust_direction = (Vector3){1, 0, 0}; // Right
    ship->components[1].data.thruster.current_thrust = 1.0f;
    
    // Clear forces
    ship->physics->force_accumulator = (Vector3){0, 0, 0};
    ship->physics->torque_accumulator = (Vector3){0, 0, 0};
    
    // Apply thruster forces
    ship_apply_thruster_forces(ship);
    
    // Forces should cancel out (both 10kN in opposite directions)
    TEST_ASSERT_FLOAT_WITHIN(100.0f, 0.0f, ship->physics->force_accumulator.x);
    
    // But there should be significant torque (spinning motion)
    float torque_magnitude = sqrtf(
        ship->physics->torque_accumulator.x * ship->physics->torque_accumulator.x +
        ship->physics->torque_accumulator.y * ship->physics->torque_accumulator.y +
        ship->physics->torque_accumulator.z * ship->physics->torque_accumulator.z
    );
    
    TEST_ASSERT_GREATER_THAN_FLOAT(10000.0f, torque_magnitude); // Should have strong torque
    
    destroy_test_ship(ship);
}

void test_thruster_throttle_control(void) {
    Ship* ship = create_test_ship();
    
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    
    ShipComponent* thruster = &ship->components[0];
    thruster->data.thruster.max_thrust = 25000.0f;
    thruster->data.thruster.thrust_direction = (Vector3){0, 1, 0};
    
    // Test different throttle levels
    float throttle_levels[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    
    for (int i = 0; i < 5; i++) {
        thruster->data.thruster.current_thrust = throttle_levels[i];
        
        ship->physics->force_accumulator = (Vector3){0, 0, 0};
        ship_apply_thruster_forces(ship);
        
        float expected_force = 25000.0f * throttle_levels[i];
        TEST_ASSERT_FLOAT_WITHIN(10.0f, expected_force, ship->physics->force_accumulator.y);
    }
    
    destroy_test_ship(ship);
}

// ============================================================================
// COMPONENT INTERACTION TESTS
// ============================================================================

void test_tank_component(void) {
    Ship* ship = create_test_ship();
    
    // Add fuel tank
    ship_add_component(ship, COMPONENT_TANK, (Vector3){0.0f, 1.0f, 0.0f}, 200.0f);
    
    // Configure tank data
    ShipComponent* tank = &ship->components[0];
    tank->data.tank.capacity = 1000.0f;      // 1000kg fuel capacity
    tank->data.tank.current_fuel = 750.0f;   // 75% full
    tank->data.tank.flow_rate = 10.0f;       // 10 kg/s max flow
    
    TEST_ASSERT_EQUAL_FLOAT(1000.0f, tank->data.tank.capacity);
    TEST_ASSERT_EQUAL_FLOAT(750.0f, tank->data.tank.current_fuel);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, tank->data.tank.flow_rate);
    
    destroy_test_ship(ship);
}

void test_complex_ship_assembly(void) {
    Ship* ship = create_test_ship();
    
    // Create a complex ship: hull + main engine + 4 RCS + 2 tanks
    
    // Main engine at bottom
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    
    // RCS thrusters on sides
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){1.0f, 0.0f, 0.0f}, 25.0f);
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){-1.0f, 0.0f, 0.0f}, 25.0f);
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, 0.0f, 1.0f}, 25.0f);
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, 0.0f, -1.0f}, 25.0f);
    
    // Fuel tanks
    ship_add_component(ship, COMPONENT_TANK, (Vector3){0.0f, 1.0f, 0.0f}, 200.0f);
    
    ship_recalculate_mass_properties(ship);
    
    // Check total component count
    TEST_ASSERT_EQUAL_INT(6, ship->component_count);
    
    // Check total mass: 500 (hull) + 167 (main) + 4*25 (RCS) + 200 (tank) = 967kg
    TEST_ASSERT_EQUAL_FLOAT(967.0f, ship->total_mass);
    
    // Configure main thruster
    ship->components[0].data.thruster.max_thrust = 25000.0f;
    ship->components[0].data.thruster.thrust_direction = (Vector3){0, 1, 0};
    ship->components[0].data.thruster.current_thrust = 1.0f;
    
    // Test force application
    ship->physics->force_accumulator = (Vector3){0, 0, 0};
    ship->physics->torque_accumulator = (Vector3){0, 0, 0};
    ship_apply_thruster_forces(ship);
    
    // Should have significant upward force from main engine
    TEST_ASSERT_GREATER_THAN_FLOAT(20000.0f, ship->physics->force_accumulator.y);
    
    destroy_test_ship(ship);
}

// ============================================================================
// ERROR HANDLING AND EDGE CASES
// ============================================================================

void test_invalid_component_attachment(void) {
    Ship* ship = create_test_ship();
    
    // Try to attach to invalid points
    bool result;
    
    result = ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){100.0f, 100.0f, 100.0f}, 167.0f);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(0, ship->component_count);
    
    result = ship_add_component(ship, COMPONENT_TANK, (Vector3){0.0f, -10.0f, 0.0f}, 200.0f);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(0, ship->component_count);
    
    destroy_test_ship(ship);
}

void test_component_capacity_limits(void) {
    Ship* ship = create_test_ship();
    
    // Fill up to max capacity
    for (int i = 0; i < ship->max_components; i++) {
        Vector3 attach_point = pod_basic_vertices[i % pod_basic_mesh.vertex_count];
        bool result = ship_add_component(ship, COMPONENT_THRUSTER, attach_point, 10.0f);
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL_INT(ship->max_components, ship->component_count);
    
    // Try to add one more (should fail)
    bool result = ship_add_component(ship, COMPONENT_TANK, (Vector3){0.0f, -1.2f, 0.0f}, 50.0f);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(ship->max_components, ship->component_count);
    
    destroy_test_ship(ship);
}

void test_zero_mass_components(void) {
    Ship* ship = create_test_ship();
    
    // Add component with zero mass
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 0.0f);
    ship_recalculate_mass_properties(ship);
    
    // Total mass should still be hull mass
    TEST_ASSERT_EQUAL_FLOAT(500.0f, ship->total_mass);
    
    // COM should be unchanged
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ship->total_com.y);
    
    destroy_test_ship(ship);
}

// ============================================================================
// PHYSICS INTEGRATION TESTS
// ============================================================================

void test_ship_physics_simulation(void) {
    Ship* ship = create_test_ship();
    
    // Add upward-pointing thruster
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){0.0f, -1.2f, 0.0f}, 167.0f);
    ship_recalculate_mass_properties(ship);
    
    // Configure thruster for vertical thrust
    ShipComponent* thruster = &ship->components[0];
    thruster->data.thruster.max_thrust = 25000.0f; // 25kN
    thruster->data.thruster.thrust_direction = (Vector3){0, 1, 0}; // Up
    thruster->data.thruster.current_thrust = 1.0f; // Full throttle
    
    // Initial state
    ship->physics->velocity = (Vector3){0, 0, 0};
    ship->transform->position = (Vector3){0, 0, 0};
    
    // Run physics simulation for several frames
    RenderConfig dummy_config = {0};
    
    for (int frame = 0; frame < 10; frame++) {
        // Clear forces
        ship->physics->force_accumulator = (Vector3){0, 0, 0};
        ship->physics->torque_accumulator = (Vector3){0, 0, 0};
        
        // Apply thruster forces
        ship_apply_thruster_forces(ship);
        
        // Run physics update
        physics_system_update(&test_world, &dummy_config, 0.016f);
    }
    
    // Ship should be moving upward
    TEST_ASSERT_GREATER_THAN_FLOAT(0.1f, ship->physics->velocity.y);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.001f, ship->transform->position.y);
    
    destroy_test_ship(ship);
}

void test_ship_rotation_physics(void) {
    Ship* ship = create_test_ship();
    
    // Add thruster offset from center to create rotation
    ship_add_component(ship, COMPONENT_THRUSTER, (Vector3){1.0f, 0.0f, 0.0f}, 167.0f);
    ship_recalculate_mass_properties(ship);
    
    // Configure thruster to create pure torque (force perpendicular to offset)
    ShipComponent* thruster = &ship->components[0];
    thruster->data.thruster.max_thrust = 1000.0f;
    thruster->data.thruster.thrust_direction = (Vector3){0, 1, 0}; // Up (creates Z-axis rotation)
    thruster->data.thruster.current_thrust = 1.0f;
    
    // Enable 6DOF physics for rotation
    physics_set_6dof_enabled(ship->physics, true);
    
    // Initial state
    ship->physics->velocity = (Vector3){0, 0, 0};
    ship->physics->angular_velocity = (Vector3){0, 0, 0};
    
    // Run simulation
    RenderConfig dummy_config = {0};
    
    for (int frame = 0; frame < 10; frame++) {
        ship->physics->force_accumulator = (Vector3){0, 0, 0};
        ship->physics->torque_accumulator = (Vector3){0, 0, 0};
        
        ship_apply_thruster_forces(ship);
        physics_system_update(&test_world, &dummy_config, 0.016f);
    }
    
    // Ship should be rotating around Z-axis
    TEST_ASSERT_GREATER_THAN_FLOAT(0.01f, fabsf(ship->physics->angular_velocity.z));
    
    destroy_test_ship(ship);
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_large_ship_performance(void) {
    Ship* ship = create_test_ship();
    
    // Add maximum number of components
    int components_added = 0;
    for (int i = 0; i < ship->max_components && i < pod_basic_mesh.vertex_count; i++) {
        Vector3 attach_point = pod_basic_vertices[i];
        bool result = ship_add_component(ship, COMPONENT_THRUSTER, attach_point, 25.0f);
        if (result) components_added++;
    }
    
    ship_recalculate_mass_properties(ship);
    
    // Configure all thrusters
    for (int i = 0; i < ship->component_count; i++) {
        ship->components[i].data.thruster.max_thrust = 1000.0f;
        ship->components[i].data.thruster.thrust_direction = (Vector3){0, 1, 0};
        ship->components[i].data.thruster.current_thrust = 1.0f;
    }
    
    // Time the force application
    clock_t start = clock();
    
    for (int iteration = 0; iteration < 1000; iteration++) {
        ship->physics->force_accumulator = (Vector3){0, 0, 0};
        ship->physics->torque_accumulator = (Vector3){0, 0, 0};
        ship_apply_thruster_forces(ship);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete 1000 force calculations quickly (< 10ms)
    TEST_ASSERT_TRUE(elapsed < 0.01);
    
    printf("Performance: %d components, 1000 iterations in %.3f ms\n", 
           components_added, elapsed * 1000.0);
    
    destroy_test_ship(ship);
}

// ============================================================================
// TEST RUNNER
// ============================================================================

void run_simplified_ship_design_tests(void) {
    printf("\n🚀 Simplified Ship Design System Tests\n");
    printf("=====================================\n");
    
    // Basic system tests
    printf("🔧 Testing Basic Component System...\n");
    RUN_TEST(test_ship_creation);
    RUN_TEST(test_attachment_point_validation);
    RUN_TEST(test_component_addition);
    RUN_TEST(test_mass_calculation);
    RUN_TEST(test_center_of_mass_calculation);
    
    // Thruster system tests
    printf("🚀 Testing Thruster System...\n");
    RUN_TEST(test_thruster_configuration);
    RUN_TEST(test_thruster_force_application);
    RUN_TEST(test_multiple_thruster_forces);
    RUN_TEST(test_thruster_throttle_control);
    
    // Component interaction tests
    printf("🔧 Testing Component Interactions...\n");
    RUN_TEST(test_tank_component);
    RUN_TEST(test_complex_ship_assembly);
    
    // Error handling tests
    printf("⚠️  Testing Error Handling...\n");
    RUN_TEST(test_invalid_component_attachment);
    RUN_TEST(test_component_capacity_limits);
    RUN_TEST(test_zero_mass_components);
    
    // Physics integration tests
    printf("⚡ Testing Physics Integration...\n");
    RUN_TEST(test_ship_physics_simulation);
    RUN_TEST(test_ship_rotation_physics);
    
    // Performance tests
    printf("🏃 Testing Performance...\n");
    RUN_TEST(test_large_ship_performance);
    
    printf("✅ Simplified Ship Design Tests Complete\n\n");
}

int main(void) {
    UNITY_BEGIN();
    run_simplified_ship_design_tests();
    return UNITY_END();
}
