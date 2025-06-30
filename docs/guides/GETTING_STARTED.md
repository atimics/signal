# Getting Started with CGame

This guide provides a concise overview for new developers to get the CGame engine up and running.

## 1. Prerequisites

Ensure the following are installed on your system:

*   **Git**: For version control.
*   **Clang or GCC**: A C99-compliant compiler.
*   **Python 3.11+**: For the asset pipeline.
*   **Make**: For running the build system.

## 2. Environment Setup

1.  **Clone the Repository**:
    ```bash
    git clone https://github.com/your-repo/cgame.git
    cd cgame
    ```

2.  **Set up Python Virtual Environment**:
    ```bash
    python3 -m venv .venv
    source .venv/bin/activate
    pip install -r requirements.txt
    ```

## 3. Build and Run

*   **Build the Engine and Assets**:
    ```bash
    make
    ```
*   **Run the Engine**:
    ```bash
    make run
    ```

This will launch the CGame engine and load the default test scene.

## 4. Next Steps

*   **Development Workflow**: For detailed information on the asset pipeline, coding standards, and debugging, see the [Development Workflow Guide](./DEVELOPMENT_WORKFLOW.md).
*   **Engine Architecture**: To understand the core design of the engine, including the ECS and rendering systems, refer to the [Architecture Guide](./ARCHITECTURE.md).
