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

### Decal-Surface Interaction
1.  **Surface Normal Integration**: How to properly blend decal normals with underlying surface normals.
    *   **Best Practice**: In a deferred rendering pipeline, decals can write to the G-buffer's normal target, effectively replacing the underlying surface normal. This is the most robust method.
    *   **Resource**: [Blending in Detail (Stephen Hill)](https://blog.selfshadow.com/publications/blending-in-detail/)
2.  **UV Coordinate Generation**: Algorithms for generating proper UV coordinates for projected decals.
    *   **Best Practice**: The decal's projection matrix transforms world-space positions into clip space. In the shader, this clip-space position is converted into UV coordinates for sampling the decal texture.
3.  **Edge Fading**: Techniques for smoothly fading decal edges to avoid harsh boundaries.
    *   **Best Practice**: Use the decal's projection depth and the scene's depth buffer to calculate a fade factor near the edges of the decal's bounding box.

### Performance Optimization
1.  **Frustum Culling**: Efficient culling of decals outside the view frustum.
    *   **Best Practice**: Represent each decal as a bounding sphere or box and perform a standard frustum-cull test on the CPU before rendering.
2.  **Spatial Partitioning**: Data structures for fast decal-mesh intersection queries.
    *   **Best Practice**: If not using a pure deferred approach, use an octree or BVH to quickly find which meshes a decal's bounding box intersects with.
3.  **LOD Systems**: Level-of-detail techniques for decals based on distance and size.
    *   **Best Practice**: Fade out or disable decals based on their distance from the camera or their screen-space size.

### ECS Integration
1.  **Decal Components**: Best practices for integrating decals into an ECS architecture.
    *   **Best Practice**: Create a `Decal` component that stores the decal's properties (type, texture, lifetime). A separate `DecalSystem` can then manage the rendering of all entities with this component.
2.  **Event-Driven Decals**: Techniques for spawning decals in response to game events.
    *   **Best Practice**: Use an event system where game events (e.g., `BulletImpactEvent`) trigger the creation of a decal entity at the impact location.
