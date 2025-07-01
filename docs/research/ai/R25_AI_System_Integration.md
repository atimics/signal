# Research & Design: AI System Integration

**ID**: `R25_AI_System_Integration`
**Author**: Gemini, Chief Science Officer
**Status**: **Draft**
**Related Vision**: `R17_Gameplay_Vision`, `LORE_02_Factions_of_the_Void`

## 1. Overview

This document provides a detailed technical specification for integrating our planned generative AI system into the CGame engine. The goal is to create a robust, performant, and scalable architecture that allows Large Language Models (LLMs) via `llama.cpp` to drive the behavior of our AI factions in a way that is deeply integrated with our existing Entity-Component-System (ECS) architecture.

## 2. Core Architectural Principles

*   **Asynchronous by Design**: Direct, synchronous calls to an LLM during the main game loop are not feasible; they would cause the game to freeze while waiting for a response. Therefore, the entire system must be designed to be asynchronous. The game will request an AI decision, and the AI will provide one on a future frame.
*   **Data-Oriented**: The AI's "personality" and long-term memory will not be hardcoded. They will be stored in an external SQLite database, allowing for easy modification and expansion.
*   **System-Driven**: The AI's actions will be translated into changes in our existing ECS components. The AI system does not directly control the ship; it changes the `target_position` in the `AI` component, and the existing physics and navigation systems take over from there.

## 3. The AI Integration Pipeline

The following diagram illustrates the flow of information from the game world to the AI and back:

```
[Game World (ECS)] <--> [AI System (C)] <--> [Task Queue] <--> [LLM Service (C++)] <--> [Personality DB (SQLite)]
```

### 3.1. The `AI` Component (The "Brain")

We will introduce a new, more detailed `AI` component in `src/core.h`.

```c
// In core.h
struct AI {
    // --- State & Personality ---
    int64_t personality_id; // The unique ID for this AI in the SQLite DB
    enum Faction faction;
    enum AIState current_state; // e.g., AI_STATE_IDLE, AI_STATE_ATTACKING
    EntityID current_target;

    // --- Decision Making ---
    float time_since_last_decision;
    float decision_interval; // How often this AI should "think"

    // --- Action Execution ---
    Vector3 move_target_position;
    Vector3 look_target_position;
    bool fire_weapon;
};
```

### 3.2. The Personality Database (SQLite)

A new database, `data/ai_personalities.db`, will be created. It will contain a single table, `personalities`, with the following schema:

*   `id` (INTEGER, PRIMARY KEY): The unique ID for the AI.
*   `faction` (TEXT): e.g., "Chrome-Baron", "Echo-Scribe".
*   `base_prompt` (TEXT): A detailed "system prompt" that defines the AI's core personality, goals, and rules of engagement.
*   `long_term_memory` (TEXT): A text blob where the AI can store key facts and memories (e.g., "The Drifter has attacked me twice.").

### 3.3. The AI System (`ai_system_update`)

This is the heart of the integration. It runs on the main game thread as part of our existing system scheduler.

1.  **Iterate Entities**: The system iterates through all entities with an `AI` component.
2.  **Check Decision Timer**: It checks if `time_since_last_decision` has exceeded the `decision_interval`.
3.  **Gather Context**: If it's time for a new decision, the system gathers all relevant information about the game world from the perspective of the AI entity. This includes:
    *   Its own health and status.
    *   The position and status of its current target (if any).
    *   The positions of any nearby allies or enemies.
    *   The player's current `ResonanceSignature`.
4.  **Format the Prompt**: The system formats this context into a concise, natural-language string.
5.  **Push to Task Queue**: It then pushes a "decision request" to a new, thread-safe task queue. This request contains the AI's `personality_id` and the formatted context prompt.
6.  **Execute Current State**: While waiting for a new decision, the AI continues to execute its *current* state (e.g., if its state is `AI_STATE_ATTACKING`, it will continue to set `fire_weapon = true`).

### 3.4. The LLM Service (`llama.cpp` Wrapper)

This will be a separate, long-running thread (or process) that is started when the engine launches.

1.  **Monitor Task Queue**: The service constantly monitors the task queue for new decision requests.
2.  **Retrieve Personality**: When a request is received, it uses the `personality_id` to query the SQLite database and retrieve the AI's `base_prompt` and `long_term_memory`.
3.  **Invoke `llama.cpp`**: It combines the base prompt, long-term memory, and the context from the game to create a final prompt, which is then passed to the `llama.cpp` inference engine.
4.  **Parse the Response**: The LLM will respond with a structured output (e.g., a simple JSON object) that defines the AI's new state and targets. Example:
    ```json
    {
      "new_state": "AI_STATE_FLEEING",
      "new_target_id": 0,
      "move_target_pos": {"x": 1500, "y": 200, "z": -4000},
      "speak_dialogue": "My shields are down! I need to get out of here!",
      "memory_update": "The Drifter's weapons are highly effective."
    }
    ```
5.  **Push to Result Queue**: The parsed result is pushed to a "result queue," tagged with the entity's ID.

### 3.5. Back to the AI System

On subsequent frames, the `ai_system_update` function will also check the result queue. If a result for one of its entities is available, it will:
1.  **Update Component**: Parse the result and update the entity's `AI` component with the new state, targets, etc.
2.  **Update Database**: If the result contains a `memory_update`, it will write this new information back to the `long_term_memory` field in the SQLite database for that AI.

## 4. Conclusion

This asynchronous, database-driven architecture provides a robust and scalable solution for integrating generative AI into our ECS-based engine. It keeps the high-latency work of the LLM off the main game thread, ensures that AI personalities are data-driven and easily modifiable, and provides a clear pathway for implementing complex, emergent behaviors for our factions. This is the technical blueprint for fulfilling the promise of a truly dynamic and intelligent game world.
