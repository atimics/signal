# Sprint 04: Transition to Sokol API

**Goal:** Complete the transition from the legacy SDL backend to the modern, cross-platform Sokol graphics API. This involves replacing all windowing and input handling with `sokol_app` and implementing the new PBR-like rendering pipeline on top of `sokol_gfx`.

**Period:** July 2025

## Key Objectives:

1.  **Integration of Sokol:**
    *   [ ] Add the `sokol_gfx.h`, `sokol_app.h`, and `sokol_glue.h` headers to the project's `src/` directory.
    *   [ ] Update the `Makefile` to correctly compile the Sokol implementation files and link against the necessary platform-specific graphics libraries (e.g., OpenGL, Metal).

2.  **Replace SDL with `sokol_app`:**
    *   [ ] Remove all remaining SDL window and event handling code.
    *   [ ] Replace the main loop with the `sokol_app` entry point (`sapp_main`) and its callback mechanism.
    *   [ ] Map Sokol input events to the existing input handling system.

3.  **Implement PBR Rendering on `sokol_gfx`:**
    *   [ ] Refactor `render_init` to initialize `sokol_gfx` instead of the SDL renderer.
    *   [ ] Convert the asset loading process to create `sg_buffer` (for meshes) and `sg_image` (for textures) resources.
    *   [ ] Write a PBR-like shader in GLSL that utilizes the enhanced `Material` properties (diffuse, normal, specular maps; roughness, metallic uniforms).
    *   [ ] Create an `sg_pipeline` object that configures the render state for the PBR shader.
    *   [ ] Replace the existing CPU-based rendering logic with `sg_draw` calls, passing the camera matrices and material properties to the shader as uniforms.

4.  **Ensure Full Functionality:**
    *   [ ] Confirm that all 3D models are rendered correctly using the new PBR pipeline.
    *   [ ] Verify that the component-based camera system functions as expected, providing the correct view/projection matrices to the shader.
    *   [ ] Implement a temporary debug overlay for critical information, as the existing UI is SDL-dependent and will be replaced in a future sprint.

## Expected Outcomes:

*   The project will be completely decoupled from SDL.
*   The rendering code will be significantly cleaner and more modern, with a shader-based PBR pipeline.
*   The engine will be able to target multiple graphics backends (OpenGL, Metal, etc.) with minimal code changes.
*   The foundation will be set for more advanced rendering techniques, such as shadow mapping and post-processing effects.

## Pre-computation/Pre-analysis:

*   The rendering backend has already been significantly refactored into a modular, component-based architecture.
*   The new `CameraComponent` and enhanced `Material` struct provide a clear data model for the new rendering pipeline.
*   This preparatory work has made the transition to Sokol much more straightforward, as it is now primarily a task of adapting the existing architecture to a new graphics API rather than redesigning it from scratch.
