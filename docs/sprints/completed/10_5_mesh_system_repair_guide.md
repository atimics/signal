# Sprint 10.5 Guide: Critical Architectural Repair (Completed)

**Status**: ✅ **Completed**
**JIRA**: `CG-175`

## 1. Sprint Summary

This sprint successfully executed a critical architectural repair of the CGame engine's rendering pipeline. The primary goal—to fix the broken mesh loading system and establish a robust, modular rendering architecture—has been fully achieved. This document serves as the final, consolidated record of the work performed, from initial problem analysis to the final, industry-standard implementation.

The engine can now reliably load and render complex 3D assets, and the underlying architecture is significantly more stable, maintainable, and scalable. This work unblocks all future rendering development, most notably the PBR implementation.

## 2. The Challenge: A Cascade of Failures

The sprint was initiated to address a cascade of failures that prevented any 3D meshes from being rendered:

1.  **Broken Data Pipeline**: The asset loader in `src/assets.c` was failing to resolve correct file paths for compiled `.cobj` assets.
2.  **Invalid GPU Resources**: This silent loading failure led to attempts to create zero-sized GPU buffers, triggering validation errors in the Sokol GFX library.
3.  **Brittle Architecture**: A hardcoded test triangle in `src/render_3d.c` was masking the severity of the issue, and a persistent header dependency conflict between `core.h` and `sokol_gfx.h` was causing recurring compilation errors.

## 3. The Solution: A Two-Phase Implementation

The sprint was executed in two main phases: an initial repair phase to restore functionality, followed by a final architectural refactoring to ensure long-term stability.

### Phase 1: Initial Repair and Restoration

*   **Asset Loading Fix**: The asset pathing logic was repaired, and comprehensive validation was added to the `load_compiled_mesh_absolute` function to prevent the creation of invalid GPU resources.
*   **Modular Renderer Restoration**: The disabled `render_mesh.c` module was restored, and the main render loop in `render_3d.c` was refactored to delegate drawing responsibility to this new module.

### Phase 2: Permanent Architectural Refactor (PIMPL Idiom)

The initial fix, while functional, left a lingering architectural flaw related to header dependencies. The final and most critical task of the sprint was to implement the **Opaque Pointer (PIMPL)** pattern, as recommended in research document `R06`.

**Key Implementation Steps**:

1.  **Opaque Pointer in `core.h`**: The `Renderable` component in [`src/core.h`](../../src/core.h) was refactored to remove all direct references to Sokol GFX types. It now holds an opaque pointer (`struct GpuResources*`) to the graphics-specific data. This permanently breaks the dependency cycle.

    ```c
    // In core.h
    struct GpuResources; // Forward declaration

    struct Renderable {
        struct GpuResources* gpu_resources; // Opaque pointer
        uint32_t index_count;
        // ...
    };
    ```

2.  **Dedicated GPU Resource Management**: A new file, [`src/render_gpu.c`](../../src/render_gpu.c), was created to exclusively manage the lifecycle of the `GpuResources` struct. This module encapsulates the creation, destruction, and access of the underlying Sokol GFX handles, providing a clean abstraction layer.

3.  **Lifecycle Management**: The entity component system was updated to correctly `malloc` the `GpuResources` struct when a `Renderable` component is added and `free` it upon removal, ensuring proper memory management.

## 4. Final Outcome

*   **Stability**: The engine is no longer prone to crashes caused by invalid mesh data or header conflicts.
*   **Modularity**: The rendering logic is now correctly separated. `core.h` is graphics-agnostic, and `render_mesh.c` handles mesh rendering.
*   **Maintainability**: The codebase is cleaner, easier to understand, and aligned with industry best practices, which will accelerate future development.
*   **Unblocked Development**: The successful completion of this sprint unblocks the high-priority PBR rendering sprint.

This sprint was a resounding success, transforming a critical architectural weakness into a robust and scalable foundation for the future of the CGame engine.
