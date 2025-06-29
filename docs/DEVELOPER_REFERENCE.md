# CGame ECS Engine - Developer Quick Reference

This document provides a concise reference for developers and AI assistants working with the CGame Entity-Component-System engine.

## Project Identity

**CGame** is a high-performance, data-oriented 3D space game engine written in C99 using SDL2. It implements a sophisticated Entity-Component-System architecture with scheduled systems, designed for creating emergent, AI-driven space simulation games.

## Core Architecture Patterns

### Entity-Component-System (ECS)
```c
// Entity = Unique ID + Component Mask
typedef uint32_t EntityID;

// Components = Pure Data (NO behavior)
struct Transform { Vector3 position; Quaternion rotation; Vector3 scale; bool dirty; };
struct Physics { Vector3 velocity; Vector3 acceleration; float mass; float drag; bool kinematic; };

// Systems = Process Components (ALL behavior)
void physics_system_update(struct World* world, float delta_time);  // 60 FPS
void collision_system_update(struct World* world, float delta_time); // 20 FPS
void ai_system_update(struct World* world, float delta_time);        // 2-10 FPS (LOD)
```

### Data-Oriented Design Principles
- **Components stored in arrays by type** (cache-friendly)
- **Systems iterate over component arrays** (not entities)
- **No dynamic allocation in game loop** (pre-allocated pools)
- **EntityID references** (not pointers - can be invalidated)

## Essential Code Patterns

### Entity Creation Pattern
```c
// Create entity
EntityID entity = entity_create(world);

// Add components (can be combined with |)
entity_add_component(world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);

// Configure components via accessors
struct Transform* transform = entity_get_transform(world, entity);
transform->position = (Vector3){x, y, z};

struct Physics* physics = entity_get_physics(world, entity);
physics->mass = 100.0f;
```

### Safe Component Access Pattern
```c
struct Transform* transform = entity_get_transform(world, entity_id);
if (!transform) {
    printf("Entity %d missing transform\n", entity_id);
    return; // Handle missing component gracefully
}
// Use transform safely...
```

### System Iteration Pattern
```c
// Process all entities with specific components
for (uint32_t i = 0; i < world->entity_count; i++) {
    struct Entity* entity = &world->entities[i];
    
    // Check component mask before accessing
    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
    
    struct Physics* physics = entity->physics;
    struct Transform* transform = entity->transform;
    
    // Process components...
}
```

## Component Types Reference

| Component | Purpose | Key Fields |
|-----------|---------|------------|
| `Transform` | Position/rotation/scale | `position`, `rotation`, `scale`, `dirty` |
| `Physics` | Movement/forces | `velocity`, `acceleration`, `mass`, `drag` |
| `Collision` | Collision detection | `radius`, `layer_mask`, `is_trigger` |
| `AI` | Autonomous behavior | `state`, `update_frequency`, `target_position` |
| `Renderable` | Visual representation | `mesh`, `material`, `visible`, `lod_distance` |
| `Player` | Input handling | Player-specific input mapping data |

## System Frequencies & Responsibilities

| System | Frequency | Responsibility |
|--------|-----------|----------------|
| Physics | 60 FPS | Apply forces, update positions, handle movement |
| Collision | 20 FPS | Detect intersections, spatial partitioning |
| AI | 2-10 FPS | Decision making, pathfinding (LOD-based) |
| Render | 60 FPS | 3D rendering, culling, LOD selection |

## File Organization

```
src/
├── core.h/c           # ECS foundation: World, Entity, Component management
├── systems.h/c        # System scheduler + core game systems
├── data.h/c           # Data-driven entity templates and scene loading
├── assets.h/c         # Asset management: meshes, textures, materials
├── render_3d.c        # 3D rendering pipeline with SDL2
├── ui.h/c             # Debug UI and performance monitoring
└── test.c             # Main entry point and example entity factories

data/
├── templates/entities.txt  # Reusable entity type definitions
└── scenes/spaceport.txt    # Complete scene configurations

assets/meshes/         # OBJ files and textures organized by type
```

## Essential Functions Reference

### World Management
```c
bool world_init(struct World* world);
void world_destroy(struct World* world);
void world_update(struct World* world, float delta_time);
```

