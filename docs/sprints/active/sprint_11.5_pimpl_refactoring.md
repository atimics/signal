# Sprint 11.5: PIMPL Refactoring - Plan & Implementation Guide

**ID**: `sprint_11.5`
**Status**: **ACTIVE & CRITICAL**
**Lead**: Gemini (Lead Scientist and Researcher)
**Related Research**: `R06_C_Header_Architecture.md`

## 1. Sprint Goal

To achieve full compliance with the `R06` architectural guide by refactoring all graphics-related data structures to use the **Opaque Pointer (PIMPL) Idiom**. This will completely decouple the engine's public headers from the underlying Sokol GFX implementation, eliminating header conflicts and creating a stable foundation for future rendering work.

## 2. Definition of Done

This sprint is **DONE** when:
1.  No public engine header (`.h` files in `src/`) includes `sokol_gfx.h` or any other Sokol header.
2.  No public engine header exposes any `sg_*` types in its struct definitions or function signatures. All such types must be hidden behind opaque pointers.
3.  The `make test-full` command passes, confirming that the refactoring has not broken existing functionality.
4.  The main application (`make run`) compiles and runs, visually confirming that the `wedge_ship` can still be rendered.

---

## 3. Mid-Sprint Review & Guidance (As of June 30)

### 3.1. Progress Assessment
The development team has begun the PIMPL refactoring as per the implementation guide. An analysis of the `git status` shows that the relevant headers and implementation files (`assets.c/h`, `render_mesh.c/h`, etc.) have been modified.

### 3.2. Validation Failure Analysis
A validation run of `make test-full` **failed with compilation errors**. This is an expected and positive outcome, as it demonstrates that our test suite is correctly catching the breaking changes introduced by the refactoring.

**Root Cause of Failure:**
1.  **Out-of-Sync Tests**: The test files (`tests/test_assets.c`, `tests/test_rendering.c`) were written for the old architecture and are still trying to directly access Sokol members (e.g., `mesh->sg_vertex_buffer`) that no longer exist on the public structs.
2.  **Makefile Misconfiguration**: The `test-full` build rule is not correctly passing the `-DSOKOL_DUMMY_BACKEND` flag, causing it to fail when compiling Sokol's platform-specific application code.

### 3.3. Actionable Guidance for Development Team

The remainder of this sprint is now focused on **fixing the test suite** to align with the new PIMPL architecture.

*   **Primary Task**: Update the test files to use the new PIMPL-compliant accessor functions. The tests are the specification; making them pass *is* the final part of the implementation.

*   **Example Required Change in `tests/test_rendering.c`**:
    ```c
    // OLD, BROKEN CODE:
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ship_mesh->sg_vertex_buffer.id);

    // NEW, CORRECT CODE:
    // This requires that you first implement `mesh_get_gpu_buffers` in `assets.c/h`
    sg_buffer vbuf = {0};
    sg_buffer ibuf = {0};
    mesh_get_gpu_buffers(ship_mesh, &vbuf, &ibuf); 
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, vbuf.id);
    TEST_ASSERT_NOT_EQUAL(SG_INVALID_ID, ibuf.id);
    ```

*   **Makefile Fix**:
    *   Ensure the `clang` command for the `test-full` target includes the `-DSOKOL_DUMMY_BACKEND` flag.

The sprint is on the right track. Completing these final steps will achieve the "Definition of Done".

---

## 4. Implementation Guide & Technical Tasks

**The Core Principle**: Header files (`.h`) define the **what** (the public API). Implementation files (`.c`) define the **how** (the implementation details). Low-level library details like Sokol should *never* be in a public header.

### Task 1: Refactor `assets.h` (`Mesh` and `Texture` Structs)
*   **Action**: Modify the `Mesh` and `Texture` structs to use opaque pointers (`MeshGpuResources*`, `TextureGpuResources*`).
*   **Details**:
    *   In `assets.h`, forward-declare `struct MeshGpuResources;` and `struct TextureGpuResources;` and change the struct members to pointers.
    *   In `assets.c`, define these structs to contain the `sg_*` types.
    *   In `assets.c`, update the allocation logic (e.g., `load_compiled_mesh_absolute`) to `calloc` memory for the new `gpu_resources` struct.
    *   In `assets.c`, update `assets_cleanup` to `free` the `gpu_resources` pointer.

### Task 2: Refactor `render_mesh.h` (`MeshRenderer` Struct)
*   **Action**: Modify the `MeshRenderer` struct to use an opaque pointer (`MeshRendererGpuResources*`).
*   **Details**:
    *   In `render_mesh.h`, forward-declare `struct MeshRendererGpuResources;` and change the struct member to a pointer.
    *   In `render_mesh.c`, define this struct to contain the `sg_pipeline` and `sg_shader`.
    *   Update `mesh_renderer_init` and `mesh_renderer_cleanup` to manage the allocation and freeing of this struct.

### Task 3: Update All Usage Sites
*   **Action**: This is the most tedious, but critical, part. You must now find every place in the code that accessed the old Sokol types directly and update it to use the new opaque pointer.
*   **Example Change**:
    *   **Old Code:** `sg_apply_bindings(mesh->sg_vertex_buffer, ...)`
    *   **New Code:** `sg_apply_bindings(mesh->gpu_resources->sg_vertex_buffer, ...)`

### Task 4: Create Accessor Functions (For Tests)
*   **Action**: To fix the test suite, you will need to create new functions that expose the underlying Sokol handles in a controlled way.
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
