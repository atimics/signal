# Sprint 15: Research - Automated Testing Framework Integration

**Date**: June 30, 2025  
**Priority**: High (Foundation for future stability and CI/CD)  
**Status**: Backlog  
**JIRA**: `CG-210`

## 1. Research Goal

To investigate, evaluate, and recommend a comprehensive automated testing strategy for the CGame engine. The primary objective is to establish a framework that supports unit testing for core logic, integration testing for systems, and a pathway towards automated validation of the rendering pipeline. This research is critical for improving code quality, reducing regressions, and enabling a robust continuous integration (CI) pipeline.

## 2. Problem Statement

The CGame engine currently lacks any form of automated testing. All validation is performed manually by compiling the engine and visually inspecting the output. This approach is slow, error-prone, and not scalable. As the engine's complexity grows with features like PBR and advanced AI, the risk of introducing breaking changes increases exponentially.

Without an automated testing safety net, developers cannot refactor with confidence, and the CI/CD pipeline is limited to build verification rather than true quality assurance.

## 3. Key Research Areas

This research project will focus on the following areas:

### **Area 3.1: C Unit Testing Frameworks**

*   **Objective**: Identify and compare suitable unit testing frameworks for C.
*   **Candidates**:
    *   **Google Test (`gtest`)**: A widely-used C++ framework that can be used with C code. Known for its rich feature set (assertions, fixtures, mocking).
    *   **Unity**: A lightweight and portable unit testing framework specifically for C, designed for embedded systems but highly effective for general use.
    *   **CMocka**: A unit testing framework for C with a focus on mocking. It is particularly useful for testing functions with complex dependencies.
    *   **CTest**: A testing tool distributed with CMake. It's more of a test runner than a framework itself but can be used to orchestrate tests from various frameworks.
*   **Evaluation Criteria**:
    *   Ease of integration with the current `Makefile` build system.
    *   Clarity and simplicity of test syntax.
    *   Support for test fixtures (setup/teardown).
    *   Mocking capabilities (for isolating code under test).
    *   Community support and documentation.

### **Area 3.2: Integration Testing Strategy**

*   **Objective**: Define a strategy for testing the interaction between different engine systems (e.g., ECS, Physics, Asset Loading).
*   **Topics**:
    *   How to initialize a "headless" version of the engine for testing purposes (i.e., without a graphical window).
    *   Techniques for loading test-specific scenes and data.
    *   Writing tests that span multiple systems, for example:
        *   Does applying a force in the `physics_system` correctly update the `Transform` component?
        *   Does `assets_load_all_in_directory` correctly populate the `AssetRegistry` and can entities be created from those assets?

### **Area 3.3: Rendering and Graphics Testing**

*   **Objective**: Investigate methods for automated validation of the rendering output. This is notoriously difficult but crucial for a game engine.
*   **Techniques to Explore**:
    *   **Screenshot Comparison (Image Diffing)**:
        1.  Render a reference scene and save a "golden" screenshot.
        2.  On subsequent test runs, render the same scene and programmatically compare the new screenshot to the golden one.
        3.  Research tools like ImageMagick or custom pixel-based comparison algorithms.
    *   **GPU State Validation**:
        *   Can we query the state of GPU resources (buffers, textures) after a render pass to ensure they are valid and have the expected properties? (The `sg_query_*_state` functions are a good starting point).
    *   **Headless Rendering**:
        *   Investigate the feasibility of running Sokol GFX in a headless environment (e.g., using a software renderer like Mesa/LLVMpipe on a CI server) to perform rendering tests without a physical GPU.

### **Area 3.4: Mocking External Dependencies**

*   **Objective**: Propose a strategy for mocking the Sokol GFX library.
*   **Problem**: Unit tests for modules like `render_mesh.c` should not depend on a live GPU. To test the logic within these modules, we need to replace calls to `sg_make_buffer`, `sg_apply_pipeline`, etc., with mock functions that simulate success or failure.
*   **Approach**:
    *   Using preprocessor macros to redirect `sg_*` calls to mock implementations during test builds.
    *   Creating a lightweight "mock Sokol" header that provides fake implementations of the necessary functions.

## 4. Deliverables

The output of this research sprint will be a comprehensive engineering report (`PBR_IMPLEMENTATION_GUIDE.md` style) that includes:

1.  **Framework Recommendation**: A clear recommendation for a specific C unit testing framework, with a justification based on the evaluation criteria.
2.  **Implementation Roadmap**: A step-by-step plan for integrating the chosen framework into the CGame engine.
    *   Phase 1: Basic unit testing for a core, non-rendering module (e.g., `src/core.c` math functions).
    *   Phase 2: Integration tests for the asset pipeline.
    *   Phase 3: A proof-of-concept for rendering tests (e.g., a test that loads a mesh, "renders" it with a mocked backend, and validates the state).
3.  **CI/CD Integration Plan**: A proposal for how to add a "test" stage to the existing `.github/workflows/build.yml` file.
4.  **Example Test Suite**: A small, well-documented example test suite for a single C file to serve as a template for future test development.

This foundational research will pave the way for a more stable, reliable, and professional development workflow.
