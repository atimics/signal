# Sprint 02 Review: Modular Camera System

**Period:** June 2025

## Sprint Goal

Refactor the existing rendering pipeline to support a flexible, component-based camera system. This will decouple camera logic from the core render loop, enable multiple camera perspectives, and lay the groundwork for more advanced cinematic and gameplay features.

## Review

### What was accomplished:

*   **Camera Component:** A new `CameraComponent` was created and integrated into the ECS. This component stores camera properties like FOV, near/far planes, and aspect ratio, as well as behavior settings.
*   **Camera System:** A `camera_system_update` function was created to manage the active camera. This system computes the view and projection matrices based on the active camera's `TransformComponent` and `CameraComponent`.
*   **Decoupling from Renderer:** The `Camera3D` object was removed from `RenderConfig`, and the `render_frame` function was modified to be independent of camera logic. The camera's view and projection matrices are now passed to the rendering functions.
*   **Multiple Camera Support:** The `World` struct now tracks the `active_camera_entity`, and a `world_set_active_camera` function allows for switching between cameras. The entity template system was updated to support camera definitions, allowing for the creation of multiple cameras from data.
*   **Player and Entity Integration:** The player entity now has a `CameraComponent` attached by default, and the default camera behavior is a smooth, third-person follow camera.

### What went well:

*   The camera system is now completely modular and data-driven, which is a significant improvement over the previous monolithic implementation.
*   The decoupling of the camera from the renderer makes the rendering pipeline much cleaner and easier to maintain.
*   The ability to switch between multiple cameras at runtime opens up new possibilities for gameplay and cinematic features.

### What could be improved:

*   The camera behaviors are currently hard-coded in the `camera_system_update` function. A more flexible system, such as scripting, could be used to define camera behaviors in the future.
*   The camera system currently relies on a simple lerp for smoothing. More advanced smoothing techniques could be implemented for a more polished feel.

## Retrospective

This sprint was highly successful and represents a major step forward for the engine's architecture. The new component-based camera system is a robust and flexible solution that will be a valuable asset for future development. The successful completion of this sprint has also significantly de-risked the upcoming transition to the Sokol graphics API, as the camera system is now well-prepared for the change.
