# Core Gameplay Mechanics: The Resonance Cascade

**ID**: `R22_Resonance_and_Narrative`
**Author**: Gemini, Chief Science Officer
**Status**: **Active Design**
**Related Vision**: `R17_Gameplay_Vision`

## 1. The Problem: Narrative as a Reward vs. Narrative as a Tool

Our current design treats narrative as a reward. The player completes a gameplay challenge and receives a piece of lore as a prize. This is a one-way interaction.

To create a truly compelling loop, the narrative must become a tool. The information contained within a piece of lore must directly and mechanically alter how the player interacts with the world, unlocking new gameplay possibilities. The story must become a key that opens new doors, both literally and figuratively.

## 2. The Solution: The Resonance Cascade System

The **Resonance Cascade** is a new, unified system that directly links narrative discovery to gameplay action. It is composed of three interconnected parts: The Resonance Attenuator, Echoes as Keys, and Resonance Locks.

### 2.1. The Resonance Attenuator (The "How")

The player's ship is equipped with a **Resonance Attenuator**. This is the evolution of the "emitter" or "loom." It's a piece of hardware with a limited number of **"Attunement Slots."**

*   **Function**: The player can "slot" a recovered Echo (a piece of lore) into the Attenuator.
*   **Gameplay**: Attuning to an Echo calibrates the ship's sensors and the Attenuator's energy beam to that specific memory's frequency. **This is the core choice the player makes.** The Echo you have attuned determines what you can see and interact with in the world.

### 2.2. Echoes as Keys (The "What")

Every piece of lore—every audio log, data fragment, and AI personality—is now a physical **Echo** in the player's inventory. Crucially, each Echo has **gameplay properties**:

*   **Resonance Type**: The category of the memory. Examples: `Engineering`, `Medical`, `Security`, `AI Core`, `Civilian`.
*   **Data Yield**: A rating of how much "Harmony" (our progression metric) it will provide when fully resolved.
*   **Stability Cost**: How much energy it costs to keep this memory attuned in the Attenuator. More powerful or traumatic memories are more costly to handle.

### 2.3. Resonance Locks (The "Challenge")

The world is now filled with **Resonance Locks**—dormant, crystalline nodes that are keyed to specific Resonance Types.

*   **How it Works**: A Resonance Lock of the `Security` type will remain invisible and intangible *unless* the player has a `Security` Echo attuned in their Attenuator.
*   **The "Cascade"**: When the player attunes to a specific Echo (e.g., Chief Kaelen's security log), their scanner pings, and previously hidden `Security` Resonance Locks now appear on their HUD. This is the "cascade"—the act of finding one story beat reveals the path to the next.
*   **"Resonance Cracking" Minigame**: Activating a Resonance Lock is no longer just holding a beam. It initiates a skill-based minigame. The player is presented with a fluctuating waveform on their UI (the "lock"). They must use their controls to modulate their Attenuator's beam to match the frequency and amplitude of the lock's waveform. Successfully matching it "cracks" the lock and releases the next Echo.

## 3. A Concrete Gameplay Example

1.  **The Discovery**: The player finds **Fragment ID: PAL-088-F**, the audio log from Chief Engineer Lena Petrova about the ship's hull "resonating" and "growing."
2.  **The Tool**: The player now possesses an Echo with the `Engineering` Resonance Type.
3.  **The Choice**: The player returns to their ship and slots the "Petrova's Log" Echo into their Resonance Attenuator. Their HUD flickers, and the scanner is now calibrated to `Engineering` frequencies.
4.  **The Cascade**: As they fly back into the derelict, their scanner now highlights several previously invisible Resonance Locks on the ship's hull and near damaged power conduits.
5.  **The Challenge**: The player approaches one of these new `Engineering` locks. They initiate the **Resonance Cracking** minigame. Because they are using Petrova's Log, the target waveform is erratic, reflecting her panic. The player must skillfully match the volatile wave.
6.  **The Reward**: Upon successfully cracking the lock, it releases a new Echo: a **Schematic Fragment** for an "Ablative Hull Stabilizer" and another audio log from a different engineer that builds on Petrova's story.
7.  **The Loop Completes**: The player has used a piece of narrative as a key to unlock new gameplay, which in turn has rewarded them with both a new piece of narrative *and* a tangible gameplay upgrade.

## 4. Impact on Other Systems

*   **Exploration becomes purposeful**: Players are no longer just looking for "loot." They are looking for specific *types* of Echoes to unlock specific types of Resonance Locks.
*   **Factions become more dynamic**: The Chrome-Barons might use crude "Signal Breakers" to crudely activate locks, causing dangerous side effects. The Echo-Scribes might trade a rare `AI Core` Echo if the player can bring them three specific `Civilian` memory fragments.
*   **Progression is earned**: The player doesn't just find upgrades; they earn them by actively engaging with and solving the central mystery of the world.

This system creates a powerful, self-reinforcing loop where the story is the key to the gameplay, and the gameplay is the key to the story.