# Sprint 02: Modular Camera System

**Goal:** Refactor the existing rendering pipeline to support a flexible, component-based camera system. This will decouple camera logic from the core render loop, enable multiple camera perspectives, and lay the groundwork for more advanced cinematic and gameplay features.

**Period:** June 2025

## Key Objectives:

1.  **Camera Component:**
    *   [ ] Define a new `CameraComponent` that can be attached to any entity.
    *   [ ] The component will store camera properties (FOV, near/far planes, aspect ratio, etc.) and state (e.g., `is_active`).
    *   [ ] This replaces the monolithic `Camera3D` struct in `RenderConfig`.

2.  **Camera System:**
    *   [ ] Create a `camera_system_update` function that runs each frame.
    *   [ ] This system will identify the currently active camera entity.
    *   [ ] It will compute the view and projection matrices based on the active camera's `TransformComponent` and `CameraComponent`.
    *   [ ] The system will pass the final view matrix to the rendering functions, removing direct camera manipulation from the `render_frame` loop.

3.  **Decoupling from Renderer:**
    *   [ ] Remove the `Camera3D` object from `RenderConfig`.
    *   [ ] Modify `render_frame` and other rendering functions to accept a `view_matrix` and `projection_matrix` as arguments, rather than accessing a global camera.
    *   [ ] The `camera_follow_entity` logic will be moved into a new, reusable camera behavior function or script.

4.  **Multiple Camera Support:**
    *   [ ] The `World` struct will track the `active_camera_entity`.
    *   [ ] Implement a function `world_set_active_camera(EntityID camera_entity)` to switch between cameras.
    *   [ ] Create at least two camera entities in a test scene:
        *   A third-person chase camera attached to the player.
        *   A static, wide-angle security camera overlooking the scene.

5.  **Player & Entity Integration:**
    *   [ ] The player entity will have a `CameraComponent` attached by default.
    *   [ ] The default camera behavior will be a smooth, third-person follow camera.
    *   [ ] Ensure that creating or destroying entities with cameras is handled cleanly.

## Expected Outcomes:

*   The engine will no longer rely on a single, global camera.
*   Camera logic will be encapsulated within its own system, improving code organization and modularity.
*   It will be possible to switch between different camera views at runtime.
*   The foundation will be set for more complex camera behaviors, such as cinematic sequences, free-look cameras, and dynamic camera transitions.

## Pre-computation/Pre-analysis:

*   The current camera implementation is a single `Camera3D` struct within the `RenderConfig`.
*   The main render loop in `render_3d.c` directly calls `camera_follow_entity`, tightly coupling the camera's behavior to the player.
*   The projection logic in `project_3d_to_2d` is dependent on the global camera state.
*   The entity-component system in `core.h` and `core.c` will need to be extended to include a `COMPONENT_CAMERA` and its associated data pool.
