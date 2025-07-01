# CGame Engine Architecture

**Status**: Current | **Last Updated**: June 30, 2025

## 1. Core Philosophy: Data-Oriented Design

The CGame engine is built from the ground up using a **data-oriented** approach. This means we prioritize the layout and access patterns of our data to maximize hardware performance. Instead of focusing on object-oriented hierarchies, we focus on:

*   **Data as the Primary Citizen**: Components are simple C structs containing only data.
*   **Cache-Friendly Operations**: Systems are functions that iterate over tightly packed arrays of components, minimizing CPU cache misses.
*   **Decoupling**: Data and logic are kept separate, which makes the codebase easier to maintain, test, and reason about.

## 2. The Three Architectural Pillars

The engine's design is built on three primary pillars: the ECS, the Asset Pipeline, and the Graphics Abstraction Layer.

### Pillar 1: Entity-Component-System (ECS)

The ECS is the backbone of the engine's runtime.

*   **Entities**: Simple integer IDs that "own" a collection of components. They are the "who".
*   **Components**: Pure data structs that represent a single facet of an entity (e.g., `Transform`, `Renderable`). They are the "what".
*   **Systems**: Global functions that operate on entities with a specific combination of components (e.g., `render_system`, `physics_system`). They are the "how".

This design allows for immense flexibility and performance. A `System` can operate on any `Entity` that has the `Components` it cares about, without needing to know what the entity "is".

### Pillar 2: The Data-Driven Asset Pipeline

The engine's content is managed by a robust, data-driven asset pipeline. This pipeline ensures that the runtime engine deals only with simple, optimized data, which is crucial for fast loading and performance.

The flow is as follows:

1.  **Source Assets (`/assets`)**: Artist-created files like `.obj`, `.png`, etc.
2.  **Asset Compiler (`/tools/asset_compiler.py`)**: A Python script that processes source assets into an optimized, engine-ready format (`.cobj`, `.ctex`).
3.  **Asset Index (`/build/assets/index.json`)**: The **single source of truth** for the runtime. This file maps logical asset names (e.g., "wedge_ship") to the physical, compiled data files.
4.  **Runtime Loader (`/src/assets.c`)**: The engine's asset loader reads *only* the `index.json` to find and load assets. This decouples the engine from the physical file layout and makes the loading process simple and robust.

### Pillar 3: The Graphics Abstraction Layer (PIMPL)

To ensure the engine's core logic is not tightly coupled to a specific graphics library, we use the **Opaque Pointer (PIMPL) Idiom**. This is a critical architectural pattern that hides implementation details.

*   **The Problem**: High-level headers like `assets.h` should not know about low-level Sokol types like `sg_buffer`. Including `sokol_gfx.h` in a public header leads to header conflicts and slow compilation.
*   **The Solution**:
    1.  A public header (e.g., `assets.h`) only declares a **pointer** to an incomplete struct (e.g., `struct MeshGpuResources*`). This is the "opaque pointer".
    2.  The implementation file (e.g., `assets.c`) includes the low-level library (`sokol_gfx.h`) and provides the full definition of the struct.
    3.  The engine accesses the low-level data through the pointer.

This pattern ensures that if we ever change or upgrade the graphics library, the changes are confined to the `.c` files, and the public headers remain stable. This is the cornerstone of a maintainable rendering architecture.

## 3. Key Engine Systems

*   **System Scheduler (`src/systems.c`)**: Manages the execution of all other systems. It uses a frequency-based approach, allowing different systems to be updated at different rates for performance optimization (e.g., run AI less frequently than rendering).
*   **Modular Mesh Renderer (`src/render_mesh.c`)**: A self-contained module responsible for rendering meshes. It is initialized with its own shaders and pipelines and is called by the main render loop. This enforces a clean separation of concerns.
*   **Testing Framework (`/tests`)**: The engine uses the **Unity Test Framework** for automated testing. All new features and bug fixes should be accompanied by tests. The test suite is run via the `make test` command.