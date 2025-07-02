# CGame Project Guide for Claude

## ✅ UPDATE: Sprint 21 Near Complete!
**All major physics and control bugs fixed!**
- **Fixed**: Thrust direction, drag, angular stability ✅
- **Fixed**: Control remapping (W/S pitch, Space thrust, A/D banking) ✅
- **Fixed**: Numerical overflow (velocity/force clamping) ✅
- **Fixed**: Auto-deceleration (now gentle 5% vs aggressive 30%) ✅
- **Pending**: Only visual thruster rendering remains
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
- **Status**: 90% complete, blocked by thrust bug
- **Definition of Done**: Human approval of flight feel (4.0+/5.0 rating)

### What's Working
- ✅ ECS architecture with component composition
- ✅ 6DOF physics system
- ✅ Angular physics (rotation)
- ✅ Input processing pipeline
- ✅ Ship type configurations
- ✅ Performance (60+ FPS)

### What's Broken
- ❌ Thrust direction transformation (forces in wrong coordinate space)
- ❌ Ships can only move forward relative to camera

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

### Fix the Thrust Bug
1. Read [Implementation Guide](docs/sprints/active/SPRINT_21_IMPLEMENTATION_GUIDE.md)
2. Implement `quaternion_rotate_vector()` in `src/core/core.c`
3. Update `src/system/thrusters.c:77`
4. Run thrust tests
5. Test in game with `--test-flight`

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

## Next Steps After Thrust Bug Fix

1. Human validation testing of flight mechanics
2. Polish based on feedback
3. Complete Sprint 21
4. Begin Sprint 22: Canyon Racing Prototype