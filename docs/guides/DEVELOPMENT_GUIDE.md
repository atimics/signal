# Development Guide

**Status:** Active
**Last Updated:** June 29, 2025
**Owner:** System Analysis

This guide provides all the necessary information for developers to get started with the CGame engine project. It covers environment setup, build procedures, coding standards, and the overall development workflow.

/**
 * @file DEVELOPMENT_GUIDE.md
 * @brief CGame ECS Engine - Developer Documentation and Code Conventions
 * 
 * This file provides comprehensive documentation for developers working on the
 * CGame Entity-Component-System engine. It serves as a reference for code
 * organization, architectural decisions, and development practices.
 */

# CGame ECS Engine - Development Guide

## Project Structure and Philosophy

### Entity-Component-System Architecture
The engine is built around a pure ECS architecture where:
- **Entities** are unique identifiers with component masks
- **Components** contain only data, no behavior
- **Systems** process components and implement all game logic

### Data-Oriented Design
- Components are stored in contiguous arrays for cache efficiency
- Systems iterate over component arrays, not entity lists
- Memory layout optimized for performance over convenience
- Minimal dynamic allocation during runtime

## File Organization and Responsibilities

### Core Engine Files

#### `src/core.h` - ECS Foundation
- Entity and component type definitions
- World state management
- Component storage pools
- Core API declarations

**When to modify**: Adding new component types, changing entity limits, core architecture changes

#### `src/core.c` - ECS Implementation
- Entity creation/destruction
- Component allocation/deallocation
- Component accessor functions
- Memory management

**When to modify**: Implementing new component types, optimizing memory layout

#### `src/systems.h` - System Declarations
- System function signatures
- System scheduler definitions
- Performance tracking structures

**When to modify**: Adding new systems, changing system interfaces

#### `src/systems.c` - Game Logic Implementation
- All system update functions
- System scheduler implementation
- Inter-system communication

**When to modify**: Implementing new gameplay features, optimizing system performance

### Support Systems

#### `src/assets.h/.c` - Asset Management
- Mesh, texture, and material loading
- Asset registry and caching
- OBJ file parsing

**When to modify**: Adding new asset types, optimizing loading performance

#### `src/data.h/.c` - Data-Driven Configuration
- Entity template system
- Scene loading and configuration
- Runtime data management

**When to modify**: Extending template system, adding new configuration options

#### `src/render_3d.c` - 3D Rendering Pipeline
- SDL2 3D rendering implementation
- Matrix transformations
- Camera management

**When to modify**: Improving rendering performance, adding visual features

#### `src/ui.h/.c` - Debug Interface
- Performance monitoring
- Entity inspection tools
- Development utilities

**When to modify**: Adding new debug features, improving developer experience

#### `src/test.c` - Main Entry Point
- Application initialization
- Scene setup and testing
- Entity factory functions

**When to modify**: Creating new test scenarios, debugging engine features

## Component Design Guidelines

### Component Structure Template
```c
struct NewComponent {
    // Pure data fields only - NO function pointers
    float some_value;
    Vector3 some_position;
    bool some_flag;
    
    // Optional: metadata for system optimization
    uint32_t last_update_frame;  // For temporal optimizations
    float cache_distance;        // For spatial optimizations
};
```

### Component Design Rules
1. **Pure Data**: No function pointers, no behavior
2. **POD Types**: Plain Old Data structures only
3. **Cache-Friendly**: Organize fields by access patterns
4. **Minimal Size**: Remove unnecessary fields
5. **Clear Names**: Self-documenting field names

### Adding a New Component
1. Define struct in `src/core.h`
2. Add to `ComponentType` enum
3. Add array to `ComponentPools` struct
4. Add pointer to `Entity` struct
5. Implement add/remove/get functions in `src/core.c`
6. Update relevant systems to process the component

## System Design Guidelines

### System Structure Template
```c
void new_system_update(struct World* world, float delta_time) {
    // Performance tracking (optional)
    static uint32_t call_count = 0;
    call_count++;
    
    // Iterate over entities with required components
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Check component mask - early exit if missing components
        if (!(entity->component_mask & (COMPONENT_REQUIRED1 | COMPONENT_REQUIRED2))) {
            continue;
        }
        
        // Get component pointers (these should not be NULL after mask check)
        struct RequiredComponent1* comp1 = entity->required_component1;
        struct RequiredComponent2* comp2 = entity->required_component2;
        
        // Optional components - check for NULL
        struct OptionalComponent* opt = entity_get_optional_component(world, entity->id);
        
        // Process components - implement system logic here
        // Modify component data, don't call other systems directly
        comp1->value += comp2->delta * delta_time;
        
        if (opt && opt->enabled) {
            // Handle optional component logic
        }
    }
}
```

