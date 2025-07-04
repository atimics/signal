# SIGNAL

[![Build and Test](https://github.com/ratimics/cgame/actions/workflows/build.yml/badge.svg)](https://github.com/ratimics/cgame/actions/workflows/build.yml)
[![Test Suite](https://github.com/ratimics/cgame/actions/workflows/test.yml/badge.svg)](https://github.com/ratimics/cgame/actions/workflows/test.yml)

**In the silent graveyards of space, a ghost signal is calling. And you are the only one who can answer.**

**SIGNAL** is a high-performance, data-oriented 3D game engine and the foundation for a single-player space exploration game. It is written in C99, features a pure Entity-Component-System (ECS) architecture, and uses the modern, cross-platform **Sokol** graphics API for rendering.

**[📊 Project Status](docs/PROJECT_STATUS.md)**: 98% test coverage | 60+ FPS | Sprint 24 (MicroUI System Improvements) Active

**[📝 Engineering Report](docs/sprints/active/engineering_report.md)** | **[✅ Sprint 24 Plan](docs/sprints/active/SPRINT_24_PLAN.md)**

### 🎮 Current Features

- **6DOF Flight Mechanics**: Full six degrees of freedom spaceflight with realistic physics
- **Entity Component System**: Pure ECS architecture supporting 10,000+ entities at 60 FPS
- **Banking Flight Controls**: Intuitive control scheme with coordinated turns
- **Multiple Ship Types**: Fighter, Interceptor, Cargo, and Explorer configurations
- **Auto-Deceleration**: Optional flight assistance for arcade-style gameplay
- **Performance Optimized**: Maintains 60+ FPS with complex physics simulations

## 🕵️ SIGNAL CLI Minigame

Explore the game's lore through an interactive classified document interface:

```bash
./signal
```

The SIGNAL CLI provides an immersive terminal experience for discovering and decrypting classified intelligence documents hidden in the repository. Use passwords discovered during gameplay to progressively unlock secret lore about the Black Armada and the Aethelian Network.

## 🚀 Getting Started

### Prerequisites

- A C99-compliant compiler (e.g., `clang` or `gcc`).
- **macOS**: Xcode Command Line Tools for Metal support.
- **Linux**: `build-essential`, `libgl1-mesa-dev`, `libx11-dev`.
- Python 3 for the asset compilation pipeline (`pip install -r requirements.txt`).
- `clang-format` for code formatting (e.g., `brew install clang-format` or `apt-get install clang-format`).

### Building and Running

The project uses a simple Makefile. The first build will automatically compile the assets.

```bash
# Clean, build, and run the engine
make clean && make && make run

# Run the automated test suite (98% pass rate - near perfect!)
make test

# Run specific test suites
make test-physics
make test-thrusters

# Run with flight test scene
./build/cgame --test-flight
```

### 🎮 Flight Controls

**Keyboard:**
- W/S - Pitch control (dive/climb)
- A/D - Banking turns (coordinated yaw + roll)
- Space/X - Forward/backward thrust
- R/F - Vertical thrust (up/down)
- Q/E - Pure roll (barrel roll)
- Shift - Boost (50% extra thrust)
- Alt - Brake + Auto-deceleration
- Tab - Cycle camera modes

**Xbox Controller:**
- Left Stick - Pitch/Yaw control
- Right Stick X - Banking turns
- Right Stick Y - Vertical thrust
- Right Trigger - Forward thrust
- Left Trigger - Reverse thrust
- Bumpers - Roll control

## 📚 Documentation

For all technical documentation, development guides, and project management resources, please see the main **[Documentation Hub](./docs/README.md)**.