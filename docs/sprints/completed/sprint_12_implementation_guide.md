# Sprint 12: Implementation Guide - Rendering Stabilization

**ID**: `sprint_12.0`
**Status**: **ACTIVE & CRITICAL**
**Author**: Gemini (Lead Scientist and Researcher)

## 1. Progress Update

**COMPLETED**: The Python-based asset compilation pipeline has been successfully implemented and tested.
- A new `tools/compile_mesh.py` script now converts source `.obj` files into a high-performance binary `.cobj` format.
- The new format includes pre-calculated tangents and Axis-Aligned Bounding Box (AABB) data.
- The main `tools/build_pipeline.py` now correctly uses this script to compile all source assets.
- The `build/assets/meshes` directory is populated with the new binary assets.

**NEXT STEPS**: The focus now shifts entirely to the C-side implementation to load and render these new binary assets.

## 2. Sprint Goal

This sprint has two primary, non-negotiable goals:
1.  **Implement the New Asset Loader**: To replace the engine's slow, text-based loader with a high-performance loader for the new binary `.cobj` format.
2.  **Fix Rendering**: To resolve all remaining rendering artifacts, ensuring the engine can correctly load and display textured 3D models, matching the `mesh_viewer`'s output.

This sprint will also pay down the remaining technical debt from Sprint 11.6 by enforcing a consistent code style and documentation standard.

## 3. Core Tasks & Implementation Plan

This plan is designed to be executed sequentially by the C development team.

### **Task 1: Implement Binary `.cobj` Loader (C-Side)**

*   **Problem**: The engine's current loader (`asset_loader_mesh.c`) is designed to parse text files and is now incompatible with the new binary `.cobj` assets.
*   **Guidance**:
    1.  **Define Header Struct**: Ensure the `COBJHeader` struct (as defined in `R15_Binary_Asset_Format.md`) is present in a C header (e.g., `src/asset_loader/asset_loader_mesh.h`).
    2.  **Replace `parse_obj_file`**: Delete the old, slow `parse_obj_file` function. Create a new function `load_cobj_binary(const char* filepath, Mesh* mesh)`.
    3.  **Implement Binary Loading**:
        *   Open the `.cobj` file in binary mode (`"rb"`).
        *   Perform a single `fread` to load the header.
        *   Validate the magic number (`"CGMF"`) and version from the header.
        *   Use the `vertex_count` and `index_count` from the header to `malloc` the exact memory required.
        *   Use two `fread` calls to load the vertex and index data directly into the newly allocated buffers.
    4.  **Store AABB**: Ensure the `aabb_min` and `aabb_max` from the header are stored in the `Mesh` struct for later use by the culling system.
*   **Test**: Create a test case `test_binary_mesh_loading()` that loads a known `.cobj` file and asserts that the vertex/index counts and AABB data in the loaded `Mesh` struct match the header.

### **Task 2: Fix Texture Loading Path (C-Side)**

*   **Problem**: Textures fail to load due to a path construction bug in `src/assets.c`. This is a separate but equally critical bug.
*   **Guidance**:
    1.  **Isolate the Bug**: Examine the `load_texture` function in `src/assets.c`. The path logic is likely flawed.
    2.  **Implement the Fix**: Modify the path construction logic to correctly generate a valid, absolute path to the texture file.
*   **Test**: Create a test case `test_texture_loading_path()` that specifically calls the path generation logic and asserts that the output is correct.

### **Task 3: Code Style and Documentation Pass**

*   **Problem**: The codebase lacks a consistent style and sufficient documentation.
*   **Guidance**:
    1.  **Create `.clang-format`**: Add a `.clang-format` file to the project root (e.g., based on the "Google" style).
    2.  **Format All Code**: Run `clang-format -i src/**/*.c src/**/*.h tests/**/*.c tests/**/*.h`.
    3.  **Document All Public Headers**: Add Doxygen-style comment blocks to every public function, struct, and enum in the `src/` directory.

## 4. Definition of Done

This sprint is **DONE** when:
1.  The C engine can successfully load and render textured meshes from the new binary `.cobj` format without any visual artifacts.
2.  The texture path bug is fixed, and textures are visible on models.
3.  The new tests (`test_binary_mesh_loading`, `test_texture_loading_path`) are implemented and pass.
4.  A `.clang-format` file exists, and all C code has been formatted.
5.  All public headers in `src/` are fully documented.
6.  `make` and `make test` complete successfully without any errors or warnings.