# Sprint 11: PBR Implementation Guide - Phase 1

**ID**: `guide_sprint_11`
**Status**: **ACTIVE**
**Author**: Gemini (Lead Scientist and Researcher)
**Related Plan**: `sprints/backlog/11_pbr_rendering_pipeline.md`

## 1. Overview

This guide provides the detailed technical steps for completing **Phase 1** of the PBR sprint. The objective of this phase is to lay the architectural groundwork for the full PBR pipeline by refactoring our shader data pipeline. We will not be writing the final PBR shader yet; instead, we will focus on creating the data pathways to get the necessary information to the GPU.

---

## 2. Task 1.1: Uniform Buffer Object (UBO) for Materials

*   **Objective**: To create a dedicated UBO for passing PBR material properties to the GPU.

### Step 2.1: Define the UBO Struct

In a new header file, `src/render_pbr_ubos.h`, define the C-side structs that will map directly to the uniform blocks in our future shaders. This centralizes all shader data structures.

```c
// src/render_pbr_ubos.h
#ifndef RENDER_PBR_UBOS_H
#define RENDER_PBR_UBOS_H

#include "core.h"

// Uniforms for the Vertex Shader
typedef struct {
    float model[16];
    float view[16];
    float projection[16];
    float normal_matrix[9]; // 3x3 matrix
} PbrVertexUniforms;

// Uniforms for the Fragment Shader
typedef struct {
    Vector3 camera_pos;
    float _pad1; // Ensure alignment
    Vector3 albedo;
    float metallic;
    float roughness;
    float ao;
    float _pad2[2]; // Ensure alignment
} PbrFragmentUniforms;

#endif // RENDER_PBR_UBOS_H
```

### Step 2.2: Update the `MeshRenderer`

The `MeshRenderer` will now be responsible for managing these UBOs. Update `render_mesh.c` to create and manage these buffers.

**In `render_mesh.c` (inside `MeshRendererGpuResources`):**
```c
struct MeshRendererGpuResources {
    sg_pipeline pipeline;
    sg_shader shader;
    sg_buffer vs_ubo; // New
    sg_buffer fs_ubo; // New
};
```

**In `mesh_renderer_init()`:**
```c
// In mesh_renderer_init, after creating the PIMPL struct...
renderer->gpu_resources->vs_ubo = sg_make_buffer(&(sg_buffer_desc){
    .size = sizeof(PbrVertexUniforms),
    .type = SG_BUFFERTYPE_UNIFORM,
    .usage = SG_USAGE_DYNAMIC,
    .label = "pbr-vs-ubo"
});

renderer->gpu_resources->fs_ubo = sg_make_buffer(&(sg_buffer_desc){
    .size = sizeof(PbrFragmentUniforms),
    .type = SG_BUFFERTYPE_UNIFORM,
    .usage = SG_USAGE_DYNAMIC,
    .label = "pbr-fs-ubo"
});
```

### Step 2.3: Create the Test

In `tests/test_rendering.c`, create a new test `test_material_ubo_update`. This test will:
1.  Create a `Material` struct and populate it with test PBR values (e.g., `metallic = 0.8`, `roughness = 0.2`).
2.  Create a new function `pbr_update_material_ubo(const Material* material)` which populates a `PbrFragmentUniforms` struct and updates the `fs_ubo` buffer using `sg_update_buffer`.
3.  This test is difficult to assert directly without reading back from the GPU. For now, the test will pass if the function can be called without errors. The true validation will come from visual inspection later.

---

## 3. Task 1.2: Multi-Texture Binding System

*   **Objective**: To enable the binding of multiple PBR textures for a single draw call.

### Step 3.1: Define PBR Texture Slots

In a new header, `src/render_pbr_textures.h`, define an enum for the PBR texture slots. This avoids "magic numbers" in the rendering code.

```c
// src/render_pbr_textures.h
#ifndef RENDER_PBR_TEXTURES_H
#define RENDER_PBR_TEXTURES_H

typedef enum {
    PBR_TEXTURE_ALBEDO,
    PBR_TEXTURE_NORMAL,
    PBR_TEXTURE_METALLIC_ROUGHNESS,
    PBR_TEXTURE_AMBIENT_OCCLUSION,
    PBR_TEXTURE_EMISSIVE,
    PBR_TEXTURE_COUNT
} PbrTextureType;

#endif // RENDER_PBR_TEXTURES_H
```

### Step 3.2: Update the `Renderable` Component

The `Renderable`'s `GpuResources` will now hold an array of texture handles instead of a single one.

**In `gpu_resources.c` (inside `GpuResources` struct):**
```c
#include "render_pbr_textures.h" // Include the new header

struct GpuResources {
    sg_buffer vbuf;
    sg_buffer ibuf;
    sg_image textures[PBR_TEXTURE_COUNT]; // Array of textures
};
```
*You will need to update the `gpu_resources_...` functions to handle this array.*

### Step 3.3: Update the Draw Call

Modify `mesh_renderer_draw` to apply all the textures in the `Renderable`'s `gpu_resources`.

**In `mesh_renderer_draw()`:**
```c
// ... inside mesh_renderer_draw ...

sg_bindings bindings = {
    .vertex_buffers[0] = ...
    .index_buffer = ...
};

// Bind all PBR textures
for (int i = 0; i < PBR_TEXTURE_COUNT; ++i) {
    bindings.fs.images[i] = renderable->gpu_resources->textures[i];
}

sg_apply_bindings(&bindings);

// ... apply uniforms and draw ...
```

### Step 3.4: Create the Test

In `tests/test_rendering.c`, create a new test `test_multi_texture_binding`. This test will:
1.  Create a `Renderable` and assign several mock `sg_image` handles to its `textures` array.
2.  Use a mocked version of `sg_apply_bindings` to capture the `sg_bindings` struct that was passed.
3.  Assert that the `images` array in the captured struct contains the correct mock handles at the correct indices.

This completes the foundational work. After this phase, we will have a robust data pipeline ready for the actual PBR shader implementation in Phase 2.
