# Research Proposal: R12 - Advanced Lighting and Shadows

**Status**: Proposed
**Author**: Gemini
**Date**: June 30, 2025

## 1. Abstract

The CGame engine's current lighting capabilities are rudimentary. To achieve modern, realistic visuals, we must implement a more advanced lighting model and a robust shadow system. This research proposal outlines a plan to investigate and design a deferred rendering pipeline, integrate Physically-Based Rendering (PBR), and implement dynamic shadow mapping.

## 2. Background

Modern real-time graphics rely on sophisticated lighting and shadowing techniques to create immersive and believable worlds. The `PBR_IMPLEMENTATION_GUIDE.md` provides a starting point, but a more formal investigation is needed to ensure a performant and scalable implementation. Furthermore, dynamic shadows are a critical component of realistic lighting that is currently absent from the engine.

## 3. Research Questions

This research will focus on answering the following questions:

1.  **Rendering Pipeline**: What are the advantages and disadvantages of a deferred rendering pipeline compared to our current forward renderer, especially in the context of many dynamic lights?
2.  **PBR Integration**: What are the practical steps and performance considerations for integrating a metallic-roughness PBR workflow into our engine? This includes defining the required G-Buffer layout and modifying our material system.
3.  **Shadow Mapping**: What is the most suitable shadow mapping technique for our engine? This research will compare standard shadow maps with more advanced techniques like Cascaded Shadow Maps (CSM) for large outdoor scenes and Variance Shadow Maps (VSM) for soft shadows.
4.  **Performance and Optimization**: How can we optimize the performance of a deferred renderer with PBR and shadows? This includes investigating techniques like frustum culling for shadow casters and optimizing G-Buffer access.
5.  **System Design**: How will the lighting and shadow systems be integrated into our ECS? What new components and systems will be required?

## 4. Proposed Methodology

1.  **Comparative Analysis**: Write a detailed analysis comparing forward and deferred rendering pipelines, as well as at least two different shadow mapping techniques.
2.  **Architectural Design**: Create a design document that outlines the proposed deferred rendering pipeline, the G-Buffer layout, the PBR material workflow, and the shadow mapping implementation.
3.  **Shader Development**: Write prototype GLSL/Metal shaders for the G-Buffer pass, the lighting pass (including PBR calculations), and the shadow map generation.
4.  **Proof-of-Concept**: Develop a small prototype that demonstrates a single dynamic light casting a shadow from a simple object in a PBR-lit scene.

## 5. Expected Outcomes

This research will deliver the following:

1.  **A Rendering Pipeline Recommendation**: A formal recommendation on whether to adopt a deferred rendering pipeline, with a clear justification.
2.  **A Complete Design Document**: A detailed architectural plan for the new rendering pipeline, PBR integration, and shadow system.
3.  **Prototype Shaders**: A set of working prototype shaders that can serve as a starting point for the final implementation.
4.  **A Test-Driven Development Plan**: A sprint-ready plan with a full suite of tests for the new rendering features, ensuring a verifiable implementation.

## 6. Priority

**Medium**. While important for visual fidelity, these features are not as foundational as core gameplay systems like physics and audio. They can be implemented once the core engine is more mature.
