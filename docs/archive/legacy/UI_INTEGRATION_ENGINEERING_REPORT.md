# Engineering Report: UI Integration for Sprint 05

**Report Date:** June 29, 2025
**Author:** System Analysis
**Status:** Finalized Guidance

## 1. Executive Summary

This report provides a detailed technical guide for **Sprint 05: UI Integration**. The goal is to implement a robust, real-time user interface system, replacing the currently disabled placeholder UI.

The chosen technology stack is **Dear ImGui** in conjunction with its C-API wrapper, **cimgui**, and the **`sokol_imgui.h`** utility header. This stack is a mature, well-documented, and widely adopted solution within the Sokol ecosystem. It provides the power and flexibility needed for both in-game HUDs and complex debugging tools.

This document outlines the necessary components, provides a step-by-step integration plan, and defines the initial UI features to be implemented. The successful execution of this plan will result in a clean, data-driven, and decoupled UI system, forming a critical foundation for future development.

## 2. Technology Stack Analysis

*   **Dear ImGui:** A bloat-free, immediate-mode graphical user interface library for C++. It is highly performant and offers an extensive set of widgets, making it ideal for game development tooling and debug interfaces.
*   **cimgui:** A C-API wrapper for Dear ImGui. This is essential for our C-based project, as it provides direct, linkable access to the underlying C++ ImGui library without requiring a full C++ toolchain in our main build process.
*   **`sokol_imgui.h`:** A utility header from the Sokol project that acts as a rendering and input backend. It seamlessly bridges Dear ImGui's drawing commands and Sokol's rendering pipeline (`sokol_gfx`) and event system (`sokol_app`), dramatically simplifying integration.

This combination is the de-facto standard for UI development with Sokol and represents the lowest-risk, highest-reward path forward.

## 3. Integration and Build Plan

The integration process requires careful setup of the build system to incorporate the C++ source files of Dear ImGui.

### 3.1. Component Acquisition

1.  **Dear ImGui:** Clone or download the source files from the official repository. They should be placed in a `vendor/imgui` directory.
2.  **cimgui:** Clone or download the source files. These should be placed in `vendor/cimgui`.
3.  **`sokol_imgui.h`:** This header should already be part of the project's Sokol header collection. If not, acquire it from the Sokol repository and place it in `src/`.

### 3.2. Makefile Configuration

The `Makefile` must be updated to compile the necessary C++ files from Dear ImGui and the C files from cimgui.

**Makefile Snippet:**

```makefile
# Source files for UI
IMGUI_DIR = vendor/imgui
CIMGUI_DIR = vendor/cimgui
UI_SRC = $(CIMGUI_DIR)/cimgui.cpp \
         $(IMGUI_DIR)/imgui.cpp \
         $(IMGUI_DIR)/imgui_draw.cpp \
         $(IMGUI_DIR)/imgui_tables.cpp \
         $(IMGUI_DIR)/imgui_widgets.cpp \
         $(IMGUI_DIR)/backends/imgui_impl_sokol.cpp # Or similar if using sokol_imgui

# Update your main source list
SRC += $(UI_SRC)

# Add C++ compiler and flags
CXX = g++
CXXFLAGS = $(CFLAGS) -std=c++11 # Or a newer standard

# Update compilation rule to handle C++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
```
*Note: The exact file paths and compiler flags may need adjustment based on the project's structure.*

## 4. Implementation Guide

The UI implementation should be encapsulated within `src/ui.c` and `src/ui.h`.

### 4.1. `ui.h` - Public Interface

This header will define the public API for the UI system.

```c
#pragma once

// Initialize the UI system
void ui_init(void);

// Shut down the UI system
void ui_shutdown(void);

// Handle input events
// Returns true if ImGui has captured the event, false otherwise
bool ui_handle_event(const sapp_event* event);

// Render the UI for the current frame
void ui_render(void);
```

### 4.2. `ui.c` - Implementation Details

This file will contain the core logic.

**Key Implementation Steps:**

1.  **Includes and Globals:**
    *   Include `cimgui.h`, `sokol_app.h`, `sokol_gfx.h`, and `sokol_imgui.h`.
    *   Define `SOKOL_IMGUI_IMPL` before the include.

2.  **`ui_init()`:**
    *   Call `igCreateContext(NULL)`.
    *   Set up ImGui style and configuration (e.g., `igStyleColorsDark(NULL)`).
    *   Initialize `sokol_imgui` by calling `simgui_setup()` with an appropriate descriptor.

3.  **`ui_shutdown()`:**
    *   Call `simgui_shutdown()`.
    *   Call `igDestroyContext(NULL)`.

4.  **`ui_handle_event()`:**
    *   Pass the Sokol event to `simgui_handle_event()`.
    *   Return the boolean result to the main event loop to prevent game systems from processing events consumed by the UI (e.g., mouse clicks on a button).

5.  **`ui_render()`:**
    *   Call `simgui_new_frame()` at the beginning of the function.
    *   **Implement UI Widgets:**
        *   **HUD:** Create a non-interactive, borderless window (`ImGuiWindowFlags_NoInputs`, `ImGuiWindowFlags_NoTitleBar`, etc.) positioned in a corner. Use `igText()` to display world data (e.g., camera position, FPS).
        *   **Debug Panel:** Create a standard window with `igBegin()` and `igEnd()`. Add widgets like `igCheckbox()` for toggling wireframe rendering or `igSliderFloat()` for adjusting settings.
    *   Call `simgui_render()` at the end of the function to draw the UI.

### 4.3. Main Application Loop Integration

The main application file (`src/main.c`) will be updated to use the new UI system.

*   **`init_cb`:** Call `ui_init()`.
*   **`cleanup_cb`:** Call `ui_shutdown()`.
*   **`event_cb`:** Call `ui_handle_event()` and only process game input if it returns `false`.
*   **`frame_cb`:**
    1.  Update all game logic.
    2.  Call `ui_render()`.
    3.  Perform the `sokol_gfx` rendering pass (`sg_begin_default_pass`, etc.).
    4.  **Crucially, call `simgui_draw()` inside the main render pass.**
    5.  Commit the frame (`sg_commit()`).

## 5. Sprint Objectives (Redefined)

1.  **Week 1: Integration & Basic Window:**
    *   [ ] **Task:** Configure Makefile and successfully compile Dear ImGui/cimgui.
    *   [ ] **Task:** Implement `ui_init`, `ui_shutdown`, and `ui_handle_event`.
    *   [ ] **Goal:** Get a blank ImGui window to render on top of the 3D scene.

2.  **Week 2: Debug Panel & HUD:**
    *   [ ] **Task:** Create the `ui_render` function.
    *   [ ] **Task:** Implement a toggleable debug panel showing FPS and entity count.
    *   [ ] **Task:** Implement a basic, non-interactive HUD showing camera position.

3.  **Week 3: Refinement & Control:**
    *   [ ] **Task:** Add runtime-modifiable settings to the debug panel (e.g., a wireframe mode checkbox).
    *   [ ] **Task:** Ensure the UI correctly captures mouse and keyboard input, preventing the camera from moving when interacting with a UI window.

## 6. Conclusion

By following this engineering report, the development team can systematically integrate a powerful and flexible UI system into the engine. This will not only satisfy the immediate requirements for a HUD and debug tools but also provide a scalable foundation for all future UI development.
