# CGame Engine - Development Backlog

**Last Updated**: June 30, 2025
**Backlog Manager**: Gemini (Lead Scientist and Researcher)

## Overview

This document outlines the prioritized development sprints for the CGame engine, guided by the official [**Strategic Plan**](../../STRATEGIC_PLAN.md). The backlog is actively managed to reflect the current state of the codebase and strategic priorities. Sprints are ordered based on dependency and foundational importance.

---

## Phase 1: Core System Repair (COMPLETED)

### Sprint 10.5: Critical Mesh System Architecture Repair

*   **Status**: **✅ COMPLETED**
*   **Outcome**: The core asset and rendering pipeline has been successfully repaired and stabilized. See the [Sprint 10.5 Review](../../completed/10_5_sprint_review.md) for full details.

### Sprint 11.7: Shader Unification

*   **Status**: **✅ COMPLETED**
*   **Outcome**: The rendering pipeline now uses a single, unified set of shaders, resolving the hardcoded shader conflict.

---

## Phase 2: Foundational Stability (IN PROGRESS)

### Sprint 12.0: Mesh Rendering Stabilization & Optimization

*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To achieve feature parity with the `mesh_viewer` tool by fixing all mesh rendering artifacts, including distorted geometry and missing textures. This sprint will also implement foundational performance optimizations and complete the outstanding technical debt from Sprint 11.6.
*   **Implementation Guide**: `../../active/12.0_mesh_rendering_stabilization.md`

---

## Phase 3: Feature Advancement

### Sprint 11: PBR Rendering Pipeline Implementation

*   **Status**: **BLOCKED** (by Sprint 12.0)
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline.
*   **Justification**: With a stable core and a testing framework in place, we can now confidently proceed with major feature development. PBR is the highest priority feature for the engine.
*   **Relevant Research**: `R03`, `R14`

---

## Phase 4: AI & World Simulation

### Sprint 13: AI Foundation (Proof of Concept)

*   **Status**: **BACKLOG**
*   **Goal**: To create a "single agent" proof-of-concept by integrating the core technologies needed for the generative swarm AI system.
*   **Justification**: This R&D sprint will de-risk the immense technical challenges of the AI vision by solving the core integration problems on a small scale before building the full swarm infrastructure.
*   **Tasks**:
    1. Integrate the **SQLite** library for database operations.
    2. Integrate the **`llama.cpp`** library for flexible, multi-model inference (Gemma, Llama, etc.).
    3. Create a test program that loads a character prompt from a `.db` file, passes it to `llama.cpp` using a Gemma-2B model, and prints the response.
*   **Relevant Research**: `R07`, `R08`