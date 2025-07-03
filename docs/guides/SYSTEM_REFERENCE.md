# System Reference

This document provides a complete reference for all systems in the CGame ECS architecture. Systems contain the game logic and process components each frame.

## System Overview

Systems in CGame follow a consistent pattern:
- Process specific component types
- Run in a defined order each frame
- Communicate through shared components
- Have no internal state (stateless design)

### System Function Signature

```c
typedef void (*SystemUpdateFunc)(struct World* world, RenderConfig* config, float delta_time);
```

## Core Systems

### Transform System
**Purpose**: Manages spatial hierarchies and transform updates  
**Processes**: Transform components  
**Source**: `src/system/transform.c`

**Responsibilities**:
- Update world matrices from local transforms
- Handle parent-child relationships
- Optimize transform calculations

**Dependencies**: None (runs first)

### Physics System
**Purpose**: Simulates physics for all entities  
**Processes**: Physics + Transform components  
**Source**: `src/system/physics.c`

**Responsibilities**:
- Integrate forces and torques
- Update velocities and positions
- Apply drag and environmental effects
- Handle 6DOF rotation

**Dependencies**: Runs after force-generating systems

**Key Functions**:
```c
void physics_system_update(World* world, RenderConfig* config, float delta_time);
void physics_apply_force(Physics* physics, Vector3 force);
void physics_apply_torque(Physics* physics, Vector3 torque);
```

### Input System
**Purpose**: Processes player input from keyboard and gamepad  
**Processes**: Global input state  
**Source**: `src/system/input.c`

**Responsibilities**:
- Poll input devices
- Update input state
- Handle device hot-plugging
- Apply dead zones and calibration

**Dependencies**: Runs first in update cycle

## Movement Systems

### Control System
**Purpose**: Interprets input for entity control  
**Processes**: ControlAuthority components  
**Source**: `src/system/control.c`

**Responsibilities**:
- Map input to control commands
- Apply control authority scaling
- Handle auto-deceleration
- Support multiple control modes

**Dependencies**: Runs after Input System

### Thruster System
**Purpose**: Converts thrust commands to physics forces  
**Processes**: ThrusterSystem + Physics + Transform  
**Source**: `src/system/thruster.c`

**Responsibilities**:
- Transform thrust commands to world space
- Apply environmental efficiency
- Generate forces and torques
- Handle thrust ramping

**Dependencies**: 
- Requires: Control System (for commands)
- Before: Physics System (generates forces)

**Key Functions**:
```c
void thruster_system_update(World* world, RenderConfig* config, float delta_time);
void thruster_set_linear_command(ThrusterSystem* thrust, Vector3 command);
void thruster_set_angular_command(ThrusterSystem* thrust, Vector3 command);
```

### Scripted Flight System
**Purpose**: Autonomous waypoint-based movement  
**Processes**: ScriptedFlight components  
**Source**: `src/system/scripted_flight.c`

**Responsibilities**:
- Follow predefined flight paths
- Control entity through thruster system
- Handle waypoint transitions
- Support multiple path types

**Dependencies**:
- Requires: Transform, Physics, ThrusterSystem
- Before: Physics System

## Rendering Systems

### Camera System
**Purpose**: Manages viewports and camera movement  
**Processes**: Camera + Transform components  
**Source**: `src/system/camera.c`

**Responsibilities**:
- Update view matrices
- Handle camera following
- Smooth camera movements
- Support multiple camera modes

**Dependencies**: After Transform System

### Render System
**Purpose**: Main rendering pipeline  
**Processes**: Mesh + Transform components  
**Source**: `src/system/render.c`

**Responsibilities**:
- Frustum culling
- Draw call batching
- Material sorting
- GPU state management

**Dependencies**: Runs last in frame

### Lighting System
**Purpose**: Dynamic lighting calculations  
**Processes**: Light + Transform components  
**Source**: `src/system/lighting.c`

**Responsibilities**:
- Update light uniforms
- Shadow map generation
- Light culling
- Ambient lighting

**Dependencies**: Before Render System

### Particle System
**Purpose**: Particle effect simulation  
**Processes**: ParticleEmitter + Transform  
**Source**: `src/system/particle.c`

**Responsibilities**:
- Spawn particles
- Update particle positions
- Handle particle death
- Render particle batches

**Dependencies**: After Physics, before Render

## Gameplay Systems

### Collision System
**Purpose**: Collision detection and response  
**Processes**: Collision + Transform components  
**Source**: `src/system/collision.c`

**Responsibilities**:
- Broad phase detection
- Narrow phase testing
- Trigger volume handling
- Collision callbacks

**Dependencies**: After Physics System

### Health System
**Purpose**: Damage and health management  
**Processes**: Health components  
**Source**: `src/system/health.c`

**Responsibilities**:
- Apply damage
- Handle regeneration
- Death detection
- Invulnerability timing

**Dependencies**: After Collision System

### AI System
**Purpose**: NPC behavior and decision making  
**Processes**: AI + Transform + Physics  
**Source**: `src/system/ai.c`

