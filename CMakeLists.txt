# SIGNAL Engine - Unified CMake Build System
# ============================================================================
# Component-Based Game Engine with ECS Architecture
# Created: July 6, 2025
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(SIGNAL 
    VERSION 1.0.0
    DESCRIPTION "Component-based Game Engine with ECS Architecture"
    LANGUAGES C
)

# ============================================================================
# OPTIONS
# ============================================================================
option(SIGNAL_BUILD_TESTS "Build test suite" ON)
option(SIGNAL_BUILD_DOCS "Build documentation" OFF)
option(SIGNAL_BUILD_ASSETS "Build game assets" ON)
option(SIGNAL_ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(SIGNAL_ENABLE_TSAN "Enable Thread Sanitizer" OFF)
option(SIGNAL_ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
option(SIGNAL_PARALLEL_TESTS "Run tests in parallel" ON)
option(SIGNAL_BUILD_WASM "Build WebAssembly version" OFF)

# ============================================================================
# CMAKE CONFIGURATION
# ============================================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Module path for custom Find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ============================================================================
# MODULAR CMAKE INCLUDES
# ============================================================================
include(cmake/Platform.cmake)
include(cmake/Dependencies.cmake)

# ============================================================================
# SOURCE FILES
# ============================================================================

# Main source files (from sources.mk)
set(SIGNAL_SOURCES
    src/core.c
    src/math/pd_controller.c
    src/systems.c
    src/system/physics.c
    src/system/collision.c
    src/system/ai.c
    src/system/camera.c
    src/system/lod.c
    src/system/performance.c
    src/system/memory.c
    src/system/material.c
    src/system/thrusters.c
    src/system/thruster_points_system.c
    src/system/control.c
    src/system/unified_control_system.c
    src/system/scripted_flight.c
    src/event_router.c
    src/event_handlers.c
    src/component/look_target.c
    src/component/unified_flight_control.c
    src/component/thruster_points_component.c
    src/thruster_points.c
    src/render_thrust_cones.c
    src/render_thrust_simple.c
    src/assets.c
    src/asset_loader/asset_loader_index.c
    src/asset_loader/asset_loader_mesh.c
    src/asset_loader/asset_loader_material.c
    src/render_3d.c
    src/render_camera.c
    src/render_lighting.c
    src/render_mesh.c
    src/render_layers.c
    src/render_pass_guard.c
    src/microui/microui.c
    src/ui_microui.c
    src/ui_microui_adapter.c
    src/ui.c
    src/ui_api.c
    src/ui_scene.c
    src/ui_components.c
    src/ui_adaptive_controls.c
    src/ui_menu_system.c
    src/ui_navigation_menu_impl.c
    src/ui_navigation_menu_microui.c
    src/data.c
    src/graphics_api.c
    src/gpu_resources.c
    src/graphics_health.c
    src/pipeline_manager.c
    src/scene_state.c
    src/scene_script.c
    src/scene_yaml_loader.c
    src/entity_yaml_loader.c
    src/scripts/logo_scene.c
    src/scripts/navigation_menu_scene.c
    src/scripts/flight_test_scene.c
    src/scripts/template_scene.c
    src/scripts/controller_calibration_scene_simple.c
    src/game_input.c
    src/hal/input_hal_sokol.c
    src/hal/input_hal_mock.c
    src/services/input_service.c
    src/services/input_action_maps.c
    src/services/controller_config.c
    src/input_state.c
    src/config.c
    src/utils/json_parser.c
)

# Platform-specific sources
if(APPLE)
    list(APPEND SIGNAL_SOURCES src/hidapi_mac.c)
    
    # Sokol requires Objective-C on macOS
    set_source_files_properties(
        src/graphics_api.c
        src/gpu_resources.c
        src/pipeline_manager.c
        PROPERTIES 
        COMPILE_FLAGS "-x objective-c"
        LANGUAGE C
    )
endif()

# Main executable source
set(SIGNAL_MAIN_SOURCE src/main.c)

# ============================================================================
# ENGINE LIBRARY
# ============================================================================
add_library(signal_engine STATIC ${SIGNAL_SOURCES})

# Include compiler flags after target is created
include(cmake/CompilerFlags.cmake)

target_include_directories(signal_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Platform-specific definitions
if(APPLE)
    target_compile_definitions(signal_engine PUBLIC SOKOL_METAL)
elseif(UNIX)
    target_compile_definitions(signal_engine PUBLIC SOKOL_GLCORE33)
elseif(EMSCRIPTEN)
    target_compile_definitions(signal_engine PUBLIC SOKOL_GLES3)
endif()

# ============================================================================
# DEPENDENCIES
# ============================================================================

# Math library
target_link_libraries(signal_engine PUBLIC m)

# Platform libraries
if(APPLE)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    
    target_link_libraries(signal_engine PUBLIC
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
        ${APPKIT_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREFOUNDATION_FRAMEWORK}
    )
elseif(UNIX AND NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(signal_engine PUBLIC
        OpenGL::GL
        ${X11_LIBRARIES}
        dl
        pthread
    )
endif()

# External dependencies
include(cmake/FindODE.cmake)
include(cmake/FindYAML.cmake)

if(ODE_FOUND)
    target_include_directories(signal_engine PUBLIC ${ODE_INCLUDE_DIRS})
    target_link_libraries(signal_engine PUBLIC ${ODE_LIBRARIES})
endif()

if(YAML_FOUND)
    target_include_directories(signal_engine PUBLIC ${YAML_INCLUDE_DIRS})
    target_link_libraries(signal_engine PUBLIC ${YAML_LIBRARIES})
endif()

# ============================================================================
# EXECUTABLE
# ============================================================================
add_executable(signal ${SIGNAL_MAIN_SOURCE})
target_link_libraries(signal PRIVATE signal_engine)

# Set output directory
set_target_properties(signal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================================
# ASSET PIPELINE
# ============================================================================
if(SIGNAL_BUILD_ASSETS)
    include(cmake/Assets.cmake)
endif()

# ============================================================================
# TESTING
# ============================================================================
if(SIGNAL_BUILD_TESTS)
    enable_testing()
    include(CTest)
    
    # Pass source list to tests
    set(SIGNAL_SOURCES_FOR_TESTS ${SIGNAL_SOURCES} PARENT_SCOPE)
    
    # Include test configuration
    add_subdirectory(tests)
    
    # Compatibility targets
    add_custom_target(run-tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS signal_engine
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running all tests..."
    )
    
    if(SIGNAL_PARALLEL_TESTS)
        add_custom_target(test-parallel
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel
            DEPENDS signal_engine
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests in parallel..."
        )
    endif()
    
    # Test categories for compatibility
    add_custom_target(test-core
        COMMAND ${CMAKE_CTEST_COMMAND} -R "test_core" --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    add_custom_target(test-systems
        COMMAND ${CMAKE_CTEST_COMMAND} -R "test_.*system" --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    add_custom_target(test-rendering
        COMMAND ${CMAKE_CTEST_COMMAND} -R "test_render" --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# ============================================================================
# DOCUMENTATION
# ============================================================================
if(SIGNAL_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
        doxygen_add_docs(docs
            ${CMAKE_SOURCE_DIR}/src
            COMMENT "Generating API documentation with Doxygen"
        )
    endif()
endif()

# ============================================================================
# CONVENIENCE TARGETS
# ============================================================================

# Run target
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/signal
    DEPENDS signal
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running SIGNAL..."
)

# Run with scene
add_custom_target(run-scene
    COMMAND ${CMAKE_BINARY_DIR}/signal --scene ${SCENE}
    DEPENDS signal
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running SIGNAL with scene..."
)

# Clean all
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/data
    COMMENT "Cleaning all build artifacts..."
)

# Memory check targets
if(UNIX AND NOT APPLE)
    find_program(VALGRIND valgrind)
    if(VALGRIND)
        add_custom_target(memcheck
            COMMAND ${VALGRIND} --leak-check=full --show-leak-kinds=all 
                    --track-origins=yes ${CMAKE_BINARY_DIR}/signal
            DEPENDS signal
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

# Help target
add_custom_target(show-help
    COMMAND ${CMAKE_COMMAND} -E echo "SIGNAL Engine Build Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  all          - Build everything"
    COMMAND ${CMAKE_COMMAND} -E echo "  signal        - Build the game executable"
    COMMAND ${CMAKE_COMMAND} -E echo "  test         - Run all tests"
    COMMAND ${CMAKE_COMMAND} -E echo "  test-parallel- Run tests in parallel"
    COMMAND ${CMAKE_COMMAND} -E echo "  assets       - Build game assets"
    COMMAND ${CMAKE_COMMAND} -E echo "  run          - Run the game"
    COMMAND ${CMAKE_COMMAND} -E echo "  docs         - Generate documentation"
    COMMAND ${CMAKE_COMMAND} -E echo "  clean        - Clean build artifacts"
    COMMAND ${CMAKE_COMMAND} -E echo "  clean-all    - Clean everything"
    COMMENT "Available targets"
)

# ============================================================================
# STATUS MESSAGES
# ============================================================================
message(STATUS "")
message(STATUS "SIGNAL Engine Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Graphics Backend: ${GRAPHICS_BACKEND}")
message(STATUS "  Build Tests: ${SIGNAL_BUILD_TESTS}")
message(STATUS "  Build Docs: ${SIGNAL_BUILD_DOCS}")
message(STATUS "  Build Assets: ${SIGNAL_BUILD_ASSETS}")
message(STATUS "")