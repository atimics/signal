# Developer Onboarding Guide

Welcome to the CGame engine development team! This guide will walk you through the process of setting up your development environment, building the engine, and making your first contribution.

## 1. Setting Up Your Development Environment

### 1.1. Prerequisites

Before you begin, please ensure that you have the following software installed on your system:

*   **Git**: For version control.
*   **A C99-compliant compiler**: Such as Clang or GCC.
*   **Python 3.11+**: For the asset pipeline.
*   **Make**: For running the build system.

### 1.2. Forking and Cloning the Repository

1.  **Fork the repository**: Start by forking the main CGame repository to your own GitHub account.
2.  **Clone your fork**: Clone your forked repository to your local machine:

    ```bash
    git clone https://github.com/YOUR_USERNAME/cgame.git
    cd cgame
    ```

### 1.3. Setting Up the Python Virtual Environment

The CGame engine uses a Python virtual environment to manage its dependencies. To set up the virtual environment, run the following commands from the root of the project directory:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 2. Building and Running the Engine

### 2.1. Building the Engine

To build the engine, run the following command from the root of the project directory:

```bash
make
```

This will compile the engine and all of its dependencies.

### 2.2. Running the Engine

To run the engine, run the following command:

```bash
make run
```

This will launch the CGame engine and load the default test scene.

### 2.3. Running the Tests

To run the test suite, run the following command:

```bash
make test
```

This will run all of the unit tests for the project.

## 3. Making Your First Contribution

### 3.1. Finding an Issue to Work On

A great place to start is by looking at our list of [good first issues](../.github/GOOD_FIRST_ISSUES.md). These are issues that are well-defined, self-contained, and a great way to get familiar with the project.

### 3.2. Creating a New Branch

Once you've found an issue to work on, create a new branch for your feature:

```bash
git checkout -b your-feature-branch
```

### 3.3. Making Your Changes

Now you're ready to make your changes to the codebase. As you work, please be sure to follow the project's coding style and conventions.

### 3.4. Submitting a Pull Request

Once you've finished your changes, you're ready to submit a pull request. Please be sure to include a clear and concise description of your changes, and to link to the issue that your pull request addresses.

## 4. Getting Help

If you have any questions or need help with any of these steps, please don't hesitate to open an issue on GitHub. We're here to help!