**Responsibilities**:
- Behavior state machines
- Target selection
- Pathfinding
- Command generation

**Dependencies**: Before Control System

## UI Systems

### HUD System
**Purpose**: Heads-up display rendering  
**Processes**: UIElement components + game state  
**Source**: `src/system/hud.c`

**Responsibilities**:
- Update UI elements
- Handle screen scaling
- Process UI events
- Render UI batches

**Dependencies**: After game logic, before final render

### Menu System
**Purpose**: Menu navigation and rendering  
**Processes**: Menu state  
**Source**: `src/system/menu.c`

**Responsibilities**:
- Menu state management
- Input handling
- Transition effects
- Settings management

**Dependencies**: Separate from game loop

## System Execution Order

The standard frame execution order:

```c
// Input Phase
1. input_system_update()        // Poll devices

// Logic Phase
2. ai_system_update()           // AI decisions
3. control_system_update()      // Process controls
4. thruster_system_update()     // Generate forces
5. scripted_flight_update()     // Autonomous movement
6. physics_system_update()      // Integrate physics
7. collision_system_update()    // Detect collisions
8. health_system_update()       // Process damage

// Rendering Phase
9. transform_system_update()    // Update matrices
10. camera_system_update()      // Update view
11. lighting_system_update()    // Prepare lights
12. particle_system_update()    // Update particles
13. hud_system_update()         // Update UI
14. render_system_update()      // Draw frame
```

## System Communication

Systems communicate through components:

```
Input → ControlAuthority → ThrusterSystem → Physics → Transform → Render
                ↓                               ↑
                AI ─────────────────────────────┘
```

### Event System
For decoupled communication:

```c
// Collision system emits event
event_emit(world, EVENT_COLLISION, &collision_data);

// Health system handles event
void health_handle_collision(World* world, Event* event) {
    // Apply damage based on collision
}
```

## Performance Considerations

### System Optimization
1. **Cache Efficiency**: Process components in arrays
2. **Parallelization**: Independent systems can run parallel
3. **Early Exit**: Skip processing when no components
4. **Batch Operations**: Group similar operations

### Profiling Systems
```c
// Time system execution
double start = get_time();
physics_system_update(world, config, dt);
double physics_time = get_time() - start;
```

## Adding New Systems

### System Template
```c
// Header: src/system/new_system.h
void new_system_update(World* world, RenderConfig* config, float delta_time);
void new_system_init(void);
void new_system_cleanup(void);

// Implementation: src/system/new_system.c
void new_system_update(World* world, RenderConfig* config, float delta_time) {
    // Early exit if no components
    if (world->new_component_count == 0) return;
    
    // Process all components
    for (int i = 0; i < world->new_component_count; i++) {
        EntityID entity = world->new_component_entities[i];
        NewComponent* comp = &world->new_components[i];
        
        // System logic here
        process_component(comp, delta_time);
    }
}
```

### Integration Steps
1. Create system source files
2. Add to Makefile
3. Insert in frame execution order
4. Write unit tests
5. Profile performance
6. Document in this reference

## System Best Practices

### Design Principles
1. **Stateless**: No persistent state in systems
2. **Deterministic**: Same input = same output
3. **Decoupled**: Communicate through components
4. **Focused**: One clear responsibility

### Performance Guidelines
1. **Minimize Lookups**: Cache component pointers
2. **Batch Operations**: Process similar work together
3. **Early Exit**: Skip when no work
4. **Profile Regular**: Monitor frame time

### Testing Systems
```c
void test_physics_system() {
    World world;
    world_init(&world);
    
    // Create test entity
    EntityID entity = entity_create(&world);
    entity_add_components(&world, entity, 
        COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    // Set initial state
    Physics* phys = entity_get_physics(&world, entity);
    physics_apply_force(phys, (Vector3){0, 100, 0});
    
    // Update system
    physics_system_update(&world, NULL, 0.016f);
    
    // Verify results
    assert(phys->velocity.y > 0);
    
    world_cleanup(&world);
}
```

## Debug Utilities

### System Inspector
```c
void debug_print_system_stats(World* world) {
    printf("System Statistics:\n");
    printf("  Entities: %d\n", world->entity_count);
    printf("  Transforms: %d\n", world->transform_count);
    printf("  Physics: %d\n", world->physics_count);
    printf("  Meshes: %d\n", world->mesh_count);
    // ... etc
}
```

### Performance Monitor
```c
typedef struct {
    const char* name;
    double total_time;
    double avg_time;
    double max_time;
    int call_count;
} SystemStats;

void monitor_system(SystemStats* stats, SystemUpdateFunc func,
                   World* world, float dt) {
    double start = get_time();
    func(world, NULL, dt);
    double elapsed = get_time() - start;
    
    stats->total_time += elapsed;
    stats->max_time = fmax(stats->max_time, elapsed);
    stats->call_count++;
    stats->avg_time = stats->total_time / stats->call_count;
}
```