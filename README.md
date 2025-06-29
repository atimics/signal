# CGame: Entity-Component-System 3D Space Game Engine

A high-performance, data-oriented 3D space game engine written in C99 using SDL2. Features an advanced Entity-Component-System (ECS) architecture with scheduled systems, data-driven entity creation, and a focus on creating emergent, AI-driven gameplay in a living universe.

## 🏗️ Architecture Overview

### Core Design Principles
- **Data-Oriented Design**: Components are pure data structures, systems process them efficiently
- **Entity-Component-System**: Modular, composable entity architecture for scalability
- **Scheduled Systems**: Systems run at optimal frequencies (Physics: 60Hz, AI: 2-10Hz, Collision: 20Hz)
- **Data-Driven**: Entities and scenes defined in external data files, not hardcoded
- **Performance First**: Cache-friendly data layout, minimal dynamic allocation in game loop

### Key Components
```c
// Core component types - pure data structures
struct Transform   // Position, rotation, scale in 3D space
struct Physics     // Velocity, acceleration, mass, drag
struct Collision   // Shape, collision layers, trigger flags
struct AI          // State, decision timers, goals, behavior data
struct Renderable  // Mesh, material, LOD settings
struct Player      // Input mapping, camera settings
```

### System Architecture
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Physics       │    │   Collision      │    │      AI         │
│   System        │    │   System         │    │   System        │
│   (60 FPS)      │    │   (20 FPS)       │    │  (2-10 FPS)     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   Event System      │
                    │   (Message Bus)     │
                    └─────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   Render System     │
                    │   (60 FPS)          │
                    └─────────────────────┘
```

## 📁 Project Structure

```
src/
├── core.h/c           # ECS foundation: entities, components, world management
├── systems.h/c        # System scheduler and core game systems
├── data.h/c           # Data-driven entity templates and scene loading
├── assets.h/c         # Asset management: meshes, textures, materials
├── render_3d.c        # 3D rendering pipeline with SDL2
├── ui.h/c             # Debug UI and HUD rendering
└── test.c             # Main entry point and scene testing

data/
├── templates/
│   └── entities.txt   # Entity template definitions
└── scenes/
    └── spaceport.txt  # Scene configurations

assets/
├── meshes/            # 3D models (.mesh) and textures (.png)
│   ├── platonic_solids/
│   ├── sun/
│   └── *_ship/
```

## 🚀 Quick Start

### Prerequisites
- **macOS**: `brew install sdl2`
- **Linux**: `sudo apt-get install libsdl2-dev build-essential`
- **GCC** with C99 support

### Build and Run
```bash
# Build the engine
make

# Run the test scene
make run

# Clean build files
make clean

# Debug build
make debug
```

### Available VS Code Tasks
- **Build Game**: Compiles the project
- **Run Game**: Builds and executes the test scene
- **Clean Build**: Removes build artifacts

## 🎯 Current Features

### ✅ Implemented
- **ECS Architecture**: Full entity-component-system with efficient component storage
- **Scheduled Systems**: Physics (60Hz), Collision (20Hz), AI (2-10Hz with LOD)
- **3D Rendering**: SDL2-based 3D pipeline with mesh loading and matrix transforms
- **Data-Driven Entities**: Load entity templates from external files
- **Scene System**: Define and load complete scenes with multiple entities
- **Asset Management**: OBJ mesh loading, texture support, material system
- **Debug Systems**: Performance monitoring, entity inspection, frame timing

### 🔄 In Development
- **AI System Enhancement**: Behavior trees, goal-oriented action planning
- **Collision Detection**: Spatial partitioning (octree/grid), precise collision resolution
- **Particle Systems**: Engine exhaust, weapon effects, environmental particles
- **Audio System**: 3D positional audio, music, sound effects

### 🎯 Planned
- **Neural AI Integration**: LLM-driven entity behavior and dialog
- **Procedural Universe**: Dynamic faction systems, economic simulation
- **Networking**: Multiplayer support, client-server architecture
- **Modding API**: Scripting interface, content creation tools

## 🧪 Testing and Development

### Entity Creation Example
```c
// Create a player ship
EntityID player = entity_create(world);
entity_add_component(world, player, COMPONENT_TRANSFORM | COMPONENT_PHYSICS | COMPONENT_PLAYER);

// Configure components
struct Transform* transform = entity_get_transform(world, player);
transform->position = (Vector3){0, 0, 0};

struct Physics* physics = entity_get_physics(world, player);
physics->mass = 80.0f;
physics->drag = 0.98f;
```

### Data-Driven Entity Templates
```
# entities.txt - Define reusable entity types
template player_ship {
    name: "Player Ship"
    components: transform physics collision renderable player
    mass: 80.0
    collision_radius: 4.0
    mesh: "unique_ship"
}
```

## 🔧 Code Style Guidelines

### Naming Conventions
- **Files**: `snake_case.c/h`
- **Functions**: `module_action()` (e.g., `entity_create()`, `physics_update()`)
- **Structs**: `PascalCase` for types, `snake_case` for instances
- **Constants**: `UPPER_SNAKE_CASE`
- **Variables**: `snake_case`

### Code Organization
- **4-space indentation**
- **Descriptive variable names** (avoid abbreviations)
- **Single responsibility** per function
- **Comments for complex logic**, not obvious operations
- **Error checking** for all SDL and system calls

### Performance Guidelines
- **Minimize allocation** in game loop (prefer object pooling)
- **Cache-friendly data access** (iterate components by type)
- **Profile performance-critical sections**
- **Use const correctness** where applicable

## 📚 Documentation

- **[Project Vision](./docs/project/VISION.md)**: Long-term goals and philosophy
- **[Architecture Details](./src/ARCHITECTURE.md)**: In-depth technical design
- **[Build Instructions](./docs/project/BUILDING.md)**: Detailed compilation guide
- **[User Manual](./docs/user/MANUAL.md)**: How to play and interact
- **[API Reference](./docs/api/REFERENCE.md)**: Function and system documentation

## 🎮 Controls and Interaction

- **Arrow Keys**: Ship movement
- **Space**: Thrust/Action
- **ESC**: Exit
- **F1**: Toggle debug overlay
- **Mouse**: Camera control (planned)

## 🧠 For LLM Assistants

This project uses a **component-based architecture** where:
1. **Entities** are containers with unique IDs
2. **Components** are pure data (no behavior)
3. **Systems** process components and implement game logic
4. **World** manages all entities and provides component access

When modifying code:
- Add new **components** in `core.h` as data structures
- Add new **systems** in `systems.h/c` with update functions
- Use **entity factories** in test.c for creating common entity types
- Follow the **data-driven approach** - prefer configuration over hardcoding

The engine prioritizes **performance** and **modularity** over simplicity - each system runs at its optimal frequency and components are stored for cache-efficient access.