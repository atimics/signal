# CGame Engine Documentation

**Last Updated**: July 1, 2025

Welcome to the CGame engine documentation. This is the central hub for all technical documentation, development guides, and project management resources.

## 🚀 Getting Started

If you're new to the project, start here:

1.  **[Project Overview](../README.md)**: The main project `README.md` has instructions for building and running the engine.
2.  **[Developer's Guide](./guides/GETTING_STARTED.md)**: A guide for setting up your development environment and contributing to the project.
3.  **[Architecture Overview](./guides/Graphics_and_Asset_Architecture.md)**: A high-level overview of the engine's architecture.

## 📚 Project Index

This section provides a curated index of the most relevant files and directories in the CGame project.

### Core Engine (`src/`)

-   **`main.c`**: The main entry point for the application.
-   **`core.h` / `core.c`**: Core data structures (World, Entity, ECS).
-   **`systems.h` / `systems.c`**: System scheduler and core gameplay systems.
-   **`assets.h` / `assets.c`**: Asset loading and management.
-   **`data.h` / `data.c`**: Game data loading (scenes, entities).

### Rendering (`src/`)

-   **`render.h`**: Top-level rendering header.
-   **`render_camera.h` / `render_camera.c`**: 3D camera management.
-   **`render_lighting.h` / `render_lighting.c`**: Lighting and shadow management.
-   **`render_mesh.h` / `render_mesh.c`**: 3D mesh rendering.

### UI (`src/`)

-   **`ui.h` / `ui.c`**: UI implementation using Nuklear.

### Sokol Headers (`src/`)

-   **`sokol_app.h`**: Window creation, input, and event loop.
-   **`sokol_gfx.h`**: Core graphics API.

### Documentation (`docs/`)

-   **`README.md`**: This file - the documentation hub.
-   **`guides/`**: Detailed developer guides.
-   **`sprints/`**: Sprint plans and history.
-   **`research/`**: Technical research and analysis.
-   **`archive/`**: Historical and deprecated documents.

### Assets (`assets/`)

-   **`material_definitions.json`**: PBR material definitions.
-   **`meshes/`**: Source 3D models (`.obj`) and metadata (`.json`).
-   **`shaders/`**: GLSL and Metal shader source code.

### Tools (`tools/`)

-   **`build_pipeline.py`**: Main build script for asset compilation.
-   **`compile_mesh.py`**: Mesh compiler (OBJ to COBJ).
-   **`validate_metadata.py`**: Asset metadata validation script.

### Root Directory

-   **`Makefile`**: The main build script for the project.
-   **`README.md`**: The main entry point for the project.
-   **`CHANGELOG.md`**: A log of all notable changes.

## 📈 Project Management

- **[Current Status](./CURRENT_STATUS.md)** - Real-time project status and current focus.
- **[Strategic Plan](./STRATEGIC_PLAN.md)** - Long-term project roadmap and vision.
- **[Sprints](./sprints/)** - Sprint planning, history, and backlog.
- **[Release Standards](./RELEASE_STANDARDS.md)** - Standards for releases, branching, and quality gates.

## 🎨 Asset Pipeline

- **[Asset Pipeline Overview](./ASSET_PIPELINE.md)** - Complete asset pipeline documentation.
- **[Asset Pipeline Validation](../ASSET_PIPELINE_VALIDATION.md)** - Validation checklist for the asset pipeline.