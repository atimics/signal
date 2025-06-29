# Sprint 04: Transition to Sokol API

**Goal:** Replace the low-level, manual SDL 2D rendering backend with the modern, cross-platform Sokol graphics API. This will simplify the rendering code, improve performance, and make the engine more portable and easier to maintain.

**Period:** July 2025

## Key Objectives:

1.  **Integration of Sokol:**
    *   [ ] Add the `sokol_gfx.h`, `sokol_app.h`, and `sokol_glue.h` headers to the project's `src/` directory.
    *   [ ] Update the `Makefile` to correctly compile the Sokol implementation files and link against the necessary platform-specific graphics libraries (e.g., OpenGL, Metal).

2.  **Replace SDL with `sokol_app`:**
    *   [ ] Remove the existing SDL window and event handling loop from `render_3d.c`.
    *   [ ] Replace it with the `sokol_app` entry point (`sapp_main`).
    *   [ ] Re-implement the main loop using Sokol's frame callback mechanism.
    *   [ ] Map Sokol input events to the existing input handling system.

3.  **Adapt Rendering to `sokol_gfx`:**
    *   [ ] Refactor `render_init` to initialize `sokol_gfx` instead of the SDL renderer.
    *   [ ] Convert the existing mesh loading process to create `sg_buffer` objects for vertex and index data.
    *   [ ] Convert the texture loading process to create `sg_image` objects.
    *   [ ] Write a basic shader in GLSL for textured 3D rendering and use Sokol's shader tools to compile it for different backends if necessary.
    *   [ ] Create an `sg_pipeline` object that configures the render state (depth testing, culling, etc.) and the shader to use.
    *   [ ] Replace the `render_mesh` function with a new implementation that uses `sg_draw_` commands within a `sg_pass_action`.

4.  **Maintain Functionality:**
    *   [ ] Ensure that all existing 3D models are rendered correctly with textures and basic lighting.
    *   [ ] The camera system should continue to function as before.
    *   [ ] The UI system will likely need to be re-evaluated, as it is currently tied to the SDL renderer. A temporary solution or a new UI approach using a Sokol-compatible library (like `sokol_imgui`) will be investigated.

## Expected Outcomes:

*   The project will be completely decoupled from SDL for rendering and window management.
*   The rendering code will be significantly cleaner and more modern, using a consistent API for all platforms.
*   The engine will be able to target multiple graphics backends (OpenGL, Metal, etc.) with minimal code changes.
*   The foundation will be laid for more advanced rendering techniques, such as shadow mapping, post-processing effects, and compute shaders, which are much easier to implement with Sokol than with the previous manual pipeline.

## Pre-computation/Pre-analysis:

*   The current rendering backend is a manual 3D pipeline built on top of the SDL 2D rendering API. This is complex, inefficient, and not easily extensible.
*   Sokol provides a high-level, modern graphics API that abstracts away the complexities of the underlying platform-specific APIs.
*   The transition will require significant refactoring of the `render_3d.c` and `render.h` files.
*   The asset loading code in `assets.c` will need to be updated to create Sokol-compatible resources.
*   A decision will need to be made about the future of the UI system.
