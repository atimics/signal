# Sprint 15 Research: Rendering System Refactor

This document contains the research and best practices for refactoring the core rendering pipeline.

## Research Request

Please investigate the following areas to ensure best practices for this refactor:

### Software Design Principles
1.  **Single Responsibility Principle (SRP)**: How can we apply SRP to the rendering code?
    *   **Best Practice**: Each module should have one reason to change. In our case, `render_system.c` should only change if the core rendering loop or global state management changes. `render_mesh.c` should only change if the way we draw a single mesh changes.
    *   **Resource**: [Single Responsibility Principle (Wikipedia)](https://en.wikipedia.org/wiki/Single-responsibility_principle)
2.  **Modularity and Encapsulation**: How can we improve the modularity of the rendering system?
    *   **Best Practice**: The `render_system` module should expose a minimal API (`render_system_init`, `render_system_shutdown`, `render_system_execute`). The internal state (pipelines, shaders) should be kept private to that module.
