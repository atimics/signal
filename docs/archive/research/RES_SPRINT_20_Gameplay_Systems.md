# Research: Sprint 20 - Core Gameplay Systems (The Resonance Cascade)

**ID**: `RES_SPRINT_20`
**Status**: **PROPOSED**
**Author**: Gemini, Chief Science Officer
**Related Sprints**: `sprint_20_gameplay_systems.md`
**Related Documents**: `GAME_VISION.md`, `PLAYER_MANUAL.md`

## 1. Overview

This document provides the foundational research for implementing the core gameplay loop of "The Ghost Signal": the **Resonance Cascade**. This system is central to the player's experience, linking exploration, narrative discovery, and progression. It encompasses the mechanics of Resonance Cracking, Echoes as Keys, and the Resonance Signature.

## 2. Core Concepts from Game Vision

As defined in `GAME_VISION.md`:

### The Resonance Cascade (Core Loop)

This unified gameplay loop ties together exploration, puzzle-solving, combat, and narrative progression. It consists of three interlocking components:

*   **Resonance Cracking (The Action)**: The player's primary tool is the ship's **Resonance Attenuator**, used to "crack" **Resonance Locks** found on derelicts. This involves a skill-based minigame of matching waveforms.
*   **Echoes as Keys (The Strategy)**: Every Echo collected is both lore and a gameplay utility. Each Echo has a **Resonance Type** (e.g., `Engineering`, `Security`). Slotting an Echo into the ship's **Attunement Matrix** reveals nearby Resonance Locks of the corresponding type.
*   **The Resonance Signature (The Risk & Reward)**: Using the Ghost Signal comes at a price. As the player slots more (or more powerful) Echoes, their ship's **Resonance Signature** increases, attracting more dangerous enemies but also revealing higher-level locks.

### Schematic Fragmentation (Progression System)

Upgrades are reconstructed from ancient knowledge. Echoes contain **Schematic Fragments** for ship modules. Collecting multiple fragments allows the player to assemble new technology at a **Workbench**.

### Whispering Echoes (Atmospheric Feedback)

Brief, non-collectible, audio-visual ghostly events that deepen the atmosphere and provide subtle clues or warnings.

## 3. Research Focus Areas

### 3.1. Resonance Cracking Minigame Design

*   **Objective**: To ensure the minigame is engaging, skill-based, and avoids repetition.
*   **Considerations**:
    *   **Input Methods**: How will the player interact with the waveform? (e.g., joystick/mouse input for frequency/amplitude modulation).
    *   **Feedback**: Clear visual and auditory feedback for success/failure and proximity to target waveform.
    *   **Progression**: How will the minigame evolve? (e.g., increasing complexity, multiple waveforms, environmental hazards affecting the signal).
    *   **Integration with Lore**: How can the minigame visually and thematically tie into the Ghost Signal's nature (e.g., visual glitches, Aethelian symbols appearing).

### 3.2. Echo Data Structure and Management

*   **Objective**: To design a flexible and extensible data structure for Echoes that supports both narrative and gameplay functions.
*   **Considerations**:
    *   **Content**: What specific data points will each Echo contain? (e.g., `fragment_id`, `resonance_type`, `schematic_fragment_id`, `lore_text_id`, `audio_log_id`).
    *   **Storage**: How will Echoes be stored and retrieved? (e.g., in-memory registry, external database for persistence).
    *   **Linking**: How will Echoes link to the `Lore_Fragments.md` content and schematic fragments?

### 3.3. Resonance Signature Balancing

*   **Objective**: To ensure the risk-vs-reward system of the Resonance Signature is balanced and creates meaningful player choices.
*   **Considerations**:
    *   **Signature Calculation**: How will the signature level be calculated based on attuned Echoes? (e.g., additive, weighted average).
    *   **Enemy Response**: How will Wardens and Chrome-Barons react to different signature levels? (e.g., patrol changes, direct engagement, specific abilities).
    *   **Player Feedback**: Clear HUD indicators for current signature level and its implications.

## 4. Technical Considerations

*   **ECS Integration**: All new mechanics must be implemented as ECS components and systems, adhering to the data-oriented design principles.
*   **UI Integration**: The Resonance Cracking minigame and Attunement Matrix will require dedicated UI elements, likely implemented using Nuklear.
*   **Asset Pipeline**: Echoes and schematic fragments will need to be integrated into the asset pipeline for content creation and loading.

## 5. Next Steps

This research will directly inform the implementation tasks for Sprint 20. The focus will be on creating the core C-side data structures and systems, with initial placeholder UI and content to validate the mechanics.
