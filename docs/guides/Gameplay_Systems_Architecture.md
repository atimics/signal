# CGame Engine: Gameplay Systems Architecture

**Status**: Authoritative | **Last Updated**: June 30, 2025
**Owner**: Gemini

## 1. Core Philosophy: Data-Oriented, Modular, and Scalable Systems

The architecture of CGame's gameplay systems is guided by three core principles that ensure a robust, performant, and maintainable engine:

1.  **Data-Oriented Design**: All systems are built to operate on tightly packed, cache-friendly data. Components are pure C `structs` containing only data, and systems are functions that iterate over this data. This is the key to our performance strategy.
2.  **Modularity**: Each gameplay system (AI, Physics, Audio, etc.) is a self-contained module with a clean, well-defined C API. This separation of concerns makes the engine easier to reason about, test, and extend.
3.  **Scalability**: Systems are designed with future needs in mind. This includes support for asynchronous processing for heavy computations (like AI) and data-driven design that allows for easy expansion.

This document serves as the single source of truth for the architecture of all core gameplay systems.

## 2. The System Scheduler (`src/systems.c`)

The scheduler is the heart of the engine's update loop. It is responsible for executing all other systems at the correct frequency.

*   **Frequency-Based Execution**: Not all systems need to run at 60Hz. The scheduler allows us to define different update rates for different systems, which is a critical optimization.
    *   **High Frequency (e.g., 60Hz)**: Physics, Rendering.
    *   **Medium Frequency (e.g., 20Hz)**: Collision checks, Animation.
    *   **Low Frequency (e.g., 2-10Hz)**: AI logic, high-level decision making.
*   **Data Flow**: The scheduler passes the `World` state and `dt` (delta time) to each system's update function, giving it the context it needs to operate.

## 3. AI System Architecture

The CGame AI system is designed to support large-scale, emergent agent behavior through a modular, data-driven, and asynchronous architecture. The design is heavily influenced by the successful patterns observed in the `cosyworld` proof-of-concept.

### Core Principles:

*   **The Service-Oriented Mind**: The AI is not a single, monolithic block of code. It is a collection of collaborating C modules ("services"), each with a single responsibility.
*   **The Primacy of the Database**: Agent state, memories, and personality are not stored in flat files. They are persisted in a lightweight, embedded **SQLite** database (`universe.db`). This provides a robust, queryable, and persistent world state.
*   **The Composite Prompt**: Agent prompts are dynamically assembled at the moment of inference by a `prompt_service`, which gathers context from multiple sources (location, recent events, memories, available tools).

### Key Modules:

1.  **AI Service (`ai_service.c`)**:
    *   **Responsibility**: Manages the `llama.cpp` context and the inference process.
    *   **Asynchronous by Design**: To avoid blocking the main game loop, AI inference runs on a separate worker thread. The `ai_system_update` (main thread) pushes "inference jobs" to a thread-safe queue, and the AI worker thread pushes results back.
2.  **Memory Service (`memory_service.c`)**:
    *   **Responsibility**: Provides a clean C API for reading from and writing to the agent memory tables in the SQLite database.
3.  **Prompt Service (`prompt_service.c`)**:
    *   **Responsibility**: Constructs the final, composite prompt sent to the AI Service. It queries other systems and services to gather the necessary context.
4.  **Tool Service (`tool_service.c`)**:
    *   **Responsibility**: Exposes engine functionality to the AI agents.
    *   **Schema-Enforced**: Agent tool use is not based on parsing natural language. The AI is constrained by a JSON schema (using `llama.cpp`'s grammar features) to return a structured function call, ensuring reliability.

## 4. Physics and Collision System Architecture

The physics system is responsible for simulating the motion of objects and resolving collisions. It is a high-priority system that is foundational to nearly all gameplay mechanics.

### Core Principles:

*   **Data-Oriented Components**:
    *   `PhysicsComponent`: Contains properties like velocity, acceleration, mass, and forces.
    *   `ColliderComponent`: Defines the shape of the object for collision purposes (e.g., AABB, OBB, Sphere).
*   **Phased Approach**: The physics update is broken into distinct phases within the system's tick:
    1.  **Integration**: Update position and velocity based on forces.
    2.  **Broadphase**: Use a spatial partitioning structure (e.g., an Octree or Grid) to quickly identify potential collision pairs.
    3.  **Narrowphase**: Perform precise collision checks (e.g., using the Separating Axis Theorem) on the pairs identified in the broadphase.
    4.  **Resolution**: Resolve collisions by applying an impulse-based response to prevent objects from interpenetrating.

## 5. Audio System Architecture

The audio system provides immersive sound effects and music. It is designed with a clean abstraction layer to decouple the engine from the specific audio backend.

### Core Principles:

*   **Backend Abstraction**: We use the **Opaque Pointer (PIMPL) Idiom**, just as in our graphics system, to hide the implementation details of the chosen third-party audio library (e.g., `miniaudio` or `SoLoud`). Public headers will only contain pointers to opaque audio resource structs.
*   **Data-Oriented Components**:
    *   `AudioSourceComponent`: Represents a sound emitter in the world. Contains properties like the sound to play, volume, pitch, and whether it's looping.
    *   `AudioListenerComponent`: Typically attached to the camera, this represents the "ears" of the player, allowing for 3D spatialization.
*   **Asset Pipeline Integration**: Audio files (`.wav`, `.ogg`) are treated as assets and are managed by the `Asset Index`. The runtime loader is responsible for loading them into memory for the audio system to use.

## 6. Skeletal Animation System Architecture

The animation system is responsible for bringing characters and objects to life with skeletal animation.

### Core Principles:

*   **Data-Driven**: The system is designed to be driven by standard animation data formats like **glTF**, which will be processed by our asset pipeline.
*   **ECS Integration**:
    *   `ArmatureComponent`: Stores the hierarchy of bones (the skeleton).
    *   `AnimationComponent`: Manages the playback state of animation clips (e.g., current time, blending factor).
    *   `SkinnedMeshComponent`: Links a `Renderable` mesh to an `ArmatureComponent`, allowing it to be deformed by the bones.
*   **Performance**: The final bone transformation matrices are calculated on the CPU each frame and uploaded to the GPU via a uniform buffer. The actual vertex skinning is then performed efficiently in the vertex shader.
*   **Blending**: The system will support smooth transitions between different animation states (e.g., idle to run) through interpolation and state machine logic defined in the `AnimationComponent`.
