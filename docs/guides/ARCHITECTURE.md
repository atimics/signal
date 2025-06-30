# CGame Engine Architecture

This document provides a high-level overview of the CGame engine's architecture.

## 1. Core Philosophy: Data-Oriented Design

The engine is built from the ground up using a data-oriented approach. This means we prioritize the layout and access patterns of our data to maximize performance. Instead of focusing on object-oriented hierarchies, we focus on:

*   **Data as the primary citizen**: Components are simple C structs containing only data.
*   **Cache-friendly operations**: Systems are functions that iterate over tightly packed arrays of components, minimizing cache misses.
*   **Decoupling**: Data and logic are kept separate, which makes the codebase easier to maintain, test, and reason about.

## 2. Entity-Component-System (ECS)

The ECS is the backbone of the engine.

*   **Entities**: Simple integer IDs that "own" a collection of components.
*   **Components**: Pure data structs that represent a single facet of an entity (e.g., `Transform`, `Physics`).
*   **Systems**: Global functions that operate on entities with a specific combination of components.

## 3. Key Engine Systems

### 3.1. System Scheduler (`src/systems.c`)

The scheduler manages the execution of all other systems. It uses a frequency-based approach, allowing different systems to be updated at different rates for performance optimization.

### 3.2. Rendering Pipeline (`src/render_3d.c`)

The rendering pipeline is built on the `sokol_gfx` library. It is responsible for taking the game world's data and turning it into a 2D image. The core rendering logic is in `render_3d.c`, which iterates through all renderable entities and issues draw calls.

### 3.3. Asset Pipeline (`tools/asset_compiler.py`)

The asset pipeline is a Python-based tool that processes source assets into an optimized, engine-ready format. It handles mesh compilation, material generation, and texture processing.

### 3.4. AI System

The AI system is designed for scalability and data-driven behavior. It uses a task-based system with a priority queue and a Level-of-Detail (LOD) system to manage AI logic efficiently.
