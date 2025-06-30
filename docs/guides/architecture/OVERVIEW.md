# CGame Engine Architecture Overview

**Last Updated:** June 29, 2025

This document provides a comprehensive overview of the CGame engine's architecture. It covers the core Entity-Component-System (ECS) design, the system scheduler, the asset pipeline, and the AI integration strategy.

## 1. Core Philosophy: Data-Oriented Design

The engine is built from the ground up using a data-oriented approach. This means we prioritize the layout and access patterns of our data to maximize performance. Instead of focusing on object-oriented hierarchies, we focus on:

- **Data as the primary citizen:** Components are simple C structs containing only data.
- **Cache-friendly operations:** Systems are functions that iterate over tightly packed arrays of components, minimizing cache misses.
- **Decoupling:** Data and logic are kept separate, which makes the codebase easier to maintain, test, and reason about.

## 2. Entity-Component-System (ECS)

The ECS is the backbone of the engine.

-   **Entities:** Simple integer IDs that "own" a collection of components. An entity's identity and behavior are defined by the components attached to it.
-   **Components:** Pure data structs that represent a single facet of an entity (e.g., `Transform`, `Physics`, `Material`). They are stored in contiguous arrays within the main `World` struct.
-   **Systems:** Global functions that operate on entities that have a specific combination of components. For example, the `physics_system_update` function iterates over all entities that have both a `Transform` and a `Physics` component.

This design allows for extreme flexibility and avoids the rigid hierarchies of traditional inheritance-based models, enabling emergent gameplay behaviors.

## 3. System Scheduler

To manage the execution of game logic, the engine uses a simple, frequency-based scheduler located in `src/systems.c`.

-   **Frequency-based Execution:** Each system (e.g., Physics, AI, Camera) is assigned an update frequency in Hz.
-   **Performance Optimization:** This allows us to run expensive systems, like AI, less frequently than critical systems like Physics and Camera, ensuring optimal performance. For example:
    -   Physics: 60Hz
    -   Camera: 60Hz
    -   AI: 10Hz (or dynamically adjusted based on LOD)

## 4. Asset Pipeline

The asset pipeline is designed to separate slow, offline compilation from fast, runtime loading.

-   **Source Assets:** Raw art assets (e.g., `.obj`, `.png`, `.svg`) are stored in the `/assets` directory.
-   **Metadata:** Each asset is described by a `metadata.json` file, which includes tags for the semantic material system.
-   **Compiler:** The Python script at `tools/asset_compiler.py` reads the source assets and metadata.
-   **Output:** The compiler processes this data and outputs optimized, game-ready binary files into the `/build/assets` directory. This is the data the engine actually loads.

## 5. Rendering Pipeline

The rendering pipeline is designed for high performance and flexibility, using the `sokol_gfx` API for cross-platform graphics.

### 5.1. Rendering Backend (`src/render_3d.c`)

This file is responsible for the low-level details of rendering. It initializes the Sokol graphics context, creates the rendering pipeline, and provides functions for drawing entities. The `render_frame` function is the heart of the renderer, iterating through all renderable entities and drawing them to the screen.

### 5.2. Camera System (`src/systems.c`)

The camera system is responsible for controlling the viewpoint in the 3D world. It's implemented as a standard ECS system that runs every frame.

-   **Camera Component**: A `Camera` component can be attached to any entity to turn it into a camera. This component stores the camera's properties, such as its field of view (FOV), near and far clipping planes, and behavior (e.g., static, chase, third-person).
-   **Active Camera**: The `World` struct keeps track of the active camera. The rendering system uses this camera's view and projection matrices to render the scene.
-   **Camera Behaviors**: The `camera_system_update` function implements various camera behaviors. For example, a "chase" camera will smoothly follow a target entity.

## 6. AI Integration

The AI system is designed for scalability, aiming to support thousands of intelligent agents in a persistent universe.

-   **Inference Engine:** The architecture is designed to integrate with a lightweight, C-based neural inference engine. The current plan is to use **`gemma.cpp`** for its high performance on CPUs.
-   **Task-Based System:** AI logic is managed by a centralized task system.
    -   **Priority Queue:** Tasks are prioritized (e.g., Dialog > Combat > Navigation) to ensure that critical AI decisions are handled with low latency.
    -   **LOD Scheduling:** The update frequency of an entity's AI is determined by its distance from the player, drastically reducing the computational load of agents that are out of sight.
-   **Data-Driven Behavior:** AI personalities and decision-making logic are defined by data (base prompts and behavior trees), allowing for rich, emergent AI without hardcoding behaviors.
