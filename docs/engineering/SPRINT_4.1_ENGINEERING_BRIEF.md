# Engineering Brief: Sprint 4.1 - Rendering Refinement

**To:** CGame Development Team
**From:** Gemini
**Date:** June 29, 2025
**Subject:** Best Practices and Pitfalls for Sprint 4.1

## 1. Objective

This document provides a concise technical guide for completing Sprint 4.1. The primary goal is to bridge the gap between the CPU-side asset system and the GPU, enabling the rendering of 3D geometry using `sokol_gfx`. Adhering to these best practices will help avoid common pitfalls and ensure an efficient, robust implementation.

## 2. Core Task: CPU Data to GPU Resources

The main blocker is the lack of `sokol_gfx` resource creation. The `assets.c` module currently loads mesh and texture data into CPU memory, but this data is never sent to the GPU.

### 2.1. Resource Management Best Practices

`sokol_gfx` uses a simple, handle-based system. The lifecycle for any resource (buffers, images, shaders, pipelines) is:

1.  **Describe:** Create a `sg_..._desc` struct on the stack (e.g., `sg_buffer_desc`).
2.  **Populate:** Fill in the description. For buffers and images, this includes a pointer to the raw data on the CPU.
3.  **Make:** Call the corresponding `sg_make_*()` function (e.g., `sg_make_buffer()`). This function copies the data to the GPU and returns a 32-bit resource ID.
4.  **Store ID:** Store this ID, not the pointer to the CPU data. This ID is your handle to the GPU resource.
5.  **Bind & Draw:** Use the ID in the `sg_bindings` struct during the render loop.

**Key Pitfall:** Do not assume `sokol_gfx` takes ownership of your CPU-side data pointers. It only *inspects* the data during the `sg_make_*` call. You are responsible for freeing the CPU-side memory once the GPU resource has been created.

### 2.2. Implementing the `Renderable` Component

The plan to create a `Renderable` component is sound. This component should store the `sokol_gfx` resource IDs.

```c
// In a relevant header, e.g., core.h or a new render_component.h
typedef struct {
    sg_buffer vbuf; // Vertex buffer ID
    sg_buffer ibuf; // Index buffer ID
    sg_image tex;   // Texture ID
} Renderable;
```

The asset loading process in `assets.c` should be modified to populate these `Renderable` components.

```c
// In assets.c, during mesh loading for an entity...
Entity* entity = get_entity(world, entity_id);
if (entity) {
    // 1. Load mesh data from file into CPU buffers (e.g., vertices, indices)
    // ...

    // 2. Create Sokol buffer for vertices
    sg_buffer_desc vbuf_desc = {
        .data = SG_RANGE(vertices),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
    };
    entity->renderable->vbuf = sg_make_buffer(&vbuf_desc);

    // 3. Create Sokol buffer for indices
    sg_buffer_desc ibuf_desc = {
        .data = SG_RANGE(indices),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
    };
    entity->renderable->ibuf = sg_make_buffer(&ibuf_desc);

    // 4. Free the CPU-side vertex and index buffers, as they are now on the GPU
    // ...
}
```

## 3. Texture Loading with `stb_image`

The project already includes `stb_image.h`, which simplifies texture loading.

**Best Practice:**

1.  Define `STB_IMAGE_IMPLEMENTATION` in **one** C file (e.g., `assets.c`) before including the header.
2.  Use `stbi_load()` to get pixel data and dimensions.
3.  Create an `sg_image` resource from this data.

```c
// In assets.c
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ... inside a texture loading function
int width, height, num_channels;
unsigned char* pixels = stbi_load("path/to/texture.png", &width, &height, &num_channels, 4); // Force 4 channels (RGBA)

if (pixels) {
    sg_image_desc img_desc = {
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = pixels,
            .size = (size_t)(width * height * 4),
        }
    };
    sg_image texture_id = sg_make_image(&img_desc);

    // Free the CPU-side pixel data
    stbi_image_free(pixels);

    // Store texture_id in the relevant material/renderable component
    // ...
}
```

**Pitfall:** Forgetting to free the data returned by `stbi_load()` will cause memory leaks.

## 4. Structuring the Render Loop

The render loop in `main.c` should be structured for clarity and performance.

**Recommended Structure:**

```c
// In frame() callback
// 1. Update game logic, physics, AI, etc.
// ...

// 2. Begin the render pass
sg_pass_action pass_action = {
    .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.1f, 0.1f, 0.2f, 1.0f } }
};
sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

// 3. Apply the pipeline
sg_apply_pipeline(state.pip); // Assuming 'state.pip' holds your main pipeline ID

// 4. Iterate through renderable entities
for (uint32_t i = 0; i < world->entity_count; i++) {
    Entity* entity = &world->entities[i];
    if (entity_has_component(entity, COMPONENT_RENDERABLE) && entity_has_component(entity, COMPONENT_TRANSFORM)) {
        
        // 4a. Apply bindings (VBO, IBO, textures)
        sg_bindings binds = {
            .vertex_buffers[0] = entity->renderable->vbuf,
            .index_buffer = entity->renderable->ibuf,
            .fs_images[0] = entity->renderable->tex
        };
        sg_apply_bindings(&binds);

        // 4b. Apply uniforms (MVP matrix, material properties)
        // Calculate MVP matrix from transform component
        // ...
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(vs_params));

        // 4c. Draw
        sg_draw(0, num_indices_for_this_mesh, 1);
    }
}

// 5. Render UI
ui_render(world); // Nuklear rendering

// 6. End the pass and commit
sg_end_pass();
sg_commit();
```

**Pitfall:** Do not create Sokol resources inside the main render loop. All `sg_make_*` calls should happen during initialization or level loading. The render loop should only contain `sg_apply_*` and `sg_draw` calls.

By following this guidance, Sprint 4.1 should proceed smoothly, successfully restoring the visual output and unblocking future development.
