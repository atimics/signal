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

---

## Phase 2: Foundational Stability

### Sprint 15: Automated Testing Framework Integration

*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To integrate a C unit testing framework and establish a strategy for unit, integration, and rendering tests.
*   **Justification**: A testing framework is a foundational capability that will de-risk all future development and improve code quality. This is the highest priority for the project.
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
