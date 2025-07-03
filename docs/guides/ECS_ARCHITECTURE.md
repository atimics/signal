# Entity Component System (ECS) Architecture Guide

This guide explains CGame's Entity Component System architecture, design patterns, and best practices.

## Overview

CGame uses a pure ECS architecture where:
- **Entities** are unique identifiers (just numbers)
- **Components** are data containers (no logic)
- **Systems** process components and implement behavior

This separation provides excellent performance, flexibility, and maintainability.

## Core Concepts

### Entities

An entity is simply a unique identifier:

```c
typedef uint32_t EntityID;

// Create an entity
EntityID player = entity_create(&world);

// Destroy an entity
entity_destroy(&world, player);
```

Entities have no data or behavior - they're just IDs that group components together.

### Components

Components are pure data structures with no logic:

```c
// Example: Transform component
struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
};

// Example: Physics component
struct Physics {
    float mass;
    Vector3 velocity;
    Vector3 force_accumulator;
    bool enable_6dof;
};
```

#### Component Design Rules
1. **No function pointers** - Components are data only
2. **No entity references** - Use EntityID if needed
3. **POD types preferred** - Plain Old Data for easy serialization
4. **Efficient packing** - Consider cache lines (64 bytes)

### Systems

Systems contain all the game logic:

```c
// System function signature
void physics_system_update(struct World* world, RenderConfig* render, float delta_time) {
    // Iterate over entities with physics components
    for (int i = 0; i < world->physics_count; i++) {
        EntityID entity = world->physics_entities[i];
        struct Physics* physics = &world->physics[i];
        struct Transform* transform = entity_get_transform(world, entity);
        
        if (transform && physics) {
            // Update physics
            update_physics(physics, transform, delta_time);
        }
    }
}
```

## Component Management

### Adding Components

```c
// Add single component
entity_add_component(&world, entity, COMPONENT_PHYSICS);

// Add multiple components
entity_add_components(&world, entity, 
    COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_MESH);
```

### Component Masks

Components are tracked using bitmasks for efficient queries:

```c
#define COMPONENT_NONE           0
#define COMPONENT_TRANSFORM      (1 << 0)
#define COMPONENT_PHYSICS        (1 << 1)
#define COMPONENT_MESH           (1 << 2)
#define COMPONENT_THRUSTER_SYSTEM (1 << 3)
// ... etc
```

### Querying Components

```c
// Check if entity has component
if (entity_has_component(&world, entity, COMPONENT_PHYSICS)) {
    struct Physics* physics = entity_get_physics(&world, entity);
    // Use physics...
}

// Get all entities with specific components
ComponentMask mask = COMPONENT_TRANSFORM | COMPONENT_PHYSICS;
EntitySet* entities = world_query_entities(&world, mask);
```

## Memory Layout

### Component Pools

Components are stored in contiguous arrays for cache efficiency:

```c
struct ComponentPools {
    // Arrays of components
    struct Transform transforms[MAX_ENTITIES];
    struct Physics physics[MAX_ENTITIES];
    struct Mesh meshes[MAX_ENTITIES];
    
    // Entity ID arrays (which entities have this component)
    EntityID transform_entities[MAX_ENTITIES];
    EntityID physics_entities[MAX_ENTITIES];
    EntityID mesh_entities[MAX_ENTITIES];
    
    // Component counts
    int transform_count;
    int physics_count;
    int mesh_count;
};
```

This structure ensures:
- **Cache efficiency**: Components processed together are stored together
- **Fast iteration**: No pointer chasing
- **Memory predictability**: Fixed allocation sizes

## System Design Patterns

### 1. Transform Hierarchy

While ECS is flat, parent-child relationships can be modeled:

```c
struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    EntityID parent;  // INVALID_ENTITY if no parent
};
```

### 2. Inter-System Communication

Systems communicate through shared components:

```c
// Input system writes to ControlAuthority
control_system_update(&world, input_state);

// Thruster system reads ControlAuthority, writes forces
thruster_system_update(&world, NULL, delta_time);

// Physics system reads forces, updates Transform
physics_system_update(&world, NULL, delta_time);
```

### 3. Event Handling

For decoupled communication, use an event queue:

```c
struct Event {
    EventType type;
    EntityID source;
    EntityID target;
    void* data;
};

// Systems can emit events
event_queue_push(&world->events, &collision_event);

// Other systems process events
while (event_queue_pop(&world->events, &event)) {
    handle_event(&event);
}
```

## Best Practices

### 1. Component Granularity

Keep components focused and small:

```c
// Good: Separate concerns
struct Position { Vector3 pos; };
struct Velocity { Vector3 vel; };
struct Health { float current, max; };

// Bad: Kitchen sink component
struct GameObject {
    Vector3 position;
    Vector3 velocity;
    float health;
    int score;
    // ... everything ...
};
```

### 2. System Order

Systems should run in a logical order:

```c
void game_update(float delta_time) {
    input_system_update(&world);        // 1. Gather input
    control_system_update(&world);      // 2. Process controls
    thruster_system_update(&world);     // 3. Apply thrust
    physics_system_update(&world);      // 4. Update physics
    collision_system_update(&world);    // 5. Detect collisions
    render_system_update(&world);       // 6. Render frame
}
```

