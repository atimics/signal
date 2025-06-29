# Architecture Overview - CGame ECS Engine

This document provides a comprehensive overview of the CGame engine's Entity-Component-System architecture, designed for high-performance 3D space simulation with emergent AI-driven gameplay.

## Core Architectural Principles

The architecture is built on four foundational principles:

1. **Entity-Component-System (ECS)**: Entities are containers with unique IDs, Components are pure data structures, and Systems process components to implement game logic. This provides excellent modularity and performance.

2. **Data-Oriented Design**: Components are stored in cache-friendly arrays, systems iterate over components by type, and memory layout is optimized for CPU cache efficiency. This enables processing thousands of entities at 60+ FPS.

3. **Scheduled Systems**: Each system runs at its optimal frequency - Physics at 60Hz, Collision at 20Hz, AI at 2-10Hz with level-of-detail optimization. This balances performance with gameplay requirements.

4. **Data-Driven Development**: Entities and scenes are defined in external data files rather than hardcoded, enabling rapid content creation and iteration without recompilation.

## System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Main Game Loop                           │
│  (SDL Event Processing → System Updates → Rendering)       │
└─────────────────────┬───────────────────────────────────────┘
                      │
┌─────────────────────┼───────────────────────────────────────┐
│             System Scheduler                                │
│  ┌─────────────┐   ┌──────────────┐   ┌─────────────────┐  │
│  │  Physics    │   │  Collision   │   │       AI        │  │
│  │  System     │   │   System     │   │    System       │  │
│  │  (60 FPS)   │   │  (20 FPS)    │   │  (2-10 FPS)     │  │
│  └─────────────┘   └──────────────┘   └─────────────────┘  │
└─────────────────────┼───────────────────────────────────────┘
                      │
┌─────────────────────┼───────────────────────────────────────┐
│                ECS Core                                     │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                   World                             │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │   │
│  │  │   Entity    │  │   Entity    │  │   Entity    │  │   │
│  │  │   Pool      │  │   Pool      │  │   Pool      │  │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │               Component Storage                     │   │
│  │ Transform │ Physics │ Collision │ AI │ Renderable │  │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────┼───────────────────────────────────────┘
                      │
┌─────────────────────┼───────────────────────────────────────┐
│               Support Systems                               │
│  ┌─────────────┐   ┌──────────────┐   ┌─────────────────┐  │
│  │   Asset     │   │    Data      │   │     Render      │  │
│  │ Management  │   │   System     │   │    System       │  │
│  │   System    │   │  (Templates) │   │   (3D SDL2)     │  │
│  └─────────────┘   └──────────────┘   └─────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Component System Design

### Core Components (Pure Data Structures)

#### Transform Component
```c
struct Transform {
    Vector3 position;      // World position (x, y, z)
    Quaternion rotation;   // Orientation quaternion
    Vector3 scale;         // Scale factors
    bool dirty;            // Needs matrix recalculation
};
```
**Purpose**: Defines object position, rotation, and scale in 3D space.

#### Physics Component
```c
struct Physics {
    Vector3 velocity;      // Current velocity vector
    Vector3 acceleration;  // Applied acceleration
    float mass;            // Object mass for force calculations
    float drag;            // Drag coefficient (0.0-1.0)
    bool kinematic;        // Immune to physics forces
};
```
**Purpose**: Enables realistic movement and force-based interactions.

#### Collision Component
```c
struct Collision {
    float radius;          // Collision sphere radius
    uint32_t layer_mask;   // Which collision layers to interact with
    bool is_trigger;       // Ghost collision (no physical response)
    Vector3 offset;        // Offset from transform position
};
```
**Purpose**: Defines collision boundaries and interaction rules.

#### AI Component
```c
struct AI {
    int state;             // Current behavior state
    float update_frequency; // How often to process AI (Hz)
    float decision_timer;   // Time of last decision
    float reaction_cooldown;// Prevents decision spam
    Vector3 target_position;// Current movement target
};
```
**Purpose**: Enables autonomous behavior and decision-making.

#### Renderable Component
```c
struct Renderable {
    struct Mesh* mesh;     // 3D geometry
    struct Material* material; // Texture and appearance
    bool visible;          // Render this frame
    float lod_distance;    // Level-of-detail threshold
};
```
**Purpose**: Defines visual representation and rendering parameters.

## System Processing Architecture

### 1. Physics System (60 FPS)
- **Responsibility**: Apply forces, update velocities, move entities
- **Input**: Transform + Physics components
- **Output**: Updated Transform.position based on Physics.velocity
- **Performance**: Processes ~1000 entities efficiently per frame

### 2. Collision System (20 FPS)
- **Responsibility**: Detect intersections, generate collision events
- **Input**: Transform + Collision components
- **Algorithm**: Spatial partitioning (octree/grid) for broad phase
- **Output**: Collision events for game logic systems

### 3. AI System (2-10 FPS, LOD-based)
- **Responsibility**: Autonomous behavior, pathfinding, decision-making
- **Input**: Transform + AI components, world state
- **LOD Strategy**: 
  - Near entities (< 100 units): 10 FPS updates
  - Medium entities (100-500 units): 5 FPS updates  
  - Far entities (> 500 units): 2 FPS updates