### System Design Rules
1. **Single Responsibility**: Each system handles one aspect of game logic
2. **Component Iteration**: Process components, not entities
3. **No Direct Calls**: Systems communicate through events or shared data
4. **Performance Aware**: Consider cache efficiency and algorithm complexity
5. **Robust Error Handling**: Handle missing components gracefully

### System Scheduling
Systems run at different frequencies based on their computational cost and requirements:
- **Physics**: 60 FPS (every frame) - cheap, affects visual output
- **Collision**: 20 FPS (every 3 frames) - expensive spatial queries
- **AI**: 2-10 FPS (LOD-based) - very expensive decision making
- **Render**: 60 FPS (every frame) - output system

## Entity Factory Patterns

### Factory Function Template
```c
EntityID create_entity_type(struct World* world, Vector3 position, const char* config) {
    EntityID id = entity_create(world);
    if (id == INVALID_ENTITY) {
        printf("❌ Failed to create entity: world full\n");
        return INVALID_ENTITY;
    }
    
    // Add required components
    entity_add_component(world, id, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    // Configure components
    struct Transform* transform = entity_get_transform(world, id);
    if (transform) {
        transform->position = position;
        transform->scale = (Vector3){1.0f, 1.0f, 1.0f};
    }
    
    struct Physics* physics = entity_get_physics(world, id);
    if (physics) {
        physics->mass = 100.0f;
        physics->drag = 0.99f;
    }
    
    // Optional components based on configuration
    if (strcmp(config, "with_ai") == 0) {
        entity_add_component(world, id, COMPONENT_AI);
        struct AI* ai = entity_get_ai(world, id);
        if (ai) {
            ai->state = AI_STATE_IDLE;
            ai->update_frequency = 5.0f;
        }
    }
    
    printf("✨ Created %s entity at (%.1f, %.1f, %.1f)\n", 
           config, position.x, position.y, position.z);
    
    return id;
}
```

## Performance Optimization Guidelines

### Memory Access Patterns
```c
// ✅ Good: Cache-friendly component iteration
for (uint32_t i = 0; i < world->entity_count; i++) {
    struct Entity* entity = &world->entities[i];
    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
    
    struct Physics* physics = entity->physics;
    // Process physics component...
}

// ❌ Bad: Random memory access via entity lookups
for (uint32_t i = 0; i < physics_entity_count; i++) {
    EntityID id = physics_entities[i];
    struct Entity* entity = entity_get(world, id);  // Slow lookup
    // Process entity...
}
```

### Level-of-Detail (LOD) Implementation
```c
void ai_system_update(struct World* world, float delta_time) {
    // Get player position for LOD calculations
    Vector3 player_pos = get_player_position(world);
    
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (!(entity->component_mask & COMPONENT_AI)) continue;
        
        struct AI* ai = entity->ai;
        struct Transform* transform = entity->transform;
        
        // Calculate distance-based update frequency
        float distance = vector3_distance(transform->position, player_pos);
        float update_interval = calculate_ai_interval(distance);
        
        // Skip update if not time yet
        if (world->total_time - ai->last_update < update_interval) {
            continue;
        }
        
        // Perform AI update
        ai_process_behavior(ai, world, delta_time);
        ai->last_update = world->total_time;
    }
}
```

## Error Handling and Debugging

### Component Access Safety
```c
// ✅ Always check component existence
struct Transform* transform = entity_get_transform(world, entity_id);
if (!transform) {
    printf("⚠️  Entity %d missing transform component\n", entity_id);
    return;
}

// ✅ Use component masks for batch checks
if (!(entity->component_mask & (COMPONENT_TRANSFORM | COMPONENT_PHYSICS))) {
    // Skip entities missing required components
    continue;
}
```

### Debug Output Guidelines
```c
// Use emoji prefixes for categorization
printf("🚀 Entity created: player ship\n");      // Creation
printf("⚡ Physics update: velocity changed\n");   // System updates
printf("❌ Error: failed to load asset\n");        // Errors
printf("⚠️  Warning: component missing\n");        // Warnings
printf("🎯 System: AI processing complete\n");     // System status
printf("📊 Performance: %d entities processed\n", world->entity_count); // Performance
```

