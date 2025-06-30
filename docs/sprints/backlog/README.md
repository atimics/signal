# CGame Engine - Development Backlog

**Last Updated**: June 30, 2025
**Backlog Manager**: Gemini Assistant

## Overview

This document outlines the prioritized development sprints for the CGame engine. The backlog is actively managed to reflect the current state of the codebase and strategic priorities. Sprints are ordered based on dependency and foundational importance. Each sprint is supported by one or more research documents, which are stored in the central `docs/research/` directory.

---

## 1. Sprint 10.5: Critical Mesh System Architecture Repair

*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To fix the fundamental architectural disconnect between the asset pipeline and the rendering system, enabling the engine to load and render compiled mesh files.
*   **Justification**: This is the **highest priority** task for the entire project. No other rendering work can proceed until the engine can reliably display a 3D model. The current system is completely broken and falls back to a test triangle.
*   **Sprint Plan**: [./../active/10_5_mesh_system_repair.md](./../active/10_5_mesh_system_repair.md)
*   **Implementation Guide**: [./../active/10_5_mesh_system_repair_code_review.md](./../active/10_5_mesh_system_repair_code_review.md)
*   **Relevant Research**: `R01`

---

## 2. Sprint 15: Automated Testing Framework Integration

*   **Status**: **BACKLOG - HIGH PRIORITY**
*   **Goal**: To integrate a C unit testing framework and establish a strategy for unit, integration, and rendering tests.
*   **Justification**: The project currently has zero automated tests, making it fragile and difficult to refactor. A testing framework is a foundational capability that will de-risk all future development, improve code quality, and is a prerequisite for a reliable CI/CD pipeline. This should be implemented immediately after the mesh system is repaired.
*   **Sprint Plan**: This is currently a research task that will produce a full sprint plan.
*   **Relevant Research**: `R02`

---

## 3. Sprint 11: PBR Rendering Pipeline Implementation

*   **Status**: **BLOCKED** (by Sprint 10.5)
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline, including advanced materials, shaders, and lighting.
*   **Justification**: PBR is the industry standard for realistic rendering. Implementing this feature is the next major step in achieving modern visual fidelity for the CGame engine. It will unlock the potential of the asset pipeline and enable the creation of high-quality, professional-grade assets.
*   **Sprint Plan**: [./11_pbr_rendering_pipeline.md](./11_pbr_rendering_pipeline.md)
*   **Relevant Research**: `R03`

---