### Entity Management
```c
EntityID entity_create(struct World* world);
void entity_destroy(struct World* world, EntityID entity_id);
bool entity_add_component(struct World* world, EntityID entity_id, ComponentType component_type);
void entity_remove_component(struct World* world, EntityID entity_id, ComponentType component_type);
```

### Component Access (returns NULL if component missing)
```c
struct Transform* entity_get_transform(struct World* world, EntityID entity_id);
struct Physics* entity_get_physics(struct World* world, EntityID entity_id);
struct Collision* entity_get_collision(struct World* world, EntityID entity_id);
struct AI* entity_get_ai(struct World* world, EntityID entity_id);
struct Renderable* entity_get_renderable(struct World* world, EntityID entity_id);
struct Player* entity_get_player(struct World* world, EntityID entity_id);
```

### Entity Factories (defined in test.c)
```c
EntityID create_player(struct World* world, Vector3 position);
EntityID create_ai_ship(struct World* world, Vector3 position, const char* type);
EntityID create_sun(struct World* world, Vector3 position);
```

## Build & Development

### Quick Commands
```bash
make           # Build engine
make run       # Build and execute test scene
make clean     # Remove build artifacts
make debug     # Debug build with symbols
```

### VS Code Tasks
- **Build Game**: Compile project
- **Run Game**: Build and execute
- **Clean Build**: Remove artifacts

### Debug Features
- Press **F1** in-game to toggle performance overlay
- Debug UI shows entity counts, system timing, frame rate
- Built-in performance counters for optimization

## Data-Driven Development

### Entity Templates
```
# data/templates/entities.txt
template player_ship {
    name: "Player Ship"
    components: transform physics collision renderable player
    mass: 80.0
    collision_radius: 4.0
    mesh: "unique_ship"
}
```

### Scene Definitions
```
# data/scenes/spaceport.txt
scene spaceport {
    player: player_ship at (0, 0, 0)
    spawn ai_patrol at (100, 0, 50)
    spawn sun at (0, 1000, 0) scale 10.0
}
```

## Performance Guidelines

### DO
- **Process components by type** (cache-friendly)
- **Use component masks** to filter entities
- **Pre-allocate object pools** instead of malloc/free
- **Check component existence** before access
- **Use scheduled systems** at appropriate frequencies

### DON'T
- **Store entity pointers** (use EntityID instead)
- **Put behavior in components** (pure data only)
- **Allocate memory in game loop** (use pools)
- **Access components without checking masks**
- **Run expensive operations every frame** (use LOD)

## Common Debugging Patterns

### Entity Inspection
```c
printf("Entity %d: mask=0x%x, transform=%p, physics=%p\n", 
       entity->id, entity->component_mask, entity->transform, entity->physics);
```

### Performance Monitoring
```c
// System timing (built into scheduler)
float start_time = get_current_time();
physics_system_update(world, delta_time);
float elapsed = get_current_time() - start_time;
printf("Physics system: %.3fms\n", elapsed * 1000.0f);
```

### Memory Validation
```c
// Check world state
printf("World: %d/%d entities, frame %d, time %.2f\n",
       world->entity_count, MAX_ENTITIES, world->frame_number, world->total_time);
```

## Extension Points

### Adding New Components
1. Define struct in `core.h`
2. Add to `ComponentType` enum
3. Add pointer to `Entity` struct
4. Implement add/remove/get functions in `core.c`

### Adding New Systems
1. Define function in `systems.h`
2. Implement logic in `systems.c`
3. Add to `SystemType` enum
4. Configure frequency in scheduler

### Adding Entity Types
1. Create template in `data/templates/entities.txt`
2. Optional: Add factory function in `test.c`
3. Reference in scene files

## AI Assistant Guidelines

When working with this codebase:

1. **Respect ECS Principles**: Components = data, Systems = behavior
2. **Use Existing Patterns**: Follow established entity creation and component access patterns
3. **Consider Performance**: Think about cache efficiency and system frequencies
4. **Check Component Existence**: Always validate component pointers before use
5. **Prefer Data-Driven**: Use templates and scenes over hardcoded entities
6. **Follow Naming Conventions**: `module_action()` for functions, `snake_case` for variables
7. **Document Changes**: Explain architectural decisions in comments

This engine prioritizes **performance**, **modularity**, and **data-driven development** over simplicity. Each system runs at its optimal frequency, and the ECS architecture enables efficient processing of thousands of entities for large-scale space simulation.
