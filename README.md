# CGame: A Data-Oriented 3D Game Engine

CGame is a high-performance, data-oriented 3D game engine written in C99. It features an advanced Entity-Component-System (ECS) architecture and uses the modern, cross-platform **Sokol** graphics API for rendering.

## 🌟 Features

*   **Data-Oriented Design**: Built from the ground up for performance with a focus on cache-friendly data layouts.
*   **Pure ECS Architecture**: A flexible and scalable design that separates data (components) from logic (systems).
*   **High-Performance Rendering**: Utilizes the modern `sokol_gfx` API for efficient, cross-platform rendering (OpenGL, Metal, etc.).
*   **Frequency-Based System Scheduler**: Run expensive systems like AI less frequently than critical systems like physics and rendering.
*   **Data-Driven Development**: Define entities and scenes using simple text files for rapid iteration.
*   **Semantic Material System**: An automated, tag-based system for generating PBR-style materials.

## 🚀 Getting Started

### Prerequisites

- A C99-compliant compiler (e.g., `clang` or `gcc`).
- **macOS**: Xcode Command Line Tools for Metal support.
- **Linux**: `build-essential`, `libgl1-mesa-dev`, `libx11-dev`.
- Python 3 for the asset compilation pipeline.

### Building and Running

The project uses a simple Makefile.

```bash
# Clean, build, and run the engine
make clean && make && make run
```

## 📚 Documentation

For detailed information about the engine's architecture, development workflow, and project status, please see the **[Documentation Hub](./docs/README.md)**.

## 📈 Project Status

This project is under active development. The current focus is on stabilizing the core engine, improving the rendering pipeline, and integrating advanced features like a decal system and PBR materials. See the [Roadmap](docs/project/ROADMAP.md) for more details.