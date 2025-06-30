# CGame Engine: Gameplay Loop and State Management

**Status**: Authoritative | **Last Updated**: June 30, 2025
**Owner**: Gemini

## 1. Core Philosophy: A Predictable, Phased Approach

The CGame engine's core loop is designed to be predictable, deterministic, and easy to reason about. It follows a phased approach where each stage of the frame has a clear and distinct responsibility. This ensures that systems are updated in the correct order and that the application state remains consistent.

This document serves as the single source of truth for the engine's main loop, the order of operations, and the management of the global application state.

## 2. The Global Application State (`app_state`)

All global state required for the application to run is encapsulated in a single static `struct` within `src/main.c`. This `app_state` is the central hub of information for the entire application.

### Key State Variables:

*   **`world` (struct World)**: The heart of the engine. This contains the Entity-Component-System (ECS) data, including all entities and their components.
*   **`scheduler` (struct SystemScheduler)**: The system scheduler responsible for calling the update functions of all registered systems at their specified frequencies.
*   **`render_config` (struct RenderConfig)**: A collection of data and settings required by the rendering systems, such as screen dimensions, camera information, and lighting data.
*   **`player_id` (EntityID)**: The entity ID of the player, used for input and camera control.
*   **`simulation_time` (float)**: The total elapsed time since the simulation started.
*   **`frame_count` (int)**: The total number of frames that have been rendered.

## 3. The Three Phases of the Engine Lifecycle

The engine's lifecycle is divided into three distinct phases, which are managed by the Sokol application backend.

### Phase 1: Initialization (`init` function)

This phase runs only once at application startup. It is responsible for setting up all necessary systems and loading the initial state.

**Order of Operations**:

1.  **Initialize Sokol Graphics**: Sets up the low-level graphics API.
2.  **Initialize World**: Allocates memory for the ECS and prepares it for use.
3.  **Initialize System Scheduler**: Sets up the scheduler and registers all core engine systems (e.g., AI, Physics, Assets).
4.  **Initialize Render Config**: Populates the `render_config` struct with default values for the camera, lighting, and screen dimensions.
5.  **Initialize Renderer**: Creates the necessary GPU resources (shaders, pipelines) for the rendering systems.
6.  **Load Initial Scene**: Calls `load_scene_by_name` to populate the `world` with entities from a data file. This includes identifying the player entity.
7.  **Initialize UI**: Sets up the Nuklear UI context.

### Phase 2: The Frame Loop (`frame` function)

This is the core of the engine, running continuously once per frame. It is responsible for updating the simulation and rendering the scene.

**Order of Operations**:

1.  **Calculate Delta Time (`dt`)**: The time elapsed since the last frame is calculated. This is crucial for frame-rate independent physics and animation.
2.  **Update World**: The `world_update` function performs any necessary maintenance on the ECS itself (e.g., processing entity creation/deletion requests).
3.  **Update Systems (`scheduler_update`)**: The system scheduler is updated. It iterates through all registered systems and, based on their specified frequency, calls their respective `update` functions. This is where all game logic (AI, Physics, Animation) is executed.
4.  **Begin Render Pass**: A new rendering pass is started, clearing the screen to the background color.
5.  **Render Frame (`render_frame`)**: The main rendering function is called. It iterates through all renderable entities in the `world` and issues draw calls for them.
6.  **Render UI (`ui_render`)**: The debug UI is rendered on top of the 3D scene.
7.  **End Render Pass and Commit**: The render pass is finalized, and the completed frame is sent to the GPU to be displayed.

### Phase 3: Cleanup (`cleanup` function)

This phase runs only once when the application is shutting down. It is responsible for gracefully releasing all resources.

**Order of Operations**:

1.  **Shutdown UI**: Releases all UI resources.
2.  **Cleanup Renderer**: Frees all GPU resources.
3.  **Destroy Scheduler**: Calls the `destroy` function for all registered systems, allowing them to clean up their internal state.
4.  **Destroy World**: Frees all memory associated with the ECS.
5.  **Shutdown Sokol Graphics**: Shuts down the low-level graphics API.

## 4. Event Handling (`event` function)

User input and window events are handled in a dedicated `event` function.

*   **UI Priority**: The UI system is given the first opportunity to handle an event. If the UI captures the event (e.g., a mouse click on a button), it is not processed any further.
*   **Game Logic**: If the event is not captured by the UI, it is then processed by the game logic. This is where input like keyboard presses (e.g., for player movement, camera switching, or quitting the application) is handled.

This clear, phased approach ensures that the engine is both robust and easy to extend.
