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

## 📚 Documentation

For all technical documentation, development guides, and project management resources, please see the main **[Documentation Hub](./docs/README.md)**.