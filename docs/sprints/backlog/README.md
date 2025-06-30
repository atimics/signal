# CGame Sprint Backlog

This document outlines the planned development sprints for the CGame engine.

## Feature Backlog

### [Sprint 10.5: Critical Mesh System Architecture Repair](../active/10.5_mesh_system_repair.md) **[ACTIVE - HIGH PRIORITY]**
- **Goal**: Fix fundamental architectural disconnect between asset pipeline and rendering system. Asset compiler generates proper .cobj files but engine fails to load them correctly, causing validation errors and rendering failures.
- **Status**: Critical blocker for all PBR work - must be completed first.

### [Sprint 11: PBR and Rendering Standardization](./research/11_pbr_and_rendering_standardization.md)
- **Goal**: Standardize the rendering pipeline by implementing a full Physically-Based Rendering (PBR) workflow, from asset compilation to in-engine rendering. This will resolve the visual discrepancy between the source and compiled assets in the mesh viewer and enable a more advanced and realistic look for the game.
- **Dependencies**: Requires Sprint 10.5 completion (working mesh system)

### [Sprint 13: UI Finalization & Engine Polish](./research/13_ui_integration.md)
-   **Goal**: Finalize the UI system by building out a comprehensive suite of debug and development tools using Nuklear.

### [Sprint 14: Asynchronous AI System](./research/14_async_ai.md)
-   **Goal**: Integrate the `gemma.cpp` inference engine using an asynchronous, multi-threaded job system.
