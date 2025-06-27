# Architecture Overview

This document provides a high-level overview of the CGame engine's architecture. It is intended for developers who want to understand how the different systems in the game work together.

## Core Architectural Pillars

The architecture is built on three core principles:

1.  **Modular Design:** The engine is divided into distinct, independent modules with clear responsibilities (e.g., Renderer, Universe, AI Engine). This separation of concerns makes the codebase easier to understand, maintain, and extend.
2.  **Data-Oriented Design:** The engine prioritizes efficient data access patterns. Instead of a traditional object-oriented approach, we use a data-oriented design that organizes data in a way that is friendly to the CPU cache, allowing for high-performance processing of thousands of game entities.
3.  **Scalable AI:** The AI system is designed to be highly scalable, capable of supporting thousands of intelligent, autonomous entities. It uses a centralized task-based system with Level-of-Detail (LOD) optimizations to ensure real-time performance.

## System Breakdown

The engine is composed of the following major systems:

*   **Game:** The top-level system that manages the main game loop and orchestrates the other systems.
*   **Renderer:** A low-level system responsible for all drawing operations, using SDL2 as the backend.
*   **Universe:** The heart of the simulation. It manages all game entities, their spatial relationships, and the overall state of the game world.
*   **Entity:** A scalable Entity-Component System (ECS) that represents all objects in the game world, from the player's ship to individual asteroids.
*   **AI Engine:** A centralized system that manages all AI processing. It uses a priority-based task queue to schedule AI calculations and integrates with a neural language model (Llama.cpp) to generate intelligent behavior.
*   **Dialog System:** Manages real-time conversations with AI entities, featuring streaming text with a typewriter effect.
*   **Error Handling:** A robust, centralized system for logging, validation, and error recovery.

For a more detailed look at the AI system, see the [AI Integration Summary](./AI_INTEGRATION.md).
For the response to the last architectural review, see [Architectural Review Response](./REVIEW_RESPONSE.md).
