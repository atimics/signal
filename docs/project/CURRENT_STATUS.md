# Project Status: CGame Engine

**Last Updated:** June 29, 2025

This document provides a high-level, authoritative overview of the CGame engine's current development status, architecture, and technology stack. It is intended to be the primary entry point for any developer wishing to understand the project.

## 1. Current Development Status

The project is currently in a **post-migration, pre-rendering** phase.

The major architectural effort to migrate the engine from a legacy SDL backend to the modern, cross-platform **Sokol API** is complete. This has successfully unified the codebase, removed significant technical debt, and established a stable foundation for future development.

However, the immediate and critical priority is to **restore visual output**. While the core simulation and ECS systems are running, the rendering pipeline is not yet drawing any geometry to the screen. The current development sprint, **Sprint 4.1**, is entirely focused on bridging this gap.

## 2. Technology Stack

*   **Core Language:** C99
*   **Graphics & Windowing:** [Sokol](https://github.com/floooh/sokol) (specifically `sokol_gfx`, `sokol_app`, `sokol_glue`)
    *   **Backend (macOS):** Metal
    *   **Backend (Linux):** OpenGL
*   **UI Library:** [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) (via `sokol_nuklear.h`)
*   **Asset Pipeline:** Python 3 with `trimesh`, `cairosvg`, `numpy`, `scipy`, and `jsonschema`.
*   **Build System:** GNU Make

## 3. Core Architecture

The engine is built on a data-oriented **Entity-Component-System (ECS)** architecture.

*   **Entities:** Simple integer IDs.
*   **Components:** Plain C structs representing data (e.g., `Transform`, `Physics`, `Material`). Stored in contiguous arrays in the `World` struct for cache-friendly access.
*   **Systems:** Functions that operate on entities possessing a specific set of components (e.g., `physics_system_update`, `render_system_update`).
*   **Scheduler:** A simple system scheduler in `systems.c` calls the various systems at different frequencies for performance.

## 4. Key Repositories & Directories

*   **`/src`**: All C99 source code for the engine.
*   **`/docs`**: All project documentation. The most up-to-date, high-level reports are in `docs/project`.
*   **`/assets`**: Raw game assets (e.g., `.obj` files, `.mtl` files, textures).
*   **`/tools`**: Python scripts for the asset compilation pipeline.
*   **`/build`**: Build artifacts, including compiled assets and the final executable. This directory is generated and can be safely deleted.

## 5. Next Steps & Priorities

1.  **Complete Sprint 4.1:** This is the top priority. The goal is to render textured 3D meshes on screen.
2.  **Clean up Documentation & Build System:** As outlined in the [Comprehensive Project Review](./COMPREHENSIVE_PROJECT_REVIEW.md), the `Makefile` and project documentation need to be updated to reflect the current state of the codebase.
3.  **Finalize UI Implementation:** Complete the Nuklear debug UI to provide essential developer feedback.
4.  **Implement PBR:** Begin work on the PBR shader and material system.
