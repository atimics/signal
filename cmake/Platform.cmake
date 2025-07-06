# Platform Detection and Configuration
# ============================================================================

# Detect platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CGAME_PLATFORM "macOS")
    set(CGAME_PLATFORM_MACOS TRUE)
    set(GRAPHICS_BACKEND "Metal")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CGAME_PLATFORM "Linux") 
    set(CGAME_PLATFORM_LINUX TRUE)
    set(GRAPHICS_BACKEND "OpenGL")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CGAME_PLATFORM "Windows")
    set(CGAME_PLATFORM_WINDOWS TRUE)
    set(GRAPHICS_BACKEND "D3D11")
elseif(EMSCRIPTEN)
    set(CGAME_PLATFORM "Web")
    set(CGAME_PLATFORM_WEB TRUE)
    set(GRAPHICS_BACKEND "WebGL")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Architecture detection
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CGAME_ARCH "x64")
else()
    set(CGAME_ARCH "x86")
endif()

# Apple Silicon detection
if(CGAME_PLATFORM_MACOS)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE CGAME_CPU_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(CGAME_CPU_ARCH STREQUAL "arm64")
        set(CGAME_APPLE_SILICON TRUE)
        # Add Homebrew paths for Apple Silicon
        list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
    else()
        # Intel Mac Homebrew paths
        list(APPEND CMAKE_PREFIX_PATH /usr/local)
    endif()
endif()

# Platform-specific settings
if(CGAME_PLATFORM_LINUX)
    # Linux-specific paths
    list(APPEND CMAKE_PREFIX_PATH /usr/local)
endif()

message(STATUS "Platform Configuration:")
message(STATUS "  System: ${CGAME_PLATFORM}")
message(STATUS "  Architecture: ${CGAME_ARCH}")
message(STATUS "  Graphics: ${GRAPHICS_BACKEND}")
if(CGAME_APPLE_SILICON)
    message(STATUS "  Apple Silicon: YES")
endif()