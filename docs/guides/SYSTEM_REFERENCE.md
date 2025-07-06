# System Reference

This document provides a comprehensive reference for all systems in the CGame engine.

## Core Systems

*   **AI System**: Updates AI-controlled entities, managing their state and decision-making. (`src/system/ai.h`)
*   **Camera System**: Manages camera entities, including movement, targeting, and projection. (`src/system/camera.h`)
*   **Collision System**: Performs collision detection between entities. (`src/system/collision.h`)
*   **Control System**: Processes player input and applies it to controllable entities. (`src/system/control.h`)
*   **Lighting System**: Manages lights in the scene and calculates lighting for materials. (`src/system/lighting.h`)
*   **LOD (Level of Detail) System**: Adjusts the level of detail of entities based on their distance from the camera to optimize performance. (`src/system/lod.h`)
*   **Memory System**: Manages memory allocation and tracking for the engine. (`src/system/memory.h`)
*   **ODE Physics System**: Integrates the Open Dynamics Engine (ODE) for physics simulation. (`src/system/ode_physics.h`)
*   **Performance System**: Monitors and reports on engine performance metrics. (`src/system/performance.h`)
*   **Physics System**: Manages the physics simulation for all entities. (`src/system/physics.h`)
*   **Scripted Flight System**: Controls entities along predefined flight paths. (`src/system/scripted_flight.h`)
*   **Thruster Points System**: A simplified thruster system that applies forces from discrete points. (`src/system/thruster_points_system.h`)
*   **Thruster System**: The primary thruster system that manages propulsion for entities. (`src/system/thrusters.h`)
