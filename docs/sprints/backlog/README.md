# CGame Engine - Development Backlog

**Last Updated**: June 30, 2025
**Backlog Manager**: Gemini (Lead Scientist and Researcher)

## Overview

This document outlines the prioritized development sprints for the CGame engine, guided by the official [**Strategic Plan**](../../STRATEGIC_PLAN.md). The backlog is actively managed to reflect the current state of the codebase and strategic priorities. Sprints are ordered based on dependency and foundational importance.

---

## Phase 1: Core System Repair

### Sprint 10.5: Critical Mesh System Architecture Repair

*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To fix the fundamental architectural disconnect between the asset pipeline and the rendering system.
*   **Justification**: This is the **highest priority** for the project. No other rendering work can proceed until the engine can reliably display a 3D model.
*   **Relevant Research**: `R01`
*   **Strategic Alignment**: This sprint is the sole focus of **Phase 1** of the strategic plan.

---

## Phase 2: Foundational Stability

### Sprint 15: Automated Testing Framework Integration

*   **Status**: **BLOCKED** (by Sprint 10.5)
*   **Goal**: To integrate a C unit testing framework and establish a strategy for unit, integration, and rendering tests.
*   **Justification**: A testing framework is a foundational capability that will de-risk all future development and improve code quality. This should be implemented immediately after the mesh system is repaired.
*   **Relevant Research**: `R02`
*   **Strategic Alignment**: This sprint is the sole focus of **Phase 2** of the strategic plan.

---

## Phase 3: Feature Advancement

### Sprint 11: PBR Rendering Pipeline Implementation

*   **Status**: **BLOCKED** (by Sprints 10.5 and 15)
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline.
*   **Justification**: PBR is the industry standard for realistic rendering and is the next major step in achieving modern visual fidelity.
*   **Relevant Research**: `R03`
*   **Strategic Alignment**: This sprint is the first major objective of **Phase 3** of the strategic plan.

---
