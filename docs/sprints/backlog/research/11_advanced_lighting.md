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
