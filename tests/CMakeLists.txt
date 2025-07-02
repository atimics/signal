# CGame Engine - Main Test Suite Configuration
# ============================================================================
# Comprehensive test suite configuration maintaining compatibility with
# existing test organization and Makefile targets

cmake_minimum_required(VERSION 3.20)

# ============================================================================
# TEST DISCOVERY & CONFIGURATION
# ============================================================================
include(cmake/TestDiscovery.cmake)

# ============================================================================
# TEST SUITE DEFINITION
# ============================================================================

# Core Engine Tests - Individual executables
cgame_add_test_suite(
    NAME cmake-core_test_components
    SOURCES core/test_components.c
    LIBS core physics
)

cgame_add_test_suite(
    NAME cmake-core_test_math
    SOURCES core/test_math.c
    LIBS core physics
)

cgame_add_test_suite(
    NAME cmake-core_test_world
    SOURCES core/test_world.c
    LIBS core physics
)

# Rendering Tests - Basic only (no graphics dependencies)
cgame_add_test_suite(
    NAME cmake-rendering
    SOURCES rendering/test_rendering.c
    LIBS core gpu_resources assets physics memory
    STUBS graphics_api_test_stub
)

# UI System Tests
cgame_add_test_suite(
    NAME cmake-ui
    SOURCES ui/test_ui_system.c
    LIBS core ui ui_api ui_scene ui_components
    STUBS graphics_api_test_stub nuklear_test_stubs
)

# Integration Tests
cgame_add_test_suite(
    NAME cmake-integration
    SOURCES integration/test_flight_integration.c
    LIBS core systems physics control thrusters input camera
    STUBS graphics_api_test_stub
)

# Performance Tests - Individual executables
cgame_add_test_suite(
    NAME cmake-performance_test_memory_isolated
    SOURCES performance/test_memory_isolated.c
    LIBS core systems physics performance
    STUBS graphics_api_test_stub
)

cgame_add_test_suite(
    NAME cmake-performance_test_memory_perf
    SOURCES performance/test_memory_perf.c
    LIBS core systems physics performance
    STUBS graphics_api_test_stub
)

cgame_add_test_suite(
    NAME cmake-performance_test_performance_critical
    SOURCES performance/test_performance_critical.c
    LIBS core systems physics performance
    STUBS graphics_api_test_stub
)

cgame_add_test_suite(
    NAME cmake-performance_test_performance_simple
    SOURCES performance/test_performance_simple.c
    LIBS core systems physics performance
    STUBS graphics_api_test_stub
)

# ============================================================================
# CTEST CONFIGURATION
# ============================================================================
enable_testing()

# The 'test' target is automatically created by enable_testing()
# and will run all tests defined with add_test().
# We can still define a custom target if we need to pass specific
# arguments to ctest.
add_custom_target(run-ctest
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
