# CGame Engine: Clean Codebase Organization

## Project Structure Overview

This document describes the clean, organized structure of the CGame engine codebase after consolidation and cleanup.

## Source Code Organization

### Root Source Directory (`src/`)

```
src/
├── core.h/c                    # ECS foundation - entities, components, world
├── systems.h/c                 # System scheduler and management
├── render.h                    # Rendering system types and configuration
├── main.c                      # Application entry point
├── data.h/c                    # Scene and entity template loading
├── assets.h/c                  # Asset management system
├── ui.h/c                      # Debug UI and interface
├── graphics_api.h/c           # Low-level graphics abstraction (Sokol)
├── gpu_resources.h/c          # GPU buffer and resource management
├── scene_state.h/c            # Scene state management
├── scene_script.h/c           # Scene scripting system
├── render_*.h/c               # Rendering subsystems
│   ├── render_3d.c            # 3D rendering pipeline
│   ├── render_camera.h/c      # Camera management
│   ├── render_lighting.h/c    # Lighting system
│   ├── render_mesh.h/c        # Mesh rendering
│   └── render_gpu.h/c         # GPU rendering utilities
├── asset_loader/              # Asset loading subsystems
│   ├── asset_loader_index.c   # Asset index loading
│   ├── asset_loader_mesh.c    # Mesh loading
│   └── asset_loader_material.c # Material loading
├── scripts/                   # Scene-specific scripts
│   └── logo_scene.c           # Logo scene script
├── system/                    # ECS System modules (ORGANIZED)
│   ├── camera.h/c             # Camera system
│   ├── physics.h/c            # Physics system
│   ├── collision.h/c          # Collision detection system
│   ├── ai.h/c                 # AI system
│   ├── lod.h/c                # Level-of-Detail system
│   └── performance.h/c        # Performance monitoring system
└── external/                  # Third-party headers (Sokol, Nuklear, STB)
    ├── sokol_*.h              # Sokol graphics library headers
    ├── nuklear.h              # Nuklear UI library
    └── stb_image.h            # STB image loading library
```

## System Organization Principles

### 1. ECS Architecture
- **Entities**: Unique IDs managed in `core.h/c`
- **Components**: Pure data structures (no behavior)
- **Systems**: Process components, implement game logic in `src/system/`
- **World**: Central manager for all entities and components

### 2. System Module Structure

Each system in `src/system/` follows this pattern:

```c
// system/example.h
#ifndef SYSTEM_EXAMPLE_H
#define SYSTEM_EXAMPLE_H

#include "../core.h"
#include "../render.h"

// System update function (called by scheduler)
void example_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Additional system-specific functions...

#endif // SYSTEM_EXAMPLE_H
```

```c
// system/example.c
#include "example.h"
#include <stdio.h>
// Additional standard library includes...

void example_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    // System implementation...
}
```

### 3. Include Pattern

- **System headers**: Use relative paths (`#include "../core.h"`)
- **System implementations**: Include only their own header and standard library
- **Main source files**: Use direct includes from `src/` root

## Build System

### Makefile Organization
- Source files listed in `SOURCES` variable
- Uses organized system paths: `system/physics.c system/camera.c` etc.
- No legacy `system_*.c` files

### Compilation
```bash
make clean && make    # Clean build
make test            # Run test suite
make run             # Build and run
```

## Removed Legacy Files

The following legacy files were removed during cleanup:

- `src/system_camera.h/c` → Consolidated into `src/system/camera.h/c`
- `src/system_physics.h/c` → Consolidated into `src/system/physics.h/c`
- `src/system_collision.h` → Consolidated into `src/system/collision.h/c`
- `src/test_legacy.c.backup` → Removed backup file
- `src/assets.c.backup` → Removed backup file

## System Integration

### System Scheduler (`systems.h/c`)
All systems are registered in the scheduler with their update frequencies:

```c
scheduler->systems[SYSTEM_PHYSICS] = (SystemInfo){ 
    .name = "Physics", 
    .frequency = 60.0f, 
    .update_func = physics_system_update 
};
scheduler->systems[SYSTEM_CAMERA] = (SystemInfo){ 
    .name = "Camera", 
    .frequency = 60.0f, 
    .update_func = camera_system_update 
};
scheduler->systems[SYSTEM_LOD] = (SystemInfo){ 
    .name = "LOD", 
    .frequency = 30.0f, 
    .update_func = lod_system_update 
};
// etc.
```

### System Update Signature
All systems use the consistent signature:
```c
void system_update(struct World* world, RenderConfig* render_config, float delta_time);
```

## Benefits of Clean Organization

1. **Modular Design**: Each system is self-contained
2. **Clear Dependencies**: Consistent include patterns
3. **Easy Testing**: Isolated systems can be unit tested
4. **Maintainable**: Clear separation of concerns
5. **Extensible**: Easy to add new systems following the pattern
6. **No Duplication**: Legacy duplicate files removed
7. **Consistent Naming**: All system files follow same pattern

## Development Guidelines

When adding new systems:

1. Create `src/system/newsystem.h` and `src/system/newsystem.c`
2. Follow the consistent include pattern
3. Add system to `SystemType` enum in `systems.h`
4. Register system in `scheduler_init()` in `systems.c`
5. Add system files to `SOURCES` in `Makefile`
6. Write unit tests in `tests/test_newsystem.c`

This organization ensures the codebase remains clean, maintainable, and follows the established patterns.
