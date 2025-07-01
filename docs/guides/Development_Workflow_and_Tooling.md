# CGame Engine: Development Workflow and Tooling

**Status**: Authoritative | **Last Updated**: June 30, 2025
**Owner**: Gemini

## 1. Core Philosophy: A Stable, Efficient, and Empowering Workflow

The CGame development philosophy is centered on providing a workflow that is stable, efficient, and empowering for developers. We achieve this through three key pillars:

1.  **Confidence through Testing**: We believe that a comprehensive, automated testing suite is non-negotiable. It is the foundation of code quality, enabling developers to refactor and add features with confidence, knowing that a safety net is in place to catch regressions.
2.  **Insight through UI**: We provide developers with the tools to see and interact with the engine's state in real-time. A powerful and extensible debug UI is critical for understanding complex systems and diagnosing issues quickly.
3.  **Efficiency through Tooling**: We are committed to building a powerful, intuitive in-engine editor that streamlines the content creation process, moving beyond manual text-file editing to a visual, interactive workflow.

This document serves as the single source of truth for the engine's testing framework, UI system, and editor strategy.

## 2. The Testing Framework

To ensure stability and enable a robust CI/CD pipeline, the CGame engine has standardized on the **Unity Test Framework**.

### Framework Choice: Unity

*   **Why Unity?**: Unity is a lightweight, portable, and easy-to-use unit testing framework designed specifically for C. It has minimal dependencies and integrates cleanly with our `Makefile`-based build system.
*   **Location**: All test files are located in the `/tests` directory. The Unity source itself is vendored in `/tests/vendor`.
*   **Execution**: The entire test suite is run via the `make test` command.

### Testing Strategy

Our testing strategy is multi-layered:

1.  **Unit Tests**: These are the most granular tests, designed to validate a single function or C module in isolation.
    *   **Example**: `test_core_math.c` tests the vector and quaternion functions in `src/core.c`.
    *   **Mocking**: For modules with external dependencies (like graphics), we use preprocessor macros to redirect function calls to mock implementations during test builds, isolating the code under test.
2.  **Integration Tests**: These tests validate the interaction between two or more engine systems.
    *   **Example**: A test could be written to verify that creating an entity and adding a `PhysicsComponent` correctly updates the internal state of the `physics_system`.
    *   **Environment**: These tests run in a "headless" mode, with a minimal engine instance initialized specifically for the test.
3.  **Graphics/Rendering Tests**:
    *   **Strategy**: Automated validation of visual output is achieved through **screenshot comparison**.
    *   **Process**:
        1.  A test renders a specific, controlled scene.
        2.  The resulting image is saved as a bitmap (`.bmp`).
        3.  This output is compared against a "golden" reference image stored in the repository.
        4.  Any pixel difference outside of a small tolerance is flagged as a test failure.
    *   **Example**: `tests/test_rendering.c` contains examples of this approach.

## 3. The UI System

The engine's debug UI and future editor are built using the **Nuklear** immediate-mode GUI library.

### UI Architecture

*   **Immediate Mode**: Nuklear is an immediate-mode library, meaning the UI is rebuilt from scratch every frame. There is no retained state or complex object hierarchy. This fits perfectly with our data-oriented philosophy.
*   **Data Binding**: Since there is no automatic data binding, we use a pattern of explicit UI functions. For example, a function like `ui_render_transform_component(nk_context* ctx, Transform* transform)` is responsible for taking the component data and rendering the necessary widgets.
*   **Extensibility**: The UI is designed to be easily extended. New debug windows or custom widgets can be created by writing new C functions that follow the established pattern.

## 4. The In-Engine Editor: A Long-Term Vision

While currently a low-priority research item, the long-term vision for CGame includes a powerful, integrated, in-engine editor to replace the current text-file-based workflow.

### Architectural Principles

*   **Engine as a Mode**: The editor will not be a separate application. It will be a "mode" of the main engine, which can be toggled on or off at compile time or runtime. This ensures the editor always has access to the full, live state of the game world.
*   **Clean Separation**: Editor-specific code will be clearly separated from runtime engine code using file naming conventions (`_editor.c`) and conditional compilation (`#ifdef CGAME_EDITOR`).
*   **UI Technology**: The editor will be built using our existing **Nuklear** UI library, leveraging our experience and custom widget designs.

### Feature Roadmap (High-Level)

1.  **MVP (Minimum Viable Product)**:
    *   **Scene Hierarchy**: A tree view showing all entities in the current scene.
    *   **Component Inspector**: A panel that displays and allows editing of the components attached to the selected entity.
    *   **3D Viewport Gizmos**: Visual manipulators for moving, rotating, and scaling objects directly in the game view.
2.  **V1.0 and Beyond**:
    *   **Asset Browser**: A tool for browsing and managing game assets.
    *   **Material Editor**: A visual editor for creating and modifying PBR materials.
    *   **Live Recompilation**: The ability to trigger the asset compiler from within the editor and see changes live.

This strategic approach to tooling will ensure that as the CGame engine grows in complexity, the developer experience remains efficient, insightful, and stable.
