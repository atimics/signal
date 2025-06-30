# Sprint 12: Implementation Guide - Rendering Stabilization

**ID**: `sprint_12.0`
**Status**: **ACTIVE & CRITICAL**
**Author**: Gemini (Lead Scientist and Researcher)

## 1. Sprint Goal

This sprint has two primary, non-negotiable goals:
1.  **Fix Rendering**: To completely resolve all mesh rendering artifacts, ensuring the engine can correctly load and display textured 3D models with proper geometry, matching the `mesh_viewer`'s output.
2.  **Overhaul Asset Pipeline**: To transition the engine from a slow, text-based asset loading system to a high-performance, pre-compiled binary format, as detailed in research brief `R15_Binary_Asset_Format`.

This sprint will also pay down the remaining technical debt from Sprint 11.6 by enforcing a consistent code style and documentation standard.

## 2. Core Tasks & Implementation Plan

This plan is designed to be executed sequentially by an AI agent developer. Each task builds upon the last.

### **Task 1: Fix Texture Loading Path**

*   **Problem**: The engine cannot load any textures due to a bug in how file paths are constructed in `src/assets.c`.
*   **Guidance**:
    1.  **Isolate the Bug**: Examine the `load_texture` function in `src/assets.c`. The line `snprintf(full_path, sizeof(full_path), "%s/textures/%s", registry->asset_root, texture_path);` is likely incorrect, as the `texture_path` may already be a full path.
    2.  **Implement the Fix**: Modify the path construction logic to correctly handle both relative and absolute paths, ensuring a valid path is always generated.
    3.  **Write a Test**: Create a new test case in `tests/test_assets.c`, `test_texture_loading_path()`, that specifically calls the path generation logic and asserts that the output is correct. This test must fail before the fix and pass after.

### **Task 2: Transition to a Binary `.cobj` Format**

*   **Problem**: The current text-based asset format is a major performance bottleneck and prevents the storage of pre-computed data like tangents and bounding boxes.
*   **Guidance**:
    1.  **Implement the `COBJHeader`**: Add the `COBJHeader` struct definition from `R15_Binary_Asset_Format` to a relevant header file (e.g., `src/assets.h`).
    2.  **Update the Python Compiler (`tools/asset_compiler.py`)**:
        *   Modify the script to parse the source `.obj` file.
        *   **Crucially, implement tangent and bitangent calculation** for each vertex. The MikkTSpace algorithm is the standard.
        *   Calculate the Axis-Aligned Bounding Box (AABB) for the entire mesh.
        *   Using Python's `struct` module, pack and write the `COBJHeader`, followed by the raw vertex data (including tangents), and finally the index data to a new `.cobj` file.
    3.  **Rewrite the C Loader (`src/asset_loader/asset_loader_mesh.c`)**:
        *   Replace the entire `parse_obj_file` function with a new `load_cobj_binary` function.
        *   This new function will open the `.cobj` file in binary mode (`"rb"`).
        *   It will perform a single `fread` to load the header, validate the magic number and version, allocate the exact memory required for vertices and indices, and then use two more `fread` calls to load the vertex and index data directly into the allocated buffers.
    4.  **Write a Test**: Create a new test case, `test_binary_mesh_loading()`, that loads a known binary `.cobj` file and asserts that the `vertex_count`, `index_count`, and AABB data in the loaded `Mesh` struct match the header.

### **Task 3: Implement View Frustum Culling**

*   **Problem**: The engine wastes GPU cycles rendering objects that are not visible to the camera.
*   **Guidance**:
    1.  **Augment the `Mesh` Struct**: Add `Vector3 aabb_min` and `Vector3 aabb_max` to the `Mesh` struct in `src/assets.h`. The `load_cobj_binary` function should populate this from the file header.
    2.  **Implement Frustum Extraction**: In `src/core.c`, create a new function `camera_extract_frustum_planes(struct Camera* camera, Plane frustum[6])`. This function will calculate the six planes of the view frustum from the camera's view-projection matrix.
    3.  **Implement the Culling Check**: In `render_frame()` within `src/render_3d.c`, before an entity is rendered, perform an intersection test between the entity's world-space AABB and the six frustum planes. If the AABB is outside any plane, skip the draw call for that entity.
    4.  **Write a Test**: Create a test `test_frustum_culling()` that creates two entities, one inside and one outside the frustum, and asserts that only one draw call is made.

### **Task 4: Code Style and Documentation Pass**

*   **Problem**: The codebase lacks a consistent style and sufficient documentation, increasing technical debt.
*   **Guidance**:
    1.  **Create `.clang-format`**: Add a `.clang-format` file to the project root. Use a standard style like "Google" or "LLVM" as a base.
    2.  **Format All Code**: Run `clang-format -i src/**/*.c src/**/*.h tests/**/*.c tests/**/*.h` to format the entire C codebase.
    3.  **Document All Public Headers**: Go through every `.h` file in `src/` and add Doxygen-style comment blocks to every public function, struct, and enum, explaining its purpose, parameters, and return value.

## 3. Definition of Done

This sprint is **DONE** when:
1.  All new tests (`test_texture_loading_path`, `test_binary_mesh_loading`, `test_frustum_culling`) are implemented and pass.
2.  The engine can successfully load and render textured meshes from the new binary `.cobj` format without any visual artifacts.
3.  View frustum culling is operational and demonstrably reduces draw calls for off-screen objects.
4.  A `.clang-format` file exists, and all C code has been formatted.
5.  All public headers in `src/` are fully documented with Doxygen-style comments.
6.  `make` and `make test` complete successfully without any errors or warnings.
