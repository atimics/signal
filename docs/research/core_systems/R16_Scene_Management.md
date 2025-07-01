# Research Brief: A Scalable Scene Management Architecture

**ID**: `R16_Scene_Management`
**Author**: Gemini (Lead Scientist and Researcher)
**Status**: Draft

## 1. Overview

This document provides a research-driven analysis of the CGame engine's current scene management system and proposes a new, scalable architecture to support our long-term vision. A robust scene system is fundamental for creating complex, dynamic worlds, managing engine resources efficiently, and enabling advanced features like streaming and dynamic level-of-detail (LOD).

Our current system, while functional for prototyping, has significant limitations that will impede future development. This document outlines a clear, phased path toward a professional-grade scene management architecture.

## 2. Analysis of the Current System (`src/data.c`)

The current system is based on two types of text files:
*   `entities.txt`: A flat list of "templates" that define the components and properties of different types of entities.
*   `scenes/*.txt`: Files that define a scene as a list of entity templates to spawn at specific locations.

### Strengths:

*   **Data-Driven**: The approach is fundamentally data-driven, which is excellent. It allows designers to create new entity types and build scenes without modifying C code.
*   **Simplicity**: The key-value and `spawn:` syntax is simple and easy to understand for basic use cases.

### Weaknesses:

*   **Monolithic Loading**: The entire scene and all its required assets are loaded into memory at once. This is not scalable for large worlds and will lead to long loading times and high memory usage.
*   **Text-Based Parsing**: The runtime parsing of these text files is inefficient. It relies on `fgets` and `sscanf`, which are slow compared to reading a pre-compiled binary format.
*   **Lack of Hierarchy**: The scene is a flat list of entities. There is no concept of a "scene graph" or parent-child relationships, which makes it impossible to create complex, nested objects (e.g., a turret on a ship, a light on a building).
*   **No Dynamic Loading/Unloading**: The system cannot stream parts of the world in and out, a critical feature for open-world games or large levels.
*   **Limited Expressiveness**: The format cannot express complex relationships, instance-specific property overrides, or references between entities.

## 3. Proposed Architecture: A Hybrid Scene Graph Model

I propose we evolve our system into a hybrid model that combines the best of our data-driven approach with the power of a traditional scene graph, using a pre-compiled binary format for performance.

### 3.1. The `.cscene` Binary Format

Similar to how we optimized `.cobj` files, we will introduce a pre-compiled binary scene format: `.cscene`. The offline Python toolchain will be responsible for compiling the source `.txt` files into this efficient format.

The `.cscene` file would be structured as a series of "chunks," each identified by a 4-byte magic number.

*   **`HEAD` Chunk**: Contains header information (version, number of entities, etc.).
*   **`NODE` Chunk**: Defines a node in the scene graph. Each node has a transform (position, rotation, scale) and a list of its children.
*   **`COMP` Chunk**: Defines a component and its data, associated with a specific node.
*   **`ASST` Chunk**: A table of all unique asset paths (meshes, textures) required by the scene, allowing for efficient string management.

### 3.2. The Scene Graph

At runtime, the engine will load the `.cscene` file and build a tree-like data structure called a **scene graph**.

*   **Nodes**: Each node in the tree represents an entity's `Transform` (its position, rotation, and scale in the world).
*   **Hierarchy**: Nodes can be parented to other nodes. A child node's transform is relative to its parent. This allows us to create complex, articulated objects. For example, a `ship` node could have a `turret` node as a child. When the ship moves, the turret automatically moves with it.
*   **Components**: Components (like `Renderable`, `Physics`, `AI`) are attached to the nodes.

### 3.3. Key Advantages of the New Architecture

*   **Performance**: Loading a pre-compiled binary scene graph is orders of magnitude faster than parsing text files.
*   **Hierarchy**: Enables the creation of complex, nested objects and prefabs.
*   **Scalability**: Forms the foundation for advanced features:
    *   **World Streaming**: We can load and unload different parts of the scene graph (sub-graphs) dynamically as the player moves through the world.
    *   **Efficient Culling**: The hierarchical nature of the graph allows for efficient culling algorithms (e.g., if a parent node is outside the view frustum, we know none of its children need to be rendered).
*   **Flexibility**: Allows for instance-specific overrides. A scene file could specify that a particular instance of a `wedge_ship` should have its scale overridden to be twice as large, without needing a new entity template.

## 4. Phased Implementation Plan

This is a significant architectural upgrade that should be implemented in phases.

### **Phase 1: Implement Scene Graph and Binary Loader (Next Major Sprint)**

*   **Objective**: Replace the current text-based loading system with the new binary `.cscene` format and a runtime scene graph.
*   **Tasks**:
    1.  **Update Python Tools**: Extend the `build_pipeline.py` to compile scene and entity template files into the `.cscene` format.
    2.  **Implement Scene Graph in C**: Create the `SceneNode` and related data structures in `src/core.h`.
    3.  **Implement `.cscene` Loader**: Rewrite `src/data.c` to load the binary `.cscene` format and build the scene graph in memory.
    4.  **Update Systems**: Modify the `render_frame`, `physics_system_update`, etc., to traverse the new scene graph instead of iterating over a flat list of entities.

### **Phase 2: Prefab System**

*   **Objective**: Allow designers to create complex, multi-part objects as reusable "prefabs."
*   **Tasks**:
    1.  Extend the tooling to allow saving a sub-graph from a scene as a `.cprefab` file.
    2.  Implement logic in the engine to instantiate a prefab at runtime.

### **Phase 3: World Streaming**

*   **Objective**: Implement dynamic loading and unloading of scene sections for large worlds.
*   **Tasks**:
    1.  Divide the world into a grid of "cells."
    2.  Each cell will correspond to a separate `.cscene` file.
    3.  Implement a system that loads and unloads these cells based on the player's position.

## 5. Conclusion

Our current scene management system has served its purpose for prototyping, but it is a critical bottleneck for future development. By transitioning to a pre-compiled, binary scene graph architecture, we will unlock a new level of performance, scalability, and design flexibility. This is an essential and logical next step in the evolution of the CGame engine, and I recommend we prioritize **Phase 1** of this plan after the current performance and PBR sprints are complete.
