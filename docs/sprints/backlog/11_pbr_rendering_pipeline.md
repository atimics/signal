# Sprint 11: PBR Rendering Pipeline Implementation

**Date**: June 30, 2025  
**Priority**: High  
**Status**: **BLOCKED** (by Sprint 15)
**JIRA**: `CG-180`

## 1. Sprint Goal

To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline. This involves upgrading the material system, implementing advanced shaders, enhancing the lighting system, and ensuring the asset pipeline can produce and feed PBR-compatible assets to the engine.

## 2. Blockers & Prerequisites

This sprint is critically dependent on the successful completion of **Sprint 15**, which will provide the testing framework necessary to de-risk this complex refactoring effort.

*   **CRITICAL DEPENDENCY**: [**Sprint 15: Automated Testing Framework Integration**](../active/15_automated_testing_integration.md)

## 3. Test-Driven Implementation Phases

This sprint will be executed in a series of test-driven phases. Each task will have a corresponding set of unit or integration tests that must pass for the task to be considered complete.

---

### **Phase 1: Foundational Shader Architecture**

*   **Objective**: To refactor the shader and material system to support the data requirements of PBR.

*   **Task 1.1: Uniform Buffer Object (UBO) for Materials**
    *   **Description**: Create a dedicated UBO to pass material properties (albedo, metallic, roughness, AO) to the GPU.
    *   **Test**: A unit test will create a `Material` struct, populate it with PBR data, and call a new function `material_system_update_ubo(material)`. The test will then use `sg_query_buffer_info` to verify that the UBO on the GPU contains the correct byte-for-byte data.

*   **Task 1.2: Multi-Texture Binding System**
    *   **Description**: Implement a system in `render_mesh.c` that can bind multiple textures (Albedo, Normal, Metallic-Roughness) for a single draw call.
    *   **Test**: An integration test will create a material with multiple texture slots, assign mock `sg_image` handles to them, and call `mesh_renderer_draw`. The test will use a mocked version of `sg_apply_bindings` to assert that the correct texture handles were bound to the correct shader slots.

---

### **Phase 2: PBR Shader Implementation**

*   **Objective**: To implement the core Cook-Torrance BRDF shader and integrate it with the material system.

*   **Task 2.1: Cook-Torrance BRDF Shader**
    *   **Description**: Write a new GLSL/Metal shader that implements the Cook-Torrance BRDF, including NDF, Geometry, and Fresnel components.
    *   **Test**: This is difficult to unit test. The "test" will be a visual one: rendering a sphere with the new shader and comparing it to a reference image from a tool like Marmoset Toolbag.

*   **Task 2.2: Normal Mapping Integration**
    *   **Description**: Add support for normal maps to the PBR shader, including the calculation of the TBN matrix in the vertex shader.
    *   **Test**: A visual test showing a sphere or plane with a normal map applied, correctly interacting with a moving light source.

---

### **Phase 3: Advanced Lighting**

*   **Objective**: To upgrade the lighting system to support a modern, GPU-driven workflow.

*   **Task 3.1: GPU-Based Lighting Array**
    *   **Description**: Move all lighting calculations to the GPU by passing an array of light data (position, color, intensity, type) in a UBO.
    *   **Test**: An integration test will create multiple `Light` structs, update the lighting UBO, and then use `sg_query_buffer_info` to verify that the GPU buffer contains the correct data for all lights.

*   **Task 3.2: HDR Rendering & Tone Mapping**
    *   **Description**: Implement an HDR rendering pipeline using a floating-point framebuffer and an ACES tone mapping post-processing pass.
    *   **Test**: A visual test comparing a scene with and without HDR/tone mapping. The HDR version should show more detail in both bright and dark areas.

---

## 4. Relevant Research & Documentation

*   **Architectural Analysis**: [Sprint 11: PBR and Rendering Standardization - Deep Implementation Analysis](./11_pbr_implementation_analysis.md)
*   **External Guide**: [LearnOpenGL - PBR Theory](https://learnopengl.com/PBR/Theory)
