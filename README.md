# SIGNAL

[![Build and Test](https://github.com/ratimics/cgame/actions/workflows/build.yml/badge.svg)](https://github.com/ratimics/cgame/actions/workflows/build.yml)
[![Test Suite](https://github.com/ratimics/cgame/actions/workflows/test.yml/badge.svg)](https://github.com/ratimics/cgame/actions/workflows/test.yml)

**In the silent graveyards of space, a ghost signal is calling. And you are the only one who can answer.**

**SIGNAL** is a high-performance, data-oriented 3D game engine and the foundation for a single-player space exploration game. It is written in C99, features a pure Entity-Component-System (ECS) architecture, and uses the modern, cross-platform **Sokol** graphics API for rendering.

## 🚀 Getting Started

### Prerequisites

- A C99-compliant compiler (e.g., `clang` or `gcc`).
- **macOS**: Xcode Command Line Tools for Metal support.
- **Linux**: `build-essential`, `libgl1-mesa-dev`, `libx11-dev`.
- Python 3 for the asset compilation pipeline (`pip install -r requirements.txt`).
- `clang-format` for code formatting (e.g., `brew install clang-format` or `apt-get install clang-format`).

### Building and Running

The project uses a simple Makefile. The first build will automatically compile the assets.

```bash
# Clean, build, and run the engine
make clean && make && make run

# Run the automated test suite
make test
```

## 📚 Documentation Hub

For detailed information about the engine's architecture, development workflow, and creative vision, please see the main **[Documentation Hub](./docs/README.md)**. This is the primary entry point for all developers.

## 📈 Project Status

This project is under active development.

**Recent Accomplishments**:
*   **Asset Pipeline Overhaul**: Migrated from a slow, text-based asset system to a high-performance, pre-compiled binary format.
*   **Rendering Stabilization**: Resolved all critical rendering bugs, enabling the stable loading and display of textured 3D models.
*   **Vision Solidified**: Established a clear and compelling creative vision, "The Ghost Signal," to guide all future development.

**Current Focus**:
*   **Sprint 13**: Implementing foundational performance optimizations (View Frustum Culling) and paying down remaining technical debt by enforcing a universal code style.

See the [Active Sprint Guide](./docs/sprints/active/sprint_13_optimization_and_style.md) for more details.
