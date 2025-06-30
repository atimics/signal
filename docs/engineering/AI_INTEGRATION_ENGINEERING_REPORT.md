# AI Integration Engineering Report

**Report Date:** June 29, 2025
**Author:** Gemini
**Status:** Final

## 1. Executive Summary

This report provides a detailed architectural blueprint for integrating a high-performance, scalable AI system into the CGame engine. The core objective is to power a persistent universe with thousands of intelligent, dynamic entities by leveraging modern, lightweight language models running on-device.

The proposed architecture is a **hybrid model** that combines the creative, emergent decision-making of a Large Language Model (LLM) with the reliability and performance of traditional game AI techniques (Behavior Trees). The LLM will serve as the "brain" or "strategy layer," deciding *what* an entity should do, while a data-driven Behavior Tree will act as the "nervous system," executing that decision with frame-by-frame precision.

To achieve the required performance, the entire stack is designed to be **C/C++ native and asynchronous**. Expensive inference tasks will be offloaded to a dedicated worker thread, ensuring the main render loop remains non-blocking and maintains a consistent 60fps. This document outlines the chosen technology stack, the detailed data flow of the AI pipeline, and a phased implementation plan.

## 2. Core Philosophy

The AI system is guided by four key principles:

1.  **Native Performance:** The entire stack is C/C++ native to eliminate cross-language overhead, minimize dependencies, and achieve the highest possible performance on the target hardware.
2.  **Edge-Optimized:** The system is designed to run small, efficient language models (like Google's Gemma) directly on the user's CPU, avoiding reliance on cloud services and ensuring a responsive, self-contained experience.
3.  **Scalability:** The architecture must support thousands of concurrent AI agents. This is achieved through an asynchronous job system and Level of Detail (LOD) for AI processing, where an entity's decision-making frequency is based on its relevance to the player.
4.  **Data-Driven Design:** AI personalities, behaviors, and decision logic are defined in external data files (JSON), allowing designers and writers to craft and iterate on AI without touching the core engine code.

## 3. Proposed Native Stack

-   **Inference Engine: `gemma.cpp`**
    -   **Rationale:** A lightweight, standalone C++ inference engine for Gemma models. Its minimal dependencies, focus on CPU performance, and direct compatibility with the `ggml` tensor library make it the ideal choice for our native stack. It is a significant step up from the more general-purpose `llama.cpp`.
-   **Tensor Library: `ggml`**
    -   **Rationale:** The underlying tensor library for `gemma.cpp`. It provides the highly optimized mathematical operations required for neural network inference in a C-native environment.
-   **Threading & Synchronization: C11 `threads.h`**
    -   **Rationale:** Using the C11 standard library for threads, mutexes, and condition variables ensures maximum portability across macOS, Linux, and Windows, avoiding platform-specific APIs like pthreads or Win32 threads.
-   **Behavior Execution: Custom Behavior Trees**
    -   **Rationale:** A simple, data-driven Behavior Tree (BT) system provides a reliable and well-understood method for executing the high-level goals decided by the LLM. This prevents the LLM from having direct, unpredictable control over an entity's frame-to-frame actions.

## 4. Detailed Architectural Breakdown: The AI Decision Lifecycle

The process of an entity making and acting on a decision is broken down into six distinct stages:

#### Stage 1: AI Stimulus & Task Generation

An AI decision process doesn't run constantly for every entity. It is triggered by a "stimulus."
-   **Triggers:**
    -   **Player Proximity:** An entity's AI update frequency increases as it gets closer to the player (LOD).
    -   **Game Events:** A combat system event (`EVENT_TOOK_DAMAGE`), a dialog system event (`EVENT_PLAYER_INTERACT`), or a world event.
    -   **Scheduled Updates:** A low-frequency timer for background entities to make decisions about their routine (e.g., "patrol," "idle").
-   **Task Creation:** When triggered, the relevant game system (e.g., `ai_system_update`) creates an `AITask` struct. This struct contains the `entity_id` and the `stimulus_type`.

#### Stage 2: The Asynchronous Job System

To avoid blocking the main thread, the task is submitted to a thread-safe queue.
-   **Producer (Main Thread):** Pushes the `AITask` to a global task queue. This is a fire-and-forget operation.
-   **Consumer (AI Worker Thread):** A dedicated thread, created at startup, waits on this queue. When a task appears, it wakes up, processes it, and goes back to sleep. This ensures the expensive work happens concurrently with rendering.

#### Stage 3: Dynamic Prompt Engineering

This is the most critical stage for generating high-quality AI behavior. The AI worker thread takes the simple `AITask` and constructs a detailed, context-aware prompt for the LLM.
-   **Components of the Prompt:**
    1.  **Base Prompt (Personality):** Loaded from the entity's template file (e.g., `"You are a cynical, veteran cargo pilot..."`).
    2.  **Current State:** Key-value pairs describing the entity's current situation (e.g., `"health: 50%", "location: spaceport_docking_bay"`).
    3.  **Recent Events:** A short log of recent stimuli (e.g., `"log: took damage from player, heard explosion"`).
    4.  **The "Question":** The specific decision to be made, based on the stimulus (e.g., `"Question: A player is hailing you. What is your immediate response?"`).
    5.  **Output Formatting Instructions:** A crucial instruction to the LLM to format its output in a structured way (see next stage).

#### Stage 4: Inference with `gemma.cpp`

The worker thread passes the fully constructed prompt to the `gemma_generate()` function. This is the long-running, computationally expensive step where the LLM generates a response.

#### Stage 5: Parsing the Result

The raw text output from the LLM is not directly usable. It must be parsed into a structured format. By instructing the LLM in the prompt to respond in a specific format (e.g., JSON), we can make this process reliable.

-   **LLM Output Example:**
    ```json
    {
      "decision": "Engage in combat.",
      "action": "TRIGGER_BEHAVIOR_TREE",
      "behavior_tree_name": "bt_combat_strafe",
      "target_entity_id": 123,
      "dialog_response": null
    }
    ```
-   **Parsing:** The worker thread parses this JSON into an `AIResult` struct.

#### Stage 6: Behavior Execution (Behavior Trees)

The `AIResult` is pushed to a thread-safe results queue, which is consumed by the main thread.
-   **Applying the Result:** A system on the main thread (`ai_results_system_update`) checks this queue each frame. When a result appears, it doesn't directly execute the action. Instead, it sets the high-level goal for the entity.
-   **Triggering the Behavior Tree:** The `action` from the result (`TRIGGER_BEHAVIOR_TREE`) tells the entity's `AI` component to switch its active behavior tree to `bt_combat_strafe`.
-   **Frame-by-Frame Execution:** The Behavior Tree system then takes over, running every frame to handle the low-level details of combat: turning towards the target, firing weapons, managing shields, etc. This ensures that the entity's actions are reliable, performant, and predictable from a gameplay perspective.

## 5. Implementation Roadmap

This architecture can be implemented incrementally:

1.  **Phase 1: The Job System (Sprint 14):** Implement the multi-threaded foundation with thread-safe queues and a simple placeholder task that performs a `sleep()` to simulate inference workload.
2.  **Phase 2: `gemma.cpp` Integration:** Integrate the `gemma.cpp` library and replace the placeholder task with actual inference calls.
3.  **Phase 3: Prompt & Parsing:** Develop the dynamic prompt engineering system and the JSON parsing for results.
4.  **Phase 4: Behavior Trees:** Implement the data-driven Behavior Tree system that reads BT definitions from files and executes the AI's chosen strategy.

This phased approach will allow for the systematic construction of a sophisticated, high-performance AI system that is a core pillar of the CGame engine's vision.
