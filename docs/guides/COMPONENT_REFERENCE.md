# Component Reference

This document provides a complete reference for all components available in the CGame ECS architecture.

## Component Overview

Components in CGame are pure data structures with no behavior. They are combined to create entities with specific capabilities.

### Component Bitmasks

```c
#define COMPONENT_NONE              0
#define COMPONENT_TRANSFORM         (1 << 0)
#define COMPONENT_PHYSICS           (1 << 1)
#define COMPONENT_MESH              (1 << 2)
#define COMPONENT_THRUSTER_SYSTEM   (1 << 3)
#define COMPONENT_CONTROL_AUTHORITY (1 << 4)
#define COMPONENT_CAMERA            (1 << 5)
#define COMPONENT_AI                (1 << 6)
#define COMPONENT_LIGHT             (1 << 7)
#define COMPONENT_COLLISION         (1 << 8)
#define COMPONENT_HEALTH            (1 << 9)
#define COMPONENT_UI_ELEMENT        (1 << 10)
#define COMPONENT_PARTICLE_EMITTER  (1 << 11)
```

## Core Components

### Transform
**Purpose**: Position, rotation, and scale in 3D space  
**Required By**: Almost all visible entities  
**Header**: `src/component/transform.h`

```c
struct Transform {
    Vector3 position;       // World position
    Quaternion rotation;    // Orientation
    Vector3 scale;          // Scale factors
    EntityID parent;        // Parent entity (INVALID_ENTITY if none)
};
```

**Usage**:
- Foundation for all spatial entities
- Supports hierarchical transforms via parent
- Updated by physics and animation systems

### Physics
**Purpose**: Physical simulation properties  
**Requires**: Transform  
**Header**: `src/component/physics.h`

```c
struct Physics {
    float mass;                    // Mass in kg
    Vector3 velocity;              // Linear velocity (m/s)
    Vector3 angular_velocity;      // Angular velocity (rad/s)
    Vector3 force_accumulator;     // Forces this frame
    Vector3 torque_accumulator;    // Torques this frame
    float drag;                    // Linear drag coefficient
    float angular_drag;            // Angular drag coefficient
    bool enable_6dof;              // Enable rotation
    PhysicsEnvironment environment; // Space or atmosphere
};
```

**Usage**:
- Enables physics simulation
- Accumulates forces from various systems
- Updated by physics system each frame

### Mesh
**Purpose**: Visual representation  
**Requires**: Transform  
**Header**: `src/component/mesh.h`

```c
struct Mesh {
    char asset_name[64];      // Asset identifier
    char material_name[64];   // Material identifier
    bool visible;             // Visibility flag
    float lod_bias;           // LOD adjustment
    Color tint;               // Color multiplier
};
```

**Usage**:
- Links entity to visual assets
- Rendered by render system
- Supports LOD and tinting

## Movement Components

### ThrusterSystem
**Purpose**: Spacecraft propulsion  
**Requires**: Transform, Physics  
**Header**: `src/component/thruster_system.h`

```c
struct ThrusterSystem {
    Vector3 max_linear_force;      // Maximum forces per axis
    Vector3 max_angular_torque;    // Maximum torques per axis
    Vector3 current_linear_thrust; // Current thrust commands
    Vector3 current_angular_thrust;// Current torque commands
    float thrust_response_time;    // Ramp time to full thrust
    float atmosphere_efficiency;   // Efficiency in atmosphere
    float vacuum_efficiency;       // Efficiency in space
    bool thrusters_enabled;        // Master enable
};
```

**Usage**:
- Converts thrust commands to physics forces
- Handles environmental efficiency
- Supports gradual thrust changes

### ControlAuthority
**Purpose**: Maps input to vehicle control  
**Requires**: ThrusterSystem  
**Header**: `src/component/control_authority.h`

```c
struct ControlAuthority {
    EntityID controlled_by;        // Controlling entity
    Vector3 linear_authority;      // Linear control scaling
    Vector3 angular_authority;     // Angular control scaling
    float input_dead_zone;         // Dead zone threshold
    float input_sensitivity;       // Overall sensitivity
    bool auto_deceleration;        // Auto-stop when no input
    ControlMode mode;              // Direct, assisted, etc.
};
```

**Usage**:
- Interprets player/AI input
- Applies control scaling and dead zones
- Supports multiple control modes

## Perception Components

### Camera
**Purpose**: Viewport configuration  
**Requires**: Transform  
**Header**: `src/component/camera.h`

```c
struct Camera {
    CameraMode mode;          // First person, third person, etc.
    float fov;                // Field of view (degrees)
    float near_plane;         // Near clipping distance
    float far_plane;          // Far clipping distance
    Vector3 offset;           // Offset from target
    EntityID target;          // Entity to follow
    float smoothing;          // Movement smoothing factor
};
```

**Usage**:
- Defines rendering viewport
- Supports multiple camera modes
- Can follow entities smoothly

### Light
**Purpose**: Dynamic lighting  
**Requires**: Transform  
**Header**: `src/component/light.h`

