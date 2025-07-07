# CGame Vision Alignment Summary

This document clarifies the relationship between the various vision and design documents in the project.

---

## Current State vs. Aspirational Vision

### What's Actually Implemented (2025)

**CGame: Canyon Racing** - The current working game

* **Core Gameplay:** High-speed precision flying through canyon courses
* **Progression:** Medal-based track unlocking (Bronze → Silver → Gold → Platinum)
* **Challenges:** Environmental hazards (wind zones, fog banks, narrow passages)
* **Controls:** 6DOF flight controls optimized for racing (keyboard + gamepad)
* **Camera:** Multiple racing-optimized views (chase, cockpit, cinematic)
* **Audio:** Spatial audio for navigation and immersion

**Technical Foundation:**
* Sokol/C graphics engine
* Cross-platform (desktop focus)
* Robust input handling and physics
* Modular architecture supporting expansion

### What's Documented as Worldbuilding

**The Ghost Signal** - Rich background lore for inspiration

* **Purpose:** Provides atmospheric context and visual inspiration
* **Setting:** The Aethelian Fleet Graveyard with massive derelict ships
* **Aesthetics:** Five faction visual styles for ship skins and environments
* **Tone:** Haunted, mysterious space setting for immersive racing
* **Status:** Worldbuilding only - not a development target

---

## Document Breakdown

### Current Implementation Docs
* `PLAYER_MANUAL.md` - Documentation for the canyon racing game as it exists
* `RES_CANYON_RACING_UNIVERSE.md` - Procedural generation aligned with current racing focus

### Worldbuilding Docs  
* `GAME_VISION.md` - The Ghost Signal universe (worldbuilding inspiration only)
* `RES_TOPIC_UNIVERSE.md` - Procedural generation concepts (reference material)
* `RES_TOPIC_A_DRIVE.md` - Physics research (may inform racing mechanics)
* `fragment-001.md` & `ftl-system.md` - Lore documents (atmospheric inspiration)

---

## Development Strategy

### Immediate Focus (Spaceship Racing Evolution)
1. **Scale Up Environments:** Move from canyons to space environments around massive objects
2. **Signal-Based Objectives:** Time-limited targets that create dynamic racing routes
3. **Faction Ship Aesthetics:** Five visual ship variants inspired by worldbuilding
4. **A-Drive Surface Racing:** High-speed racing mechanics across large structures

### Core Concept: **Basic Spaceship Racing Game**
Players race spaceships around massive derelict structures, chasing time-limited signals. Simple, focused, and achievable scope that uses the worldbuilding for visual inspiration without complex narrative systems.

### Bridge Elements (Immediate Evolution)
1. **Massive Derelict Environments:** Scale up from canyons to huge derelict ship exteriors
2. **Signal-Based Racing:** Time-limited signals appear that must be intercepted quickly
3. **Dynamic Course Generation:** Race routes vary based on where signals appear
4. **Faction Ship Skins:** Different visual styles representing the five factions

### Refined Core Concept
**Signal Intercept Racing:** Players race across massive derelict ships to reach time-limited signals before they disappear. Each faction provides a different ship skin/aesthetic, but the core gameplay remains high-speed precision racing with the A-Drive surface-skimming mechanic.

---

## Technical Considerations

### Current Architecture Strengths
* **Modular Design:** Can support different game modes and mechanics
* **Performance Focus:** Optimized for smooth racing experience
* **Cross-platform:** Foundation suitable for various deployment targets
* **Expandable:** Input, audio, and rendering systems can grow

### Architecture Needs for Spaceship Racing
* **Space Environments:** Rendering large structures in space setting
* **Dynamic Objectives:** Time-based target spawning system
* **Ship Variety:** Multiple ship models with different aesthetics
* **Surface Racing:** Physics for racing across curved surfaces at high speed

---

## Recommendation

**Focus on Basic Spaceship Racing**

This practical approach provides:
1. **Achievable Scope:** Evolution of existing canyon racing to space setting
2. **Clear Goal:** Simple, fun spaceship racing game
3. **Visual Inspiration:** Use worldbuilding for atmosphere without complexity
4. **Technical Growth:** Builds naturally on current foundation
5. **Immediate Payoff:** Leverages proven racing mechanics

**Use Worldbuilding as Inspiration Only:**
* **Visual Design:** Faction aesthetics for ship variety
* **Environment Design:** Massive structures create interesting racing environments  
* **Atmospheric Tone:** Space setting with mysterious/haunted feel
* **No Complex Systems:** Keep narrative, survival, and exploration elements as background only

---

## Conclusion

**Basic Spaceship Racing Game** - this is the realistic, achievable target. Take the solid canyon racing foundation and evolve it into a space setting with dynamic objectives and visual variety.

**Key Focus:**
1. **Keep It Simple:** Racing mechanics work, just change the setting
2. **Visual Variety:** Use faction aesthetics for ship and environment diversity
3. **Live in the Moment:** Build what's achievable now, not distant aspirations
4. **Use Worldbuilding Wisely:** Draw inspiration for visuals and atmosphere only

The worldbuilding provides rich visual inspiration and atmosphere, but the game remains focused on what it does well: **fast, precise, engaging spaceship racing**.
