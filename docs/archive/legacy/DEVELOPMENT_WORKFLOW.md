# CGame Development Workflow

This guide details the tools, processes, and standards for contributing to the CGame engine.

## 1. Build System

The project uses a `Makefile` for all build-related tasks.

*   `make`: Compiles the engine and assets.
*   `make run`: Builds and runs the engine.
*   `make clean`: Removes all build artifacts.
*   `make wasm`: Compiles the engine to WebAssembly.

## 2. Asset Pipeline

The asset pipeline, located in `tools/`, is responsible for processing source assets into an engine-ready format.

*   **Source Assets**: Located in `assets/`. These are the raw, artist-created files (e.g., `.obj`, `.png`).
*   **Compiled Assets**: Located in `build/assets/`. These are the optimized, engine-ready files.
*   **Asset Compiler**: The `tools/asset_compiler.py` script handles this conversion. It is run automatically as part of the `make` command.

For a detailed guide on the asset pipeline and its capabilities, see the [Asset Pipeline README](../../tools/README.md).

## 3. Coding Standards

*   **Language**: All engine code is written in C99.
*   **Formatting**: Follow the existing code style. Use a tool like `clang-format` to ensure consistency.
*   **Naming Conventions**:
    *   `snake_case` for files and functions (e.g., `render_mesh.c`, `entity_create`).
    *   `PascalCase` for structs and enums (e.g., `RenderConfig`, `ComponentType`).
    *   `UPPER_SNAKE_CASE` for constants and macros.
*   **Comments**: Use comments to explain the *why*, not the *what*. Focus on complex logic or design decisions.

## 4. Automated Testing

The engine uses the **Unity Test Framework** to ensure code quality and prevent regressions. A robust suite of automated tests is a cornerstone of our development process.

*   **Running Tests**: The entire test suite can be run from the command line:
    ```bash
    make test
    ```
*   **Writing Tests**:
    *   All new features or bug fixes should be accompanied by corresponding tests.
    *   Unit tests for core logic should be placed in `tests/`.
    *   Follow the existing structure in `tests/test_core_math.c` as a template for new test suites.
*   **Continuous Integration (CI)**: The test suite is run automatically by GitHub Actions on every commit and pull request. A commit will not be merged if the test suite fails.

## 5. Debugging

*   **Debug UI**: The engine includes a Nuklear-based debug UI with various panels for inspecting engine state.
*   **Logging**: Use the `printf`-based logging for temporary debugging. Remove debug logs before committing.
*   **Wireframe Mode**: The debug UI provides a wireframe mode to inspect scene geometry.
*   **Normal Visualization**: The debug UI provides a mode to visualize vertex normals.
