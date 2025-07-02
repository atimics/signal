# CGame Engine - Modern C Game Development
# ============================================================================
# CMake configuration for cross-platform, high-performance game engine
# Supports: macOS, Linux, WebAssembly
# Architecture: Entity-Component-System (ECS) with scheduled systems

cmake_minimum_required(VERSION 3.20)
project(CGame 
    VERSION 1.0.0 
    DESCRIPTION "Component-Based Game Engine in C"
    LANGUAGES C OBJC
)

# ============================================================================
# BUILD CONFIGURATION
# ============================================================================

# Set C standard (C99 as specified in project requirements)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()

# CMake modules path
list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_SOURCE_DIR}/cmake/modules"
    "${CMAKE_SOURCE_DIR}/cmake/platforms"
)

# ============================================================================
# PLATFORM DETECTION AND CONFIGURATION
# ============================================================================

# Detect platform and include appropriate configuration
if(APPLE)
    # Compile graphics_api.c as Objective-C on macOS
    set_source_files_properties(src/graphics_api.c PROPERTIES
        COMPILE_FLAGS "-x objective-c"
    )
    include(MacOS)
    set(CGAME_PLATFORM "macOS")
elseif(UNIX AND NOT APPLE)
    include(Linux)
    set(CGAME_PLATFORM "Linux")
elseif(EMSCRIPTEN)
    include(WebAssembly)
    set(CGAME_PLATFORM "WebAssembly")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "Building CGame for platform: ${CGAME_PLATFORM}")

# ============================================================================
# COMPILER FLAGS AND WARNINGS
# ============================================================================

# Include compiler flags configuration
include(CGameCompilerFlags)

# Core compiler flags (matching existing Makefile)
set(CGAME_C_FLAGS
    -Wall
    -Wextra
    -Werror
    -std=c99
    -g
)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CGAME_C_FLAGS -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND CGAME_C_FLAGS -O2)
elseif(CMAKE_BUILD_TYPE STREQUAL "Performance")
    list(APPEND CGAME_C_FLAGS -O3 -DNDEBUG)
endif()

# Apply flags to all targets
add_compile_options(${CGAME_C_FLAGS})

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

# ============================================================================
# DEPENDENCIES AND LIBRARIES
# ============================================================================

# Math library (required for all platforms)
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    list(APPEND CGAME_SYSTEM_LIBS ${MATH_LIBRARY})
endif()

# Platform-specific libraries (configured in platform files)
list(APPEND CGAME_SYSTEM_LIBS ${CGAME_PLATFORM_LIBS})

# ============================================================================
# SOURCE FILE CONFIGURATION
# ============================================================================

# Define source files for the main game engine library
set(CGAME_SOURCES
    src/main.c
    src/core.c
    src/systems.c
    src/render_3d.c
    src/render_camera.c
    src/render_mesh.c
    src/render_lighting.c
    src/gpu_resources.c
    src/graphics_api.c
    src/ui.c
    src/ui_api.c
    src/ui_scene.c
    src/ui_components.c
    src/data.c
    src/scene_state.c
    src/scene_script.c
    ${CMAKE_SOURCE_DIR}/third_party/hidapi/mac/hid.c
)

# Add the main executable
add_executable(cgame ${CGAME_SOURCES})

# Suppress warnings for third-party code included in main.c and ui.c
set_source_files_properties(
    src/main.c
    src/ui.c
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-but-set-variable -Wno-null-pointer-subtraction"
)

add_dependencies(cgame assets)

# ============================================================================
# ENGINE LIBRARY DEFINITIONS
# ============================================================================

# Core Engine Library
add_library(core src/core.c)
target_include_directories(core PUBLIC src)

# Systems Library
add_library(systems src/systems.c)
add_library(physics src/system/physics.c)
add_library(control src/system/control.c)
add_library(thrusters src/system/thrusters.c)
add_library(input src/system/input.c)
add_library(camera src/system/camera.c)
add_library(performance src/system/performance.c)
target_link_libraries(systems PRIVATE core physics control thrusters input camera performance)

