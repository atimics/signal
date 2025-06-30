# Sprint 10.5: Critical Mesh System Repair - Test-Driven Plan

**ID**: `sprint_10.5`
**Status**: **ACTIVE**
**Lead**: Gemini (Lead Scientist and Researcher)
**Related Research**: `R01_Resilient_Mesh_Architecture.md`

## 1. Sprint Goal

To repair the core mesh rendering pipeline by implementing a robust, data-driven asset loading system and a modular, testable rendering component. This sprint is considered complete when a compiled mesh asset (`.cobj`) can be reliably loaded from the `assets/meshes/index.json` and rendered correctly in a scene.

## 2. Test-Driven Tasks

This sprint is broken down into a series of tasks, each driven by a specific test case. The C-level implementation is considered complete only when the corresponding test passes.

---

### **Task 1: Implement `index.json`-driven Path Resolution**

*   **Objective**: Replace the fragile, manual path construction in `assets.c` with a robust system that reads the `assets/meshes/index.json` file to locate mesh data.
*   **Test Case**:
    1.  Create a new test function, `test_asset_path_resolution()`.
    2.  In the test, create a mock `AssetRegistry` and point it to a temporary `test_assets` directory.
    3.  Create a dummy `test_assets/meshes/index.json` file containing an entry for a test mesh (e.g., `"test_ship": "props/test_ship/geometry.cobj"`).
    4.  Create the corresponding dummy file at `test_assets/meshes/props/test_ship/geometry.cobj`.
    5.  Implement a new function, `assets_get_mesh_path(registry, "test_ship", char* out_path, size_t out_size)`.
    6.  The test passes if this function correctly returns the full, unambiguous path: `test_assets/meshes/props/test_ship/geometry.cobj`.
    7.  The test **must fail** if the asset name is not in `index.json`.

---

### **Task 2: Harden Mesh Parsing with Dynamic Memory Allocation**

*   **Objective**: Remove the fixed-size static buffers in `parse_obj_file` to prevent stack overflows and crashes when loading meshes of varying complexity.
*   **Test Case**:
    1.  Create a new test function, `test_mesh_parser_stability()`.
    2.  Create two test `.cobj` files:
        *   `small_mesh.cobj` (e.g., 10 vertices).
        *   `large_mesh.cobj` (e.g., 20,000 vertices, which would have crashed the old system).
    3.  Implement the two-pass parsing strategy described in `R01`:
        *   **Pass 1**: Count the number of vertices and indices in the file.
        *   **Pass 2**: Allocate the exact amount of memory required using `malloc`.
        *   **Pass 3**: Rewind the file and populate the allocated buffers.
    4.  The test calls the refactored `parse_obj_file` on both `small_mesh.cobj` and `large_mesh.cobj`.
    5.  The test passes if both meshes are parsed successfully without crashes, and the `vertex_count` and `index_count` in the resulting `Mesh` struct exactly match the counts from the files.
    6.  The test **must** verify that `mesh->vertices` and `mesh->indices` are heap-allocated pointers.

---

### **Task 3: Implement Pre-emptive GPU Resource Validation**

*   **Objective**: Prevent GPU errors and crashes by validating mesh data *before* creating GPU resources.
*   **Test Case**:
    1.  Create a new test function, `test_gpu_resource_validation()`.
    2.  Create a new function `assets_upload_mesh_to_gpu(Mesh* mesh)` that encapsulates the validation and `sg_make_buffer` calls.
    3.  The test calls this function with several scenarios:
        *   A valid, fully populated `Mesh` struct.
        *   A `Mesh` struct with `vertex_count = 0`.
        *   A `Mesh` struct with `vertices = NULL`.
        *   A `Mesh` struct with `index_count = 0`.
        *   A `Mesh` struct with `indices = NULL`.
    4.  The test passes if the function returns `true` only for the valid mesh and `false` for all invalid scenarios.
    5.  The test **must** verify that `sg_make_buffer` is **never called** for the invalid scenarios.

---

### **Task 4: Implement Post-Upload CPU Memory Deallocation**

*   **Objective**: Reduce the engine's memory footprint by freeing CPU-side vertex and index data after it has been successfully uploaded to the GPU.
*   **Test Case**:
    1.  Create a new test function, `test_cpu_memory_deallocation()`.
    2.  Use a valid mesh and call the `assets_upload_mesh_to_gpu` function from Task 3.
    3.  The test passes if, after a successful upload, `mesh->vertices` and `mesh->indices` are both `NULL`.
    4.  The test **must** verify that if the upload fails, the original pointers are **not** freed and remain valid.

---

### **Task 5: Create a Modular Mesh Renderer**

*   **Objective**: Decouple mesh rendering from the main render loop by creating a self-contained `MeshRenderer` module.
*   **Test Case**:
    1.  Create a new test function, `test_modular_renderer_draw_call()`.
    2.  Flesh out the `mesh_renderer_init` function to create its own `sg_pipeline` and `sg_shader`.
    3.  Flesh out the `mesh_renderer_draw` function to perform a complete Sokol draw call (`sg_apply_pipeline`, `sg_apply_bindings`, `sg_draw`).
    4.  The test will:
        *   Initialize the `MeshRenderer`.
        *   Create a dummy `Renderable` component with valid (mocked) GPU buffer handles.
        *   Call `mesh_renderer_draw`.
    5.  This test is more difficult to assert directly. The initial pass will be considered successful if the function can be called without crashing and all Sokol API calls within it are passed valid (mocked) data. A visual inspection showing a rendered mesh will be the final confirmation.

## 3. Definition of Done

This sprint will be considered **DONE** when:
1.  All five test-driven tasks are implemented and their corresponding tests pass.
2.  The `wedge_ship` asset, loaded via `index.json`, is successfully rendered in the main application.
3.  The old, brittle asset loading logic is removed.
4.  A pull request is submitted with the changes, linking to this sprint plan.
