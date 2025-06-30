# Sprint 14: Mesh Generator Pipeline Integration

**ID**: `sprint_14.0`
**Status**: **ACTIVE & CRITICAL**
**Author**: Gemini, Chief Science Officer

## 1. Sprint Goal

To fully integrate the procedural mesh generator (`tools/mesh_generator`) with the modern, binary-first asset pipeline. This involves refactoring the generator to produce standard, textured `.obj` files and updating the main build pipeline to seamlessly compile these generated assets into the final `.cobj` format.

## 2. Problem Statement

The `mesh_generator` is a legacy system that operates outside of our new asset pipeline. It produces outdated, untextured text files and contains redundant, direct-call compilation logic. This creates a separate, unmanaged workflow for procedural assets, leading to the current situation where they are not rendered correctly. This sprint will unify the two systems.

## 3. Implementation Plan

### Task 1: Refactor the Mesh Generator

*   **Objective**: Modify the `mesh_generator` to be a pure "source asset" creator. It should not be responsible for compilation.
*   **Guidance**:
    1.  **Update `generate_...` functions**: Modify `generate_wedge_ship_mk2`, `generate_control_tower`, etc., to return `vertices`, `faces`, and `uvs`. The `generate_control_tower` function already does this, but the others need to be updated.
    2.  **Rewrite `write_obj_file`**: This is the most critical step. The function must be rewritten to correctly write all vertex attributes, including texture coordinates (`vt`) and normals (`vn`), into the `.obj` file. It must also write the face indices in the `v/vt/vn` format.
    3.  **Remove `compile_mesh_asset` call**: Delete the direct call to the old compiler from within `generate_mesh_with_textures`. The generator's only job is to produce the source `.obj`, `.mtl`, and `metadata.json` files in the `assets/meshes/props` directory.

### Task 2: Update the Main Build Pipeline

*   **Objective**: Ensure the main `build_pipeline.py` can discover and compile the newly generated source assets.
*   **Guidance**:
    1.  **No Changes Needed (Verification)**: Our current `build_pipeline.py` is already designed to find all `metadata.json` files and compile the associated `geometry.obj`. By refactoring the mesh generator to produce these source files in the correct location, the main pipeline should work without modification.
    2.  **Create a `generate` command**: Add a new command-line argument to `build_pipeline.py`, such as `python3 tools/build_pipeline.py --generate-all`, which will first run the mesh generator script to ensure all source assets exist, and then proceed with the normal compilation process.

### Task 3: Create a Standalone Generator Script

*   **Objective**: To make the mesh generator easier to use for developers and designers.
*   **Guidance**:
    1.  Create a new top-level script, `tools/generate_assets.py`.
    2.  This script will be simple: it will import and call the `main` function from `tools/mesh_generator/__init__.py`.
    3.  Update the `Makefile` with a new target: `make generate-assets`. This will provide a clear, one-step command for regenerating all procedural assets.

## 4. Definition of Done

1.  The `mesh_generator` script no longer performs any compilation.
2.  The `write_obj_file` function correctly writes vertices, UVs, and normals to the `.obj` file.
3.  Running `make generate-assets` successfully creates/overwrites the source `.obj` files in the `assets/meshes/props` directory.
4.  Running `make assets` (or `python3 tools/build_pipeline.py`) subsequently compiles these generated `.obj` files into textured, binary `.cobj` files.
5.  When the engine is run, the procedurally generated meshes (like the control tower) are rendered correctly, with textures.