# Rendering Libraries
add_library(render_3d src/render_3d.c)
add_library(render_camera src/render_camera.c)
add_library(render_mesh src/render_mesh.c)
add_library(render_lighting src/render_lighting.c)
add_library(gpu_resources src/gpu_resources.c)
add_library(graphics_api src/graphics_api.c)

# UI Libraries
add_library(ui src/ui.c)
add_library(ui_api src/ui_api.c)
add_library(ui_scene src/ui_scene.c)
add_library(ui_components src/ui_components.c)

# Link rendering and UI libraries
target_link_libraries(render_3d PRIVATE core graphics_api)
target_link_libraries(ui PRIVATE core graphics_api)

# Link main executable against all libraries
target_link_libraries(cgame PRIVATE core systems render_3d ui ${CGAME_SYSTEM_LIBS})

# ============================================================================
# PLATFORM-SPECIFIC CONFIGURATION
# ============================================================================

# Detect platform and include appropriate configuration
if(APPLE)
    # Compile graphics_api.c as Objective-C on macOS
    set_source_files_properties(src/graphics_api.c PROPERTIES
        COMPILE_FLAGS "-x objective-c"
    )
    include(MacOS)
    set(CGAME_PLATFORM "macOS")
elseif(UNIX AND NOT APPLE)
    include(Linux)
    set(CGAME_PLATFORM "Linux")
elseif(EMSCRIPTEN)
    include(WebAssembly)
    set(CGAME_PLATFORM "WebAssembly")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "Building CGame for platform: ${CGAME_PLATFORM}")

# ============================================================================
# ASSET PIPELINE
# ============================================================================

# Asset compilation support
find_program(PYTHON_EXECUTABLE python3)
if(PYTHON_EXECUTABLE)
    set(ASSET_COMPILER "${CMAKE_SOURCE_DIR}/tools/build_pipeline.py")
    set(BUILD_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")
    
    # Custom target for asset compilation
    add_custom_target(assets
        COMMAND ${PYTHON_EXECUTABLE} ${ASSET_COMPILER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling game assets..."
    )
else()
    message(WARNING "Python not found, skipping asset pipeline.")
endif()

# ============================================================================
# TESTING CONFIGURATION
# ============================================================================

# Enable testing
enable_testing()

# Include test configuration
add_subdirectory(tests)

# ============================================================================
# CUSTOM TARGETS (Compatibility with existing Makefile)
# ============================================================================

# Run target
add_custom_target(run
    COMMAND cgame
    DEPENDS cgame
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running CGame..."
)

# Clean assets target
add_custom_target(clean-assets
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_ASSETS_DIR}
    COMMENT "Cleaning compiled assets..."
)

# Profile target (if available)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_target(profile
        COMMAND cgame
        DEPENDS cgame
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running CGame with profiling..."
    )
endif()

# ============================================================================
# DOCUMENTATION TARGETS
# ============================================================================

# Find Doxygen for documentation generation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation with Doxygen..."
    )
    message(STATUS "Documentation target enabled")
endif()

# ============================================================================
# INSTALLATION (Future)
# ============================================================================

# Install executable
install(TARGETS cgame
    RUNTIME DESTINATION bin
)

# Install assets
install(DIRECTORY ${BUILD_ASSETS_DIR}
    DESTINATION share/cgame
    OPTIONAL
)

# ============================================================================
# BUILD INFORMATION
# ============================================================================

# Display build configuration summary
message(STATUS "")
message(STATUS "CGame Engine Build Configuration:")
message(STATUS "==================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${CGAME_PLATFORM}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Source Files: ${CMAKE_SOURCE_DIR}/src")
message(STATUS "Binary Output: ${CMAKE_BINARY_DIR}")
if(PYTHON_EXECUTABLE)
    message(STATUS "Asset Pipeline: Enabled")
else()
    message(STATUS "Asset Pipeline: Disabled (Python3 not found)")
endif()
message(STATUS "Testing: Enabled")
if(DOXYGEN_FOUND)
    message(STATUS "Documentation: Enabled")
else()
    message(STATUS "Documentation: Disabled (Doxygen not found)")
endif()
message(STATUS "")