- **Output**: Movement commands, behavior state changes

### 4. Render System (60 FPS)
- **Responsibility**: 3D rendering, frustum culling, LOD selection
- **Input**: Transform + Renderable components
- **Pipeline**: SDL2 → OpenGL-style matrix transforms → mesh rendering
- **Optimizations**: Frustum culling, distance-based LOD, batch rendering

## Data-Driven Entity Creation

### Entity Templates System
Entities are defined in external data files for easy content creation:

```
# data/templates/entities.txt
template player_ship {
    name: "Player Ship"
    components: transform physics collision renderable player
    mass: 80.0
    collision_radius: 4.0
    mesh: "unique_ship"
    material: "unique_ship"
}

template ai_patrol {
    name: "Patrol Ship"
    components: transform physics collision ai renderable
    mass: 100.0
    ai_state: patrolling
    ai_frequency: 5.0
    mesh: "wedge_ship"
}
```

### Scene Loading System
Complete game scenes are loaded from configuration files:

```
# data/scenes/spaceport.txt
scene spaceport {
    player: player_ship at (0, 0, 0)
    
    spawn ai_patrol at (100, 0, 50) patrol_route circle
    spawn ai_patrol at (-80, 0, -40) patrol_route line
    
    spawn sun at (0, 1000, 0) scale 10.0
    spawn station at (200, 0, 200) faction friendly
}
```

## Performance Architecture

### Memory Layout Optimization
- **Component Arrays**: Components stored in contiguous arrays by type
- **Cache-Friendly Iteration**: Systems iterate over component arrays, not entities
- **Minimal Allocation**: Pre-allocated component pools, no malloc in game loop
- **Entity References**: Use EntityID (uint32_t), not pointers (can be invalidated)

### System Scheduling Optimization
```c
// Example scheduling logic
if (scheduler->systems[SYSTEM_PHYSICS].last_update + (1.0f/60.0f) <= current_time) {
    physics_system_update(world, delta_time);
    scheduler->systems[SYSTEM_PHYSICS].last_update = current_time;
}

if (scheduler->systems[SYSTEM_AI].last_update + ai_update_interval <= current_time) {
    ai_system_update(world, delta_time);  // LOD-based frequency
    scheduler->systems[SYSTEM_AI].last_update = current_time;
}
```

### Level-of-Detail (LOD) Strategy
- **AI Processing**: Update frequency scales with distance to player
- **Rendering**: Switch mesh detail based on distance
- **Collision**: Use simplified shapes for distant objects
- **Audio**: 3D positional audio with distance attenuation (planned)

## Asset Management Architecture

### Mesh Loading Pipeline
1. **OBJ File Parsing**: Load vertices, faces, UV coordinates, normals
2. **Material Loading**: Parse MTL files for texture and appearance data
3. **Texture Loading**: SDL2 surface → GPU texture conversion
4. **Caching**: Keep frequently used assets in memory
5. **Reference Counting**: Automatic cleanup of unused assets

### Asset Directory Structure
```
assets/
├── meshes/
│   ├── platonic_solids/    # Basic geometric shapes
│   ├── ships/              # Spacecraft models
│   └── environment/        # Environmental objects
├── textures/               # Material textures
└── sounds/                 # Audio files (planned)
```

## Future Architecture Extensions

### Planned System Additions
- **Audio System**: 3D positional audio with SDL2_mixer
- **Particle System**: Engine exhaust, weapon effects, explosions
- **Network System**: Client-server multiplayer architecture
- **Script System**: Lua/JavaScript integration for modding
- **UI System**: Immediate-mode GUI for HUD and menus

### AI System Enhancement
- **Neural Integration**: LLM-driven entity personalities and dialog
- **Behavior Trees**: Visual scripting for complex AI behaviors
- **Goal-Oriented Action Planning**: Dynamic quest and objective generation
- **Faction System**: Emergent political and economic relationships

### Performance Scaling
- **Multithreading**: Parallel system execution with job queues
- **GPU Compute**: Physics and collision detection on GPU
- **Streaming**: Dynamic loading/unloading of distant sectors
- **Procedural Generation**: Infinite universe with deterministic seeding

## Development Guidelines

### Adding New Components
1. Define pure data structure in `core.h`
2. Add component flag to `ComponentType` enum
3. Add component pointer to `Entity` struct
4. Implement add/remove/get functions in `core.c`
5. Update relevant systems to process the new component

### Adding New Systems
1. Define system function in `systems.h`
2. Implement update logic in `systems.c`
3. Add to `SystemType` enum and scheduler configuration
4. Set appropriate update frequency for performance
5. Document system purpose and component dependencies

### Best Practices
- **Single Responsibility**: Each system handles one aspect of game logic
- **Component Composition**: Use multiple components rather than complex single components
- **Data Validation**: Always check component existence before access
- **Performance Monitoring**: Profile system update times and entity counts
- **Documentation**: Comment complex algorithms and architectural decisions
