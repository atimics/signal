# Component Reference

This document provides a comprehensive reference for all components available in the CGame engine.

## Core Components

*   **Transform**: Defines an entity's position, rotation, and scale in the world. (`src/core.h`)
*   **Physics**: Defines an entity's physical properties for simulation, including linear and angular dynamics. (`src/core.h`)
*   **Collision**: Defines an entity's collision shape and properties. (`src/core.h`)
*   **AI**: Defines the state and behavior for an AI-controlled entity. (`src/core.h`)
*   **Renderable**: Defines the data needed to render an entity. (`src/core.h`)
*   **Player**: Defines data specific to a player-controlled entity. (`src/core.h`)
*   **Camera**: Defines a camera for viewing the world. (`src/core.h`)
*   **SceneNode**: Defines a node in the scene graph hierarchy. (`src/core.h`)
*   **ThrusterSystem**: Defines a universal propulsion system for any entity. (`src/core.h`)
*   **ControlAuthority**: Defines input processing and control authority for an entity. (`src/core.h`)

## Specialized Components

*   **Controllable**: Allows entities to receive input and convert it to forces and movement. (`src/component/controllable.h`)
*   **LookTarget**: A component for the canyon racing control scheme, defining where an entity is looking. (`src/component/look_target.h`)
*   **ThrusterPointsComponent**: A simplified thruster system based on discrete points. (`src/component/thruster_points_component.h`)
