# CGame Engine - Compiler Flags Configuration
# ============================================================================
# Centralized compiler flags and warnings configuration
# Supports: Clang, GCC, and future compiler additions

# Set default compiler flags based on compiler type
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CGAME_COMPILER_TYPE "Clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CGAME_COMPILER_TYPE "GCC")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(CGAME_COMPILER_TYPE "MSVC")
else()
    set(CGAME_COMPILER_TYPE "Unknown")
    message(WARNING "Unknown compiler: ${CMAKE_C_COMPILER_ID}")
endif()

message(STATUS "Configuring for compiler: ${CGAME_COMPILER_TYPE}")

# ============================================================================
# CORE COMPILER FLAGS
# ============================================================================

# Base flags for all compilers
set(CGAME_BASE_FLAGS
    -std=c99
    -g
)

# Warning flags (aggressive but practical)
set(CGAME_WARNING_FLAGS
    -Wall
    -Wextra
    -Werror
)

# Additional warnings for better code quality
if(CGAME_COMPILER_TYPE STREQUAL "Clang" OR CGAME_COMPILER_TYPE STREQUAL "GCC")
    list(APPEND CGAME_WARNING_FLAGS
        -Wno-error=unused-function
        -Wno-error=unused-variable
        -Wno-error=unused-but-set-variable
        -Wno-error=missing-field-initializers
        -Wno-error=implicit-function-declaration
        -Wno-error=null-pointer-subtraction
        -Wno-error=implicit-int
    )
endif()

# ============================================================================
# BUILD TYPE SPECIFIC FLAGS
# ============================================================================

# Debug flags
set(CGAME_DEBUG_FLAGS
    -O0
    -DDEBUG
    -DCGAME_DEBUG_MODE
)

# Release flags
set(CGAME_RELEASE_FLAGS
    -O2
    -DNDEBUG
    -DCGAME_RELEASE_MODE
)

# Performance flags (for benchmarking)
set(CGAME_PERFORMANCE_FLAGS
    -O3
    -DNDEBUG
    -DCGAME_PERFORMANCE_MODE
    -march=native
)

# Test flags (for test builds)
set(CGAME_TEST_FLAGS
    -O2
    -g
    -DUNITY_TESTING
    -DTEST_MODE
    -DSOKOL_DUMMY_BACKEND
    -DCGAME_TEST_MODE
)

# ============================================================================
# PLATFORM SPECIFIC ADJUSTMENTS
# ============================================================================

# macOS specific adjustments
if(APPLE)
    list(APPEND CGAME_BASE_FLAGS
        -DSOKOL_METAL
    )
endif()

# Linux specific adjustments
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    list(APPEND CGAME_BASE_FLAGS
        -DSOKOL_GLCORE
        -D_POSIX_C_SOURCE=199309L
    )
endif()

# WebAssembly specific adjustments
if(EMSCRIPTEN)
    list(APPEND CGAME_BASE_FLAGS
        -DSOKOL_GLES2
        -DCGAME_WASM_BUILD
    )
endif()

# ============================================================================
# FUNCTION TO APPLY FLAGS TO TARGETS
# ============================================================================

function(cgame_apply_compiler_flags target)
    # Apply base flags
    target_compile_options(${target} PRIVATE ${CGAME_BASE_FLAGS})
    target_compile_options(${target} PRIVATE ${CGAME_WARNING_FLAGS})
    
    # Apply build-type specific flags
    target_compile_options(${target} PRIVATE
        $<$<CONFIG:Debug>:${CGAME_DEBUG_FLAGS}>
        $<$<CONFIG:Release>:${CGAME_RELEASE_FLAGS}>
        $<$<CONFIG:Performance>:${CGAME_PERFORMANCE_FLAGS}>
    )
endfunction()

function(cgame_apply_test_flags target)
    # Apply test-specific flags
    target_compile_options(${target} PRIVATE ${CGAME_BASE_FLAGS})
    target_compile_options(${target} PRIVATE ${CGAME_TEST_FLAGS})
    
    # Test-specific warning suppressions
    if(CGAME_COMPILER_TYPE STREQUAL "Clang" OR CGAME_COMPILER_TYPE STREQUAL "GCC")
        target_compile_options(${target} PRIVATE
            -Wno-error=unused-function
            -Wno-error=unused-variable
        )
    endif()
endfunction()

# ============================================================================
# SANITIZER SUPPORT (Debug builds)
# ============================================================================

option(CGAME_ENABLE_SANITIZERS "Enable sanitizers for debug builds" OFF)

if(CGAME_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CGAME_COMPILER_TYPE STREQUAL "Clang" OR CGAME_COMPILER_TYPE STREQUAL "GCC")
        set(CGAME_SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )
        
        message(STATUS "Sanitizers enabled for debug builds")
    else()
        message(WARNING "Sanitizers not supported with ${CGAME_COMPILER_TYPE}")
    endif()
endif()

function(cgame_apply_sanitizers target)
    if(CGAME_SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${CGAME_SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${CGAME_SANITIZER_FLAGS})
    endif()
endfunction()

# ============================================================================
# PROFILING SUPPORT
# ============================================================================

option(CGAME_ENABLE_PROFILING "Enable profiling support" OFF)

if(CGAME_ENABLE_PROFILING)
    set(CGAME_PROFILING_FLAGS
        -pg
        -fno-omit-frame-pointer
    )
    message(STATUS "Profiling enabled")
endif()

function(cgame_apply_profiling target)
    if(CGAME_PROFILING_FLAGS)
        target_compile_options(${target} PRIVATE ${CGAME_PROFILING_FLAGS})
        target_link_options(${target} PRIVATE ${CGAME_PROFILING_FLAGS})
    endif()
endfunction()

# ============================================================================
# COVERAGE SUPPORT
# ============================================================================

option(CGAME_ENABLE_COVERAGE "Enable code coverage" OFF)

if(CGAME_ENABLE_COVERAGE)
    if(CGAME_COMPILER_TYPE STREQUAL "Clang" OR CGAME_COMPILER_TYPE STREQUAL "GCC")
        set(CGAME_COVERAGE_FLAGS
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        message(STATUS "Code coverage enabled")
    else()
        message(WARNING "Coverage not supported with ${CGAME_COMPILER_TYPE}")
    endif()
endif()

function(cgame_apply_coverage target)
    if(CGAME_COVERAGE_FLAGS)
        target_compile_options(${target} PRIVATE ${CGAME_COVERAGE_FLAGS})
        target_link_options(${target} PRIVATE ${CGAME_COVERAGE_FLAGS})
    endif()
endfunction()

# ============================================================================
# CONVENIENCE FUNCTION FOR ALL FLAGS
# ============================================================================

function(cgame_configure_target target)
    cmake_parse_arguments(ARG "TEST;PROFILING;SANITIZERS;COVERAGE" "" "" ${ARGN})
    
    # Apply base compiler flags
    if(ARG_TEST)
        cgame_apply_test_flags(${target})
    else()
        cgame_apply_compiler_flags(${target})
    endif()
    
    # Apply optional features
    if(ARG_SANITIZERS OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CGAME_ENABLE_SANITIZERS))
        cgame_apply_sanitizers(${target})
    endif()
    
    if(ARG_PROFILING OR CGAME_ENABLE_PROFILING)
        cgame_apply_profiling(${target})
    endif()
    
    if(ARG_COVERAGE OR CGAME_ENABLE_COVERAGE)
        cgame_apply_coverage(${target})
    endif()
endfunction()
