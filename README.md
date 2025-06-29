# CGame: A Data-Oriented 3D Game Engine

CGame is a high-performance, data-oriented 3D game engine written in C99. It features an advanced Entity-Component-System (ECS) architecture and is currently undergoing a migration to the modern, cross-platform **Sokol graphics API** for rendering. The engine is designed for creating emergent, AI-driven gameplay in a dynamic universe.

## 🚀 Getting Started

### Prerequisites

- **macOS**: `brew install sdl2` (still needed for windowing temporarily)
- **Linux**: `sudo apt-get install libsdl2-dev build-essential`
- **GCC** with C99 support

### Building and Running

The project is currently in a transitional state. The default `make run` command executes the legacy SDL application. To run the new Sokol application, you must specify the `main.c` entry point.

```bash
# Build and run the new Sokol application
make clean && make SRC_FILES="src/main.c src/core.c src/systems.c src/data.c src/assets.c src/render_3d.c" && ./cgame

# Build and run the legacy SDL application (for comparison)
make clean && make && make run
```

## 🏗️ Project Structure

- **`/assets`**: Contains all game assets, including meshes, materials, and shaders.
- **`/data`**: Game data, such as scene definitions and entity templates.
- **`/docs`**: All project documentation, including architecture, guides, and reports.
- **`/src`**: The C99 source code for the game engine.
- **`/tools`**: Scripts and tools for asset processing and development.

## 📚 Documentation Hub

This project's documentation is organized to provide a clear and comprehensive overview for developers, contributors, and users.

### 1. Project Overview
*   **[README.md](./README.md)**: You are here. High-level summary and entry point.
*   **[Project Vision](./docs/project/VISION.md)**: The long-term goals and philosophy of the engine.
*   **[Project Roadmap](./docs/project/ROADMAP.md)**: A high-level overview of planned features and development milestones.
*   **[CHANGELOG.md](./CHANGELOG.md)**: A record of all notable changes made to the project.

### 2. Developer Guides
*   **[Development Guide](./docs/guides/DEVELOPMENT_GUIDE.md)**: The primary guide for setting up the development environment, building the project, and understanding the workflow.
*   **[Developer Quick Reference](./docs/guides/QUICK_REFERENCE.md)**: A quick reference for common development tasks.
*   **[API Reference](./docs/api/REFERENCE.md)**: The reference for the engine's public API.

### 3. Architecture
*   **[Architecture Overview](./docs/architecture/OVERVIEW.md)**: A detailed description of the Entity-Component-System (ECS) design, system scheduling, and core engine components.
*   **[AI Integration](./docs/architecture/AI_INTEGRATION.md)**: The plan for integrating AI systems into the engine.
*   **[AI Pipeline](./docs/architecture/AI_PIPELINE.md)**: The architecture of the AI pipeline.

### 4. Technical Deep Dives
*   **[Sokol Implementation Guide](./docs/guides/SOKOL_IMPLEMENTATION_GUIDE.md)**: Technical plan for the ongoing migration to the Sokol graphics API.
*   **[UI Implementation Guide](./docs/guides/UI_IMPLEMENTATION_GUIDE.md)**: A guide to implementing UI features.
*   **[UI Integration Engineering Report](./docs/project/UI_INTEGRATION_ENGINEERING_REPORT.md)**: Detailed plan for implementing the Dear ImGui-based UI system.
*   **[Semantic Material System](./docs/architecture/SEMANTIC_MATERIAL_SYSTEM.md)**: Design of the material system for PBR rendering.
*   **[Asset Pipeline Improvements](./docs/architecture/ASSET_PIPELINE_IMPROVEMENTS.md)**: Proposed improvements to the asset pipeline.
*   **[Clean Code & Tech Debt Report](./docs/CLEAN_CODE_AND_TECH_DEBT_ENGINEERING_REPORT.md)**: An analysis of the current state of the codebase and a backlog of technical debt to be addressed.

### 5. Sprints & Reviews
*   **[Sprints Index](./docs/sprints/README.md)**: Documentation for ongoing and upcoming development sprints.
*   **[Sprint Archives](./docs/sprints/archives/)**: Records of completed sprints and reviews.

### 6. User Manual
*   **[User Manual](./docs/user/MANUAL.md)**: Instructions on how to play and interact with the game.

## 🏛️ Architecture

CGame is built on three core principles:
1.  **Data-Oriented Design**: Components are simple data structures, and systems are functions that operate on them. This leads to efficient, cache-friendly code.
2.  **Entity-Component-System (ECS)**: Entities are created by composing components, allowing for flexible and emergent behaviors.
3.  **System Scheduling**: Game logic is implemented in systems that run at different frequencies (e.g., physics at 60Hz, AI at 10Hz) for optimal performance.

## 🎮 Controls (Sokol App)

- **W/A/S/D**: Move camera forward/left/back/right
- **Arrow Keys**: Turn camera
- **ESC**: Exit
- **F1**: Toggle Debug UI (planned)
