# Sprint Plan: Build System Consolidation

**Sprint Duration**: 2 weeks (14 days)  
**Priority**: Medium  
**Type**: Technical Debt Reduction  
**Created**: July 6, 2025

## Executive Summary

This sprint aims to consolidate the CGame engine's complex multi-Makefile build system into a unified CMake-based solution. The project currently has 4 different Makefiles serving various purposes, creating maintenance overhead and potential confusion. A unified CMake system will provide better cross-platform support, modern tooling integration, and reduced maintenance burden.

## Current State Analysis

### Existing Build Files
1. **Makefile** - Modern modular build system (primary)
2. **Makefile.new** - Variant with parallel testing default
3. **Makefile.test** - Legacy monolithic Makefile
4. **Makefile.cmake** - Transition compatibility layer

### Key Findings
- Project is already transitioning to CMake (Makefile.cmake exists)
- Tests already have partial CMake configuration (`tests/CMakeLists.txt`)
- 98% test coverage with comprehensive test suite
- Strong modular architecture in newer Makefiles
- Asset pipeline using Python scripts
- Platform support: macOS (Metal), Linux (OpenGL), WebAssembly

## Sprint Goals

### Primary Objectives
1. Create unified root `CMakeLists.txt` that replaces all Makefiles
2. Maintain 100% feature parity with existing build system
3. Preserve all test targets and coverage reporting
4. Integrate asset pipeline into CMake workflow
5. Document migration process for developers

### Success Criteria
- Single CMake configuration handles all build scenarios
- All existing make targets work via CMake
- No regression in build times or functionality
- Clear migration documentation
- Simplified developer onboarding

## Sprint Tasks

### Week 1: Analysis & Design (Days 1-7)

#### Day 1-2: Deep Dive Analysis
- [ ] Analyze modular makefiles structure (platform.mk, compiler.mk, sources.mk, etc.)
- [ ] Document all build targets and their dependencies
- [ ] Map Make variables to CMake equivalents
- [ ] Identify platform-specific configurations

#### Day 3-4: CMake Structure Design
- [ ] Design CMake module structure
- [ ] Create CMake toolchain files for each platform
- [ ] Design asset pipeline integration strategy
- [ ] Plan test suite organization

#### Day 5-6: Prototype Implementation
- [ ] Create root CMakeLists.txt with basic structure
- [ ] Implement core library builds
- [ ] Set up platform detection and configuration
- [ ] Create FindPackage modules for dependencies (ODE, YAML)

#### Day 7: Review & Refinement
- [ ] Review prototype with team
- [ ] Gather feedback on structure
- [ ] Refine design based on feedback
- [ ] Update sprint plan if needed

### Week 2: Implementation & Migration (Days 8-14)

#### Day 8-9: Core Implementation
- [ ] Implement full CMake build configuration
- [ ] Add all library targets
- [ ] Configure all executable targets
- [ ] Set up installation rules

#### Day 10-11: Test Suite & Asset Pipeline
- [ ] Integrate existing test CMake configuration
- [ ] Add CTest support for all test categories
- [ ] Implement asset pipeline as CMake custom commands
- [ ] Add test coverage reporting

#### Day 12: Platform-Specific Features
- [ ] Configure macOS Metal backend
- [ ] Configure Linux OpenGL backend
- [ ] Add WebAssembly/Emscripten support
- [ ] Test cross-compilation scenarios

#### Day 13: Documentation & Polish
- [ ] Write migration guide for developers
- [ ] Update README with new build instructions
- [ ] Create troubleshooting guide
- [ ] Add CMake presets for common configurations

#### Day 14: Testing & Validation
- [ ] Full regression testing on all platforms
- [ ] Performance comparison (build times)
- [ ] Developer acceptance testing
- [ ] Final documentation review

## Proposed CMake Structure

```
cgame/
├── CMakeLists.txt                 # Root configuration
├── cmake/
│   ├── Platform.cmake             # Platform detection
│   ├── CompilerFlags.cmake        # Compiler settings
│   ├── Dependencies.cmake         # External dependencies
│   ├── Assets.cmake               # Asset pipeline
│   └── Testing.cmake              # Test configuration
├── src/
│   └── CMakeLists.txt            # Source configuration
├── tests/
│   └── CMakeLists.txt            # Test suites (exists)
└── assets/
    └── CMakeLists.txt            # Asset compilation
```

## Technical Considerations

### Dependency Management
- **Sokol**: Header-only, easy integration
- **ODE**: Use FindODE.cmake or pkg-config
- **YAML**: Use FindYAML.cmake or pkg-config
- **HidAPI**: Already has CMake support

### Asset Pipeline Integration
```cmake
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/assets/meshes/index.json
    COMMAND ${Python3_EXECUTABLE} tools/build_pipeline.py
    DEPENDS ${ASSET_SOURCES}
    COMMENT "Building game assets..."
)
```

### Test Organization
```cmake
# Maintain test categories
add_test_category(core)
add_test_category(systems)
add_test_category(rendering)
add_test_category(integration)
add_test_category(performance)
```

### Platform Configuration
```cmake
if(APPLE)
    set(GRAPHICS_BACKEND "Metal")
elseif(UNIX)
    set(GRAPHICS_BACKEND "OpenGL")
elseif(EMSCRIPTEN)
    set(GRAPHICS_BACKEND "WebGL")
endif()
```

## Risk Mitigation

### Identified Risks
1. **Build regression**: Mitigate with comprehensive testing
2. **Developer disruption**: Provide compatibility wrapper initially
3. **CI/CD breakage**: Update CI scripts in parallel
4. **Missing features**: Thorough analysis and feature mapping

### Rollback Plan
- Keep Makefile.cmake as fallback during transition
- Git branch protection until validation complete
- Parallel build system operation for 1-2 sprints

## Definition of Done

- [ ] All Make targets available in CMake
- [ ] Build times comparable or better
- [ ] All tests passing with same coverage
- [ ] Documentation complete and reviewed
- [ ] Successfully builds on all platforms
- [ ] Asset pipeline integrated
- [ ] Developer guide published
- [ ] Team trained on new system

## Post-Sprint Actions

1. Monitor build system performance
2. Gather developer feedback
3. Remove legacy Makefiles after validation period
4. Update CI/CD pipelines
5. Consider vcpkg/conan for dependency management

## Notes

- Current test CMake exists but is incomplete
- Makefile.cmake shows intent to migrate
- Strong modular design in current Makefiles should translate well
- Consider using CMake presets for common configurations
- Opportunity to modernize with CMake 3.20+ features