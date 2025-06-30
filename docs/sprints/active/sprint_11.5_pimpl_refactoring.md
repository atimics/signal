# Sprint 11.5: PIMPL Refactoring - Consolidated Guide

**ID**: `sprint_11.5`
**Status**: **ACTIVE & CRITICAL**
**Lead**: Gemini (Lead Scientist and Researcher)
**Related Research**: `R06_C_Header_Architecture.md`

## 1. Sprint Goal

To achieve full compliance with the `R06` architectural guide by refactoring all graphics-related data structures to use the **Opaque Pointer (PIMPL) Idiom**. This will completely decouple the engine's public headers from the underlying Sokol GFX implementation, creating a stable foundation for future rendering work.

## 2. Definition of Done

This sprint is **DONE** when:
1.  No public engine header (`.h` files in `src/`) includes or exposes any `sg_*` types.
2.  The `make test-full` command passes, confirming the refactoring has not broken existing functionality.
3.  The main application (`make run`) compiles and runs, visually confirming that the `wedge_ship` can still be rendered.

---

## 3. Mid-Sprint Review & Guidance (As of June 30)

### 3.1. Progress Assessment
The development team has made significant progress on the PIMPL refactoring, touching all the relevant source files as per the implementation guide.

### 3.2. Validation Failure Analysis
A validation run of `make test-full` **failed with a linker error**: `Undefined symbols ... "_RUN_TEST_SUITE"`.

*   **Root Cause**: The new `tests/test_runner.c` file, which correctly contains the `main` function, is missing the necessary include for the Unity framework itself. It cannot find the definition for the `RUN_TEST_SUITE` macro.

### 3.3. Actionable Guidance for Development Team

The sprint is very close to completion. The `Makefile` has been correctly updated, but the test runner needs a final correction.

*   **Primary Task**: Add `#include "vendor/unity.h"` to the top of `tests/test_runner.c`. This will provide the necessary macros and resolve the linker error.
*   **Secondary Task**: After fixing the include, run `make test-full` again. If there are any remaining test failures, they will be due to the PIMPL refactoring. Proceed with fixing those tests by implementing and using the new PIMPL accessor functions as detailed in the implementation guide below.

The sprint is in a good state. Fixing the `Makefile` and the test assertions are the final steps to completion.

---

## 4. Implementation Guide & Technical Tasks

**The Core Principle**: Header files (`.h`) define the **what** (the public API). Implementation files (`.c`) define the **how** (the implementation details). Low-level library details like Sokol should *never* be in a public header.

### Task 1: Refactor `assets.h` & `render_mesh.h`
*   **Action**: Modify the `Mesh`, `Texture`, and `MeshRenderer` structs to use opaque pointers (`...GpuResources*`) instead of directly embedding `sg_*` types.
*   **Details**:
    *   In the `.h` files, forward-declare the `...GpuResources` structs and change the members to pointers.
    *   In the corresponding `.c` files, define these structs to contain the actual `sg_*` types.
    *   Update the allocation/deallocation logic (e.g., in `assets_cleanup`, `mesh_renderer_init`) to `calloc` and `free` the new `gpu_resources` structs.

### Task 2: Update All Usage Sites
*   **Action**: Find every place in the code that accessed the old Sokol types directly and update it to use the new opaque pointer.
*   **Example**: `mesh->sg_vertex_buffer` becomes `mesh->gpu_resources->sg_vertex_buffer`.

### Task 3: Create Accessor Functions (To Fix the Tests)
*   **Action**: To fix the test suite, create new functions that expose the underlying Sokol handles in a controlled way.
*   **Example in `assets.h`**:
    ```c
    // Declaration
    void mesh_get_gpu_buffers(const Mesh* mesh, sg_buffer* out_vbuf, sg_buffer* out_ibuf);
    ```
*   **Example in `assets.c`**:
    ```c
    // Implementation
    void mesh_get_gpu_buffers(const Mesh* mesh, sg_buffer* out_vbuf, sg_buffer* out_ibuf) {
        if (mesh && mesh->gpu_resources && out_vbuf && out_ibuf) {
            *out_vbuf = mesh->gpu_resources->sg_vertex_buffer;
            *out_ibuf = mesh->gpu_resources->sg_index_buffer;
        }
    }
    ```