# CGame Project Guide for Claude

## ✅ UPDATE: Sprint 21 Complete!
**All major physics and control bugs fixed, including Xbox controller support!**
- **Fixed**: Thrust direction transformation (quaternion rotation) ✅
- **Fixed**: Drag physics (correct formula, proper values) ✅
- **Fixed**: Control remapping (W/S pitch, Space thrust, A/D banking) ✅
- **Fixed**: Numerical overflow (comprehensive clamping) ✅
- **Fixed**: Auto-deceleration (gentle 5% for natural feel) ✅
- **Fixed**: Xbox controller mapping (trigger centering, deadzone) ✅
- **Remaining**: Visual thruster rendering (cosmetic only)
- **Status**: 98% complete, flight mechanics fully functional

## Essential Documentation Links

### Must Read First
1. **[Sprint Backlog & Status](docs/sprints/README.md)** - Current sprint status and priorities
2. **[Active Sprint - Sprint 21](docs/sprints/active/SPRINT_21_DESIGN.md)** - Ship flight mechanics overhaul
3. **[Sprint 21 Findings](docs/sprints/active/SPRINT_21_FINDINGS_AND_FIXES.md)** - Critical bug details

### Quick Reference
- **Build**: `make clean && make test && make`
- **Run Tests**: `make test` (375+ tests)
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
2. **Run Tests**: `make test` - Ensure all 375+ tests pass
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

### Sprint 21: Ship Flight Mechanics Overhaul
- **Goal**: Entity-agnostic flight mechanics with 6DOF physics
- **Status**: 98% complete, all major bugs fixed
- **Definition of Done**: Human approval of flight feel (4.0+/5.0 rating)
- **Latest**: Xbox controller support fixed, proper thrust on triggers

### What's Working
- ✅ ECS architecture with component composition
- ✅ 6DOF physics system with proper force transformation
- ✅ Angular physics with banking flight model
- ✅ Input processing pipeline (keyboard + gamepad)
- ✅ Ship type configurations (Fighter, Interceptor, Cargo, Explorer)
- ✅ Performance (60+ FPS)
- ✅ Xbox controller support with proper HID parsing
- ✅ Thrust direction (quaternion rotation implemented)
- ✅ Drag physics (correct formula: vel * (1 - drag))
- ✅ Auto-deceleration for natural flight feel

### What Needs Polish
- ⚠️ Visual thruster rendering (particles not aligned correctly)
- ℹ️ This is cosmetic only - physics work correctly

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

### Test Flight Mechanics
1. Run `./build/cgame --test-flight`
2. Test keyboard controls:
   - W/S: Pitch control
   - A/D: Banking turns (yaw + roll)
   - Space: Forward thrust
   - X: Reverse thrust
   - Q/E: Roll
   - Shift: Boost
3. Test gamepad controls:
   - Left stick: Pitch/yaw
   - Right stick: Strafe/vertical
   - Right trigger: Forward thrust
   - Left trigger: Reverse thrust
   - Bumpers: Roll

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

## Recent Fixes Summary

### Thrust Direction Fix
- Implemented `quaternion_rotate_vector()` in `src/core.c`
- Transforms thrust forces from ship-local to world space
- Ships now fly correctly based on their orientation

### Physics Fixes
- Corrected drag formula: was `vel * drag`, now `vel * (1 - drag)`
- Changed drag from 0.9999 to 0.02 (98% velocity retention vs 2% loss)
- Added comprehensive clamping for velocity, acceleration, and forces
- Implemented auto-deceleration (5% counter-thrust when no input)

### Control System Fixes
- Remapped W/S from thrust to pitch control
- Implemented banking model: A/D now causes coordinated yaw + roll
- Space bar now controls forward thrust
- Fixed A/D rotation bug (both were rotating same direction)

### Xbox Controller Fixes
- Added Xbox Wireless Controller (PID:0x0B13) support
- Fixed trigger centering (Xbox triggers rest at ~127, not 0)
- Increased stick deadzone to 20% to handle drift
- Corrected HID report byte mapping for proper control

## Next Steps

1. Final testing and polish
2. Fix visual thruster rendering (cosmetic)
3. Complete Sprint 21 documentation
4. Begin Sprint 22: Canyon Racing Prototype