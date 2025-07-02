# CGame Developer Manual

**Status**: Authoritative | **Last Updated**: July 1, 2025

## 1. Introduction

This manual provides a comprehensive guide for developers and contributors to the CGame engine. It covers the project's architecture, development workflow, coding standards, and release procedures.

---

## 2. Architecture

### 2.1. Core Philosophy: Data-Oriented Design

The engine is built from the ground up using a **data-oriented** approach. This means we prioritize the layout and access patterns of our data to maximize hardware performance. Components are simple C structs containing only data, and systems are functions that iterate over tightly packed arrays of components.

### 2.2. Entity-Component-System (ECS)

The ECS is the backbone of the engine. Entities are simple integer IDs, Components are pure data structs, and Systems are global functions that operate on entities with a specific combination of components.

### 2.3. Graphics Abstraction Layer (PIMPL)

To ensure the engine's core logic is not tightly coupled to a specific graphics library, we use the **Opaque Pointer (PIMPL) Idiom**. Public headers only declare pointers to incomplete structs (e.g., `struct GpuResources*`), and the implementation files (`.c`) contain the full definition and include the low-level graphics library (Sokol). This is a critical architectural pattern for maintainability.

---

## 3. Development Workflow

### 3.1. Getting Started

1.  **Prerequisites**: Ensure you have Git, a C99 compiler (Clang or GCC), Python 3.11+, and Make installed.
2.  **Clone the Repository**: `git clone https://github.com/your-repo/cgame.git && cd cgame`
3.  **Set up Python Environment**: `python3 -m venv .venv && source .venv/bin/activate && pip install -r requirements.txt`
4.  **Build and Run**: `make && make run`
5.  **Run Tests**: `make test`

### 3.2. Coding Standards

*   **Language**: C99.
*   **Formatting**: We use `clang-format` with the Google style. Run `make format` to automatically format your code.
*   **Naming Conventions**:
    *   `snake_case` for files and functions.
    *   `PascalCase` for structs and enums.
    *   `UPPER_SNAKE_CASE` for constants and macros.

### 3.3. Testing

The engine uses the **Unity Test Framework**. All new features or bug fixes should be accompanied by corresponding tests in the `/tests` directory. The test suite is run automatically by GitHub Actions on every commit and pull request.

**Current Status**: The test suite has a 60% pass rate as of July 2, 2025. Sprint 23 will focus on remediating test failures and improving test coverage for new components (THRUSTER_SYSTEM, CONTROL_AUTHORITY).

---

## 4. Asset Pipeline

The asset pipeline is a data-driven system that converts source assets into an optimized, engine-ready format.

### 4.1. Pipeline Architecture

1.  **Asset Generation (`tools/clean_asset_pipeline.py`)**: Generates mesh geometry, UV layout SVGs, and material files.
2.  **Asset Compilation (`tools/build_pipeline.py`)**: Converts `.obj` files to binary `.cobj` format, copies materials and textures, and generates the asset index.
3.  **Runtime Loading (`src/assets.c`)**: The engine loads the compiled binary assets at runtime.

### 4.2. Usage

*   **Generate All Assets**: `python3 tools/clean_asset_pipeline.py --all`
*   **Compile Assets**: `python3 tools/build_pipeline.py`

---

## 5. Release Management

### 5.1. Branch Strategy

*   **`main`**: Production-ready, protected branch. Releases are tagged from here.
*   **`develop`**: Integration branch for new features.
*   **`feature/*`**: Individual feature development branches.

### 5.2. Semantic Versioning

We follow strict semantic versioning (`MAJOR.MINOR.PATCH`).

### 5.3. Pull Requests

All changes must go through pull requests with required CI checks and code reviews.
