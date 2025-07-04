---
id: KB-KORVUS-DESIGN
title: 'Project KORVUS: A Design for an LLM-Powered Knowledge Management Pipeline'
tags: [korvus, knowledge-management, automation, agentic-loop, design]
status: verified
last_verified: '2025-07-04'
related_files:
  - tools/korvus.py
---

# Project KORVUS: A Design for an LLM-Powered Knowledge Management Pipeline

**Author:** Gemini, Lead Scientist and Researcher
**Date:** July 4, 2025
**Status:** Revised

## 1. Vision & Mission

**Vision:** To create a zero-friction, self-maintaining knowledge system that evolves in lockstep with the codebase, proactively identifying insights and ensuring all documentation is perpetually current, accurate, and useful.

**Mission:** To eliminate the manual toil of documentation management by implementing an automated, LLM-driven pipeline that handles the creation, verification, consolidation, and synthesis of project knowledge, culminating in a live, actionable research guide for the active sprint.

## 2. Core Principles

1.  **Docs as Code, Managed by AI:** All documentation will live in Markdown files within the Git repository. The AI assistant (Gemini) is the primary owner and maintainer of this documentation.
2.  **Continuous Integration for Knowledge (CI-K):** The pipeline will be triggered by Git hooks and scheduled jobs, making knowledge management a seamless part of the development workflow, not a separate chore.
3.  **Synthesis over Categorization:** The primary output of the system is not a perfectly organized library, but a synthesized, actionable research guide tailored to the current sprint's goals.
4.  **Human-in-the-Loop:** The system generates drafts, reports, and updates for human review. The goal is to augment, not replace, the development team's judgment.
5.  **Minimalism and Efficiency:** The pipeline will be built with lightweight, cross-platform Python scripts and leverage the existing toolchain (`git`, `python`). It will be fast and unobtrusive.

## 3. System Architecture

The KORVUS pipeline is centered around a new key artifact: **The Active Sprint Research Guide**. This is a dynamically generated Markdown file that serves as the single source of truth for the current sprint.

```
+------------------------------------------------------------------------------------+
| Developer Workflow                                                                 |
|------------------------------------------------------------------------------------|
| `git commit` -> [Trigger: Git Hook] -> [KORVUS Engine] -> [LLM Agent] -> Update to Research Guide |
+------------------------------------------------------------------------------------+

+------------------------------------------------------------------------------------+
| Sprint Planning                                                                    |
|------------------------------------------------------------------------------------|
| `korvus.py --new-sprint` -> [KORVUS Engine] -> [LLM Agent] -> New Research Guide    |
+------------------------------------------------------------------------------------+
```

### 3.1. The Active Sprint Research Guide (`docs/sprints/active/RESEARCH_GUIDE.md`)

This is the heart of the new system. It is a single, comprehensive document that is continuously updated by the KORVUS pipeline. It contains:

-   **Sprint Goals & Status:** High-level objectives and real-time progress.
-   **Relevant Knowledge Synthesis:** A narrative summary of all existing documentation relevant to the sprint's goals. This is not a list of links, but a synthesized explanation.
-   **Code Hotspots:** A list of key files and functions that are likely to be modified during the sprint, with links to the source code.
-   **Recent Changes:** A summary of recent commits that are relevant to the sprint.
-   **External Research Brief:** A summary of web research on best practices, relevant libraries, or potential pitfalls related to the sprint's tasks.
-   **Open Questions & Risks:** A list of open questions and potential risks identified by the LLM agent.

### 3.2. The Knowledge Core (`docs/kb/`)

The knowledge core still exists, but its primary purpose is to be the raw material for the **Active Sprint Research Guide**. It is a repository of atomic knowledge fragments, each with YAML frontmatter.

### 3.3. Triggers

The triggers remain the same: Git hooks and scheduled jobs.

### 3.4. The KORVUS Engine (`tools/korvus.py`)

The KORVUS Engine is now responsible for maintaining the **Active Sprint Research Guide**. Its key tasks are:

-   **On Sprint Start:** Generating a brand new Research Guide by synthesizing the sprint plan and the existing knowledge core.
-   **On Commit:** Updating the "Recent Changes" and "Code Hotspots" sections of the Research Guide.
-   **On Demand:** Performing targeted web research and updating the "External Research Brief" section.

### 3.5. The LLM Agent (Gemini)

My role is now more focused on synthesis and research:

-   **Synthesizing** the knowledge core into a coherent narrative for the Research Guide.
-   **Analyzing** code changes and updating the Research Guide accordingly.
-   **Performing** targeted web research and summarizing the findings.
-   **Identifying** risks and open questions based on the available information.

## 4. Key Automated Workflows

### Workflow 1: Starting a New Sprint

1.  The sprint lead runs `python tools/korvus.py --new-sprint "Fix MicroUI vertex generation"`.
2.  The KORVUS Engine creates a new sprint plan and prompts the LLM Agent: "Generate an Active Sprint Research Guide for a sprint focused on fixing MicroUI vertex generation."
3.  The LLM Agent reads the sprint plan, searches the knowledge core for relevant documents (e.g., `microui-integration.md`, `rendering-pipeline.md`), performs a web search for "Microui common issues", and generates the initial `RESEARCH_GUIDE.md`.

**Benefit:** The sprint starts with a comprehensive, synthesized brief that gives every team member the context they need to be effective.

### Workflow 2: Continuous Update on Commit

1.  A developer modifies `src/ui_microui.c` and runs `git commit`.
2.  The `pre-commit` hook triggers `python tools/korvus.py --on-commit`.
3.  The KORVUS Engine identifies the changed file and prompts the LLM Agent: "`ui_microui.c` has changed. Update the Active Sprint Research Guide."
4.  The LLM Agent updates the "Recent Changes" section of the guide and may update the "Code Hotspots" or "Relevant Knowledge Synthesis" sections if the change is significant.

**Benefit:** The Research Guide is a living document that always reflects the latest state of the codebase.

## 5. Implementation Plan

**Phase 1: Foundation (1-2 Sprints)**
1.  Create the `docs/kb/` directory structure.
2.  Develop the initial `korvus.py` script with the `--new-sprint` and `--on-commit` commands.
3.  Migrate the top 5 most critical documents into the new KB format.
4.  Implement the `pre-commit` hook.

**Phase 2: Expansion (2-3 Sprints)**
1.  Implement the `--proactive-research` command to perform targeted web research.
2.  Integrate KORVUS with the issue tracking system to automatically link issues to the Research Guide.

## 6. Conclusion

This revised vision for Project KORVUS moves beyond simple documentation automation. It creates a dynamic, intelligent system that actively supports the development process by providing synthesized, actionable research guides for each sprint. This will not only reduce the burden of documentation but will also improve the quality of our work by ensuring that every developer has the information they need, when they need it.