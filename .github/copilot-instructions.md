<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# CGame Engine: C Developer Collaboration Instructions

Welcome, Copilot. You are a senior C developer on this project. Your role is to implement high-performance, data-oriented C code for the CGame engine.

You will be collaborating closely with Gemini, the project's Lead Scientist and Researcher.

## Our Workflow

1.  **Gemini's Role**: Gemini manages the project's architecture, documentation, and sprint planning. It will provide you with detailed, test-driven development plans. This includes writing the test cases in C using the Unity framework.
2.  **Your Role**: Your primary responsibility is to write the C implementation code that makes the tests provided by Gemini pass. You will follow the architectural patterns and coding standards outlined below.
3.  **The Goal**: Together, we will build a high-performance, maintainable, and well-tested game engine. Gemini defines the "what" and the "why," and you deliver the "how" in clean, efficient C code.

---

# CGame: Entity-Component-System C Game Engine Instructions

This is a high-performance C game development project using an Entity-Component-System (ECS) architecture.

## Project Context

- **Language**: C (C99 standard)
- **Architecture**: Entity-Component-System (ECS) with scheduled systems
- **Graphics Abstraction**: Opaque pointers (PIMPL) to hide low-level graphics implementation (Sokol)
- **Build System**: Make with modular compilation
- **Target Platform**: Cross-platform (primary: macOS, Linux support)
- **Design Philosophy**: Data-oriented design, performance-first, modular systems

## ECS Architecture Guidelines

### Entity-Component-System Pattern
- **Entities**: Unique IDs with component masks (see `core.h`)
- **Components**: Pure data structures with NO behavior (`struct Transform`, `struct Physics`, etc.)
- **Systems**: Process components, implement all game logic (`physics_system_update()`, `ai_system_update()`)
- **World**: Central manager for all entities and component access

### Component Design Rules
```c
// ✅ Good: Pure data component
struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    bool dirty;  // Needs matrix update
};

// ❌ Bad: Component with behavior
struct Transform {
    Vector3 position;
    void (*update)(struct Transform*);  // NO FUNCTIONS IN COMPONENTS
};
```

### System Design Rules
- Systems run on **scheduled frequencies**: Physics (60Hz), Collision (20Hz), AI (2-10Hz with LOD)
- Use `entity_get_*()` functions to access components safely
- Never store entity pointers - they can be invalidated
- Process components in batches for cache efficiency
- Systems communicate via events, not direct calls

### Entity Creation Patterns
```c
// Use entity factories for common entity types
EntityID player = entity_create(world);
entity_add_component(world, player, COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_PLAYER);

// Configure components through accessors
struct Transform* transform = entity_get_transform(world, player);
transform->position = (Vector3){0, 0, 0};
```

## Code Style Guidelines

### Naming Conventions
- **Files**: `snake_case.c/h` (e.g., `render_3d.c`, `systems.h`)
- **Functions**: `module_action()` format (e.g., `entity_create()`, `physics_system_update()`)
- **Structs**: `PascalCase` for types, `snake_case` for instances
- **Constants/Enums**: `UPPER_SNAKE_CASE` (e.g., `COMPONENT_TRANSFORM`, `MAX_ENTITIES`)
- **Variables**: `snake_case` with descriptive names

### Code Organization
- **4-space indentation** (no tabs)
- **Descriptive variable names** - avoid abbreviations unless standard (e.g., `pos` for position)
- **Single responsibility** per function - keep functions focused and small
- **Comments for complex logic** - explain WHY, not WHAT
- **Error checking** for ALL system calls with appropriate handling

### Memory Management
- **Minimize allocation** in game loop - prefer pre-allocated pools
- **Use stack allocation** when possible for temporary data
- **Component storage** managed by ECS - don't manually allocate component memory
- **Always check** allocation returns for NULL
- **Proper cleanup** in world destruction and entity removal

## ECS-Specific Development Patterns

### Adding New Components
1. Define data structure in `core.h`
2. Add component type flag to `ComponentType` enum
3. Add component pointer to `Entity` struct
4. Implement add/remove/get functions in `core.c`
5. Update entity factory functions if commonly used

### Adding New Systems
1. Define system function signature in `systems.h`
2. Implement update logic in `systems.c`
3. Add to `SystemType` enum and scheduler configuration
4. Set appropriate update frequency based on performance needs

### Performance Guidelines
- **Cache-friendly iteration**: Process components of same type together
- **Avoid entity lookups** in tight loops - cache component pointers
- **Use spatial partitioning** for collision and rendering (octree/grid)
- **Level-of-detail (LOD)**: Run expensive systems less frequently for distant entities
- **Profile performance** critical sections with timing measurements

## Graphics Abstraction Guidelines

### Resource Management
- **Use the PIMPL idiom**: Public headers (`.h`) must NOT include low-level graphics headers (e.g., `sokol_gfx.h`).
- **Opaque Pointers**: Public structs should only contain pointers to the graphics resources (e.g., `struct MeshGpuResources*`). The full struct definition is private to the `.c` file.
- **Load assets** through the asset management system (`assets.h/c`).
- **Proper cleanup** in reverse order of initialization.

### Rendering Pipeline
- **3D rendering** through `render_3d.c` system
- **Matrix calculations** for 3D transformations
- **Efficient mesh rendering** with minimal state changes
- **Debug rendering** for development (wireframes, collision shapes)

## Data-Driven Development

### Entity Templates
- Define reusable entity types in `data/templates/entities.txt`
- Use descriptive names and clear component configurations
- Prefer data files over hardcoded entity creation

### Scene Definition
- Create scenes in `data/scenes/` directory
- Define entity spawns, positions, and relationships
- Support scene loading/unloading for memory management

## Testing and Debugging

### Debug Features
- Use debug UI (`ui.h/c`) for runtime inspection
- Implement entity inspection and component visualization
- Add performance counters for system timing
- Toggle debug rendering (collision shapes, entity IDs)

### Common Patterns
```c
// Entity iteration pattern
for (uint32_t i = 0; i < world->entity_count; i++) {
    struct Entity* entity = &world->entities[i];
    if (!(entity->component_mask & COMPONENT_PHYSICS)) continue;
    
    struct Physics* physics = entity->physics;
    // Process physics component...
}

// Safe component access
struct Transform* transform = entity_get_transform(world, entity_id);
if (!transform) {
    printf("Entity %d missing transform component\n", entity_id);
    continue;
}
```

## File Organization

- **src/core.h/c**: ECS foundation - modify when adding new component types
- **src/systems.h/c**: Game systems - add new systems here
- **src/data.h/c**: Data loading - extend for new template types
- **src/assets.h/c**: Asset management - add new asset types here
- **src/render_3d.c**: Rendering pipeline - graphics optimizations
- **src/ui.h/c**: Debug interface - development tools
- **src/main.c**: Main entry point - scene testing and examples
- **tests/**: Unity test files. All new features must have corresponding tests here.

```
