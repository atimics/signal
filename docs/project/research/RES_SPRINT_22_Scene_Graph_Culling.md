# Research: Sprint 22 - Scene Graph & Culling

**ID**: `RES_SPRINT_22`
**Status**: **PROPOSED**
**Author**: Gemini, Chief Science Officer
**Related Sprints**: `sprint_22_scene_graph_culling.md`
**Related Documents**: `GAME_VISION.md`, `PLAYER_MANUAL.md`

## 1. Overview

This document provides the foundational research for implementing a hierarchical **Scene Graph** architecture and **View Frustum Culling** in the CGame engine. These are critical for efficiently managing and rendering the vast, complex derelicts described in the game's vision, ensuring both performance and design flexibility.

## 2. Core Concepts from Game Vision

As defined in `GAME_VISION.md` and `PLAYER_MANUAL.md`:

### The Setting: The Graveyard

The game takes place within a single, massive star system known as "The Graveyard," littered with colossal, silent Ark-Ships. Each derelict is many kilometers long, forming a labyrinth of metal and rock.

### High-Speed Exploration (The A-Drive)

The A-Drive allows players to "surf" along the hulls and corridors of derelicts at high speed, navigating complex wreckage with thrilling agility. This necessitates efficient rendering of complex, large-scale environments.

## 3. Research Focus Areas

### 3.1. Scene Graph Architecture

*   **Objective**: To design a hierarchical data structure that represents the relationships between objects in the game world.
*   **Considerations**:
    *   **Nodes**: What data will each node store? (e.g., local transform, world transform, parent/child references).
    *   **Traversal**: How will the scene graph be traversed for updates and rendering? (e.g., depth-first, breadth-first).
    *   **Hierarchy**: How will parent-child relationships be managed? (e.g., a turret moving with a ship).
    *   **ECS Integration**: How will the scene graph integrate with the existing ECS, potentially as a new component (`SceneNode`)?

### 3.2. View Frustum Culling

*   **Objective**: To prevent rendering objects that are outside the camera's view, significantly improving performance in large scenes.
*   **Considerations**:
    *   **Frustum Extraction**: How to accurately extract the six planes of the camera's view frustum from its view-projection matrix.
    *   **Bounding Volumes**: Using Axis-Aligned Bounding Boxes (AABBs) or Spheres for efficient intersection tests with frustum planes.
    *   **Culling Algorithm**: Implementing an efficient algorithm to test if an object's bounding volume is inside, outside, or intersecting the frustum.
    *   **Integration with Scene Graph**: Performing culling checks during scene graph traversal, allowing for early exits (e.g., if a parent node is culled, all its children are also culled).

### 3.3. Hierarchical Asset Management

*   **Objective**: To extend the asset pipeline to support hierarchical scene definitions and prefabs.
*   **Considerations**:
    *   **Scene Format**: How to represent parent-child relationships in the scene data format (e.g., `attach_to` keyword).
    *   **Binary Scene Format**: How to store hierarchical scene data in an optimized binary format (`.cscene`).
    *   **Tooling**: Updating the Python asset compilation tools to parse and generate hierarchical scene data.

## 4. Technical Considerations

*   **Matrix Math**: Accurate calculation and propagation of local and world transforms through the scene graph.
*   **Performance**: The scene graph traversal and culling logic must be highly optimized to avoid becoming a new bottleneck.
*   **Memory**: Efficient memory management for scene nodes and their associated data.

## 5. Next Steps

This research will directly inform the implementation tasks for Sprint 22. The focus will be on creating the core C-side data structures for the scene graph, implementing frustum culling, and updating the asset pipeline to support hierarchical scenes.
