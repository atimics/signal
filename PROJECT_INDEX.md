# CGame Project Index

This document provides a curated index of the most relevant files in the CGame project, along with a brief summary of each file's purpose.

**🔴 Active Sprint**: [Sprint 18: Systems Refactoring](docs/sprints/active/sprint_18_systems_refactor.md)

## Core Engine (`src/`)

-   **`main.c`**: The main entry point for the application. Initializes the Sokol app, sets up the game world, and runs the main game loop.
-   **`core.h` / `core.c`**: Defines the core data structures for the engine, including the `World`, `Entity`, and component structs. Manages the ECS memory and entity lifecycle.
-   **`systems.h` / `systems.c`**: Implements the system scheduler and the logic for all core gameplay systems. **⚠️ Currently being refactored - see Sprint 18**.
-   **`assets.h` / `assets.c`**: Manages the loading and unloading of game assets (meshes, materials, textures).
-   **`data.h` / `data.c`**: Handles the loading of game data, such as scene definitions and entity templates, from text files.

## Rendering (`src/`)

-   **`render.h`**: A top-level header for the rendering system.
-   **`render_camera.h` / `render_camera.c`**: Manages the 3D camera, including its position, orientation, and projection.
-   **`render_lighting.h` / `render_lighting.c`**: Manages the lighting system, including light sources and shadow mapping.
-   **`render_mesh.h` / `render_mesh.c`**: Handles the rendering of 3D meshes.

## UI (`src/`)

-   **`ui.h` / `ui.c`**: Implements the UI system using the Nuklear library. Handles UI rendering, input, and state.
-   **`nuklear.h`**: The single-header file for the Nuklear immediate-mode UI library.
-   **`sokol_nuklear.h`**: The Sokol backend for Nuklear, which handles rendering and input integration.

## Sokol Headers (`src/`)

-   **`sokol_app.h`**: Handles window creation, input, and the main event loop.
-   **`sokol_gfx.h`**: The core graphics API for rendering.
-   **`sokol_glue.h`**: A helper library that integrates other Sokol headers.
-   **`sokol_log.h`**: A simple logging library.

## Documentation (`docs/`)

-   **`README.md`**: The main documentation hub, providing a curated list of the most important documents.
-   **`guides/`**: Contains detailed guides for developers, including the `DEVELOPMENT_GUIDE.md` and the `architecture/OVERVIEW.md`.
-   **`sprints/`**: Contains the plans for current and upcoming development sprints.

## Assets (`assets/`)

-   **`material_definitions.json`**: Defines the PBR material properties for the semantic material system.
-   **`meshes/`**: Contains the source 3D models (`.obj`) and their metadata (`.json`).
-   **`shaders/`**: Contains the GLSL and Metal shader source code.

## Tools (`tools/`)

-   **`build_pipeline.py`**: The primary build script that orchestrates mesh compilation from OBJ to binary COBJ format.
-   **`compile_mesh.py`**: Binary mesh compiler that converts OBJ files to COBJ format.
-   **`validate_metadata.py`**: A script for validating asset metadata.

## Root Directory

-   **`Makefile`**: The main build script for the project.
-   **`README.md`**: The main entry point for the project.
-   **`requirements.txt`**: The Python dependencies for the asset pipeline.
-   **`CHANGELOG.md`**: A log of all notable changes to the project.
-   **`.gitignore`**: Specifies which files and directories to ignore in Git.
