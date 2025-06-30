# CGame Engine - Research Library

**Lead Scientist**: Gemini

## 1. Overview

This library is the intellectual core of the CGame engine project. It contains a curated collection of research briefs, architectural proposals, and strategic analyses that guide our technical decision-making. Its purpose is to ensure that our development is not just functional, but is built upon a foundation of robust, scalable, and industry-standard principles.

For all developers, both human and AI, this library should be the first point of reference when tackling a new system or refactoring an existing one. The documents herein represent the "why" behind our "what."

---

## 2. Core Systems

This area covers the fundamental architecture of the engine's simulation and world structure.

*   **[R16: Scene Management](./core_systems/R16_Scene_Management.md)**
    *   **Synopsis**: A critical analysis of our initial, text-based scene definition system and a detailed proposal for a high-performance, binary-driven scene graph architecture.
    *   **Strategic Importance**: **High**. This document outlines the necessary evolution of our world representation to support complex, hierarchical objects and large-scale environments. It is the blueprint for our future world-building capabilities.

*   **[R09: Physics and Collision](./core_systems/R09_Physics_and_Collision_Systems.md)**
    *   **Synopsis**: An investigation into real-time physics simulation, covering topics from simple bounding-box collision to more complex physics middleware integration.
    *   **Strategic Importance**: **Medium**. While our current physics is simple, this research will be critical when we move to more dynamic and interactive game worlds.

*   **[R10: Audio System Architecture](./core_systems/R10_Audio_System_Architecture.md)**
    *   **Synopsis**: A forward-looking plan for a robust audio engine, including 3D spatial audio, event-driven sound effects, and streaming for music.
    *   **Strategic Importance**: **Low**. A foundational topic, but not on the critical path for our current rendering-focused milestones.

*   **[R11: Skeletal Animation](./core_systems/R11_Skeletal_Animation_System.md)**
    *   **Synopsis**: A detailed breakdown of the requirements for a skeletal animation system, including rigging, skinning, and animation state machines.
    *   **Strategic Importance**: **Medium**. Essential for bringing characters and dynamic objects to life, this will become a priority after the core rendering and world systems are finalized.

---

## 3. Rendering & Graphics

This section contains research focused on visual fidelity and rendering performance.

*   **[R06: C Header Architecture](./rendering/R06_C_Header_Architecture.md)**
    *   **Synopsis**: An essential guide to maintaining a clean and decoupled architecture by using opaque pointers (PIMPL) to hide low-level graphics library details from public engine headers.
    *   **Strategic Importance**: **High**. Adherence to this guide is mandatory. It was instrumental in resolving the dependency issues in Sprint 11.5 and is key to our long-term code health.

*   **[R12: Advanced Lighting and Shadows](./rendering/R12_Advanced_Lighting_and_Shadows.md)**
    *   **Synopsis**: A forward-looking document that explores the techniques required for a modern lighting pipeline, including shadow mapping, HDR, and global illumination.
    *   **Strategic Importance**: **High**. This research directly informs the implementation of our PBR pipeline (Sprint 14) and our long-term visual goals.

---

## 4. Asset Pipeline & Tooling

This area covers the offline tools and processes used to create and optimize game-ready assets.

*   **[R15: Binary Asset Format](./assets/R15_Binary_Asset_Format.md)**
    *   **Synopsis**: A specification for the high-performance, binary `.cobj` format. It details the file header, vertex layout, and the rationale for moving away from text-based formats.
    *   **Strategic Importance**: **High**. This document was the blueprint for the successful asset pipeline overhaul in Sprint 12. It is the foundation of our fast loading times.

*   **[R13: Developer Tooling & Editor Strategy](./tooling/R13_Developer_Tooling_and_Editor_Strategy.md)**
    *   **Synopsis**: An analysis of the long-term vision for CGame's development tools, including the potential for an integrated, Dear ImGui-based editor.
    *   **Strategic Importance**: **Medium**. While not on the immediate roadmap, this document provides a valuable long-term vision that should inform the design of our current, smaller-scale tools.

---

## 5. Gameplay & Narrative

This section defines the creative vision and player experience for "The Ghost Signal." It is organized into four core documents that provide a complete overview of the game's design.

*   **[01: The Vision](./gameplay_and_narrative/01_Vision_The_Ghost_Signal.md)**
    *   **Synopsis**: The official, high-level design document for "The Ghost Signal." It outlines the core pillars of the game: desperate survival, exploration of derelict alien fleets, and the central mystery of the Ghost Signal. This is the "What" of our game.

*   **[02: The World Bible](./gameplay_and_narrative/02_World_Bible.md)**
    *   **Synopsis**: The single source of truth for the game's lore. It details the history of the Aethelian Fleet, the nature of the Cataclysm, the motivations of the AI factions, and the player's place in the universe. This is the "Where" of our game.

*   **[03: The Core Mechanics](./gameplay_and_narrative/03_Core_Mechanics.md)**
    *   **Synopsis**: A detailed design document that defines the core gameplay loop. It introduces the "Resonance Cascade" system, where narrative fragments (Echoes) act as keys to unlock new gameplay opportunities and challenges. This is the "How" of our game.

*   **[04: The Lore Fragments](./gameplay_and_narrative/04_Lore_Fragments.md)**
    *   **Synopsis**: A living repository of all the in-game text fragments, audio logs, and corrupted data that players can discover. This is the primary vehicle for delivering our emergent, non-linear narrative. This is the "Content" of our game.

## 6. Artificial Intelligence

This section details our ambitious plans for a generative, swarm-based AI system.

*   **[R07: Generative AI Feedback](./ai/R07_Generative_AI_Feedback.md)**
    *   **Synopsis**: An early exploration of how large language models (LLMs) can be integrated into the engine to provide dynamic, emergent behavior for NPCs.
    *   **Strategic Importance**: **High (Long-Term)**. This research is the foundation for one of the engine's key unique selling propositions. It directly informs the planning for the AI Foundation sprint (Sprint 15).
