# CGame Engine: Strategic Plan

**Document Owner**: Gemini (Lead Scientist and Researcher)
**Status**: Adopted
**Last Updated**: June 30, 2025

## 1. Overview

This document outlines the strategic vision for the CGame engine, prioritizing stability, code quality, and sustainable feature development. The project currently faces a critical architectural issue preventing the rendering of meshes, which blocks all other rendering advancements. Additionally, a lack of automated testing makes the codebase fragile and difficult to refactor.

This plan addresses these challenges through a structured, three-phase approach.

## 2. The Three-Phase Plan

### Phase 1: Core System Repair (Sprint 10.5)

*   **Priority**: **CRITICAL**
*   **Goal**: To fix the fundamental architectural disconnect between the asset pipeline and the rendering system, enabling the engine to load and render compiled mesh files.
*   **Justification**: This is the highest priority for the entire project. No other rendering work can proceed until the engine can reliably display a 3D model.
*   **Action Item**: Gemini will produce a detailed, test-driven development plan for Sprint 10.5. This plan will define the specific tests that need to be implemented in C to verify the fix.

### Phase 2: Foundational Stability (Sprint 15)

*   **Priority**: **HIGH**
*   **Goal**: To integrate a C unit testing framework and establish a strategy for unit, integration, and rendering tests.
*   **Justification**: An automated testing framework is a foundational capability that will de-risk all future development, improve code quality, and is a prerequisite for a reliable CI/CD pipeline. This must be implemented immediately after the mesh system is repaired.
*   **Action Item**: Following the completion of Sprint 10.5, Gemini will architect Sprint 15. The first tests to be implemented will be those that validate the mesh system, creating a permanent regression test suite for this critical feature.

### Phase 3: Feature Advancement (Sprint 11)

*   **Priority**: **NORMAL**
*   **Goal**: To architect and implement a modern, end-to-end Physically Based Rendering (PBR) pipeline.
*   **Justification**: With a stable, tested core engine, we can confidently pursue advanced rendering features. PBR is the next major step in achieving modern visual fidelity.
*   **Action Item**: This sprint remains blocked by the completion of Phase 1 and Phase 2. Once the foundation is secure, this will become the active development priority.

## 3. Conclusion

By adhering to this phased approach, we will transform the CGame engine from a fragile prototype into a stable, robust platform ready for future innovation. This plan ensures that we address foundational issues first, creating a high-quality codebase that can be extended with confidence.
