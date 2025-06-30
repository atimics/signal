# CGame Sprint Backlog

This document outlines the planned and active development sprints for the CGame engine. It serves as a high-level roadmap for future work.

## Active Sprint

### [Sprint 09: Texture System Implementation](./02_active/09_texture_system.md)
-   **Goal**: Implement a complete texture loading and GPU binding system, enabling meshes to display with proper textures instead of solid colors.
-   **Status**: In Progress

### [Sprint 9.1: Code Quality and Rendering Refactor](./02_active/09.1_code_quality_and_rendering_refactor.md)
-   **Goal**: Improve the quality, maintainability, and modularity of the core C codebase, with a special focus on refactoring the rendering system.
-   **Status**: In Progress

## Feature Backlog

### [Sprint 10: PBR Materials & Advanced Lighting](./sprint_plans/10_pbr_materials_research.md)
-   **Goal**: Implement Physically Based Rendering (PBR) materials with proper lighting calculations, enabling realistic material appearance with metallic, roughness, and normal mapping support.

### [Sprint 11: Advanced Lighting & Shadow Mapping](./sprint_plans/11_advanced_lighting_research.md)
-   **Goal**: Implement advanced lighting techniques including shadow mapping, point lights, spot lights, and environmental lighting to create realistic and dynamic lighting scenarios.

### [Sprint 12: Decal System](./sprint_plans/12_decal_system_research.md)
-   **Goal**: Implement a decal system for dynamic surface details like bullet holes, scorch marks, and projected textures.

### [Sprint 13: UI Finalization & Engine Polish](./sprint_plans/13_ui_integration_research.md)
-   **Goal**: Finalize the UI system by building out a comprehensive suite of debug and development tools using Nuklear.

### [Sprint 14: Asynchronous AI System](./sprint_plans/14_async_ai_research.md)
-   **Goal**: Integrate the `gemma.cpp` inference engine using an asynchronous, multi-threaded job system to prevent blocking the main render loop.
