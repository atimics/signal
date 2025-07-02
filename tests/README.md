# CGame Engine Test Suite

## Test Organization Philosophy

This test suite follows a **mirror architecture** that reflects the main codebase structure, organized by folders rather than underscores for better maintainability and discoverability.

## Directory Structure

```
tests/
├── core/                    # Core ECS and engine tests (mirrors src/core.*)
│   ├── test_world.c
│   ├── test_entities.c
│   ├── test_components.c
│   └── test_math.c
├── systems/                 # System tests (mirrors src/system/)
│   ├── test_physics.c
│   ├── test_camera.c
│   ├── test_input.c
│   ├── test_control.c
│   ├── test_thrusters.c
│   ├── test_ai.c
│   ├── test_collision.c
│   ├── test_memory.c
│   ├── test_performance.c
│   └── test_lod.c
├── rendering/               # Rendering tests (mirrors render_*)
│   ├── test_render_3d.c
│   ├── test_render_camera.c
│   ├── test_render_mesh.c
│   ├── test_render_lighting.c
│   └── test_gpu_resources.c
├── assets/                  # Asset system tests (mirrors assets.* + asset_loader/)
│   ├── test_assets.c
│   ├── test_asset_loader.c
│   └── test_material_definitions.c
├── ui/                      # UI system tests (mirrors ui_*)
│   ├── test_ui_api.c
│   ├── test_ui_scene.c
│   └── test_ui_components.c
├── scenes/                  # Scene system tests (mirrors scene_* + scripts/)
│   ├── test_scene_state.c
│   ├── test_scene_script.c
│   └── scripts/
│       ├── test_logo_scene.c
│       ├── test_flight_test_scene.c
│       └── test_scene_selector.c
├── integration/             # Cross-system integration tests
│   ├── test_flight_integration.c
│   ├── test_full_pipeline.c
│   └── test_startup_sequence.c
├── performance/             # Performance and benchmark tests
│   ├── test_memory_perf.c
│   ├── test_physics_perf.c
│   ├── test_rendering_perf.c
│   └── benchmarks/
├── regression/              # Regression tests for specific bugs
│   ├── test_sprint_21_velocity_bug.c
│   └── test_mesh_loading_fix.c
├── backlog/                 # One-off tests created during development
│   ├── test_task_4_isolated.c
│   ├── test_quick_fixes.c
│   └── README.md
├── support/                 # Test infrastructure and utilities
│   ├── test_runner.c
│   ├── test_discovery.c
│   ├── test_coverage.c
│   ├── test_utilities.h
│   └── fixtures/
├── mocks/                   # Mock implementations for testing
│   ├── mock_graphics.c
│   ├── mock_sokol.c
│   └── mock_assets.c
└── vendor/                  # Third-party testing frameworks
    └── unity/
```

## Test Categories

### 1. **Unit Tests** (`core/`, `systems/`, `rendering/`, `assets/`, `ui/`, `scenes/`)
- Test individual components in isolation
- Fast execution, minimal dependencies
- Mock external dependencies

### 2. **Integration Tests** (`integration/`)
- Test interactions between multiple systems
- Validate data flow and system coordination
- Full or partial system initialization

### 3. **Performance Tests** (`performance/`)
- Benchmarking and performance regression detection
- Memory usage tracking
- Frame timing validation

### 4. **Regression Tests** (`regression/`)
- Tests for specific bugs that have been fixed
- Prevent re-introduction of known issues
- Named by sprint/issue for traceability

### 5. **Development Backlog** (`backlog/`)
- Temporary tests created during development
- One-off debugging tests
- Should be regularly reviewed and migrated to proper categories

## Test Automation

### Test Discovery
- Automatic discovery of all `test_*.c` files
- Dynamic test suite generation
- Parallel test execution support

### Coverage Analytics
- Line coverage tracking with `gcov`/`llvm-cov`
- Function coverage analysis
- Branch coverage validation
- Integration with `cloc` for code metrics

### Continuous Integration
- All tests run on every commit
- Performance regression detection
- Coverage regression prevention
- Automatic test categorization validation

## Running Tests

```bash
# Run all tests
make test

# Run specific category
make test-unit
make test-integration  
make test-performance

# Run with coverage
make test-coverage

# Run specific test file
make test-file FILE=core/test_world.c

# Generate coverage report
make coverage-report
```

## Test Writing Guidelines

### Naming Conventions
- Test files: `test_<module>.c`
- Test functions: `test_<feature>_<scenario>()`
- Test suites: `suite_<module>()`

### Structure
- Use Unity framework for assertions
- Include setup/teardown for each test
- Group related tests in suites
- Mock external dependencies

### Documentation
- Document test purpose and expected behavior
- Include bug reproduction steps for regression tests
- Reference sprint/task numbers where applicable

## Coverage Targets

- **Unit Tests**: 90%+ line coverage
- **Integration Tests**: 80%+ system interaction coverage
- **Critical Paths**: 100% coverage (physics, rendering, memory)
- **Performance Tests**: All critical performance paths covered

## Maintenance

- Review `backlog/` tests monthly
- Migrate stable tests to appropriate categories
- Update mirror structure when codebase changes
- Maintain mock consistency with real implementations
