# AI System Architecture: Generative Swarm Simulation

**Status**: Current | **Last Updated**: June 30, 2025

## 1. Core Philosophy: A Universe of Generative Agents

The CGame AI system is designed to simulate a universe populated by potentially vast numbers of autonomous, intelligent agents. The core philosophy moves beyond traditional state machines and behavior trees to a **generative agent** model. Each entity is not merely a collection of stats but a unique "character" whose personality, memories, and decisions are driven by a language model.

The goal is to create an emergent, dynamic universe where complex narratives and behaviors arise naturally from the interactions of these agents.

## 2. The Three Pillars of a Generative Agent

Every AI-driven entity in the engine is built upon three core components:

### Pillar 1: The Character as a Prompt

The fundamental state of an agent is its **character prompt**. This is a living document, a dynamically updated text prompt that encapsulates the agent's identity, goals, traits, and current state.

*   **Example Character Prompt:**
    ```
    You are Jax, a cynical but skilled cargo pilot of the freighter "Stardust Drifter".
    **Traits**: Pragmatic, loyal to your crew, distrustful of authority.
    **Current Goal**: Transport a shipment of medical supplies to the Cygnus X-1 colony. You are two days behind schedule.
    **Current State**: You are docked at Starbase 7, negotiating for fuel. Your ship's shields are at 55%.
    **Recent Memories**:
    - You narrowly escaped a pirate ambush in the asteroid belt.
    - You had a tense conversation with a corrupt port official who demanded a bribe.
    ```

### Pillar 2: The Evolving Memory Stream

An agent's memory is not just a list of facts but an evolving part of its character. New experiences are summarized and appended to the "Recent Memories" section of the character prompt. This ensures that an agent's future decisions are influenced by its past experiences, allowing for character growth and dynamic relationships.

### Pillar 3: The Hierarchical Agent-Tool-System Loop

Agents interact with the game world through a hierarchical, tool-using loop. This prevents the LLM from needing to control low-level actions and allows for scalable, high-level decision-making.

The loop works as follows:
1.  **Agent (The "Mind")**: The highest level is the generative agent, powered by an inference engine (e.g., `llama.cpp`). Its role is to make high-level decisions based on its character prompt.
2.  **Tools (The "Hands")**: The agent does not directly control the ship or its inventory. Instead, it has access to a set of "tools" (functions) that it can call. These tools are exposed to the LLM, often via a JSON or function-calling schema.
    *   `set_destination(system, planet)`
    *   `negotiate_trade(item, quantity, price)`
    *   `send_hail(ship_id, message)`
    *   `query_ship_status()`
3.  **Systems (The "World")**: The tools, when called by the agent, interact with the core engine systems (e.g., the physics system, the ECS). The `set_destination` tool, for example, would update the `Navigation` component for the agent's entity, which the `physics_system` would then act upon.

This creates a clean separation: the LLM is for **cognition**, and the engine systems are for **action**.

## 3. Technical Implementation

### Inference Engine

*   **Engine**: The system is designed to integrate directly with a C-based inference engine, with `llama.cpp` being the primary reference implementation. This allows for local, private, and high-performance inference.
*   **Inference LOD (Level of Detail)**: It is computationally infeasible to run inference for millions of agents simultaneously. The system will use an aggressive LOD approach:
    *   **Active Agents**: Agents directly interacting with the player (e.g., in dialog) or involved in critical, on-screen events will have their inference requests processed in real-time or near real-time.
    *   **Passive Agents**: Agents that are "off-screen" or not engaged in complex tasks will have their state updated via much simpler, non-LLM heuristics or have their "thinking" batched and processed at a very low frequency (e.g., once every few minutes).
*   **Asynchronous Processing**: All LLM inference calls will be handled by a dedicated, asynchronous task scheduler to prevent them from blocking the main game loop.

This architecture provides a scalable, powerful, and uniquely dynamic foundation for the AI in CGame.
