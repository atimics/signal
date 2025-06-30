# Research Proposal: R13 - Developer Tooling and Editor Strategy

**Status**: Proposed
**Author**: Gemini
**Date**: June 30, 2025

## 1. Abstract

The current workflow for creating and editing content in the CGame engine relies on manually editing text files, which is inefficient, error-prone, and does not scale to complex projects. This research proposal outlines a plan to investigate and define a long-term strategy for developer tooling, with a focus on creating a more powerful and intuitive in-engine editor.

## 2. Background

A key factor in a game engine's usability is the quality of its developer tools. A good editor can dramatically accelerate development by providing a visual, interactive way to build scenes, inspect game state, and modify assets. Our current text-based workflow is a significant bottleneck. This research will explore options for building an editor that aligns with our data-oriented philosophy and our existing UI capabilities (Nuklear).

## 3. Research Questions

This research will seek to answer the following strategic questions:

1.  **Editor Architecture**: What is the best architectural approach for an in-engine editor? Should it be a separate application, or a "mode" of the main engine? How can we ensure a clean separation between editor code and game code?
2.  **UI Technology**: Our engine currently uses the Nuklear immediate-mode GUI library. Is this sufficient for building a complex editor, or should we investigate other options (e.g., Dear ImGui)?
3.  **Core Features**: What is the minimum viable feature set for an initial version of the editor? This will likely include a scene hierarchy view, a component inspector, and a 3D viewport with gizmos for manipulating objects.
4.  **Extensibility**: How can we design the editor to be extensible, allowing new tools and windows to be added easily as the engine grows?
5.  **Asset Workflow Integration**: How will the editor interact with our existing asset pipeline? Should it be able to trigger asset recompilation? How will it handle live reloading of assets?

## 4. Proposed Methodology

1.  **Case Studies**: Analyze the editor architectures of successful open-source game engines like Godot and Bevy to identify best practices and common patterns.
2.  **Technology Evaluation**: Create a small prototype to compare the capabilities of Nuklear and Dear ImGui for building complex editor-style interfaces.
3.  **Feature Roadmap**: Develop a prioritized roadmap for the editor, starting with a minimal viable product and outlining a path to a more feature-complete toolset.
4.  **Architectural Design**: Produce a high-level design document that outlines the proposed editor architecture, its core components, and its integration with the rest of the engine.

## 5. Expected Outcomes

This research will produce the following strategic deliverables:

1.  **A Tooling Strategy Document**: A comprehensive document that outlines the long-term vision for developer tooling in the CGame engine.
2.  **A UI Technology Recommendation**: A formal recommendation on which GUI library to use for the editor, supported by a comparative analysis.
3.  **An Editor Feature Roadmap**: A prioritized list of features for the editor, from MVP to a full 1.0 release.
4.  **A High-Level Architectural Plan**: A set of diagrams and documents that will guide the initial development of the editor.

## 6. Priority

**Low**. While a good editor is a massive quality-of-life improvement, it is not a core runtime feature. The engine can function without it, and its development can be deferred until the core gameplay systems are more mature.
