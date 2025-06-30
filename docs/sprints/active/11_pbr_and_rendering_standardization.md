# Sprint 11: PBR and Rendering Standardization

## Goal
Standardize the rendering pipeline by implementing a full Physically-Based Rendering (PBR) workflow, from asset compilation to in-engine rendering. This will resolve the visual discrepancy between the source and compiled assets in the mesh viewer and enable a more advanced and realistic look for the game.

## Key Initiatives

### 1. Asset Pipeline Refactor (Non-Destructive & Artist-Friendly)
- **UV Unwrapping:** Replace the current "spritesheet" UV generation with a proper UV unwrapping process in the asset compiler. This will allow for the use of standard texture maps and enable artists to create more detailed and realistic textures.
- **Non-Destructive Workflow:** Ensure the pipeline treats source assets as immutable. It should read from the source, process the data, and write to a separate build directory. It should never modify the source assets.
- **Artist-Friendly Tooling:** Prioritize artist-created data. If an artist has provided a texture, material, or UV map, the pipeline should use it. Procedural generation should only be a fallback for missing data.

### 2. PBR Material System
- **PBR Fundamentals:** Implement a PBR material system following the metallic-roughness workflow.
- **Material Properties:** Support albedo, normal, metallic, roughness, and ambient occlusion (AO) maps.
- **Texture Management:** Use a single `sg_bindings` struct to bind all PBR textures in a single call for efficiency.
- **Uniform Buffer Layout:** Pack material constants and lighting information into a single uniform block.

### 3. Advanced Lighting & Shadow Mapping
- **Light Types:** Implement point lights and spot lights with physically-based quadratic attenuation.
- **Shadow Mapping:** Implement cascaded shadow maps (CSM) for directional lights and cube map shadow mapping for point lights.
- **Shadow Quality:** Use a combination of a constant depth bias and a slope-scaled depth bias to mitigate shadow acne. Implement Percentage-Closer Filtering (PCF) for soft shadows.

### 4. Rendering Pipeline Refactor
- **Re-enable `render_mesh.c`:** Refactor the `render_mesh.c` file to implement a PBR rendering pipeline using the `sokol_gfx` library.
- **Modularity:** The `render_system` module should expose a minimal API, and the internal state (pipelines, shaders) should be kept private.
- **Shader Optimization:** Use pre-calculated lookup tables (LUTs) for the BRDF integration to improve performance.

### 5. Decal System
- **Decal Projection:** Implement a projective decal system using an inverse view-projection matrix from the decal's perspective.
- **Decal Rendering:** Use deferred decals that reconstruct world position from the depth buffer for performance and simplicity.
- **Decal Batching:** Use instanced rendering to draw multiple decals of the same type in a single draw call.

## Expected Outcome
- A fully functional PBR rendering pipeline.
- Consistent rendering between the mesh viewer and the game engine.
- A more powerful and flexible asset workflow that enables the creation of high-quality, realistic assets.
- A modular and efficient rendering system.
- A flexible and performant decal system.