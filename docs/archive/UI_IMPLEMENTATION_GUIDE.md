# UI Implementation Guide: Dear ImGui and Sokol

This document provides a detailed plan for executing Sprint 05: UI Integration with Dear ImGui. It covers the integration of Dear ImGui, cimgui, and `sokol_imgui.h` into our existing Sokol-based engine.

## 1. Project Setup and Integration

The first step is to add the necessary libraries to our project and configure the build system.

### 1.1. Add Libraries

1.  **Dear ImGui:** Clone or download the Dear ImGui repository from [github.com/ocornut/imgui](https://github.com/ocornut/imgui). Copy the following files into a new `lib/imgui` directory in our project:
    *   `imgui.h`, `imgui.cpp`
    *   `imgui_widgets.cpp`
    *   `imgui_draw.cpp`
    *   `imgui_internal.h`
    *   `imconfig.h`
    *   `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`

2.  **cimgui:** Clone or download the cimgui repository from [github.com/cimgui/cimgui](https://github.com/cimgui/cimgui). Copy the `cimgui.h` and `cimgui.cpp` files into a new `lib/cimgui` directory.

3.  **sokol_imgui:** Download the `sokol_imgui.h` header from the [Sokol GitHub repository](https://github.com/floooh/sokol) and place it in the `src/` directory.

### 1.2. Update Makefile

The `Makefile` needs to be updated to compile the C++ files from Dear ImGui and cimgui and link them with our C code.

**Key changes:**

1.  **C++ Compiler:** We need to use a C++ compiler (like `g++`) to compile the ImGui source files and link everything together.

2.  **Source Files:** Add the ImGui and cimgui `.cpp` files to the list of source files in the `Makefile`.

    ```makefile
    # Add these to your Makefile
    CXX = g++
    SRCS_CPP = lib/imgui/imgui.cpp lib/imgui/imgui_widgets.cpp lib/imgui/imgui_draw.cpp lib/cimgui/cimgui.cpp
    OBJS_CPP = $(SRCS_CPP:.cpp=.o)

    # Update your main build rule to include the C++ objects
    $(TARGET): $(OBJS_C) $(OBJS_CPP)
        $(CXX) $(OBJS_C) $(OBJS_CPP) -o $(TARGET) $(LDFLAGS)
    ```

## 2. UI Module (`ui.c` and `ui.h`)

We will create a new module to encapsulate all UI-related code.

### 2.1. `ui.h`

This header will define the public API for our UI module.

```c
// ui.h
#pragma once

#include "core.h"

void ui_init(void);
void ui_shutdown(void);
void ui_handle_event(const sapp_event* event);
void ui_render(const struct World* world);
```

### 2.2. `ui.c`

This file will contain the implementation of our UI system.

**Key implementation details:**

1.  **Includes and Implementation Defines:**

    ```c
    // ui.c
    #define SOKOL_IMGUI_IMPL
    #include "sokol_imgui.h"
    #include "cimgui.h"
    #include "ui.h"
    ```

2.  **`ui_init()`:** This function will initialize `sokol_imgui`.

    ```c
    void ui_init(void) {
        simgui_setup(&(simgui_desc_t){0});
    }
    ```

3.  **`ui_shutdown()`:** This function will shut down `sokol_imgui`.

    ```c
    void ui_shutdown(void) {
        simgui_shutdown();
    }
    ```

4.  **`ui_handle_event()`:** This function will pass input events to ImGui.

    ```c
    void ui_handle_event(const sapp_event* event) {
        simgui_handle_event(event);
    }
    ```

5.  **`ui_render()`:** This function will be called every frame to build and render the UI.

    ```c
    void ui_render(const struct World* world) {
        simgui_new_frame(&(simgui_frame_desc_t){
            .width = sapp_width(),
            .height = sapp_height(),
            .delta_time = sapp_frame_duration(),
            .dpi_scale = sapp_dpi_scale(),
        });

        // --- Build your UI here ---
        // Example: igText("Hello, world!");

        // Render the UI
        simgui_render();
    }
    ```

## 3. Integration with the Main Loop

We need to call our new UI functions from the main application callbacks in `systems.c`.

**`systems.c` changes:**

*   In `init()`: Call `ui_init()`.
*   In `cleanup()`: Call `ui_shutdown()`.
*   In `event()`: Call `ui_handle_event(e)`.
*   In `frame()`: Call `ui_render(world)` after rendering the 3D scene but before `sg_commit()`.

## 4. HUD and Debug Panel Implementation

Inside `ui_render()`, we will use the cimgui API to create our UI elements.

### 4.1. Basic HUD

```c
// In ui_render()
igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Always, (ImVec2){0, 0});
igSetNextWindowBgAlpha(0.3f);
if (igBegin("HUD", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
    // Add text for player position, velocity, etc.
    igText("Position: %.1f, %.1f, %.1f", world->player.position.x, ...);
    igEnd();
}
```

### 4.2. Debug Panel

```c
// In ui_render()
static bool show_debug_panel = false;
if (igButton("Toggle Debug Panel", (ImVec2){0, 0})) {
    show_debug_panel = !show_debug_panel;
}

if (show_debug_panel) {
    igBegin("Debug Panel", &show_debug_panel, 0);
    igText("FPS: %.1f", 1.0f / sapp_frame_duration());
    igText("Entities: %d", world->entity_count);
    // Add more debug info and controls
    igEnd();
}
```

## 5. Action Plan

1.  **Week 1: Library Integration**
    *   [ ] Add Dear ImGui, cimgui, and `sokol_imgui.h` to the project.
    *   [ ] Update the `Makefile` to compile and link the new libraries.
    *   [ ] Create the `ui.c` and `ui.h` files.
    *   [ ] Get a basic "Hello, world!" ImGui window to render.

2.  **Week 2: HUD Implementation**
    *   [ ] Design the layout for the HUD.
    *   [ ] Implement the HUD using ImGui windows and text widgets.
    *   [ ] Pass the necessary data from the `World` struct to the `ui_render` function.

3.  **Week 3: Debug Panel**
    *   [ ] Design the layout and features of the debug panel.
    *   [ ] Implement the panel with a toggle button.
    *   [ ] Add real-time statistics and controls to the panel.

4.  **Week 4: Refactoring and Cleanup**
    *   [ ] Ensure the UI code is well-organized and documented.
    *   [ ] Test the UI on different platforms if possible.
    *   [ ] Review and merge the changes.
