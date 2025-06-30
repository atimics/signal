# Sprint 06: Cross-Platform Compilation

**Goal:** Establish a continuous integration (CI) pipeline that automatically compiles and builds the project for macOS, Linux, and Windows. This will ensure that the codebase remains portable and provides ready-to-use builds for each target platform.

**Period:** September 2025

## Key Objectives:

1.  **CI Pipeline Setup (GitHub Actions):**
    *   [ ] Create a new GitHub Actions workflow file (e.g., `.github/workflows/build.yml`).
    *   [ ] Configure the workflow to trigger on every push to the `main` branch and on pull requests.
    *   [ ] Define separate build jobs for macOS, Linux, and Windows.

2.  **macOS Build Job:**
    *   [ ] Configure a job to run on a `macos-latest` runner.
    *   [ ] Install necessary build dependencies.
    *   [ ] Add a step to execute the `make` command.
    *   [ ] Archive the final executable as a build artifact.

3.  **Linux Build Job:**
    *   [ ] Configure a job to run on an `ubuntu-latest` runner.
    *   [ ] Install Linux-specific build dependencies (e.g., `build-essential`, `libgl1-mesa-dev`, `xorg-dev`).
    *   [ ] Add a step to execute the `make` command.
    *   [ ] Archive the final executable as a build artifact.

4.  **Windows Cross-Compilation Job:**
    *   [ ] Investigate and implement cross-compilation from a Linux environment using MinGW-w64.
    *   [ ] Update the `Makefile` to support the MinGW-w64 toolchain (e.g., setting the `CC` variable and platform-specific libraries).
    *   [ ] The job will run on an `ubuntu-latest` runner, install MinGW, and build the Windows executable (`.exe`).
    *   [ ] Archive the final `.exe` as a build artifact.

5.  **Build Artifacts Management:**
    *   [ ] Ensure all three jobs upload their compiled executables as clearly named artifacts (e.g., `cgame-macos`, `cgame-linux`, `cgame-windows.exe`).
    *   [ ] Configure the workflow for easy download of artifacts for testing and distribution.

## Expected Outcomes:

*   Every push to the `main` branch will automatically trigger a build for macOS, Linux, and Windows.
*   Build status checks will be integrated into the pull request process, preventing merges that break compilation on any target platform.
*   Compiled executables for all three platforms will be available for download from each workflow run, streamlining testing and distribution.
*   The project's cross-platform compatibility will be continuously verified, reducing the risk of platform-specific regressions.

## Pre-computation/Pre-analysis:

*   The project uses a `Makefile` with existing logic for differentiating between macOS and Linux.
*   The Sokol libraries are inherently cross-platform, which is a significant advantage for this effort.
*   The primary technical challenge will be adapting the `Makefile` to support a cross-compiler toolchain (MinGW-w64) for the Windows build.
*   GitHub Actions provides hosted runners for all target platforms, which should be sufficient for this task. The user's AWS subscription can be reserved for future, more complex CI/CD needs like self-hosted runners if required.
