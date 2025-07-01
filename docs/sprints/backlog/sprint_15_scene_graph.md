# Sprint 15: Implementation Guide - Scene Graph & Culling

**ID**: `sprint_15.0`
**Status**: **ACTIVE & CRITICAL**
**Author**: Gemini, Chief Science Officer
**Related Research**: `R16_Scene_Management`, `R14_Efficient_Mesh_Rendering`

## 1. Sprint Goal

To evolve the engine from rendering a flat list of entities to a modern, hierarchical **Scene Graph** architecture. This sprint will implement the core data structures and traversal logic for the scene graph and leverage this new structure to implement our first major performance optimization: **View Frustum Culling**.

## 2. Problem Statement

The engine's current scene management is a critical bottleneck. It treats every object as an independent entity, making it impossible to create complex, multi-part objects (e.g., a ship with turrets and lights) or to efficiently determine which objects are visible to the camera. This sprint will replace the flat "list of things" with a structured, hierarchical world representation, unlocking new levels of performance and design complexity.

## 3. Implementation Plan

This plan is designed to be executed sequentially by the C development team.

### Task 1: The `SceneNode` Component

*   **Objective**: To create the core data structure for our scene graph.
*   **Guidance**:
    1.  In `src/core.h`, define a new `ComponentType` flag: `COMPONENT_SCENENODE`.
    2.  Define a new `struct SceneNode` that contains:
        *   `EntityID entity_id`: The ID of the entity this node belongs to.
        *   `EntityID parent`: The ID of the parent entity.
        *   `EntityID children[MAX_CHILDREN]`: An array of child entity IDs.
        *   `uint32_t num_children`: The current number of children.
        *   `float local_transform[16]`: The transform of this node relative to its parent.
        *   `float world_transform[16]`: The final, calculated transform in world space.
    3.  Add this new component to the `ComponentPools` and the main `Entity` struct.

### Task 2: Scene Graph Traversal & World Matrix Calculation

*   **Objective**: To replace the simple iteration in the render loop with a proper, recursive scene graph traversal.
*   **Guidance**:
    1.  In `src/systems.c` or a new `src/scene.c` module, create a new function: `scene_graph_update(struct World* world)`.
    2.  This function should iterate through all `SceneNode` components. For each node that has no parent (`parent == INVALID_ENTITY`), it will begin a recursive traversal.
    3.  The recursive function (`update_node_transform`) will take a parent's world transform and a child node. It will calculate the child's `world_transform` by multiplying the parent's world transform with the child's `local_transform`. It will then call itself for all of that node's children.
    4.  The `render_frame` function in `src/render_3d.c` must be updated to use the new `world_transform` from the `SceneNode` component instead of composing the transform on the fly.

### Task 3: Implement View Frustum Culling

*   **Objective**: To leverage the new scene graph to efficiently cull non-visible objects.
*   **Guidance**:
    1.  **Frustum Extraction**: The `camera_extract_frustum_planes` function (from Sprint 13's plan) should be implemented in `src/core.c`.
    2.  **Culling Check**: The culling logic should be integrated directly into the `scene_graph_update` traversal.
    3.  Before recursively visiting a node's children, perform a frustum check on the parent node using its world-space AABB (from the `Mesh` component).
    4.  **If the parent node is outside the frustum, the entire traversal of its children and all their descendants is skipped.** This is the primary performance gain.
    5.  A `bool is_visible` flag should be added to the `Renderable` component, which is set by this culling process each frame. The `render_frame` function will now simply check this flag before issuing a draw call.

### Task 4: Update Asset Pipeline for Hierarchy

*   **Objective**: To allow our data files to define the new parent-child relationships.
*   **Guidance**:
    1.  **Update Scene Format**: The `data/scenes/*.txt` format needs a new keyword. I propose `attach_to: <parent_entity_name>`.
    2.  **Update Python Tools**: The `tools/build_pipeline.py` and/or a new scene compiler script must be updated to parse this new keyword and store the parent-child relationships in the binary `.cscene` format we researched previously (`R16_Scene_Management`).
    3.  **Update C Loader**: The `data.c` loader must be updated to read this new hierarchical scene format and correctly create the `SceneNode` components with the appropriate parent/child links.

## 4. Definition of Done

1.  The engine uses a scene graph for rendering, not a flat list.
2.  Parent-child relationships are respected (e.g., a "turret" entity moves with its "ship" parent).
3.  View frustum culling is implemented and demonstrably reduces draw calls for off-screen objects.
4.  The asset pipeline and data formats are updated to support scene hierarchies.
5.  All existing tests pass, and new tests for the scene graph and culling logic are created and pass.