```c
struct Light {
    LightType type;           // Point, directional, spot
    Color color;              // Light color
    float intensity;          // Light strength
    float range;              // Affect distance (point/spot)
    float inner_cone;         // Spot light inner angle
    float outer_cone;         // Spot light outer angle
    bool cast_shadows;        // Shadow generation
};
```

**Usage**:
- Adds dynamic lighting to scenes
- Supports various light types
- Performance impact with shadows

## Gameplay Components

### Health
**Purpose**: Damage tracking  
**Header**: `src/component/health.h`

```c
struct Health {
    float current;            // Current health
    float maximum;            // Maximum health
    float regeneration_rate;  // Health/second
    float armor;              // Damage reduction
    bool invulnerable;        // Damage immunity
    float last_damage_time;   // Time of last damage
};
```

**Usage**:
- Tracks entity damage state
- Supports regeneration
- Used by damage and UI systems

### AI
**Purpose**: Autonomous behavior  
**Requires**: Transform, Physics  
**Header**: `src/component/ai.h`

```c
struct AI {
    AIBehavior behavior;      // Current behavior mode
    EntityID target;          // Current target entity
    float aggression;         // Aggression level (0-1)
    float detection_range;    // Awareness radius
    float reaction_time;      // Response delay
    Vector3 patrol_center;    // Patrol area center
    float patrol_radius;      // Patrol area size
    AIState state;            // Current AI state
};
```

**Usage**:
- Enables NPC behavior
- Supports various behavior patterns
- Integrates with movement systems

### Collision
**Purpose**: Collision detection shape  
**Requires**: Transform  
**Header**: `src/component/collision.h`

```c
struct Collision {
    CollisionShape shape;     // Sphere, box, capsule
    Vector3 size;             // Shape dimensions
    Vector3 offset;           // Offset from transform
    uint32_t layer;           // Collision layer
    uint32_t mask;            // Layers to collide with
    bool is_trigger;          // Trigger vs solid
    CollisionCallback on_hit; // Collision callback
};
```

**Usage**:
- Defines collision boundaries
- Supports layers for filtering
- Can be solid or trigger volume

## UI Components

### UIElement
**Purpose**: 2D interface elements  
**Header**: `src/component/ui_element.h`

```c
struct UIElement {
    UIType type;              // Text, button, panel, etc.
    Vector2 position;         // Screen position (0-1)
    Vector2 size;             // Element size (0-1)
    Vector2 anchor;           // Anchor point
    char text[256];           // Display text
    Color color;              // Element color
    bool visible;             // Visibility
    bool interactive;         // Can be clicked
};
```

**Usage**:
- Creates HUD and menu elements
- Supports various UI types
- Handles screen-space positioning

## Effects Components

### ParticleEmitter
**Purpose**: Particle effects  
**Requires**: Transform  
**Header**: `src/component/particle_emitter.h`

```c
struct ParticleEmitter {
    ParticleType type;        // Effect type
    float emission_rate;      // Particles per second
    float particle_lifetime;  // Particle duration
    Vector3 emission_cone;    // Emission direction/spread
    float initial_velocity;   // Particle speed
    Color start_color;        // Initial color
    Color end_color;          // Final color
    float start_size;         // Initial size
    float end_size;           // Final size
    bool active;              // Currently emitting
};
```

**Usage**:
- Creates visual effects
- Supports various particle behaviors
- Performance sensitive component

## Component Combinations

### Common Archetypes

**Flying Vehicle**:
- Transform
- Physics
- Mesh
- ThrusterSystem
- ControlAuthority
- Health

**Static Prop**:
- Transform
- Mesh
- Collision

**Pickup Item**:
- Transform
- Mesh
- Collision (trigger)
- ParticleEmitter

**Enemy Ship**:
- Transform
- Physics
- Mesh
- ThrusterSystem
- AI
- Health
- Collision

**Camera Entity**:
- Transform
- Camera

**Light Source**:
- Transform
- Light
- (Optional) Mesh

## Best Practices

### Component Design
1. Keep components small and focused
2. Use POD types when possible
3. Avoid pointers to other components
4. Initialize with sensible defaults

### Performance
1. Align data to cache lines
2. Group frequently accessed data
3. Avoid dynamic allocations
4. Use component pools

### Dependencies
1. Document required components
2. Validate dependencies at creation
3. Handle missing components gracefully
4. Use component masks for queries

## Adding New Components

1. Create header in `src/component/`
2. Define data structure
3. Add to ComponentType enum
4. Update component pools
5. Create init/cleanup functions
6. Write unit tests
7. Document in this reference

## Component Lifecycle

### Creation
```c
entity_add_component(world, entity, COMPONENT_PHYSICS);
Physics* phys = entity_get_physics(world, entity);
physics_init(phys);
```

### Update
```c
// Components updated by systems
physics_system_update(world, render_config, delta_time);
```

### Removal
```c
entity_remove_component(world, entity, COMPONENT_PHYSICS);
```

### Destruction
```c
// Automatic when entity destroyed
entity_destroy(world, entity);
```