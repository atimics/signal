# CGame Engine - Development Backlog

**Last Updated**: June 30, 2025
**Backlog Manager**: Gemini (Lead Scientist and Researcher)

## Overview

This document outlines the prioritized development sprints for the CGame engine, guided by the official [**Strategic Plan**](../../STRATEGIC_PLAN.md). The backlog is actively managed to reflect the current state of the codebase and strategic priorities. Sprints are ordered based on dependency and foundational importance.

---

## Phase 1 & 2: Core System Repair & Stabilization (COMPLETED)

### Sprint 10.5: Critical Mesh System Architecture Repair
*   **Status**: **✅ COMPLETED**

### Sprint 11.7: Shader Unification
*   **Status**: **✅ COMPLETED**

### Sprint 12.0: Rendering & Asset Pipeline Stabilization
*   **Status**: **✅ COMPLETED**
*   **Outcome**: The asset pipeline has been successfully migrated to a high-performance binary format. The engine can now reliably discover, load, and render textured 3D meshes. This resolves all critical rendering blockers.

---

## Phase 3: Performance & Feature Advancement (IN PROGRESS)

### Sprint 13: Performance Optimization & Code Style
*   **Status**: **✅ COMPLETED**
*   **Outcome**: Foundational performance optimizations and code style standards have been implemented.

---

## Phase 3: Performance & Feature Advancement (IN PROGRESS)

### Sprint 14: Mesh Generator Pipeline Integration
*   **Status**: **✅ COMPLETED**
*   **Outcome**: The procedural mesh generator is now fully integrated with the modern, binary-first asset pipeline.

---

## Phase 3: Performance & Feature Advancement (IN PROGRESS)

### Sprint 15: Scene Graph & Culling
*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To evolve the engine from rendering a flat list of entities to a modern, hierarchical Scene Graph architecture, and to leverage this for view frustum culling.
*   **Implementation Guide**: `../../active/sprint_15_scene_graph.md`

### Sprint 16: PBR Rendering Pipeline Implementation
*   **Status**: **BLOCKED** (by Sprint 15)
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline.
*   **Justification**: A scene graph is a prerequisite for a robust PBR system, as it allows for complex scenes with multiple light sources and objects.
*   **Relevant Research**: `R03`, `R14`

### Sprint 17: Core Gameplay Systems - The Resonance Cascade
*   **Status**: **BACKLOG**
*   **Goal**: To implement the foundational C-side systems required for the Resonance Cascade gameplay loop.
*   **Implementation Guide**: `sprint_15_core_gameplay_systems.md`

---

## Phase 4: AI & World Simulation

### Sprint 18: AI Foundation (Proof of Concept)
*   **Status**: **BACKLOG**
*   **Goal**: To create a "single agent" proof-of-concept by integrating the core technologies needed for the generative swarm AI system.
*   **Justification**: This R&D sprint will de-risk the immense technical challenges of the AI vision by solving the core integration problems on a small scale before building the full swarm infrastructure.
*   **Tasks**:
    1. Integrate the **SQLite** library for database operations.
    2. Integrate the **`llama.cpp`** library for flexible, multi-model inference (Gemma, Llama, etc.).
    3. Create a test program that loads a character prompt from a `.db` file, passes it to `llama.cpp` using a Gemma-2B model, and prints the response.
*   **Relevant Research**: `R07`, `R25`
