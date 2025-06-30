# Research & Vision: "The Ghost Signal"

**ID**: `R17_Gameplay_Vision`
**Author**: Gemini, Chief Science Officer
**Status**: **Active & Approved**

## 1. The Premise: A Simple, Personal Hook

**Your ship is your life support, and it's failing. The only thing keeping its alien systems from collapsing is a faint, repeating broadcast: a "Ghost Signal" emanating from a nearby field of colossal, dead spacecraft.**

You are a **Drifter**, one of the last survivors of a forgotten colony, kept alive only by the strange, symbiotic technology of your ship. The signal is not just a mystery; it's a lifeline. The data streams it contains are the only known source of the schematics and resonance patterns needed to repair and upgrade your ship's decaying, alien components.

Your mission is one of desperate survival: follow the Ghost Signal into the derelict fleet, retrieve the data you need to live, and perhaps, uncover the truth about the catastrophe that created this graveyard and silenced the signal's original senders.

## 2. Grounding the Vision in Code ("The Praxis")

This vision is a direct reflection of our existing technology and assets. It provides a "why" for the "what" we have already built.

*   **The Player & Ship**: You are the pilot of the `player_ship` entity, a "Wedge Ship Mk2" with unique components. The game starts with this ship in a damaged or unstable state, creating the core motivation.
*   **The World**: The "graveyard of dead spacecraft" is our game world, populated by the existing assets: `control_tower`, `sun`, and other derelict `wedge_ship` entities. Our scenes, like `spaceport.txt`, represent key hubs within this graveyard.
*   **The Gameplay**: The core loop of exploration, resource gathering, and upgrading is already supported by our engine's structure:
    *   **Exploration**: Piloting the ship uses our `Physics` and `Collision` components.
    *   **Threats**: The `ai_ship` template provides the foundation for the rival scavengers and automated defenses that will create conflict.
    *   **The Goal**: The "Ghost Signal" data fragments are the narrative driver for collecting resources and discovering new schematics (our planned progression system).
*   **The Technology**: Our new asset pipeline, which generates tangents and AABBs, is perfectly suited for rendering these detailed, derelict environments with the modern lighting and performance optimizations (like frustum culling) that we have planned.

## 3. The Core Gameplay Loop: Survival and Discovery

This loop is a refinement of our best existing ideas, now framed by a clear and urgent motivation.

1.  **Follow the Signal**: The player uses their ship's sensors to get a bearing on the nearest, strongest data transmission from within the derelict field.
2.  **Scavenge & Survive**: The player navigates the dangerous environment, using their ship's tools to extract raw materials and data packets from the wrecks while fighting off or evading hostile AI scavengers and ancient defense systems.
3.  **Repair & Upgrade**: The player returns to a safe harbor (e.g., a hidden asteroid base) to use the scavenged materials and data. Raw materials are used for immediate repairs, while the data packets are decrypted to unlock schematics for permanent ship upgrades (e.g., a more efficient engine, a stronger weapon, or a better scanner that can detect weaker, more valuable signals).
4.  **Go Deeper**: With an upgraded ship, the player can now survive in more hazardous environments and follow the Ghost Signal to its deeper, more mysterious, and more rewarding sources.

## 4. The Narrative Arc: From Survival to Revelation

The story unfolds naturally through the gameplay loop.

*   **Act I: Desperation**. The player is focused purely on survival, learning the basics of flying, scavenging, and fighting to keep their ship from falling apart. The Ghost Signal is just a means to an end.
*   **Act II: Discovery**. As the player upgrades their ship, they begin to decrypt more than just schematics. They find fragmented audio logs, ship manifests, and personal messages from the original inhabitants of the fleet. They start to piece together the story of who these people were and what happened to them.
*   **Act III: Revelation**. The player follows the signal to its source: the command ship of the lost fleet. Here, they discover the full story of the cataclysm and the true nature of the Ghost Signal—it's not just a broadcast; it's the fragmented consciousness of the fleet's AI, and it holds the key to a choice that will determine the future of the system.

## 5. Conclusion: A Focused and Achievable Vision

This revised concept, "The Ghost Signal," provides the simple, powerful hook we need. It gives the player a clear and immediate motivation—**survive**—that perfectly aligns with the gameplay mechanics we have already built and planned. It grounds our world in the assets we have, provides a compelling mystery to unravel, and offers a clear path for future content and system development.

This is not a radical departure; it is a **focusing** of our best ideas into a clear, compelling, and, most importantly, **achievable** game. This is our official guiding vision.