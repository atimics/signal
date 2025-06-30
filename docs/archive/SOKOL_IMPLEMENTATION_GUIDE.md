# Sokol Transition: Implementation Guide

This document provides a detailed plan for executing Sprint 04: Transition to Sokol API. Its purpose is to guide the development process, ensuring a smooth and efficient migration from the current SDL-based rendering backend to the Sokol graphics API.

**This guide has been updated to reflect the new component-based camera system and the refactored rendering pipeline.**

## 1. Project Setup and Integration

The first step is to integrate the Sokol headers into our existing project structure and build system.

### 1.1. Add Sokol Headers

Download the following headers from the [Sokol GitHub repository](https://github.com/floooh/sokol) and place them in the `src/` directory:

*   `sokol_app.h`: For windowing and application lifecycle management.
*   `sokol_gfx.h`: For 3D graphics rendering.
*   `sokol_glue.h`: For integrating the various Sokol parts.
*   `sokol_log.h`: For logging.

### 1.2. Update Makefile

The `Makefile` needs to be updated to handle the Sokol implementation and link against the correct platform-specific libraries.

**Key changes:**

1.  **Define Sokol Implementation:** In one of our `.c` files (e.g., `render_3d.c`), we need to define `SOKOL_IMPL` before including the Sokol headers. This will generate the implementation code.

    ```c
    #define SOKOL_IMPL
    #include "sokol_gfx.h"
    #include "sokol_app.h"
    #include "sokol_log.h"
    #include "sokol_glue.h"
    ```

2.  **Platform-Specific Linker Flags:** We need to add platform-specific linker flags to the `Makefile`.

    ```makefile
    # Add this to your Makefile
    ifeq ($(OS),Darwin)
        LDFLAGS += -framework Metal -framework AppKit
    else
        LDFLAGS += -lGL -lX11 -lm
    endif
    ```

## 2. Application Entry Point and Main Loop

We will replace the SDL-based main loop with Sokol's event-driven approach.

### 2.1. Create `sokol_main`

The current `main` function in `systems.c` will be replaced by a `sokol_main` function. This function will define the application's callbacks and initial configuration.

**Example `sokol_main` in `systems.c`:**

```c
// Forward declarations for our callback functions
void init(void);
void frame(void);
void cleanup(void);
void event(const sapp_event* e);

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1280,
        .height = 720,
        .window_title = "CGGame - Sokol",
        .logger.func = slog_func,
    };
}
```

### 2.2. Implement Callbacks

*   **`init()`**: This function will contain the one-time setup code, including initializing `sokol_gfx`, loading assets, and setting up the game world.
*   **`frame()`**: This function will be called every frame. It will contain the game logic update and rendering calls.
*   **`cleanup()`**: This function will be called when the application exits. It should clean up all resources, including `sokol_gfx`.
*   **`event()`**: This function will handle user input. We will need to map Sokol's event types to our existing input handling logic.

## 3. Rendering with `sokol_gfx`

This is the most significant part of the transition. We will replace all SDL rendering calls with their `sokol_gfx` equivalents.

### 3.1. Initialization

In the `init()` callback, we will initialize `sokol_gfx`:

```c
// In init()
sg_setup(&(sg_desc){
    .context = sapp_sgcontext(),
    .logger.func = slog_func,
});
```

### 3.2. Resource Creation

We need to modify our asset loading functions to create `sokol_gfx` resources.

*   **Vertex and Index Buffers:** The `load_mesh_from_file` function in `assets.c` should be updated to create `sg_buffer` objects.

    ```c
    // Example of creating a vertex buffer
    sg_buffer_desc buffer_desc = {
        .data = SG_RANGE(mesh->vertices),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
    };
    mesh->sg_vertex_buffer = sg_make_buffer(&buffer_desc);
    ```

*   **Images/Textures:** The texture loading code should be updated to create `sg_image` objects for each texture in the material (diffuse, normal, specular, etc.).

    ```c
    // Example of creating an image
    sg_image_desc img_desc = {
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = { .ptr = pixels, .size = width * height * 4 },
    };
    texture->sg_image = sg_make_image(&img_desc);
    ```

### 3.3. Shaders and Pipelines

We will need to create a PBR-like shader and a pipeline object to define how our meshes are rendered.

*   **Shader:** We'll create a GLSL shader that supports the new material properties. This shader will take the MVP matrix, as well as material properties like roughness and metallic, as uniforms. It will also need to sample from multiple textures.

    ```c
    // In render_mesh.c
    sg_shader_desc shader_desc = {
        // ... GLSL source code for a PBR-like vertex and fragment shader ...
    };
    sg_shader shd = sg_make_shader(&shader_desc);
    ```

*   **Pipeline:** The pipeline object will configure the render state for our PBR shader.

    ```c
    // In render_mesh.c
    sg_pipeline_desc pip_desc = {
        .shader = shd,
        .layout = { ... }, // Define vertex layout
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .cull_mode = SG_CULLMODE_BACK,
    };
    sg_pipeline pip = sg_make_pipeline(&pip_desc);
    ```

### 3.4. The Render Loop

The `render_frame` function will be rewritten to use the `sokol_gfx` drawing commands.

**New `render_frame` structure:**

```c
void render_frame(void) {
    // 1. Update all game systems (physics, AI, camera, etc.)

    // 2. Get the view and projection matrices from the active camera
    mat4 view_proj = camera_get_view_proj_matrix(world, world_get_active_camera(world));

    // 3. Begin render pass
    sg_pass_action pass_action = {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } }
    };
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

    // 4. Apply pipeline
    sg_apply_pipeline(pip);

    // 5. Loop through entities and draw them
    for (/* each renderable entity */) {
        sg_bindings bind = {
            .vertex_buffers[0] = entity->mesh->sg_vertex_buffer,
            .index_buffer = entity->mesh->sg_index_buffer,
            .fs_images[0] = entity->material->diffuse_texture->sg_image,
            .fs_images[1] = entity->material->normal_texture->sg_image,
            // ... bind other textures
        };
        sg_apply_bindings(&bind);

        // Calculate the MVP matrix and apply it as a uniform
        mat4 model = entity_get_model_matrix(entity);
        mat4 mvp = mat4_mul(view_proj, model);
        
        // Pack all uniforms into a struct and apply them
        vs_params_t vs_params = { .mvp = mvp };
        fs_params_t fs_params = { .roughness = entity->material->roughness, .metallic = entity->material->metallic };
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(vs_params));
        sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &SG_RANGE(fs_params));

        sg_draw(0, entity->mesh->index_count, 1);
    }

    // 6. End render pass and commit
    sg_end_pass();
    sg_commit();
}
```

## 4. UI System

The existing UI system is tightly coupled with the SDL renderer. We have two options:

1.  **Temporary Solution:** For this sprint, we can disable the UI or use a very basic text-based debug overlay.
2.  **New UI Library:** We can investigate and integrate a Sokol-compatible UI library, such as `sokol_imgui.h`. This would be a good goal for a future sprint.

For Sprint 4, we will aim for the temporary solution to keep the scope manageable.

## 5. Updated Action Plan

1.  **Week 1: Integration and Basic Window**
    *   [ ] Add Sokol headers to the project.
    *   [ ] Update the `Makefile`.
    *   [ ] Implement the `sokol_main` entry point and callbacks.
    *   [ ] Get a blank window to open and close successfully.

2.  **Week 2: PBR Shader and Pipeline**
    *   [ ] Write a basic PBR-like shader that supports diffuse and normal maps, and roughness/metallic properties.
    *   [ ] Create the pipeline object for the PBR shader.
    *   [ ] Update the asset loading code to create `sg_buffer` and `sg_image` resources for the new material system.
    *   [ ] Render a single, hard-coded mesh to the screen with the new PBR shader.

3.  **Week 3: Full Scene Rendering**
    *   [ ] Implement a `camera_get_view_proj_matrix` function that uses the active camera's component data.
    *   [ ] Generalize the rendering loop to iterate through all entities, calculating and applying the MVP matrix and material uniforms for each one.
    *   [ ] Ensure all textures are correctly bound and sampled in the shader.

4.  **Week 4: Cleanup and UI**
    *   [ ] Remove all remaining SDL code.
    *   [ ] Implement a temporary debug text solution for the UI.
    *   [ ] Clean up and document the new rendering code.
    *   [ ] Review and merge the changes.
