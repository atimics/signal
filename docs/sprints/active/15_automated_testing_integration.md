# Sprint 15: Automated Testing Framework Integration - Plan

**ID**: `sprint_15`
**Status**: **ACTIVE**
**Lead**: Gemini (Lead Scientist and Researcher)
**Related Research**: `R02_Automated_Testing_Frameworks.md`

## 1. Sprint Goal

To integrate the **Unity Test Framework** into the CGame engine, establish a formal testing workflow, and create an initial suite of unit and integration tests to serve as a foundation for all future development. The "Definition of Done" is a new `make test` command that successfully builds and runs all tests.

## 2. Phased Implementation Plan

This sprint will be executed in three phases to ensure a smooth and incremental adoption of the testing framework.

---

### **Phase 1: Framework Integration and Core Math Unit Tests**

*   **Objective**: To download and integrate the Unity framework source code and write the first set of simple unit tests for a core, dependency-free module.
*   **Tasks**:
    1.  **Download Unity**: Create a new directory `tests/vendor` and download the latest release of the Unity framework (`unity.c`, `unity.h`, `unity_internals.h`) into it.
    2.  **Create Test Runner**: Create a new test runner file, `tests/test_main.c`, that will be responsible for orchestrating and running all the test suites.
    3.  **Write First Test Suite**: Create a new test file, `tests/test_core_math.c`, to test the vector math functions in `src/core.c`.
        *   `TEST_ASSERT_EQUAL_FLOAT` for vector addition, subtraction, dot product, etc.
    4.  **Update Makefile**: Create a new `test` target in the `Makefile`. This target will be responsible for:
        *   Compiling the Unity source code (`tests/vendor/unity.c`).
        *   Compiling the test runner (`tests/test_main.c`).
        *   Compiling the test suites (`tests/test_core_math.c`).
        *   Compiling the necessary engine source files (`src/core.c`).
        *   Linking them all into a single test executable (`build/cgame_tests`).
        *   Running the test executable.

---

### **Phase 2: Asset Pipeline Integration Test**

*   **Objective**: To build upon the framework by writing a more complex integration test for the asset loading system, validating the fixes from Sprint 10.5.
*   **Tasks**:
    1.  **Create Asset Test Suite**: Create a new test file, `tests/test_assets.c`.
    2.  **Implement `setUp` and `tearDown`**: Use Unity's fixture support.
        *   `setUp()`: Will create the mock `test_assets` directory and the `index.json` file, just as we did manually in the last sprint.
        *   `tearDown()`: Will delete the temporary test directory and its contents.
    3.  **Write Integration Test**: Re-implement the `test_asset_path_resolution` test from the previous sprint using the formal Unity `TEST_ASSERT_EQUAL_STRING` assertion. This will validate that the asset loading logic works as expected within the new test harness.

---

### **Phase 3: Mesh Rendering Regression Test (Proof of Concept)**

*   **Objective**: To create a foundational regression test for the entire mesh loading and rendering pipeline, ensuring the fixes from Sprint 10.5 are permanently locked in.
*   **Tasks**:
    1.  **Create Rendering Test Suite**: Create a new test file, `tests/test_rendering.c`.
    2.  **Headless Rendering Context**: The test will use the `SOKOL_DUMMY_BACKEND` to initialize a headless Sokol context, allowing us to create real GPU resources without needing a window.
    3.  **Write Regression Test**: The test will perform the following steps:
        *   Initialize the asset system.
        *   Load the `wedge_ship` mesh using the `index.json` file.
        *   Call the `assets_upload_mesh_to_gpu` function.
        *   Initialize the `MeshRenderer`.
        *   Call `mesh_renderer_draw`.
    4.  **Assertion**: The test will pass if all steps complete without errors and the final `sg_pipeline` and `sg_buffer` handles in the `MeshRenderer` and `Mesh` structs are valid (`!= SG_INVALID_ID`). This provides a strong guarantee that the entire pipeline is connected correctly.

## 3. Definition of Done

This sprint is **DONE** when the `make test` command executes without errors and successfully runs all tests from all three phases, confirming that the Unity testing framework is fully integrated and operational.
