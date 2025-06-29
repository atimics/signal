# Sokol API Guide

This guide provides a comprehensive overview of the Sokol APIs used in this project. It is intended to be a living document that is updated as the project evolves.

## Introduction

Sokol is a collection of minimal, cross-platform, standalone C headers. It's designed to be simple and efficient, with a focus on WebAssembly as a primary platform.

## Core Components

The following Sokol headers are used in this project:

*   **`sokol_app.h`**: A cross-platform application wrapper that handles window creation, input, and 3D context initialization.
*   **`sokol_gfx.h`**: A 3D API wrapper that works with OpenGL, Metal, Direct3D 11, and WebGPU.
*   **`sokol_glue.h`**: Provides glue helper functions between `sokol_gfx.h` and `sokol_app.h`.
*   **`sokol_log.h`**: A common logging callback for other Sokol headers.

### `sokol_app.h`

`sokol_app.h` provides a minimalistic cross-platform API which handles window creation, input, and 3D context initialization.

#### Key Functions

*   **`sapp_desc sokol_main(int argc, char* argv[])`**: The entry point of the application. This function returns a `sapp_desc` structure that describes the application's properties.
*   **`sapp_run(const sapp_desc* desc)`**: Runs the application's main loop.
*   **`sapp_width()`** and **`sapp_height()`**: Return the current width and height of the framebuffer.
*   **`sapp_event`**: A structure that describes an event, such as a key press or mouse movement.

### `sokol_gfx.h`

`sokol_gfx.h` is a 3D API wrapper that works with OpenGL, Metal, Direct3D 11, and WebGPU.

#### Key Functions

*   **`sg_setup(const sg_desc* desc)`**: Sets up the graphics context.
*   **`sg_shutdown(void)`**: Shuts down the graphics context.
*   **`sg_begin_default_pass(const sg_pass_action* pass_action, int width, int height)`**: Begins a render pass to the default framebuffer.
*   **`sg_apply_pipeline(sg_pipeline pip)`**: Applies a pipeline object.
*   **`sg_apply_bindings(const sg_bindings* bindings)`**: Applies resource bindings.
*   **`sg_draw(int base_element, int num_elements, int num_instances)`**: Draws a range of elements.
*   **`sg_end_pass(void)`**: Ends a render pass.
*   **`sg_commit(void)`**: Commits the frame.

### `sokol_glue.h`

`sokol_glue.h` provides glue helper functions between `sokol_gfx.h` and `sokol_app.h`.

#### Key Functions

*   **`sglue_get_context(void)`**: Returns a `sg_context_desc` structure that can be used to initialize `sokol_gfx`.

### `sokol_log.h`

`sokol_log.h` provides a common logging callback for other Sokol headers.

#### Key Functions

*   **`slog_func_t`**: A function pointer type for the logging callback.
*   **`slog_set_func(slog_func_t func, void* user_data)`**: Sets the logging callback function.

## Key Concepts

### Main Loop

The main loop is the heart of the application. It is responsible for handling events, updating the application state, and rendering the scene.

### Rendering Pipeline

The rendering pipeline is a sequence of steps that are performed to render a frame. The steps are:

1.  Begin a render pass.
2.  Apply a pipeline object.
3.  Apply resource bindings.
4.  Draw the scene.
5.  End the render pass.
6.  Commit the frame.

### Resource Management

Sokol provides a set of functions for creating and managing resources, such as buffers, images, and shaders.

## Getting Started

The following example shows how to initialize Sokol and render a basic shape:

```c
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

static void init(void) {
    sg_setup(&(sg_desc){
        .context = sglue_get_context(),
        .logger.func = slog_func,
    });
}

static void frame(void) {
    sg_begin_default_pass(&(sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } }
    }, sapp_width(), sapp_height());
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .window_title = "Sokol Example",
        .logger.func = slog_func,
    };
}
```
