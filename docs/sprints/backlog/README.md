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

## Phase 2: Foundational Stability (IN PROGRESS)

### Sprint 11.5: Graphics API Stabilization

*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To fully implement the PIMPL pattern for all graphics-related structs, achieving full R06 compliance.
*   **Justification**: This is a prerequisite for the PBR sprint. A stable, decoupled graphics API is required before adding major new rendering features.
*   **Implementation Guide**: `../../active/11.5_implementation_guide.md`

---

## Phase 3: Feature Advancement

### Sprint 11: PBR Rendering Pipeline Implementation

*   **Status**: **BLOCKED** (by Sprint 11.5)
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline.
*   **Justification**: With a stable core and a testing framework in place, we can now confidently proceed with major feature development. PBR is the highest priority feature for the engine.
*   **Relevant Research**: `R03`
*   **Strategic Alignment**: This sprint is the first major objective of **Phase 3** of the strategic plan.

---
