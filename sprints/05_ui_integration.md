# Sprint 05: UI Integration with Dear ImGui

**Goal:** Integrate a flexible and powerful immediate mode UI system using Dear ImGui. This will replace the temporary debug text and provide a foundation for creating a rich in-game user interface, including a heads-up display (HUD), menus, and debug panels.

**Period:** August 2025

## Key Objectives:

1.  **Integrate Dear ImGui and cimgui:**
    *   [ ] Add the Dear ImGui source files to the project.
    *   [ ] Add the `cimgui` C-bindings to the project.
    *   [ ] Add the `sokol_imgui.h` header to the `src/` directory.
    *   [ ] Update the `Makefile` to correctly compile the Dear ImGui and cimgui source files.

2.  **Initialize and Render the UI:**
    *   [ ] In the `init()` callback, initialize `sokol_imgui` after `sokol_gfx`.
    *   [ ] In the `frame()` callback, create a new ImGui frame, render the UI, and handle input.
    *   [ ] Ensure that the UI is rendered correctly on top of the 3D scene.

3.  **Create a Basic HUD:**
    *   [ ] Design and implement a simple heads-up display (HUD) that shows key player information (e.g., position, velocity, throttle).
    *   [ ] The HUD should be non-interactive and positioned in a corner of the screen.
    *   [ ] Use ImGui's windowing and text rendering features to create the HUD.

4.  **Develop a Debug Panel:**
    *   [ ] Create a toggleable debug panel that displays detailed information about the game world.
    *   [ ] The panel should show:
        *   The current frame rate (FPS).
        *   The number of active entities.
        *   The camera's position and orientation.
        *   Render settings that can be changed at runtime (e.g., wireframe mode).

5.  **Refactor UI Code:**
    *   [ ] Create a new `ui.c` and `ui.h` to encapsulate all UI-related code.
    *   [ ] The `ui.c` file will be responsible for initializing ImGui and rendering the various UI components (HUD, debug panel, etc.).
    *   [ ] The main `frame()` callback will simply call a `ui_render()` function.

## Expected Outcomes:

*   The project will have a fully functional, data-driven UI system.
*   A basic HUD will provide the player with essential information.
*   A powerful debug panel will be available to developers for inspecting and modifying the game state at runtime.
*   The UI code will be well-organized and decoupled from the core game logic and rendering.

## Pre-computation/Pre-analysis:

*   The current UI is limited to basic debug text printed to the console.
*   Dear ImGui is a popular and powerful immediate mode GUI library that is well-suited for game development.
*   The `sokol_imgui.h` header provides a convenient bridge between Sokol and Dear ImGui.
*   The integration will require careful management of the build system to include the necessary C++ source files for Dear ImGui and the C bindings from cimgui.
