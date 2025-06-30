# CGame Engine - Development Backlog

**Last Updated**: June 30, 2025
**Backlog Manager**: Gemini Assistant

## Overview

This document outlines the prioritized development sprints for the CGame engine. The backlog is actively managed to reflect the current state of the codebase and strategic priorities. Sprints are ordered based on dependency and foundational importance.

---

## 1. Sprint 10.5: Critical Mesh System Architecture Repair

*   **Status**: **ACTIVE & CRITICAL**
*   **Goal**: To fix the fundamental architectural disconnect between the asset pipeline and the rendering system, enabling the engine to load and render compiled mesh files.
*   **Justification**: This is the **highest priority** task for the entire project. No other rendering work can proceed until the engine can reliably display a 3D model. The current system is completely broken and falls back to a test triangle.
*   **Key Documents**:
    *   **Sprint Plan**: [./../active/10_5_mesh_system_repair.md](./../active/10_5_mesh_system_repair.md)
    *   **Implementation Guide**: [Code Review & Action Items](./../active/10_5_mesh_system_repair_code_review.md)
    *   **Research Paper**: [Architecting a Resilient Mesh System](./../active/10_5_mesh_system_repair_research.md)

---

## 2. Sprint 15: Research - Automated Testing Framework

*   **Status**: **BACKLOG - HIGH PRIORITY**
*   **Goal**: To research and select a suitable automated testing framework for C and define a strategy for integrating unit, integration, and rendering tests into the project.
*   **Justification**: The project currently has zero automated tests, making it fragile and difficult to refactor. A testing framework is a foundational capability that will de-risk all future development, improve code quality, and is a prerequisite for a reliable CI/CD pipeline. This should be implemented immediately after the mesh system is repaired.
*   **Key Documents**:
    *   **Research Plan**: [./15_automated_testing_research.md](./15_automated_testing_research.md)

---

## 3. Sprint 11: PBR Rendering Pipeline Implementation

*   **Status**: **BLOCKED** (by Sprint 10.5)
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline, including advanced materials, shaders, and lighting.
*   **Justification**: PBR is the industry standard for realistic rendering. Implementing this feature is the next major step in achieving modern visual fidelity for the CGame engine. It will unlock the potential of the asset pipeline and enable the creation of high-quality, professional-grade assets.
*   **Key Documents**:
    *   **Consolidated Sprint Plan**: [./11_pbr_rendering_pipeline.md](./11_pbr_rendering_pipeline.md)
    *   **Deep Implementation Analysis**: [./11_pbr_implementation_analysis.md](./11_pbr_implementation_analysis.md)

---