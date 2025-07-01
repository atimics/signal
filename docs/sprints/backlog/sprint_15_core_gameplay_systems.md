# Sprint 15: Core Gameplay Systems - The Resonance Cascade

**ID**: `sprint_15.0`
**Status**: **BACKLOG**
**Author**: Gemini, Chief Science Officer
**Related Design Doc**: `docs/research/gameplay_and_narrative/03_Core_Mechanics.md`

## 1. Sprint Goal

To implement the foundational C-side systems required for the **Resonance Cascade** gameplay loop. This sprint focuses on creating the data structures and core logic that will allow the player to use Echoes as "keys" to interact with the world. This is the first major step in translating our design documents into a playable experience.

## 2. Implementation Plan

This sprint is broken down into three main tasks, each focused on a key component of the new system.

### Task 1: The Echo Component

*   **Objective**: To create a new `Echo` component that can store the gameplay-critical data for a lore fragment.
*   **Guidance**:
    1.  In `src/core.h`, define a new `ComponentType` flag: `COMPONENT_ECHO`.
    2.  Define a new `struct Echo` that contains:
        *   An `enum ResonanceType` (e.g., `RESONANCE_ENGINEERING`, `RESONANCE_SECURITY`, etc.).
        *   A `float stability_cost`.
        *   A unique `char fragment_id[64]` to link it to the lore in the `04_Lore_Fragments.md` document.
    3.  Add the `Echo` component to the `ComponentPools` and the main `Entity` struct.

### Task 2: The Resonance Attenuator

*   **Objective**: To create the player-facing system for managing attuned Echoes. This will be a new component on the player's ship.
*   **Guidance**:
    1.  In `src/core.h`, define a new `ComponentType` flag: `COMPONENT_ATTENUATOR`.
    2.  Define a new `struct Attenuator` that contains:
        *   An array of `EntityID`s representing the "slotted" Echoes.
        *   An integer for the number of available slots.
        *   The current total `ResonanceSignature` value.
    3.  Add the `Attenuator` component to the `ComponentPools` and the main `Entity` struct.
    4.  Create a new system, `attenuator_system_update()`, that calculates the total `ResonanceSignature` based on the slotted Echoes.

### Task 3: The Resonance Lock

*   **Objective**: To create the world-space objects that the player interacts with using the Resonance Cascade system.
*   **Guidance**:
    1.  In `src/core.h`, define a new `ComponentType` flag: `COMPONENT_RESONANCE_LOCK`.
    2.  Define a new `struct ResonanceLock` that contains:
        *   An `enum ResonanceType` that this lock is keyed to.
        *   A boolean `is_visible`.
    3.  Create a new system, `resonance_system_update()`, that runs every frame. This system will:
        *   Get the player's currently attuned `ResonanceType` from their `Attenuator`.
        *   Iterate through all entities with a `ResonanceLock` component.
        *   Set the `is_visible` flag on the lock to `true` if its type matches the player's attuned type, and `false` otherwise. The rendering system will later use this flag to show/hide the locks.

## 3. Definition of Done

1.  All new components (`Echo`, `Attenuator`, `ResonanceLock`) are defined and integrated into the ECS.
2.  The `attenuator_system_update` correctly calculates the player's Resonance Signature.
3.  The `resonance_system_update` correctly updates the visibility of Resonance Locks based on the player's attuned Echoes.
4.  The project compiles successfully (`make`), and all existing tests pass (`make test`).
5.  A new test suite, `tests/test_resonance.c`, is created with unit tests that validate the logic of the new systems.
