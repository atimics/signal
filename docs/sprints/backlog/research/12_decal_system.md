# Sprint 12 Research: Decal System

This document contains the research and best practices for implementing a decal system.

## Research Request

Please investigate the following areas to ensure best practices for decal system implementation:

### Decal Rendering Techniques
1.  **Decal Projection Mathematics**: Detailed explanation of projective decal mapping and OBB intersection.
    *   **Best Practice**: Use an inverse view-projection matrix from the decal's perspective (as if it were a camera) to project the decal onto the world. The decal itself is rendered as a simple cube, and the projection is handled in the shader.
    *   **Resource**: [LearnOpenGL - Deferred Shading](https://learnopengl.com/Advanced-Lighting/Deferred-Shading-Technique) (The G-buffer approach is highly relevant for decals).
2.  **Screen-Space Decals**: Comparison of object-space vs screen-space decal techniques.
    *   **Best Practice**: For performance and simplicity, start with deferred decals that reconstruct world position from the depth buffer. This avoids complex mesh-decal intersection tests.
    *   **Resource**: [Screen Space Decals (Unity Whitepaper)](https://docs.unity3d.com/Manual/decal-shader.html)
3.  **Decal Batching**: Strategies for efficiently rendering multiple decals with minimal draw calls.
    *   **Best Practice**: Use instanced rendering to draw multiple decals of the same type in a single draw call. Pass per-decal data (transform, color, etc.) in an instance buffer.
4.  **Depth Buffer Usage**: Techniques for proper depth testing and avoiding z-fighting.
    *   **Best Practice**: Render decals with depth testing disabled but depth writing enabled. Project the decal's bounding box and use the existing scene depth to clip decal fragments. A small depth bias can help avoid z-fighting.
