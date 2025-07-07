# Component Reference

This document provides a comprehensive reference for all components available in the CGame engine. Components are pure data containers that define entity properties.

## Core Components

These components are defined in `src/core.h` and form the foundation of the entity system.

### Transform
**Location**: `src/core.h`  
**Purpose**: Defines an entity's position, rotation, and scale in world space.

**Fields**:
- `position` (Vector3): World position
- `rotation` (Quaternion): Orientation
- `scale` (Vector3): Per-axis scale factors

**Usage**:
```c
Transform* transform = world_add_component(world, entity, COMPONENT_TRANSFORM);
transform->position = (Vector3){0, 10, 0};
transform->rotation = quaternion_identity();
transform->scale = (Vector3){1, 1, 1};
```

### Physics
**Location**: `src/core.h`  
**Purpose**: Enables 6DOF physics simulation with forces and torques.

**Key Fields**:
- `mass`: Mass in kg
- `linear_velocity`: Current velocity vector
- `angular_velocity`: Rotation speed (rad/s)
- `force_accumulator`: Forces applied this frame
- `torque_accumulator`: Torques applied this frame
- `drag_linear/angular`: Drag coefficients
- `has_6dof`: Enable full rotational physics
- `kinematic`: Not affected by forces
- `use_ode`: Use ODE physics engine

**Usage**:
```c
Physics* physics = world_add_component(world, entity, COMPONENT_PHYSICS);
physics->mass = 1000.0f;
physics->has_6dof = true;
physics->drag_linear = 0.5f;
physics->drag_angular = 1.0f;
```

### Collision
**Location**: `src/core.h`  
**Purpose**: Defines collision shapes and properties for physics interactions.

**Fields**:
- `shape`: COLLISION_SPHERE, COLLISION_BOX, or COLLISION_CAPSULE
- `radius/box_size/capsule`: Shape-specific dimensions
- `is_trigger`: Ghost object (detects but doesn't collide)
- `layer_mask`: Collision filtering

### AI
**Location**: `src/core.h`  
**Purpose**: Controls AI entity behavior and decision-making.

**States**:
- AI_STATE_IDLE
- AI_STATE_PATROLLING
- AI_STATE_REACTING
- AI_STATE_COMMUNICATING
- AI_STATE_FLEEING

**Fields**:
- `state`: Current AI state
- `target_position`: Movement target
- `target_entity`: Entity to track/follow
- `update_frequency`: AI think rate (2-10 Hz)

### Renderable
**Location**: `src/core.h`  
**Purpose**: Marks an entity for rendering with mesh and material data.

**Fields**:
- `gpu_resources`: GPU buffer handles
- `index_count`: Number of indices to draw
- `material_id`: Material/shader selection
- `visible`: Visibility flag
- `lod_level`: Current LOD (0=highest detail)

### Camera
**Location**: `src/core.h`  
**Purpose**: Defines viewpoint for rendering the scene.

**Fields**:
- `position/target/up`: View vectors
- `fov`: Field of view in degrees
- `near_plane/far_plane`: Clipping distances
- `behavior`: THIRD_PERSON, FIRST_PERSON, STATIC, CHASE, ORBITAL
- `follow_target`: Entity to follow (for chase cameras)
- `is_active`: Currently rendering camera

### SceneNode
**Location**: `src/core.h`  
**Purpose**: Hierarchical scene graph relationships.

**Fields**:
- `parent`: Parent entity ID
- `children[]`: Child entity array
- `local_transform`: Transform relative to parent
- `world_transform`: Final world space transform

### ThrusterSystem
**Location**: `src/core.h`  
**Purpose**: Universal propulsion system for spacecraft and vehicles.

**Features**:
- Multiple thruster groups (main, maneuvering, retro)
- Per-thruster force vectors
- Fuel consumption tracking
- Visual effects integration

### ControlAuthority
**Location**: `src/core.h`  
**Purpose**: Maps player/AI input to entity actions.

**Fields**:
- Input mapping configuration
- Control sensitivity curves
- Authority levels (player vs AI)
- Input filtering parameters

## Specialized Components

These components provide additional functionality for specific game features.

### Controllable
**Location**: `src/component/controllable.h`  
**Purpose**: Allows entities to receive and process input commands.

**Features**:
- Input-to-action mapping
- Control scheme selection
- Response curves
- Dead zone configuration

### LookTarget
**Location**: `src/component/look_target.h`  
**Purpose**: Implements look-at targeting for canyon racing gameplay.

**Fields**:
- `target_position`: Where the entity is looking
- `look_direction`: Normalized direction vector
- `tracking_speed`: How fast to track targets

### ThrusterPointsComponent
**Location**: `src/component/thruster_points_component.h`  
**Purpose**: Simplified thruster system using discrete thrust points.

**Features**:
- Fixed thrust point positions
- Per-point force magnitudes
- Simplified control mapping
- Lower computational cost than full ThrusterSystem

## Component Patterns

### Adding Components
```c
// Add single component
Transform* transform = world_add_component(world, entity, COMPONENT_TRANSFORM);

// Add multiple components
world_add_components(world, entity,
    COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_RENDERABLE);
```

### Querying Components
```c
// Check if entity has component
if (world_has_component(world, entity, COMPONENT_PHYSICS)) {
    Physics* physics = world_get_component(world, entity, COMPONENT_PHYSICS);
    // Use physics...
}

// Iterate entities with specific components
ComponentIterator it = world_iterate_components(world, COMPONENT_PHYSICS);
while (component_iterator_next(&it)) {
    EntityID entity = it.entity;
    Physics* physics = it.component;
    // Process physics...
}
```

### Component Lifecycle
1. **Creation**: Components are zero-initialized when added
2. **Configuration**: Set initial values after creation
3. **Runtime**: Systems read/write component data
4. **Removal**: Components are automatically cleaned up

## Best Practices

1. **Keep Components Pure Data**: No logic, only state
2. **Use Composition**: Combine simple components for complex behavior
3. **Minimize Component Size**: Cache-friendly data layouts
4. **Avoid Pointers**: Use entity IDs for references
5. **Initialize Properly**: Set all fields to sensible defaults

## Performance Considerations

- Components are stored in contiguous arrays for cache efficiency
- Hot/cold data splitting: frequently accessed fields first
- Component pools minimize allocation overhead
- Bitset queries enable fast entity filtering

## See Also

- [System Reference](SYSTEM_REFERENCE.md)
- [ECS Architecture](../architecture/ECS_ARCHITECTURE.md)
- [Entity Creation Guide](../tutorials/ENTITY_CREATION.md)