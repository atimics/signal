# CGame Project Guide for Claude

## 🔥 CURRENT SPRINT: MicroUI System Improvements
**Sprint 24** - Critical UI Rendering Fix (July 4-18, 2025)
- **Status**: ACTIVE 🚀 - Day 1 of 14
- **Goal**: Fix MicroUI zero vertex generation blocking all UI development
- **Scope**: Diagnose rendering issue, implement core widgets, establish UI foundation
- **Strategic Objective**: Unblock gameplay UI development

### Sprint 24 Progress
- **Week 1**: Diagnosis and core fixes (vertex generation, command pipeline)
- **Week 2**: Widget implementation and comprehensive testing
- **Critical Issue**: MicroUI processes frames but generates 0 vertices
- **Target**: Functional UI system with visible elements

## ✅ Sprint 23 Complete - Technical Excellence Achieved!
**98% Test Coverage & Same-Day Completion**
- **Achievement**: 56/57 tests passing, bulletproof build system
- **Bonus Feature**: Scripted flight system for cinematic paths
- **Completion**: July 3, 2025 (single day sprint!)
- **Status**: Archived to `docs/sprints/completed/sprint_23/`

## ✅ Sprint 22 Complete - Strategic Pivot Success!
**Advanced Input Processing → Gameplay Vertical Slice Focus**
- **Achievement**: 93% test coverage foundation established
- **Strategic Decision**: Removed neural network complexity 
- **Result**: Clear pathway to canyon racing prototype
- **Value**: Technical excellence without feature debt
- **Status**: Archived to `docs/sprints/completed/sprint_22/`

## ✅ Sprint 21 Complete (98%)
**All major physics and control bugs fixed!**
- Thrust direction, drag physics, control remapping all working
- Xbox controller fully supported
- Only visual thruster rendering remains (cosmetic)

## Essential Documentation Links

### Must Read First
1. **[Current Sprint Status](docs/sprints/active/CURRENT_SPRINT_STATUS.md)** - Sprint 24 MicroUI improvements
2. **[Sprint 24 Plan](docs/sprints/active/SPRINT_24_MICROUI_IMPROVEMENTS.md)** - UI system fixes and improvements
3. **[Sprint 23 Complete](docs/sprints/completed/sprint_23/CURRENT_SPRINT_STATUS.md)** - Technical excellence achieved
4. **[Sprint Backlog & History](docs/sprints/README.md)** - All sprint status and priorities

### Quick Reference
- **Build**: `make clean && make test && make`
- **Run Tests**: `make test` (56/57 tests passing - 98% coverage)
- **Run Game**: `./build/cgame`
- **Test Flight**: `./build/cgame --test-flight`

## Project Structure

```
cgame/
├── src/                    # Source code
│   ├── component/         # ECS components (physics, transform, etc.)
│   ├── system/           # ECS systems (physics, thrusters, control)
│   ├── core/             # Core utilities (math, memory, logging)
│   ├── render/           # Rendering systems
│   └── main.c            # Entry point
├── tests/                 # Comprehensive test suite
├── docs/                  # Documentation
│   └── sprints/          # Sprint documentation
│       ├── active/       # Current sprint docs
│       └── completed/    # Historical sprints
├── assets/               # Game assets
└── build/               # Build output
```

## Architecture Overview

### Entity Component System (ECS)
The game uses a pure ECS architecture where:
- **Entities**: Just IDs, no data or behavior
- **Components**: Pure data containers (Physics, Transform, ThrusterSystem, etc.)
- **Systems**: Process components and implement game logic

### Key Components
1. **Transform**: Position and orientation
2. **Physics**: 6DOF physics with forces and torques
3. **ThrusterSystem**: Propulsion for ships
4. **ControlAuthority**: Maps input to thrust commands
5. **Mesh**: Visual representation

### Data Flow
```
Input → ControlAuthority → ThrusterSystem → Physics → Transform → Render
```

## Safe Workflow Process

### Before Making Changes
1. **Check Sprint Status**: Read [Sprint Backlog](docs/sprints/README.md)
2. **Run Tests**: `make test` - Ensure all tests pass (currently 56/57)
3. **Read Active Sprint**: Understand current goals and blockers
4. **Use TodoWrite**: Track all tasks and changes

### Making Changes
1. **Follow ECS Patterns**: Components = data, Systems = logic
2. **Test First**: Write tests before implementation
3. **Small Commits**: Make focused, atomic changes
4. **Document Findings**: Update sprint docs with discoveries

### After Changes
1. **Run Tests**: `make test` - Must maintain 100% pass rate
2. **Test in Game**: `./build/cgame --test-flight` for flight mechanics
3. **Update Docs**: Document any new findings or issues
4. **Update Todo List**: Mark completed tasks, add new discoveries

## Current Sprint Context

### Sprint 24: MicroUI System Improvements
- **Goal**: Fix critical UI rendering issues preventing any interface visibility
- **Status**: Day 1 of 14 - Diagnosis phase
- **Problem**: MicroUI generates 0 vertices despite proper frame processing
- **Latest**: Comprehensive logging being added to trace issue