### Performance Monitoring
```c
// System timing measurement
static float total_time = 0.0f;
static uint32_t call_count = 0;

float start_time = get_current_time();
// ... system processing ...
float elapsed = get_current_time() - start_time;

total_time += elapsed;
call_count++;

if (call_count % 300 == 0) {  // Report every 5 seconds at 60 FPS
    printf("📊 %s system: avg %.3fms per call\n", 
           system_name, (total_time / call_count) * 1000.0f);
}
```

## Data-Driven Development

### Entity Template Format
```
# data/templates/entities.txt
template fighter_ship {
    name: "Fighter Ship"
    description: "Fast, maneuverable combat vessel"
    
    # Required components
    components: transform physics collision ai renderable
    
    # Component configuration
    mass: 50.0
    drag: 0.95
    collision_radius: 3.0
    collision_layers: 0x0001
    
    ai_state: patrolling
    ai_frequency: 8.0
    
    mesh: "fighter"
    material: "fighter_blue"
    visible: true
}
```

### Scene Definition Format
```
# data/scenes/combat_zone.txt
scene combat_zone {
    description: "Small-scale combat demonstration"
    
    # Player spawn
    player: player_ship at (0, 0, 0)
    
    # AI entities
    spawn fighter_ship at (100, 0, 50) faction blue patrol_route circle
    spawn fighter_ship at (-100, 0, -50) faction red patrol_route line
    
    # Environment
    spawn asteroid_field at (0, 0, 200) count 20 radius 100
    spawn station at (500, 0, 0) faction neutral
}
```

## Testing and Validation

### Unit Testing Patterns
```c
// Component system test
bool test_component_system() {
    struct World world;
    world_init(&world);
    
    // Test entity creation
    EntityID entity = entity_create(&world);
    assert(entity != INVALID_ENTITY);
    
    // Test component addition
    bool result = entity_add_component(&world, entity, COMPONENT_TRANSFORM);
    assert(result == true);
    
    // Test component access
    struct Transform* transform = entity_get_transform(&world, entity);
    assert(transform != NULL);
    
    // Test component removal
    entity_remove_component(&world, entity, COMPONENT_TRANSFORM);
    transform = entity_get_transform(&world, entity);
    assert(transform == NULL);
    
    world_destroy(&world);
    return true;
}
```

### Performance Benchmarking
```c
void benchmark_entity_creation() {
    struct World world;
    world_init(&world);
    
    clock_t start = clock();
    
    for (int i = 0; i < 1000; i++) {
        EntityID entity = entity_create(&world);
        entity_add_component(&world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("📊 Created 1000 entities in %.3f seconds (%.1f entities/sec)\n",
           time_taken, 1000.0 / time_taken);
    
    world_destroy(&world);
}
```

## Code Style and Conventions

### Naming Conventions
```c
// Files: snake_case
// src/physics_system.c, src/render_3d.c

// Functions: module_action format
bool entity_create(struct World* world);
void physics_system_update(struct World* world, float delta_time);
struct Transform* entity_get_transform(struct World* world, EntityID id);

// Structs: PascalCase for types, snake_case for instances
struct Transform player_transform;
struct Physics enemy_physics;

// Constants and enums: UPPER_SNAKE_CASE
#define MAX_ENTITIES 4096
typedef enum {
    COMPONENT_TRANSFORM,
    COMPONENT_PHYSICS
} ComponentType;

// Variables: snake_case with descriptive names
float delta_time;
uint32_t entity_count;
Vector3 player_position;
```

### Comment Guidelines
```c
// Brief function description
void physics_system_update(struct World* world, float delta_time) {
    // Performance optimization: early exit if no physics entities
    if (world->components.physics_count == 0) {
        return;
    }
    
    // Process all entities with physics components
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Skip entities without physics
        if (!(entity->component_mask & COMPONENT_PHYSICS)) {
            continue;
        }
        
        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;
        
        // Apply velocity to position (basic integration)
        Vector3 velocity_delta = vector3_multiply(physics->velocity, delta_time);
        transform->position = vector3_add(transform->position, velocity_delta);
        
        // Apply drag to velocity
        physics->velocity = vector3_multiply(physics->velocity, physics->drag);
    }
}
```

## Future Development Guidelines

### Extensibility Considerations
- Design systems to be independent and loosely coupled
- Use event systems for cross-system communication
- Maintain clear interfaces between modules
- Consider performance implications of new features

### Architectural Evolution
- Keep the ECS pattern pure - components are data, systems are behavior
- Maintain cache-friendly data layouts as the engine grows
- Consider multithreading opportunities for expensive systems
- Plan for networking by keeping systems deterministic

This development guide should be updated as the engine evolves and new patterns emerge. The goal is to maintain consistency and quality while enabling rapid development and experimentation.