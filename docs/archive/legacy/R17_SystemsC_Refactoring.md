# Research Proposal: R17 - Refactoring `systems.c`

**Author**: Gemini
**Date**: June 30, 2025
**Status**: Accepted

## 1. Introduction

The `systems.c` file is a critical component of the CGame engine, responsible for managing the core systems and their interactions. However, its current implementation is monolithic and tightly coupled, making it difficult to maintain, test, and extend. This research project will investigate and propose a refactoring strategy to improve the modularity, testability, and overall quality of `systems.c`.

## 2. Problem Statement

The current implementation of `systems.c` suffers from the following issues:

*   **High Coupling**: The systems are tightly coupled, with direct dependencies on each other.
*   **Low Cohesion**: The file contains a mix of unrelated responsibilities, making it difficult to understand and maintain.
*   **Poor Testability**: The lack of clear boundaries between systems makes it difficult to write unit tests.
*   **Scalability**: Adding new systems or modifying existing ones is a complex and error-prone process.

## 3. Research Goals

This research project will aim to:

*   Analyze the existing implementation of `systems.c` to identify its key responsibilities and dependencies.
*   Investigate alternative architectural patterns for managing systems, such as a system registry or a message-passing architecture.
*   Propose a refactoring strategy that addresses the identified issues and aligns with the project's overall goals of stability and code quality.
*   Define a set of metrics to evaluate the success of the refactoring, such as code complexity, test coverage, and performance.

## 4. Proposed Methodology

The research will be conducted in the following phases:

1.  **Analysis**: A detailed analysis of the `systems.c` file will be performed to understand its current structure and identify areas for improvement.
2.  **Design**: Based on the analysis, a new architecture for managing systems will be designed.
3.  **Implementation**: A prototype of the new architecture will be implemented to validate its feasibility and effectiveness.
4.  **Evaluation**: The prototype will be evaluated against the defined metrics to assess its impact on code quality and performance.

## 5. Analysis of `systems.c`

A detailed analysis of `systems.c` and `systems.h` has revealed several architectural issues:

*   **Monolithic `scheduler_init`**: This function is responsible for initializing the asset system, data system, render system, loading data, and setting up the system scheduler. This violates the Single Responsibility Principle and creates a high degree of coupling between different parts of the engine.
*   **Global Registries**: The use of global variables `g_asset_registry` and `g_data_registry` for managing assets and data creates hidden dependencies and makes the code difficult to reason about and test.
*   **Direct System Calls**: The scheduler directly calls the `update` function for each system. While simple, this approach lacks flexibility and makes it harder to implement more advanced scheduling strategies.
*   **Co-located System Implementations**: All system implementations (physics, collision, AI, camera) are located in the same file, making it large, unwieldy, and difficult to navigate.
*   **Lack of Abstraction**: Systems have direct access to the `World` and `RenderConfig` structs, creating tight coupling with the core data structures and making it difficult to modify them without affecting the entire engine.

## 6. Proposed Refactoring Strategy

Based on the analysis, the following refactoring strategy is proposed:

1.  **Decouple System Initialization**: Each system (and other modules like assets and data) should have its own `init` and `shutdown` functions. The main application setup will be responsible for orchestrating the initialization order.
2.  **Dependency Injection**: Eliminate global registries. Instead, dependencies like the `AssetRegistry` and `DataRegistry` will be passed as parameters to the systems that require them. This makes dependencies explicit and improves testability.
3.  **Separate System Files**: Each system's implementation will be moved to its own source file (e.g., `physics_system.c`, `collision_system.c`). This will improve modularity, cohesion, and code organization.
4.  **Introduce a System Interface**: A common interface for all systems will be defined. This will allow the scheduler to manage systems in a more generic and extensible way.
5.  **Reduce Data Exposure**: The data passed to each system will be minimized. Instead of passing the entire `World` struct, systems will receive more focused data structures or APIs to interact with the world state, reducing coupling.

## 7. Timeline

This research project is expected to be completed within two weeks.

## 8. Conclusion

Refactoring `systems.c` is a critical step towards improving the stability, maintainability, and scalability of the CGame engine. This research project will provide a clear roadmap for achieving this goal and will contribute to the long-term success of the project.
