# CGame Rendering Pipeline: Implementation Guide

**Last Updated:** June 29, 2025

This document provides a comprehensive, step-by-step guide for implementing the CGame rendering pipeline, from the foundational 3D camera to advanced lighting and decal systems. It synthesizes the goals and technical details of Sprints 07 through 12.

## 1. Foundational Layer: 3D Camera System (Sprint 07)

The first step is to establish a proper 3D perspective. This involves creating a robust camera system and the necessary matrix math to transform 3D world coordinates into 2D screen space.

### 1.1. Matrix Math

-   **`mat4_perspective`**: Creates a perspective projection matrix. This is what gives the scene its sense of depth.
-   **`mat4_lookat`**: Creates a view matrix that positions and orients the "camera" in the world. It's defined by an `eye` position, a `target` to look at, and an `up` vector.
-   **`mat4_multiply`**: Combines matrices. The final view-projection matrix is calculated as `projection * view`.

### 1.2. Camera Component

The `Camera` component in `core.h` should be enhanced to store its 3D properties and cache the resulting matrices.

```c
// In core.h
struct Camera {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    
    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;
    
    // Cached matrices
    float view_matrix[16];
    float projection_matrix[16];
    float view_projection_matrix[16];
    bool matrices_dirty; // Flag to trigger recalculation
};
```

### 1.3. Camera System Logic

The `camera_system_update` in `systems.c` should:
1.  Check if the active camera's `matrices_dirty` flag is true.
2.  If so, recalculate the `view_matrix` and `projection_matrix` using the functions from step 1.1.
3.  Combine them into the `view_projection_matrix`.
4.  Reset the `matrices_dirty` flag to false.

## 2. Mesh Rendering & Transforms (Sprint 08)

With a camera in place, we can now render 3D meshes.

### 2.1. Transform Matrices

In addition to the camera matrices, each object needs a **model matrix** to position, rotate, and scale it in the world.

-   Implement `mat4_translation`, `mat4_rotation_y`, and `mat4_scale`.
-   Create a `mat4_compose_transform` function that combines these into a single model matrix from an entity's `Transform` component.

### 2.2. The `render_entity_3d` Function

This function in `render_3d.c` is the core of entity rendering. For each entity, it must:
1.  Get the entity's `Transform` and `Renderable` components.
2.  Calculate the entity's `model_matrix` using `mat4_compose_transform`.
3.  Get the active camera's `view_projection_matrix`.
4.  Calculate the final `mvp` (Model-View-Projection) matrix: `mvp = camera.view_projection_matrix * model_matrix`.
5.  Bind the entity's mesh buffers (`sg_buffer` handles from the `Renderable` component).
6.  Pass the `mvp` matrix to the vertex shader as a uniform.
7.  Issue the `sg_draw` call.

## 3. Texture Mapping (Sprint 09)

This sprint brings the meshes to life with textures.

### 3.1. GPU Texture Loading

-   In `assets.c`, create a `load_texture_to_gpu` function.
-   Use `stbi_load` (from the existing `stb_image.h`) to load image files from disk into a CPU buffer.
-   Use `sg_make_image` to create a `sokol_gfx` texture from the CPU pixel data. **Crucially, free the `stbi_load` buffer after the `sg_image` is created to prevent memory leaks.**
-   Store the returned `sg_image` ID in the `AssetRegistry`.

### 3.2. Shader Updates

-   The vertex shader must be updated to accept UV coordinates (`texcoord`) from the vertex buffer and pass them to the fragment shader.
-   The fragment shader will now take a `sampler2D` uniform and the incoming `texcoord`.
-   Use `texture(diffuse_texture, texcoord)` (GLSL) or `diffuse_texture.sample(sampler, texcoord)` (Metal) to sample the texture color.

### 3.3. Render Pipeline Integration

-   Update the `sg_bindings` struct in `render_entity_3d` to include the `sg_image` ID for the material's diffuse texture.
-   Ensure the render pipeline is configured to expect a texture and sampler.

## 4. PBR Materials (Sprint 10)

This moves from simple texturing to a modern, physically-based rendering model.

### 4.1. PBR Material Definition

-   Extend the `Material` struct in `assets.h` to include PBR properties:
    -   Texture IDs for `albedo`, `metallic`, `roughness`, `normal`, and `ambientOcclusion`.
    -   Float factors for `metallic_factor` and `roughness_factor`.

### 4.2. PBR Shader

-   This is the most complex part. The fragment shader must implement a PBR lighting model. The Cook-Torrance BRDF is the industry standard.
-   The shader will need to sample from all the PBR textures and use the resulting values in its lighting calculations.
-   **Normal Mapping:** The shader must transform the sampled normal from tangent space to world space. This requires the vertex shader to calculate and pass the TBN (Tangent, Bitangent, Normal) matrix to the fragment shader.

## 5. Advanced Lighting & Shadows (Sprint 11)

This sprint builds on the PBR foundation to create dynamic and realistic lighting.

### 5.1. Light Types

-   Define structs for `PointLight` and `SpotLight` in `render_lighting.h`.
-   The main shader will need to be updated with a loop that iterates over active lights, accumulating their contribution to the final pixel color.

### 5.2. Shadow Mapping

This is a multi-pass technique:
1.  **Depth Pass:** For each light that casts shadows, a separate render pass is required.
    -   Bind a depth-only shader.
    -   Render the entire scene from the *light's point of view* into a depth texture (an `sg_image` with a depth format).
2.  **Main Render Pass:**
    -   In the main PBR fragment shader, for each pixel, transform its world position into the light's view space.
    -   Sample the shadow map depth texture.
    -   Compare the pixel's depth with the value from the shadow map. If it's greater, the pixel is in shadow.
    -   **PCF (Percentage-Closer Filtering):** To get soft shadows, sample the shadow map multiple times in a small radius and average the results.

## 6. Decal System (Sprint 12)

Decals add fine-grained detail to surfaces.

### 6.1. Deferred Decal Approach

The most robust method is deferred decals, which requires a G-Buffer. Since the current pipeline is forward-rendering, a simpler approach is needed:

### 6.2. Projected Decals (Forward Rendering)

1.  **Decal Geometry:** A decal is rendered as a 3D cube.
2.  **Decal Shader:**
    -   **Vertex Shader:** The vertex shader transforms the cube's vertices using a special decal projection matrix. This matrix projects the decal onto the scene geometry.
    -   **Fragment Shader:** The fragment shader reconstructs the world position of the underlying surface from the depth buffer. It then checks if this position is inside the decal's projected cube. If it is, it samples the decal texture and blends it with the surface color.
3.  **Blending:** The decal shader will need to read from the scene's depth buffer to correctly project onto surfaces and avoid "floating". This requires careful setup of the render passes.
