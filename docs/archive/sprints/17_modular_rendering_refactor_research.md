# Sprint 17 Research: Modular Rendering Pipeline Refactor

This document contains the research and best practices for refactoring the monolithic `render_3d.c` into a modular, ECS-driven rendering pipeline.

## Research Request

Please investigate the following areas to ensure best practices for this refactor:

### Software Design Principles
1.  **Single Responsibility Principle (SRP)**: How can we apply SRP to the rendering code?
    *   **Best Practice**: Each module should have one reason to change. In our case, `render_system.c` should only change if the core rendering loop or global state management changes. `render_mesh.c` should only change if the way we draw a single mesh changes.
    *   **Resource**: [Single Responsibility Principle (Wikipedia)](https://en.wikipedia.org/wiki/Single-responsibility_principle)
2.  **Modularity and Encapsulation**: How can we improve the modularity of the rendering system?
    *   **Best Practice**: The `render_system` module should expose a minimal API (`render_system_init`, `render_system_shutdown`, `render_system_execute`). The internal state (pipelines, shaders) should be kept private to that module.

### C99 Best Practices
1.  **Static Functions**: When should we use `static` for functions?
    *   **Best Practice**: Any function that is only called from within the same `.c` file should be declared `static`. This limits its scope to that file, preventing naming conflicts and improving encapsulation.
2.  **Header Guards**: How should we structure our header files?
    *   **Best Practice**: All header files must use the standard `#ifndef`/`#define`/`#endif` include guards to prevent multiple inclusion.

### Sokol Best Practices
1.  **Resource Management**: What is the recommended way to manage Sokol resources?
    *   **Best Practice**: All Sokol resources (`sg_pipeline`, `sg_shader`, `sg_buffer`, `sg_image`) should be created during an initialization phase and destroyed during a shutdown phase. Avoid creating and destroying resources in the main loop.
    *   **Resource**: [Sokol Header Docs](https://github.com/floooh/sokol)
