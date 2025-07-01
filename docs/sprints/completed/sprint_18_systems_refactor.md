# Sprint 18: Systems Refactoring

**Author**: Gemini
**Date**: June 30, 2025
**Status**: Active

## 1. Overview

This sprint focuses on a critical refactoring of the engine's system architecture. The current implementation in `systems.c` is monolithic and tightly coupled, hindering future development and testing. This refactoring will improve modularity, reduce dependencies, and establish a clean, extensible foundation for all engine systems.

**This is a pure refactoring sprint. No new features will be introduced.** The primary outcome is a more maintainable and robust codebase.

**Supporting Research**: For a detailed analysis of the problems and the justification for this refactoring, please review the full research document:
[R17: Refactoring `systems.c`](../../archive/R17_SystemsC_Refactoring.md)

## 2. Goals

*   **Decouple Systems**: Isolate each core system (Physics, Collision, AI, Camera) into its own self-contained module.
*   **Improve Code Organization**: Move system-related files into a dedicated `src/system/` directory.
*   **Clarify Dependencies**: Eliminate global state for asset and data registries by using dependency injection.
*   **Establish a Clear Interface**: Create a generic system interface to standardize how systems are managed and updated.

## 3. Technical Plan

The refactoring will be executed in the following sequence. **It is crucial to follow this order to minimize disruption and ensure a smooth transition.**

### Phase 1: File Creation and Code Migration (No-Build Phase)

*   **Action**: Create the new directory `src/system/`.
*   **Action**: For each system (Physics, Collision, AI, Camera), create a new header (`.h`) and source (`.c`) file within the `src/system/` directory (e.g., `src/system/physics.h`, `src/system/physics.c`).
    *   **Note**: As per our discussion, the `system_` prefix is redundant and should not be used in the filenames.
*   **Action**: Copy the corresponding system implementation from `systems.c` into the newly created source files.
*   **Action**: Copy the relevant function signatures and struct definitions from `systems.h` into the new header files.
*   **DO NOT MODIFY `systems.c`, `systems.h`, or the `Makefile` in this phase.** The project will not compile, which is expected.

### Phase 2: Refactoring and Integration

1.  **Update `systems.h`**:
    *   **Action**: Remove the individual system update function declarations.
    *   **Action**: Define a generic `System` interface struct. This struct should contain function pointers for `init`, `shutdown`, and `update`, as well as state information like `enabled` and `frequency`.
    *   **Action**: The `SystemScheduler` will now hold an array of these `System` structs.

2.  **Refactor `systems.c`**:
    *   **Action**: This file will now *only* contain the `SystemScheduler` logic.
    *   **Action**: Remove the global `g_asset_registry` and `g_data_registry`.
    *   **Action**: Modify `scheduler_init` to accept pointers to the `AssetRegistry` and `DataRegistry` as parameters.
    *   **Action**: Update `scheduler_init` to iterate through the new `System` array and call each system's `init` function.
    *   **Action**: Update `scheduler_update` to call the `update` function pointer from each enabled system.
    *   **Action**: Update `scheduler_destroy` to call each system's `shutdown` function pointer.

3.  **Update Individual System Files**:
    *   **Action**: Include the new header files (e.g., `#include "system/physics.h"`).
    *   **Action**: Create `init`, `shutdown`, and `update` functions for each system that match the new `System` interface.
    *   **Action**: The `init` function for each system should take any necessary dependencies (like the `AssetRegistry`) as parameters.

4.  **Update `main.c`**:
    *   **Action**: Modify the main application setup to handle the new initialization order.
    *   **Action**: `main.c` is now responsible for initializing the `AssetRegistry` and `DataRegistry`.
    *   **Action**: Pass the initialized registries to `scheduler_init`.

5.  **Update `Makefile`**:
    *   **Action**: Add the new system source files (`src/system/*.c`) to the `SRC_FILES` list so they are included in the build.
    *   **Action**: Ensure the include paths are updated if necessary.

## 4. Definition of Done

*   All tasks in the technical plan are completed.
*   The project compiles and runs without errors.
*   The application's behavior is identical to the pre-refactoring state.
*   The `systems.c` and `systems.h` files are successfully refactored to manage a generic list of systems.
*   The `Makefile` is updated, and the build is clean.
