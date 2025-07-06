# System Reference

This document provides a comprehensive reference for all systems in the CGame engine. Systems contain the game logic that operates on component data.

## Core Systems

### Physics System
**Location**: `src/system/physics.h`  
**Purpose**: Updates entity positions based on forces and velocities using 6DOF physics.

**Components Required**: Transform, Physics  
**Update Frequency**: Every frame (60Hz)

**Features**:
- Linear and angular velocity integration
- Force and torque accumulation
- Drag simulation (linear and angular)
- Kinematic entity support
- World-to-local force transformation
- Gravitational alignment for spatial orientation

**Key Functions**:
- `physics_apply_force()`: Apply force at entity center
- `physics_apply_force_at_point()`: Apply force with torque
- `physics_apply_torque()`: Apply rotational force
- `physics_clear_forces()`: Reset accumulators

### Collision System
**Location**: `src/system/collision.h`  
**Purpose**: Detects and resolves collisions between entities.

**Components Required**: Transform, Collision  
**Update Frequency**: Every frame after physics

**Features**:
- Sphere, box, and capsule collision shapes
- Trigger volumes (ghost objects)
- Layer-based filtering
- Collision callbacks
- Spatial partitioning for performance

### Camera System
**Location**: `src/system/camera.h`  
**Purpose**: Manages camera entities for scene viewing.

**Components Required**: Camera, Transform  
**Update Frequency**: Every frame before rendering

**Camera Behaviors**:
- **Third Person**: Follows target with offset
- **First Person**: Attached to entity view
- **Static**: Fixed position/orientation
- **Chase**: Smooth follow with lag
- **Orbital**: Circles around target

**Features**:
- View/projection matrix calculation
- Frustum culling support
- Smooth follow interpolation
- Multiple camera support

### AI System
**Location**: `src/system/ai.h`  
**Purpose**: Updates AI-controlled entities with behavior logic.

**Components Required**: AI, Transform, Physics  
**Update Frequency**: Variable (2-10Hz based on distance)

**AI States**:
- Idle: Stationary, observing
- Patrolling: Following waypoints
- Reacting: Responding to stimuli
- Communicating: Interacting with others
- Fleeing: Escaping threats

**Features**:
- State machine architecture
- LOD-based update frequency
- Target tracking
- Path planning hooks

### Control System
**Location**: `src/system/control.h`  
**Purpose**: Processes player input and applies to controllable entities.

**Components Required**: ControlAuthority, Transform, Physics  
**Update Frequency**: Every frame

**Features**:
- Input mapping and remapping
- Dead zone compensation
- Sensitivity curves
- Multiple control schemes
- Gamepad and keyboard support

### Thruster System
**Location**: `src/system/thrusters.h`  
**Purpose**: Manages spacecraft propulsion systems.

**Components Required**: ThrusterSystem, Transform, Physics  
**Update Frequency**: Every frame

**Features**:
- Multiple thruster groups (main, maneuvering, retro)
- Automatic thrust vectoring
- Fuel consumption simulation
- Visual effect triggers
- 6DOF thrust application

### Thruster Points System
**Location**: `src/system/thruster_points_system.h`  
**Purpose**: Simplified thrust system using discrete points.

**Components Required**: ThrusterPointsComponent, Transform, Physics  
**Update Frequency**: Every frame

**Advantages**:
- Lower computational overhead
- Easier to configure
- Predictable behavior
- Good for arcade-style games

### LOD System
**Location**: `src/system/lod.h`  
**Purpose**: Adjusts entity detail based on camera distance.

**Components Required**: Renderable, Transform  
**Update Frequency**: 4Hz (configurable)

**Features**:
- Distance-based LOD selection
- Smooth LOD transitions
- Per-entity LOD bias
- Performance scaling

### Material System
**Location**: `src/system/material.h`  
**Purpose**: Manages material properties and shader parameters.

**Components Required**: Renderable  
**Update Frequency**: On demand

**Features**:
- Material property management
- Shader uniform updates
- Texture binding
- Material hot-reloading

### Lighting System
**Location**: `src/system/lighting.h`  
**Purpose**: Calculates lighting for the scene.

**Components Required**: Light (component not yet defined)  
**Update Frequency**: Per frame or on change

**Light Types**:
- Directional (sun)
- Point (omni)
- Spot (cone)
- Ambient

**Features**:
- Forward rendering path
- Shadow mapping ready
- Light culling
- HDR support

### Performance System
**Location**: `src/system/performance.h`  
**Purpose**: Monitors and reports engine performance metrics.

**Update Frequency**: Configurable (typically 1Hz)

**Metrics Tracked**:
- Frame time (min/max/avg)
- System update times
- Draw call counts
- Entity counts
- Memory usage

### Memory System
**Location**: `src/system/memory.h`  
**Purpose**: Manages engine memory allocation and tracking.

**Features**:
- Custom allocators
- Memory pools
- Allocation tracking
- Leak detection
- Fragmentation monitoring

### Scripted Flight System
**Location**: `src/system/scripted_flight.h`  
**Purpose**: Moves entities along predefined paths for cinematics.

**Components Required**: Transform, Physics (optional)  
**Update Frequency**: Every frame when active

**Features**:
- Bezier curve paths
- Speed control
- Orientation control
- Event triggers
- Looping support

### ODE Physics System
**Location**: `src/system/ode_physics.h`  
**Purpose**: Integration with Open Dynamics Engine for advanced physics.

**Components Required**: Transform, Physics (with use_ode=true)  
**Update Frequency**: Fixed timestep (typically 120Hz)

**Features**:
- Rigid body dynamics
- Joint constraints
- Advanced collision detection
- Vehicle simulation
- Ragdoll physics

## System Patterns

### System Updates
```c
void my_system_update(World* world, float delta_time) {
    ComponentIterator it = world_iterate_components(world, 
        COMPONENT_TRANSFORM | COMPONENT_MY_COMPONENT);
    
    while (component_iterator_next(&it)) {
        Transform* transform = world_get_component(world, it.entity, COMPONENT_TRANSFORM);
        MyComponent* my_comp = world_get_component(world, it.entity, COMPONENT_MY_COMPONENT);
        
        // Update logic here
    }
}
```

### System Registration
```c
// In systems_init()
system_register("my_system", my_system_update, 
    COMPONENT_TRANSFORM | COMPONENT_MY_COMPONENT);
```

### System Ordering
Systems execute in a defined order each frame:
1. Input processing
2. AI updates
3. Physics simulation
4. Collision detection
5. Camera updates
6. Rendering preparation
7. Performance monitoring

## Best Practices

1. **Cache Component Lookups**: Store pointers when iterating
2. **Minimize Component Queries**: Use iterator pattern
3. **Respect Update Frequencies**: Not all systems need 60Hz
4. **Use System Dependencies**: Define clear execution order
5. **Profile System Performance**: Monitor update times

## Performance Guidelines

- Keep system updates under 1ms each
- Use spatial partitioning for O(n²) algorithms
- Batch similar operations together
- Consider SIMD for math-heavy systems
- Use job system for parallel updates (future)

## Creating New Systems

1. Define system header in `src/system/`
2. Implement update function
3. Register in `systems_init()`
4. Add to build system
5. Document in this reference

## See Also

- [Component Reference](COMPONENT_REFERENCE.md)
- [ECS Architecture](../architecture/ECS_ARCHITECTURE.md)
- [System Creation Tutorial](../tutorials/SYSTEM_CREATION.md)