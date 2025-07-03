# Research: Sprint 21 - PBR Rendering Pipeline Implementation

**ID**: `RES_SPRINT_21`
**Status**: **PROPOSED**
**Author**: Gemini, Chief Science Officer
**Related Sprints**: `sprint_21_pbr_pipeline.md`
**Related Documents**: `Graphics_and_Asset_Architecture.md`, `ASSET_PIPELINE.md`

## 1. Overview

This document provides the foundational research for implementing a modern, end-to-end Physically Based Rendering (PBR) pipeline in the CGame engine. PBR is crucial for achieving realistic visuals and aligning with industry standards. This research will cover the necessary architectural changes, shader implementations, and asset pipeline considerations.

## 2. Core Concepts from Graphics and Asset Architecture

As defined in `Graphics_and_Asset_Architecture.md`:

### Material and Rendering System: PBR

*   **Philosophy**: Our material system is data-driven and based on the metallic-roughness workflow.
*   **Core Properties**: Albedo/BaseColor, Metallic, Roughness, Normal Map, Ambient Occlusion (AO).
*   **Asset Pipeline**: The `asset_compiler.py` is responsible for generating these texture maps and material files from source assets.

### GPU Resource Management

*   **Validation First**: Strict, validation-first approach to creating GPU resources.
*   **Memory Optimization**: CPU-side copy of mesh data is freed after successful upload to GPU.

## 3. Research Focus Areas

### 3.1. PBR Theory and Implementation

*   **Objective**: To understand the mathematical foundations of PBR and its practical implementation.
*   **Considerations**:
    *   **Cook-Torrance BRDF**: Implementation of the Cook-Torrance Bidirectional Reflectance Distribution Function, including its components (Normal Distribution Function, Geometry Function, Fresnel).
    *   **Energy Conservation**: Ensuring that the PBR shader adheres to energy conservation principles for realistic lighting.
    *   **Metallic-Roughness Workflow**: Understanding how metallic and roughness values influence the material's appearance and how they are represented in textures.

### 3.2. Shader Development for PBR

*   **Objective**: To design and implement the vertex and fragment shaders that will perform PBR calculations.
*   **Considerations**:
    *   **Uniforms**: How to pass PBR material properties, camera data, and light information to the GPU efficiently using Uniform Buffer Objects (UBOs).
    *   **Texture Sampling**: How to sample multiple textures (Albedo, Normal, Metallic-Roughness, AO, Emissive) and combine their values in the shader.
    *   **Normal Mapping**: Implementing tangent space calculations in the vertex shader and applying normal maps in the fragment shader.

### 3.3. Lighting System Integration

*   **Objective**: To integrate PBR with a robust lighting system that supports multiple light types and realistic shadows.
*   **Considerations**:
    *   **Light Types**: Supporting directional, point, and spot lights.
    *   **Image-Based Lighting (IBL)**: Using environment maps (cubemaps) for realistic ambient and specular lighting.
    *   **Shadow Mapping**: Implementing shadow mapping techniques (e.g., Cascaded Shadow Maps for directional lights) to add realistic shadows.

## 4. Technical Considerations

*   **Asset Pipeline**: The asset pipeline will need to be extended to generate the full suite of PBR textures (Normal, Metallic-Roughness, AO) from source assets.
*   **GPU Resources**: Proper management of `sg_image` and `sg_buffer` resources for textures and UBOs.
*   **Performance**: PBR shaders are computationally more expensive. Optimization strategies like pre-computed lookup tables (LUTs) and efficient texture sampling will be crucial.

## 5. Next Steps

This research will directly inform the implementation tasks for Sprint 21. The focus will be on creating the necessary C-side data structures, updating the rendering pipeline, and developing the PBR shaders.
