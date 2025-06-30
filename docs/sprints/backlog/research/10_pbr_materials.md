# Sprint 10 Research: PBR Materials & Advanced Lighting

This document contains the research and best practices for implementing a Physically Based Rendering (PBR) material system with advanced lighting.

## Research Request

Please investigate the following areas to ensure best practices for PBR material implementation with Sokol:

### PBR Theory and Implementation
1.  **PBR Fundamentals**: Comprehensive overview of PBR theory, including the metallic-roughness workflow.
    *   **Best Practice**: Adhere to the metallic-roughness workflow as it is the industry standard and more intuitive for artists. Ensure that metallic surfaces have no diffuse color (black) and that their specular color is defined by the albedo map.
    *   **Resource**: [Basic Theory of Physically Based Rendering (Marmoset)](https://marmoset.co/posts/basic-theory-of-physically-based-rendering/)
2.  **Shader Mathematics**: Detailed explanation of BRDF calculations, Fresnel equations, and proper PBR lighting formulas.
    *   **Best Practice**: Implement the Cook-Torrance BRDF model. Use the Schlick approximation for the Fresnel term and the GGX distribution (Trowbridge-Reitz) for the normal distribution function (NDF).
    *   **Resource**: [LearnOpenGL - PBR Theory](https://learnopengl.com/PBR/Theory)
3.  **Material Properties**: Typical value ranges and best practices for albedo, metallic, roughness, and normal map usage.
    *   **Best Practice**: Albedo values for non-metals should be in the range of 30-240 sRGB. For raw metals, albedo should be 160-255 sRGB. Avoid pure black or pure white values.
    *   **Resource**: [Physically Based Rendering and You Can Too! (Joe Wilson)](https://www.joewilson.com/blog/2018/04/01/physically-based-rendering-and-you-can-too/)

### Sokol-Specific Implementation
1.  **Texture Management**: Optimal strategies for managing multiple textures per material in Sokol.
    *   **Best Practice**: Use a single `sg_bindings` struct to bind all PBR textures (albedo, normal, metallic, roughness, AO) in a single call. This is more efficient than multiple binding calls.
    *   **Resource**: [sokol_gfx.h API Documentation](https://github.com/floooh/sokol/blob/master/sokol_gfx.h#L1034)
2.  **Uniform Buffer Layout**: Best practices for organizing PBR material properties in uniform buffers.
    *   **Best Practice**: Pack material constants (base color, metallic/roughness factors) and lighting information into a single uniform block to be passed to the fragment shader. This minimizes the number of uniform updates.
3.  **Shader Optimization**: Performance optimization techniques for PBR shaders on different GPU architectures.
    *   **Best Practice**: Use pre-calculated lookup tables (LUTs) for the BRDF integration, which can be generated once and reused. This avoids expensive calculations in the fragment shader.
    *   **Resource**: [Real-Time PBR with IBL (Epic Games)](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)
