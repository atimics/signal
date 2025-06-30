# CGame: A Data-Oriented 3D Game Engine

[![Build and Test](https://github.com/ratimics/cgame/actions/workflows/build.yml/badge.svg)](https://github.com/ratimics/cgame/actions/workflows/build.yml)
[![Test Suite](https://github.com/ratimics/cgame/actions/workflows/test.yml/badge.svg)](https://github.com/ratimics/cgame/actions/workflows/test.yml)

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

# Run the automated test suite
make test
```

## 🧪 Testing

CGame uses the [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity) for automated testing. The test suite includes:

- **Unit Tests**: Core math functions, data structures, and utilities
- **Integration Tests**: Asset loading, rendering pipeline, and system interactions
- **Regression Tests**: Preventing the re-introduction of previously fixed bugs

### Running Tests

```bash
# Run all tests
make test

# The test executable is built to: build/cgame_tests
```

## 📚 Documentation

For detailed information about the engine's architecture, development workflow, and project status, please see the **[Documentation Hub](./docs/README.md)**.

## 📈 Project Status

This project is under active development.

**Recent Accomplishments**:
*   **Sprint 10.5**: Repaired and stabilized the core mesh rendering pipeline.
*   **Sprint 15**: Successfully integrated the Unity testing framework and established a CI workflow.

**Current Focus**:
*   **Sprint 11.5**: A short, focused sprint to refactor the graphics API according to the PIMPL idiom, creating a fully decoupled and stable foundation for future rendering work.

See the [Sprint Backlog](./docs/sprints/backlog/README.md) for more details.