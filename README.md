# CGame: A Data-Oriented 3D Game Engine

CGame is a high-performance, data-oriented 3D game engine written in C99. It features an advanced Entity-Component-System (ECS) architecture and uses the modern, cross-platform **Sokol** graphics API for rendering.

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

## 📚 Documentation Hub

This project's documentation is organized to provide a clear and comprehensive overview for developers.

### 1. Project Overview
*   **[README.md](./README.md)**: You are here. High-level summary and entry point.
*   **[COMPREHENSIVE_PROJECT_REVIEW.md](./docs/project/COMPREHENSIVE_PROJECT_REVIEW.md)**: The most current, high-level analysis of the project's status, strengths, and weaknesses.
*   **[Project Vision](./docs/project/VISION.md)**: The long-term goals and philosophy of the engine.
*   **[Project Roadmap](./docs/project/ROADMAP.md)**: A high-level overview of planned features and development milestones.
*   **[CHANGELOG.md](./CHANGELOG.md)**: A record of all notable changes made to the project.

### 2. Developer Guides
*   **[Development Guide](./docs/guides/DEVELOPMENT_GUIDE.md)**: The primary guide for setting up the development environment, building the project, and understanding the workflow.
*   **[Developer Quick Reference](./docs/guides/QUICK_REFERENCE.md)**: A quick reference for common development tasks.

### 3. Architecture
*   **[Architecture Overview](./docs/architecture/OVERVIEW.md)**: A detailed description of the Entity-Component-System (ECS) design, system scheduling, and core engine components.
*   **[Nuklear UI Integration Report](./docs/engineering/NUKLEAR_INTEGRATION_REPORT.md)**: A deep dive into the Nuklear UI implementation.

### 4. Sprints & Archives
*   **[Sprints Index](./docs/sprints/README.md)**: Documentation for ongoing and upcoming development sprints.
*   **[Sprint Archives](./docs/sprints/archives/)**: Records of completed sprints and reviews.
*   **[Archived Documentation](./docs/archive/)**: Outdated or superseded design documents.

## 🏛️ Architecture

CGame is built on three core principles:
1.  **Data-Oriented Design**: Components are simple data structures, and systems are functions that operate on them. This leads to efficient, cache-friendly code.
2.  **Entity-Component-System (ECS)**: Entities are created by composing components, allowing for flexible and emergent behaviors.
3.  **System Scheduling**: Game logic is implemented in systems that run at different frequencies (e.g., physics at 60Hz, AI at 10Hz) for optimal performance.

## 🎮 Controls

- **W/A/S/D**: Move camera forward/left/back/right
- **Arrow Keys**: Turn camera
- **ESC**: Exit
