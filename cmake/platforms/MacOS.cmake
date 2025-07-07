# SIGNAL Engine - macOS Platform Configuration
# ============================================================================
# Platform-specific settings for macOS builds

message(STATUS "Configuring for macOS platform")

# ============================================================================
# GRAPHICS BACKEND
# ============================================================================

# Use Metal backend for Sokol on macOS
add_compile_definitions(SOKOL_METAL)

# ============================================================================
# SYSTEM FRAMEWORKS
# ============================================================================

# Core graphics frameworks
find_library(METAL_FRAMEWORK Metal)
find_library(METALKIT_FRAMEWORK MetalKit)
find_library(APPKIT_FRAMEWORK AppKit)
find_library(QUARTZCORE_FRAMEWORK QuartzCore)

# Input and device frameworks
find_library(IOKIT_FRAMEWORK IOKit)
find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)

# Check that required frameworks are found
if(NOT METAL_FRAMEWORK)
    message(FATAL_ERROR "Metal framework not found")
endif()

if(NOT METALKIT_FRAMEWORK)
    message(FATAL_ERROR "MetalKit framework not found")
endif()

if(NOT APPKIT_FRAMEWORK)
    message(FATAL_ERROR "AppKit framework not found")
endif()

# Collect all required frameworks
set(CGAME_PLATFORM_LIBS
    ${METAL_FRAMEWORK}
    ${METALKIT_FRAMEWORK}
    ${APPKIT_FRAMEWORK}
    ${QUARTZCORE_FRAMEWORK}
    ${IOKIT_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
)

message(STATUS "macOS Frameworks configured:")
message(STATUS "  - Metal: ${METAL_FRAMEWORK}")
message(STATUS "  - MetalKit: ${METALKIT_FRAMEWORK}")
message(STATUS "  - AppKit: ${APPKIT_FRAMEWORK}")
message(STATUS "  - QuartzCore: ${QUARTZCORE_FRAMEWORK}")
message(STATUS "  - IOKit: ${IOKIT_FRAMEWORK}")
message(STATUS "  - CoreFoundation: ${COREFOUNDATION_FRAMEWORK}")

# ============================================================================
# COMPILER SETTINGS
# ============================================================================

# macOS deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "macOS deployment target")

# Architecture settings (support both Intel and Apple Silicon)
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS architectures")
endif()

message(STATUS "macOS deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "macOS architectures: ${CMAKE_OSX_ARCHITECTURES}")

# ============================================================================
# HIDAPI SUPPORT
# ============================================================================

# Look for system hidapi first
find_library(HIDAPI_LIBRARY hidapi)

if(HIDAPI_LIBRARY)
    message(STATUS "Using system HIDAPI: ${HIDAPI_LIBRARY}")
    list(APPEND CGAME_PLATFORM_LIBS ${HIDAPI_LIBRARY})
else()
    message(STATUS "Using bundled HIDAPI implementation (hidapi_mac.c)")
    # The hidapi_mac.c source will be compiled directly into the project
endif()

# ============================================================================
# DEVELOPMENT TOOLS
# ============================================================================

# Enable Xcode schemes generation
set(CMAKE_XCODE_GENERATE_SCHEME ON)

# Xcode-specific settings
if(CMAKE_GENERATOR STREQUAL "Xcode")
    # Enable automatic code signing (development only)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "")
    
    # Set bundle identifier
    set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.signal.engine")
    
    # Debug information format
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
    
    message(STATUS "Xcode project configured")
endif()

# ============================================================================
# TESTING CONFIGURATION
# ============================================================================

# macOS-specific test settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Enable additional debugging for tests
    add_compile_definitions(CGAME_MACOS_DEBUG)
endif()

# ============================================================================
# ASSET PIPELINE
# ============================================================================

# macOS-specific asset compilation settings
set(CGAME_ASSET_PLATFORM_FLAGS "")

# Check for Metal shader compiler
find_program(METAL_COMPILER xcrun)
if(METAL_COMPILER)
    message(STATUS "Metal shader compiler available")
    set(CGAME_METAL_SHADERS_ENABLED ON)
else()
    message(WARNING "Metal shader compiler not found - using precompiled shaders")
    set(CGAME_METAL_SHADERS_ENABLED OFF)
endif()

# ============================================================================
# PERFORMANCE OPTIMIZATIONS
# ============================================================================

# macOS-specific performance flags
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Performance")
    # Enable automatic vectorization
    add_compile_options(-fvectorize)
    
    # Platform-specific optimizations
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        add_compile_options(-mcpu=apple-m1)
        message(STATUS "Apple Silicon optimizations enabled")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_compile_options(-march=native)
        message(STATUS "Intel optimizations enabled")
    endif()
endif()

# ============================================================================
# BUNDLE CONFIGURATION (Future)
# ============================================================================

# Prepare for future app bundle support
set(CGAME_BUNDLE_IDENTIFIER "com.signal.engine")
set(CGAME_BUNDLE_VERSION "${PROJECT_VERSION}")
set(CGAME_BUNDLE_SHORT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

# ============================================================================
# SUMMARY
# ============================================================================

message(STATUS "macOS platform configuration complete")
message(STATUS "Frameworks: ${CGAME_PLATFORM_LIBS}")
message(STATUS "Deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "Architectures: ${CMAKE_OSX_ARCHITECTURES}")
