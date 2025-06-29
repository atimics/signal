# Sprint 01 Review: First Contact

**Period:** May 2024

## Sprint Goal

Establish the foundational elements of the game engine, focusing on core data structures, a basic entity-component system (ECS), and rudimentary 3D rendering with SDL.

## Review

### What was accomplished:

*   **Core Data Structures:** Successfully defined and implemented core data types, including `Vector3`, `Quaternion`, and `EntityID`.
*   **Entity-Component System:** A basic ECS was implemented in `core.c` and `core.h`, supporting `Transform`, `Physics`, `Collision`, `AI`, and `Renderable` components. The system includes functions for creating, destroying, and managing entities and their components.
*   **Asset Management:** A simple asset pipeline was created to load `.mesh` and `.mtl` files. An `AssetRegistry` was implemented to manage loaded assets, and a Python-based asset compiler was developed to process raw assets.
*   **3D Rendering:** A basic 3D rendering pipeline was implemented using SDL. This included an `SDL_Window` and `SDL_Renderer`, a `Camera3D` struct, 3D-to-2D projection, and wireframe mesh rendering. A basic lighting model was also introduced.
*   **Gameplay Systems:** A main game loop was created in `systems.c`, and a simple player control system was implemented. The engine can load a test scene from a text file.

### What went well:

*   The foundational ECS was implemented successfully and provides a solid base for future development.
*   The asset pipeline, while simple, allows for the easy integration of new assets.
*   The engine is capable of rendering a 3D scene with multiple objects.

### What could be improved:

*   The 3D rendering pipeline built on top of SDL's 2D renderer is complex and inefficient. This was a known trade-off to get a visual output quickly, but it confirmed the need for a proper 3D graphics API.
*   The memory management for the ECS component pools is basic and could be improved for better performance and robustness.
*   The 3D math for the camera and projection required significant debugging and highlighted the need for a more robust and tested math library.

## Retrospective

The sprint was a success, as it achieved its primary goal of establishing the core engine architecture. The challenges encountered with the SDL-based 3D rendering have provided valuable insights and a clear direction for future sprints, emphasizing the need to transition to a more suitable graphics API like Sokol. The project is in a good position to move forward with the next phase of development.