### Recently Completed: Sprint 23
- **Result**: 98% test coverage achieved same day!
- **Achievement**: Build system fixed, tests passing, scripted flight added
- **Remaining**: 1 physics test for 100% coverage

### Previously: Sprint 22
- **Result**: Strategic pivot successful - removed neural network complexity
- **Achievement**: 93% test foundation with focus on gameplay

### What's Working
- ✅ ECS architecture with component composition
- ✅ 6DOF physics system with proper force transformation
- ✅ Angular physics with banking flight model
- ✅ Statistical input filtering (73% noise reduction)
- ✅ Per-device gamepad calibration
- ✅ Hot-plug gamepad support
- ✅ Training data collection for ML
- ✅ All Sprint 21 flight mechanics
- ✅ Performance (60+ FPS)

### What's In Progress
- 🚧 MicroUI vertex generation diagnosis
- 🚧 UI command pipeline debugging
- 🚧 Core widget implementation

### Known Issues
- ⚠️ MicroUI generates 0 vertices (blocking all UI)
- ⚠️ Events skipped outside active frames
- ⚠️ Visual thruster rendering from Sprint 21
- ℹ️ 1 physics test remaining for 100% coverage

## Command Reference

### Development Commands
```bash
# Full rebuild and test
make clean && make test && make

# Run specific test
./build/test_thrusters

# Run game with flight test
./build/cgame --test-flight

# Check for memory leaks
valgrind ./build/cgame

# Format code
clang-format -i src/**/*.c
```

### Git Workflow
```bash
# Check current work
git status
git diff

# Commit pattern
git add -A
git commit -m "fix(thrusters): Transform forces to world space

Fixes thrust direction bug where forces were applied in world space
instead of ship-relative space. Ships now correctly fly based on
their orientation.

- Add quaternion_rotate_vector() utility
- Transform forces in thruster_system_update()
- Add thrust direction tests"
```

## Testing Philosophy

1. **Comprehensive Coverage**: Every component and system has tests
2. **Integration Tests**: Test component interactions
3. **Performance Tests**: Ensure 60+ FPS with many entities
4. **Regression Prevention**: Tests catch breaking changes

## Performance Targets

- **Frame Rate**: 60+ FPS minimum
- **Entity Count**: 50+ active entities with full physics
- **Memory**: Efficient component storage and access
- **Input Latency**: < 16ms response time

## Common Tasks

### Test Input Processing
1. Run `./build/cgame --debug-input`
2. Observe input overlay showing:
   - Raw input values
   - Post-calibration values
   - Post-Kalman values
   - (Future) Post-neural values
3. Test gamepad filtering:
   - Make small movements - see noise reduction
   - Make fast movements - see responsiveness maintained
   - Let stick drift - see dead zone compensation

### Collect Training Data
1. Run `./build/cgame --collect-input-data`
2. Play normally for 10-15 minutes
3. Data saved to `data/gamepad_training/session_[timestamp].csv`
4. Use for neural network training

### Add New Component
1. Create header in `src/component/`
2. Define data structure
3. Add creation/destruction functions
4. Write comprehensive tests
5. Update relevant systems

### Modify Physics
1. Check `src/system/physics.c` and `src/component/physics.c`
2. Maintain 6DOF capability
3. Test with multiple entities
4. Verify performance targets

## Important Notes

- **Always test before committing**
- **Follow ECS patterns strictly**
- **Document significant findings in sprint docs**
- **Use TodoWrite to track all work**
- **Maintain backwards compatibility**
- **Performance is critical - profile changes**

## Recent Implementation Summary

### Sprint 22 Phase 1 Complete
- **Calibration System**: Auto-detects dead zones, stick drift, and response curves
- **Kalman Filtering**: Dual-filter design reduces noise by 73% while maintaining responsiveness
- **Training Data Collection**: Automatic CSV recording for neural network training
- **Hot-Plug Support**: Seamless gamepad connection/disconnection handling

### Current Sprint 22 Phase 2 Work
- **Neural Network Structure**: 10-16-8-2 architecture for input refinement
- **C Inference Engine**: Real-time forward propagation (<0.1ms target)
- **Python Training**: PyTorch pipeline for model training
- **Weight Loading**: System to import trained models into game

### From Sprint 21 (Complete)
- All 6DOF physics working correctly
- Intuitive control scheme implemented
- Xbox controller fully supported
- 98% complete (only visual effects remaining)

## Next Steps

### Immediate (Sprint 24)
1. Add comprehensive UI logging to trace issue
2. Create minimal test case (single button)
3. Debug command generation pipeline
4. Fix vertex generation and rendering

### After Sprint 24
1. Sprint 25: Canyon Racing Prototype
   - Originally planned as Sprint 24
   - Deferred due to UI system issues
   - First gameplay vertical slice
2. Sprint 26: Multi-Camera HUD System
3. Sprint 27: Advanced Flight Mechanics