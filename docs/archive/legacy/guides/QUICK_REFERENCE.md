# CGame ECS Engine - Quick Reference Card

## Essential Commands
```bash
make              # Build engine
make run          # Build and execute
make clean        # Remove build artifacts
make debug        # Debug build with symbols
```

## Core ECS Pattern
```c
// 1. Create Entity
EntityID entity = entity_create(world);

// 2. Add Components (can combine with |)
entity_add_component(world, entity, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);

// 3. Configure Components
struct Transform* t = entity_get_transform(world, entity);
t->position = (Vector3){x, y, z};

// 4. Systems Process Components Automatically
```

## Component Types
| Component | Purpose | Key Fields |
|-----------|---------|------------|
| `Transform` | Position/rotation/scale | `position`, `rotation`, `scale` |
| `Physics` | Movement/forces | `velocity`, `acceleration`, `mass`, `drag` |
| `Collision` | Collision detection | `radius`, `layer_mask`, `is_trigger` |
| `AI` | Autonomous behavior | `state`, `update_frequency`, `target_position` |
| `Renderable` | Visual representation | `mesh_id`, `material_id`, `visible` |
| `Player` | Input handling | `throttle`, `controls_enabled` |

## System Frequencies
- **Physics**: 60 FPS (movement, forces)
- **Collision**: 20 FPS (intersection detection)  
- **AI**: 2-10 FPS (behavior, LOD-based)
- **Render**: 60 FPS (3D output)

## Safe Component Access
```c
struct Transform* transform = entity_get_transform(world, entity_id);
if (!transform) {
    printf("Entity %d missing transform\n", entity_id);
    return; // Handle gracefully
}
// Use transform safely...
```

## Entity Iteration Pattern
```c
for (uint32_t i = 0; i < world->entity_count; i++) {
    struct Entity* entity = &world->entities[i];
    
    // Check component mask first
    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
    
    struct Physics* physics = entity->physics;
    // Process component...
}
```

## Entity Factory Pattern
```c
EntityID create_ship(struct World* world, Vector3 pos) {
    EntityID id = entity_create(world);
    entity_add_component(world, id, COMPONENT_TRANSFORM | COMPONENT_PHYSICS);
    
    struct Transform* t = entity_get_transform(world, id);
    t->position = pos;
    
    return id;
}
```

## File Responsibilities
- `core.h/c` - ECS foundation, add new components here
- `systems.h/c` - Game logic, add new systems here  
- `assets.h/c` - Asset loading (meshes, textures)
- `data.h/c` - Templates and scenes
- `render_3d.c` - 3D rendering pipeline
- `test.c` - Main entry, entity factories

## Data-Driven Entity Templates
```
# data/templates/entities.txt
template fighter {
    components: transform physics collision ai renderable
    mass: 50.0
    collision_radius: 3.0
    mesh: "fighter"
}
```

## Debug Controls
- **F1**: Toggle performance overlay
- **ESC**: Exit
- **Arrow Keys**: Move player ship
- **Space**: Thrust

## Performance Rules
✅ **DO**
- Process components by type (cache-friendly)
- Use component masks to filter entities
- Pre-allocate object pools
- Check component existence before access

❌ **DON'T**  
- Store entity pointers (use EntityID)
- Put behavior in components (pure data only)
- Allocate memory in game loop
- Access components without mask checks

## Common Debug Patterns
```c
// Entity inspection
printf("Entity %d: mask=0x%x components=%d\n", 
       entity->id, entity->component_mask, __builtin_popcount(entity->component_mask));

// Performance timing
float start = get_time();
system_update(world, dt);
printf("System took %.3fms\n", (get_time() - start) * 1000);
```

## Adding New Component (5 Steps)
1. Define struct in `src/core.h`
2. Add to `ComponentType` enum  
3. Add array to `ComponentPools` struct
4. Add pointer to `Entity` struct
5. Implement add/remove/get functions in `src/core.c`

## Adding New System (4 Steps)
1. Define function in `src/systems.h`
2. Implement logic in `src/systems.c`
3. Add to `SystemType` enum
4. Configure frequency in scheduler

## Memory Layout (Cache-Friendly)
```
World
├── entities[MAX_ENTITIES]     # Entity metadata
└── components
    ├── transforms[MAX_ENTITIES]  # All transforms together
    ├── physics[MAX_ENTITIES]     # All physics together
    └── ...                       # Components grouped by type
```

## Project Goals
- **Performance**: 1000+ entities at 60 FPS
- **Modularity**: Easy component/system addition
- **Data-Driven**: External configuration files
- **Emergent Gameplay**: AI-driven narrative
- **Educational**: Demonstate modern game architecture

## Key Concepts
- **ECS**: Entities = ID, Components = Data, Systems = Behavior
- **Data-Oriented**: Optimize for cache, not objects
- **LOD**: Level-of-detail based on importance/distance
- **Scheduled Systems**: Run at optimal frequencies
- **Event-Driven**: Systems communicate via events