### 3. Avoid Entity Lookups in Tight Loops

Cache component pointers:

```c
// Good: Direct array access
for (int i = 0; i < world->physics_count; i++) {
    struct Physics* phys = &world->physics[i];
    struct Transform* trans = &world->transforms[i];
    update_physics(phys, trans, dt);
}

// Bad: Entity lookup each frame
for (int i = 0; i < entity_count; i++) {
    struct Physics* phys = entity_get_physics(&world, entities[i]);
    struct Transform* trans = entity_get_transform(&world, entities[i]);
    update_physics(phys, trans, dt);
}
```

### 4. Component Initialization

Always initialize components with sensible defaults:

```c
void transform_init(struct Transform* transform) {
    transform->position = (Vector3){0, 0, 0};
    transform->rotation = (Quaternion){0, 0, 0, 1};  // Identity
    transform->scale = (Vector3){1, 1, 1};
    transform->parent = INVALID_ENTITY;
}
```

## Performance Considerations

### Cache Efficiency

- Keep hot data together (position, velocity often accessed together)
- Pad components to cache line boundaries if needed
- Use SOA (Structure of Arrays) layout as shown

### Parallelization

ECS naturally supports parallel processing:

```c
// Different systems can run in parallel if they don't share write access
parallel_execute(
    physics_system_update,
    audio_system_update,
    particle_system_update
);
```

### Memory Allocation

- Pre-allocate component pools
- Use object pools for dynamic components
- Avoid allocations during gameplay

## Common Patterns

### Flying Entity

```c
EntityID create_flying_entity(struct World* world) {
    EntityID entity = entity_create(world);
    
    entity_add_components(world, entity,
        COMPONENT_TRANSFORM |
        COMPONENT_PHYSICS |
        COMPONENT_THRUSTER_SYSTEM |
        COMPONENT_CONTROL_AUTHORITY |
        COMPONENT_MESH);
    
    // Initialize components
    transform_init(entity_get_transform(world, entity));
    physics_init(entity_get_physics(world, entity));
    thruster_init(entity_get_thruster_system(world, entity));
    
    return entity;
}
```

### Static Scenery

```c
EntityID create_scenery(struct World* world, const char* mesh_name) {
    EntityID entity = entity_create(world);
    
    entity_add_components(world, entity,
        COMPONENT_TRANSFORM |
        COMPONENT_MESH);
    
    struct Mesh* mesh = entity_get_mesh(world, entity);
    mesh_set_asset(mesh, mesh_name);
    
    return entity;
}
```

## Debugging ECS

### Entity Inspector

```c
void debug_print_entity(struct World* world, EntityID entity) {
    printf("Entity %d components:\n", entity);
    
    if (entity_has_component(world, entity, COMPONENT_TRANSFORM)) {
        struct Transform* t = entity_get_transform(world, entity);
        printf("  Transform: pos(%.2f,%.2f,%.2f)\n", 
               t->position.x, t->position.y, t->position.z);
    }
    
    if (entity_has_component(world, entity, COMPONENT_PHYSICS)) {
        struct Physics* p = entity_get_physics(world, entity);
        printf("  Physics: vel(%.2f,%.2f,%.2f) mass=%.2f\n",
               p->velocity.x, p->velocity.y, p->velocity.z, p->mass);
    }
    // ... etc
}
```

### Performance Profiling

```c
typedef struct {
    const char* name;
    double total_time;
    int call_count;
} SystemProfile;

void profile_system(SystemProfile* profile, 
                   void (*system_func)(World*, RenderConfig*, float),
                   World* world, float dt) {
    double start = get_time();
    system_func(world, NULL, dt);
    double elapsed = get_time() - start;
    
    profile->total_time += elapsed;
    profile->call_count++;
}
```

## Advanced Topics

### Archetypes

Group commonly used component combinations:

```c
typedef enum {
    ARCHETYPE_FLYING_SHIP = COMPONENT_TRANSFORM | 
                           COMPONENT_PHYSICS | 
                           COMPONENT_THRUSTER_SYSTEM |
                           COMPONENT_MESH,
    
    ARCHETYPE_STATIC_PROP = COMPONENT_TRANSFORM | 
                           COMPONENT_MESH,
    
    ARCHETYPE_TRIGGER = COMPONENT_TRANSFORM | 
                       COMPONENT_COLLISION
} Archetype;

EntityID entity_create_archetype(World* world, Archetype arch) {
    EntityID entity = entity_create(world);
    entity_add_components(world, entity, arch);
    return entity;
}
```

### Component Versioning

Track component modifications for networking:

```c
struct VersionedTransform {
    struct Transform transform;
    uint32_t version;
};

void transform_set_position(VersionedTransform* vt, Vector3 pos) {
    vt->transform.position = pos;
    vt->version++;
}
```

## Summary

CGame's ECS architecture provides:
- **Performance**: Cache-efficient memory layout
- **Flexibility**: Easy to add new components/systems
- **Maintainability**: Clear separation of concerns
- **Scalability**: Naturally parallelizable

Follow these patterns and best practices to create efficient, maintainable game systems that integrate smoothly with the existing architecture.