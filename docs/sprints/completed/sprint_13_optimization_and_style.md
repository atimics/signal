# Sprint 13: Implementation Guide - Optimization & Code Style

**ID**: `sprint_13.0`
**Status**: **ACTIVE & CRITICAL**
**Author**: Gemini (Lead Scientist and Researcher)
**Related Research**: `R14_Efficient_Mesh_Rendering`

## 1. Sprint Goal

This sprint has two primary goals that will finalize the engine's foundational architecture:
1.  **Implement View Frustum Culling**: To introduce the engine's first major performance optimization, preventing the GPU from rendering objects that are not visible to the camera.
2.  **Enforce Code Standards**: To pay down the remaining technical debt from Sprint 11.6 by applying a universal code format and completing the documentation pass.

Completing this sprint will result in an engine that is not only correct, but also efficient and highly maintainable, creating the perfect launchpad for PBR development.

## 2. Core Tasks & Implementation Plan

This plan is designed to be executed sequentially by the C development team.

### **Task 1: Implement View Frustum Culling**

*   **Problem**: The engine currently sends every object to the GPU every frame, regardless of whether it is visible. This is a major performance bottleneck.
*   **Guidance**:
    1.  **Augment the `Mesh` Struct**: In `src/assets.h`, add `Vector3 aabb_min;` and `Vector3 aabb_max;` to the `Mesh` struct.
    2.  **Update the C Loader**: In `src/asset_loader/asset_loader_mesh.c`, inside the `load_cobj_binary` function, ensure the AABB data from the file header is copied into the new `Mesh` struct members.
    3.  **Implement Frustum Extraction**: In `src/core.c`, create a new function `camera_extract_frustum_planes(const struct Camera* camera, float frustum_planes[6][4])`. This function will calculate the six planes of the view frustum from the camera's view-projection matrix. A standard algorithm for this can be found in many graphics programming resources.
    4.  **Implement the Culling Check**: In `render_frame()` within `src/render_3d.c`, before an entity is rendered, get its AABB and perform an intersection test against the six frustum planes. A common and efficient way to do this is the "AABB vs. Plane" test. If the AABB is determined to be fully outside any single plane, the entity should be culled, and the `sg_draw()` call for it should be skipped.
*   **Test**: Create a new test case, `test_frustum_culling()`, that creates two entities, places one demonstrably outside the camera's view, and asserts that only one draw call is issued. This will likely require mocking or instrumenting the `sg_draw` function.

### **Task 2: Code Style and Documentation Pass**

*   **Problem**: The codebase lacks a consistent style and sufficient documentation, increasing technical debt and making the code harder to read and maintain.
*   **Guidance**:
    1.  **Create `.clang-format`**: Add a `.clang-format` file to the project root. A good starting point is the "Google" style: `BasedOnStyle: Google`.
    2.  **Format All Code**: Run `clang-format -i src/**/*.c src/**/*.h tests/**/*.c tests/**/*.h` to format the entire C codebase. This is a one-time action that will standardize the project's style.
    3.  **Document All Public Headers**: Perform a thorough pass on all `.h` files in the `src/` directory. Add Doxygen-style comment blocks (`/** ... */`) to every public function, struct, and enum, explaining its purpose, parameters, and return value. This is critical for long-term maintainability.

## 3. Definition of Done

This sprint is **DONE** when:
1.  View frustum culling is implemented and the `test_frustum_culling` test passes.
2.  The `make run` command demonstrably culls off-screen objects (this can be verified by adding a debug log that prints the number of draw calls per frame).
3.  A `.clang-format` file exists at the project root, and all C code has been formatted.
4.  All public headers in `src/` are fully documented with Doxygen-style comments.
5.  `make` and `make test` complete successfully without any errors or warnings.
