# V2 Architecture: Component-Based Game Engine

## First Principles Design

### Core Philosophy
- **Separation of Concerns**: Each system handles one responsibility
- **Data-Oriented Design**: Components are pure data, systems process them
- **Performance First**: Systems run on schedules, not every frame
- **Decoupled**: Systems communicate through events, not direct calls

## Component System

### Core Components (Pure Data)
```c
// Transform component - position, rotation, scale
struct Transform {
    Vector3 position;
    Vector3 rotation; 
    Vector3 scale;
};

// Physics component - velocity, forces
struct Physics {
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float drag;
};

// Collision component - shape and properties
struct Collision {
    enum CollisionShape { SPHERE, BOX, CAPSULE } shape;
    union {
        float radius;           // For sphere
        Vector3 box_size;       // For box
        struct { float radius; float height; } capsule;
    };
    bool is_trigger;           // Ghost collision vs solid
    uint32_t collision_mask;   // What layers this collides with
};

// AI component - decision making data
struct AI {
    enum AIState state;
    float decision_timer;      // When to make next decision
    float reaction_cooldown;   // Prevent spam reactions
    char* current_goal;
    Vector3 target_position;
};

// Renderable component - visual representation
struct Renderable {
    struct Mesh* mesh;
    struct Material* material;
    bool visible;
    float lod_distance;        // When to switch detail levels
};
```

### Systems (Process Components)

#### 1. Physics System (60 FPS)
- Updates Transform based on Physics components
- Handles movement, forces, drag
- **NO collision detection** (that's collision system's job)

#### 2. Collision System (20 FPS)
- Spatial partitioning for efficiency (octree/grid)
- Broad phase: Find potential collisions
- Narrow phase: Precise collision detection
- Fires collision events, doesn't resolve directly

#### 3. AI System (2-10 FPS, LOD based)
- Processes AI components based on distance to player
- Near entities: 10 FPS, Far entities: 2 FPS
- Submits movement requests to physics system
- Generates dialog/reaction events

#### 4. Render System (60 FPS)
- Culls invisible objects
- LOD based on distance
- Only renders Renderable components

#### 5. Event System
- Decouples systems via message passing
- Systems subscribe to events they care about
- Examples: CollisionEvent, AIReactionEvent, etc.

## Entity Management

### Entity = Component Container
```c
struct Entity {
    uint32_t id;
    uint64_t component_mask;    // Bitfield of which components exist
    
    // Component storage (optional components)
    struct Transform* transform;
    struct Physics* physics;
    struct Collision* collision;
    struct AI* ai;
    struct Renderable* renderable;
};
```

### Entity Queries
- Systems iterate only entities with required components
- Fast bitfield operations to filter entities
- Cache-friendly iteration patterns

## Performance Optimizations

### 1. System Scheduling
- Physics: Every frame (16.67ms)
- Collision: Every 3 frames (50ms) 
- AI Near: Every 6 frames (100ms)
- AI Far: Every 30 frames (500ms)
- Render: Every frame

### 2. Spatial Partitioning
- Octree for 3D space division
- Only check collisions within same/adjacent cells
- O(n²) → O(n log n) collision detection

### 3. Component Pooling
- Pre-allocate component arrays
- No dynamic allocation during gameplay
- Cache-friendly memory layout

### 4. Event Batching
- Collect events during frame
- Process all at once
- Prevents cascade effects

## Implementation Plan

### Phase 1: Core Framework
1. Component definitions
2. Entity management
3. Basic system scheduler

### Phase 2: Essential Systems
1. Physics system
2. Collision system (spatial partitioning)
3. Basic rendering

### Phase 3: Game Logic
1. AI system with proper scheduling
2. Event system
3. Player input handling

### Phase 4: Polish
1. LOD system
2. Performance profiling
3. Memory optimization

## Expected Performance Gains

- **Collision**: O(n²) → O(n log n) - 100x improvement with 1000 entities
- **AI**: 60 FPS → 2-10 FPS - 6-30x reduction in AI overhead  
- **Memory**: Contiguous arrays - Better cache performance
- **Scheduling**: Fixed time budgets - Consistent frame rates

This architecture will handle thousands of entities efficiently while maintaining clean, maintainable code.
