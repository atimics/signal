# Sprint 11: PBR Rendering Pipeline Implementation

**Date**: June 30, 2025  
**Priority**: High  
**Status**: **BLOCKED**  
**JIRA**: `CG-180`

## 1. Sprint Goal

To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline. This involves upgrading the material system, implementing advanced shaders, enhancing the lighting system, and ensuring the asset pipeline can produce and feed PBR-compatible assets to the engine.

## 2. Blockers & Prerequisites

This sprint is critically dependent on the successful completion of **Sprint 10.5**. The engine currently cannot render any mesh geometry, making it impossible to validate PBR materials or lighting.

*   **CRITICAL DEPENDENCY**: [**Sprint 10.5: Critical Mesh System Architecture Repair**](../active/10_5_mesh_system_repair.md)

## 3. Core Implementation Phases

This is a large, multi-week effort that should be broken down into the following logical phases.

### **Phase 1: Foundational Refactor (Week 1)**

This phase focuses on implementing the foundational components required for any PBR work.

*   **Task 1.1: Restore and Modernize `render_mesh.c`**
    *   **Goal**: Create a dedicated, modular system for rendering meshes that is separate from the main `render_3d.c` loop.
    *   **Reference**: [Code Review & Implementation Guide](./10_5_mesh_system_repair_code_review.md)

*   **Task 1.2: Expand Shader Uniform Architecture**
    *   **Goal**: Design and implement a new uniform buffer object (UBO) layout that can accommodate PBR material properties (metallic, roughness, etc.) and data for multiple light sources.
    *   **Analysis**: The current uniform system is inadequate. See [Deep Implementation Analysis](./11_pbr_implementation_analysis.md).

*   **Task 1.3: Implement Multi-Texture Binding**
    *   **Goal**: Upgrade the rendering pipeline to allow binding multiple textures (Albedo, Normal, Metallic-Roughness, AO) to a single material for a single draw call.
    *   **Reference**: Sokol GFX supports this, but the engine's abstraction layer does not.

### **Phase 2: Core PBR Implementation (Week 2)**

With the foundation in place, this phase implements the core visual components of PBR.

*   **Task 2.1: Implement Cook-Torrance BRDF Shader**
    *   **Goal**: Write a new GLSL/Metal shader that implements the Cook-Torrance Bidirectional Reflectance Distribution Function (BRDF), which is the standard for PBR.
    *   **Components**: Normal Distribution Function (NDF), Geometry Function (G), and Fresnel equations.

*   **Task 2.2: Integrate PBR Material Properties**
    *   **Goal**: Connect the `Material` struct's PBR properties (`metallic`, `roughness`) to the new PBR shader via the uniform buffer.
    *   **File**: `src/assets.h`

*   **Task 2.3: Implement Normal Mapping**
    *   **Goal**: Add support for normal maps to the PBR shader, including tangent and bitangent calculations in the vertex shader to correctly orient the normals.

### **Phase 3: Advanced Lighting & Effects (Week 3)**

This phase builds on the core PBR system to add advanced lighting and post-processing effects.

*   **Task 3.1: GPU-Based Multi-Light System**
    *   **Goal**: Move all lighting calculations to the GPU. The CPU should only be responsible for passing an array of light data (position, color, intensity) to the shader.

*   **Task 3.2: HDR Rendering and Tone Mapping**
    *   **Goal**: Implement a High Dynamic Range (HDR) rendering pipeline by rendering the scene to a floating-point framebuffer. Then, apply a tone mapping operator (e.g., ACES) to map the HDR colors to a displayable range.

*   **Task 3.3: Foundational Shadow Mapping**
    *   **Goal**: Implement basic shadow mapping for a single directional light. This involves a separate depth-pass render and modifying the PBR shader to sample the shadow map.

## 4. Relevant Research & Documentation

*   **Architectural Analysis**: [Sprint 11: PBR and Rendering Standardization - Deep Implementation Analysis](./11_pbr_implementation_analysis.md)
*   **Original Sprint Plan**: [Sprint 10: PBR Materials & Advanced Lighting](./10_pbr_materials.md)
*   **External Guide**: [LearnOpenGL - PBR Theory](https://learnopengl.com/PBR/Theory)

This consolidated sprint provides a clear, phased roadmap for a complex but critical engine feature. Each phase builds upon the last, reducing risk and providing measurable progress.
