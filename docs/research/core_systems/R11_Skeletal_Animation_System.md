# Research Proposal: R11 - Skeletal Animation System

**Status**: Proposed
**Author**: Gemini
**Date**: June 30, 2025

## 1. Abstract

The CGame engine currently only supports static 3D models, which severely limits its ability to render dynamic and lifelike characters and objects. This research proposal outlines a plan to design and specify a skeletal animation system. The goal is to create a data-oriented, performant, and extensible animation system that integrates with our existing ECS and rendering pipeline.

## 2. Background

Skeletal animation is the standard technique for animating characters and other complex objects in modern 3D games. It involves binding a mesh to a hierarchical "skeleton" of bones, allowing for complex deformations to be driven by a relatively small amount of animation data. A robust animation system must handle data import, animation blending, and the efficient calculation of final bone transformations to be sent to the GPU.

## 3. Research Questions

This research will answer the following critical questions to ensure a well-designed system:

1.  **Data Formats**: What are the most suitable data formats for storing skeletal animation data (e.g., glTF, FBX, or a custom format)? How will this data be processed by our asset pipeline?
2.  **ECS Integration**: What new components are required to support animation? This will likely include an `ArmatureComponent` (for the skeleton), an `AnimationComponent` (for playback control), and a `SkinnedMeshComponent` (to link a mesh to an armature).
3.  **Animation Blending**: How can we support smooth transitions between different animations (e.g., from an "idle" to a "run" cycle)? This research will investigate techniques like linear interpolation and state machines.
4.  **Performance**: What are the most efficient ways to compute the final bone matrices on the CPU and upload them to the GPU? How can we minimize the performance impact of animated models?
5.  **Shader Integration**: What modifications are required to our existing shaders to support skinned meshes? This will involve passing bone transformation matrices and vertex skinning information to the vertex shader.

## 4. Proposed Methodology

1.  **Format Analysis**: Analyze the glTF 2.0 specification and compare it to other common formats to determine its suitability for our engine.
2.  **Architectural Design**: Create a detailed design document that specifies the new animation-related components, the animation system's update logic, and its integration with the renderer.
3.  **Pipeline Specification**: Define the workflow for importing, processing, and loading animated models through our asset pipeline.
4.  **Prototyping**: Develop a small prototype that loads a single animated model and plays back a simple animation, demonstrating the end-to-end workflow.

## 5. Expected Outcomes

The successful completion of this research will deliver:

1.  **A Data Format Decision**: A formal decision on the data format for animated models, with a clear justification.
2.  **A Comprehensive Design Document**: A complete architectural plan for the animation system, including all component and system API definitions.
3.  **A Shader Modification Plan**: A clear outline of the required changes to our existing GLSL and Metal shaders.
4.  **A Test-Driven Development Plan**: A sprint-ready plan with a full suite of tests for the animation system, ensuring a verifiable and correct implementation.

## 6. Priority

**Medium**. While critical for many game types, this feature builds upon the foundational systems (rendering, assets) and is less urgent than core physics and audio.
