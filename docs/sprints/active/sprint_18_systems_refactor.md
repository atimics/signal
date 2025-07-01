# Sprint 18: Systems Refactoring

**Author**: Gemini
**Date**: June 30, 2025
**Status**: Proposed

## 1. Goals

This sprint focuses on refactoring the `systems.c` file to improve modularity, reduce coupling, and enhance testability. The primary goals are:

*   Separate each system into its own file.
*   Introduce dependency injection for registries.
*   Decouple system initialization.
*   Establish a clear system interface.

## 2. Scope

This sprint will cover the following tasks:

1.  **Create new files for each system**:
    *   `src/system_physics.c`, `src/system_physics.h`
    *   `src/system_collision.c`, `src/system_collision.h`
    *   `src/system_ai.c`, `src/system_ai.h`
    *   `src/system_camera.c`, `src/system_camera.h`
2.  **Move system implementations**: Relocate the code for each system from `systems.c` to its new respective file.
3.  **Update `systems.h`**: Define a generic system interface and remove system-specific declarations.
4.  **Update `systems.c`**: The existing `systems.c` will be repurposed to house the system scheduler, which will manage the system instances.
5.  **Refactor `scheduler_init`**: The initialization logic will be broken down. The main application will be responsible for initializing dependencies and passing them to the scheduler.
6.  **Update `main.c`**: The main application loop will be updated to reflect the new initialization and shutdown procedures.
7.  **Update `Makefile`**: The new system files will be added to the build process.

## 3. Out of Scope

*   No new features will be added.
*   The functionality of the systems themselves will not be changed.
*   No C-level tests will be written in this sprint, as the testing framework is not yet in place.

## 4. Definition of Done

*   All new source files are created and populated.
*   `systems.c` and `systems.h` are refactored as described.
*   The application compiles and runs successfully after the changes.
*   The `Makefile` is updated to include the new files.
*   The game's existing behavior remains unchanged.
