# Project Roadmap

This file tracks the project's development, including features, bugs, and future plans.

## Next Sprint Priorities

### [Sprint 06: Cross-Platform Compilation](./../sprints/06_cross_platform_compilation.md)
- **Goal:** Establish a CI pipeline using GitHub Actions to automatically build the project for macOS, Linux, and Windows.
- **Status:** Planned.

### [Sprint 05: UI Integration with Dear ImGui](./../sprints/05_ui_integration.md)
- **Goal:** Integrate Dear ImGui to create a flexible UI system, including a HUD and debug panels.
- **Status:** Planned.

## Feature Roadmap

- [x] **Core Physics & Collision System**
- [x] **AI System Architecture**
- [x] **Reactive AI Behavior**
- [x] **Living Universe Population**
- [x] **Modular Camera System**
- [x] **PBR-like Material System**
- [x] **Sokol Graphics API Integration**
- [ ] **Asteroid Field**: 3D obstacles to navigate around
- [ ] **Enemy Ships**: AI-controlled opponents with combat AI
- [ ] **Weapons System**: Laser cannons and projectiles
- [ ] **Sound Effects**: Engine sounds and weapon fire
- [ ] **Textures**: Replace wireframes with textured models
- [ ] **Lighting**: Dynamic lighting system
- [ ] **HUD Improvements**: Radar, health, energy systems
- [ ] **Multiple Levels**: Different space environments

## Bug Tracker

| ID  | Description | Status | Priority |
| --- | ----------- | ------ | -------- |
| ~~1~~   | ~~Player can fly through the sun~~ | ✅ **Fixed**   | ~~Medium~~   |

## Performance Metrics

Current AI performance (mock implementation):
- **Average inference time**: ~75ms (simulated)
- **Concurrent AI entities**: 6 ships
- **Reaction distance**: 100 units
- **AI task frequency**: Every 1-2 seconds per entity
