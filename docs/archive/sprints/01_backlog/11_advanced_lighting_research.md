# Sprint 11 Research: Advanced Lighting & Shadow Mapping

This document contains the research and best practices for implementing advanced lighting techniques, including shadow mapping, point lights, spot lights, and environmental lighting.

## Research Request

Please investigate the following areas to ensure best practices for advanced lighting implementation with Sokol:

### Shadow Mapping Techniques
1.  **Shadow Map Theory**: Comprehensive coverage of shadow mapping algorithms, including cascaded shadow maps.
    *   **Best Practice**: For directional lights in large, open scenes, Cascaded Shadow Maps (CSM) are the industry standard. For point lights, use cube map shadow mapping.
    *   **Resource**: [LearnOpenGL - Shadow Mapping](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping)
2.  **Shadow Artifacts**: Common shadow artifacts and their solutions (shadow acne, peter panning, light bleeding).
    *   **Best Practice**: Use a combination of a small, constant depth bias and a slope-scaled depth bias to mitigate most shadow acne. Peter panning can be reduced by using back-face culling for the shadow pass.
    *   **Resource**: [Common Techniques to Improve Shadow Depth Maps](https://docs.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps)
3.  **PCF and Advanced Filtering**: Implementation details for Percentage Closer Filtering and other soft shadow techniques.
    *   **Best Practice**: Implement a simple 2x2 Percentage-Closer Filtering (PCF) kernel as a baseline for soft shadows. This provides a good balance between quality and performance.
    *   **Resource**: [Percentage-Closer Filtering (NVIDIA Developer)](https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing)
4.  **Cascade Split Optimization**: Best practices for calculating optimal cascade split distances.
    *   **Best Practice**: Use a logarithmic or a combined linear-logarithmic (practical) split scheme for the cascades. This distributes the shadow map resolution more effectively, with more detail closer to the camera.
    *   **Resource**: [Cascaded Shadow Maps (Sascha Willems)](https://saschawillems.de/blog/2016/03/02/cascaded-shadow-mapping/)

### Light Types and Attenuation
1.  **Point Light Mathematics**: Proper attenuation curves and light falloff calculations.
    *   **Best Practice**: Use a physically-based quadratic attenuation formula: `Attenuation = 1.0 / (distance^2)`. To avoid infinite intensity at close distances, use a small epsilon or a range-based formula.
    *   **Resource**: [LearnOpenGL - Light Casters](https://learnopengl.com/Advanced-Lighting/Light-casters) (covers attenuation)
2.  **Spot Light Implementation**: Cone angle calculations and smooth falloff techniques.
    *   **Best Practice**: Use the dot product between the light's forward vector and the vector to the fragment to determine if the fragment is within the light's cone. Use `smoothstep` to create a soft penumbra between the inner and outer cone angles.
3.  **Area Light Approximation**: Techniques for approximating area lights with point/spot lights.
    *   **Best Practice**: For real-time rendering without ray tracing, area lights are often approximated using multiple point lights or by using a texture to define the light's shape and intensity.
4.  **Light Culling**: Efficient techniques for culling lights outside the view frustum.
    *   **Best Practice**: Represent lights as bounding spheres and perform a sphere-frustum intersection test to cull lights that are not visible. This should be done on the CPU before sending light data to the GPU.

### Performance Optimization
1.  **Light Batching**: Strategies for batching multiple lights in a single render pass.
    *   **Best Practice**: Use a tiled or forward+ rendering pipeline to process multiple lights in a single pass. This involves dividing the screen into tiles and creating a per-tile list of lights.
    *   **Resource**: [Tiled and Clustered Forward Shading (Intel)](https://www.intel.com/content/www/us/en/developer/articles/technical/tiled-and-clustered-forward-shading.html)
2.  **Shadow Map Caching**: Techniques for avoiding unnecessary shadow map updates.
    *   **Best Practice**: Only update the shadow map for a light if the light has moved or if objects within its frustum have moved. This is especially important for static lights.
3.  **LOD for Shadows**: Level-of-detail systems for shadow quality based on distance.
    *   **Best Practice**: Use lower resolution shadow maps or disable shadows completely for lights that are far from the camera.

### Sokol Integration
1.  **Render Target Management**: Best practices for managing shadow map render targets in Sokol.
    *   **Best Practice**: Create a `sg_pass` for each shadow map, with a depth-only attachment. Use `sg_make_image` to create the depth texture and `sg_make_pass` to create the render target.
    *   **Resource**: [sokol-samples/offscreen.c](https://github.com/floooh/sokol-samples/blob/master/sapp/offscreen-sapp.c)
2.  **Multi-pass Rendering**: Efficient organization of shadow and lighting passes.
    *   **Best Practice**: First, render all shadow-casting lights to their respective depth maps (the shadow pass). Then, render the main scene, binding the shadow maps as textures and performing the shadow calculations in the fragment shader (the lighting pass).
3.  **Uniform Buffer Organization**: Optimal layout for light data in uniform buffers.
    *   **Best Practice**: Create a single, large uniform buffer containing an array of light structs. Use this buffer to pass all light data to the shaders in a single `sg_apply_uniforms` call.
