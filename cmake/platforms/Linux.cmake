# CGame Engine - Linux Platform Configuration
# ============================================================================
# Platform-specific settings for Linux builds

message(STATUS "Configuring for Linux platform")

# ============================================================================
# GRAPHICS BACKEND
# ============================================================================

# Use OpenGL Core backend for Sokol on Linux
add_compile_definitions(SOKOL_GLCORE)

# POSIX support for threading and timing
add_compile_definitions(_POSIX_C_SOURCE=199309L)

# ============================================================================
# SYSTEM LIBRARIES
# ============================================================================

# OpenGL and graphics libraries
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

# X11 and related libraries
find_package(X11 REQUIRED)

# Find additional X11 components
if(NOT X11_Xi_FOUND)
    find_library(X11_Xi_LIB Xi)
    if(X11_Xi_LIB)
        set(X11_Xi_FOUND TRUE)
        set(X11_Xi_LIB ${X11_Xi_LIB})
    endif()
endif()

if(NOT X11_Xcursor_FOUND)
    find_library(X11_Xcursor_LIB Xcursor)
    if(X11_Xcursor_LIB)
        set(X11_Xcursor_FOUND TRUE)
        set(X11_Xcursor_LIB ${X11_Xcursor_LIB})
    endif()
endif()

if(NOT X11_Xrandr_FOUND)
    find_library(X11_Xrandr_LIB Xrandr)
    if(X11_Xrandr_LIB)
        set(X11_Xrandr_FOUND TRUE)
        set(X11_Xrandr_LIB ${X11_Xrandr_LIB})
    endif()
endif()

# System libraries for device support
find_library(UDEV_LIBRARY udev)
find_library(RT_LIBRARY rt)

# Threading support
find_package(Threads REQUIRED)

# Collect all required libraries
set(CGAME_PLATFORM_LIBS
    ${OPENGL_LIBRARIES}
    ${X11_LIBRARIES}
)

# Add optional X11 extensions
if(X11_Xi_FOUND)
    list(APPEND CGAME_PLATFORM_LIBS ${X11_Xi_LIB})
endif()

if(X11_Xcursor_FOUND)
    list(APPEND CGAME_PLATFORM_LIBS ${X11_Xcursor_LIB})
endif()

if(X11_Xrandr_FOUND)
    list(APPEND CGAME_PLATFORM_LIBS ${X11_Xrandr_LIB})
endif()

# Add system libraries
if(UDEV_LIBRARY)
    list(APPEND CGAME_PLATFORM_LIBS ${UDEV_LIBRARY})
endif()

if(RT_LIBRARY)
    list(APPEND CGAME_PLATFORM_LIBS ${RT_LIBRARY})
endif()

# Add threading library
if(Threads_FOUND)
    list(APPEND CGAME_PLATFORM_LIBS ${CMAKE_THREAD_LIBS_INIT})
endif()

message(STATUS "Linux Libraries configured:")
message(STATUS "  - OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "  - X11: ${X11_LIBRARIES}")
if(X11_Xi_FOUND)
    message(STATUS "  - Xi: ${X11_Xi_LIB}")
endif()
if(X11_Xcursor_FOUND)
    message(STATUS "  - Xcursor: ${X11_Xcursor_LIB}")
endif()
if(X11_Xrandr_FOUND)
    message(STATUS "  - Xrandr: ${X11_Xrandr_LIB}")
endif()
if(UDEV_LIBRARY)
    message(STATUS "  - udev: ${UDEV_LIBRARY}")
endif()
if(RT_LIBRARY)
    message(STATUS "  - rt: ${RT_LIBRARY}")
endif()
if(Threads_FOUND)
    message(STATUS "  - Threads: ${CMAKE_THREAD_LIBS_INIT}")
endif()

# ============================================================================
# COMPILER SETTINGS
# ============================================================================

# Linux-specific compiler warnings suppression
# (These match the existing Makefile configuration)
add_compile_options(
    -Wno-error=implicit-function-declaration
    -Wno-error=missing-field-initializers
    -Wno-error=unused-but-set-variable
    -Wno-error=null-pointer-subtraction
    -Wno-error=implicit-int
)

# ============================================================================
# HIDAPI SUPPORT
# ============================================================================

# Try to find system hidapi
pkg_check_modules(HIDAPI hidapi-libusb)

if(HIDAPI_FOUND)
    message(STATUS "Using system HIDAPI: ${HIDAPI_LIBRARIES}")
    list(APPEND CGAME_PLATFORM_LIBS ${HIDAPI_LIBRARIES})
    include_directories(${HIDAPI_INCLUDE_DIRS})
    link_directories(${HIDAPI_LIBRARY_DIRS})
else()
    # Try alternative hidapi packages
    pkg_check_modules(HIDAPI_RAW hidapi-hidraw)
    if(HIDAPI_RAW_FOUND)
        message(STATUS "Using system HIDAPI (hidraw): ${HIDAPI_RAW_LIBRARIES}")
        list(APPEND CGAME_PLATFORM_LIBS ${HIDAPI_RAW_LIBRARIES})
        include_directories(${HIDAPI_RAW_INCLUDE_DIRS})
        link_directories(${HIDAPI_RAW_LIBRARY_DIRS})
    else()
        message(WARNING "HIDAPI not found - gamepad support may be limited")
        message(STATUS "Install libhidapi-dev or libhidapi-libusb0 for full gamepad support")
    endif()
endif()

# ============================================================================
# DEVELOPMENT TOOLS
# ============================================================================

# Enable additional debugging symbols for GDB
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-ggdb3)
endif()

# Support for Valgrind debugging
option(CGAME_VALGRIND_SUPPORT "Enable Valgrind-friendly debugging" OFF)
if(CGAME_VALGRIND_SUPPORT)
    add_compile_definitions(CGAME_VALGRIND_BUILD)
    message(STATUS "Valgrind support enabled")
endif()

# ============================================================================
# TESTING CONFIGURATION
# ============================================================================

# Linux-specific test settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(CGAME_LINUX_DEBUG)
endif()

# Check for X11 display for GUI tests
if(DEFINED ENV{DISPLAY})
    set(CGAME_X11_DISPLAY_AVAILABLE ON)
    message(STATUS "X11 display available for GUI tests")
else()
    set(CGAME_X11_DISPLAY_AVAILABLE OFF)
    message(WARNING "No X11 display found - some GUI tests may be skipped")
endif()

# ============================================================================
# ASSET PIPELINE
# ============================================================================

# Linux-specific asset compilation settings
set(CGAME_ASSET_PLATFORM_FLAGS "")

# Check for GLSL compiler
find_program(GLSLC_COMPILER glslc)
if(GLSLC_COMPILER)
    message(STATUS "GLSL compiler available: ${GLSLC_COMPILER}")
    set(CGAME_GLSL_SHADERS_ENABLED ON)
else()
    find_program(GLSLANGVALIDATOR_COMPILER glslangValidator)
    if(GLSLANGVALIDATOR_COMPILER)
        message(STATUS "glslangValidator available: ${GLSLANGVALIDATOR_COMPILER}")
        set(CGAME_GLSL_SHADERS_ENABLED ON)
    else()
        message(WARNING "GLSL compiler not found - using precompiled shaders")
        set(CGAME_GLSL_SHADERS_ENABLED OFF)
    endif()
endif()

# ============================================================================
# PERFORMANCE OPTIMIZATIONS
# ============================================================================

# Linux-specific performance flags
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Performance")
    # Enable link-time optimization
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    
    # Platform-specific optimizations
    add_compile_options(-march=native)
    add_compile_options(-ffast-math)
    
    message(STATUS "Linux performance optimizations enabled")
endif()

# ============================================================================
# DISTRIBUTION SUPPORT
# ============================================================================

# Prepare for different Linux distributions
if(EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE)
    if(OS_RELEASE MATCHES "ID=ubuntu")
        set(CGAME_LINUX_DISTRO "Ubuntu")
    elseif(OS_RELEASE MATCHES "ID=fedora")
        set(CGAME_LINUX_DISTRO "Fedora")
    elseif(OS_RELEASE MATCHES "ID=arch")
        set(CGAME_LINUX_DISTRO "Arch")
    elseif(OS_RELEASE MATCHES "ID=debian")
        set(CGAME_LINUX_DISTRO "Debian")
    else()
        set(CGAME_LINUX_DISTRO "Generic")
    endif()
    message(STATUS "Detected Linux distribution: ${CGAME_LINUX_DISTRO}")
endif()

# ============================================================================
# PACKAGING SUPPORT (Future)
# ============================================================================

# Prepare for future packaging support
set(CGAME_PACKAGE_NAME "cgame-engine")
set(CGAME_PACKAGE_VERSION "${PROJECT_VERSION}")

# ============================================================================
# SUMMARY
# ============================================================================

message(STATUS "Linux platform configuration complete")
message(STATUS "Libraries: ${CGAME_PLATFORM_LIBS}")
if(DEFINED CGAME_LINUX_DISTRO)
    message(STATUS "Distribution: ${CGAME_LINUX_DISTRO}")
endif()
message(STATUS "X11 Display: ${CGAME_X11_DISPLAY_AVAILABLE}")
message(STATUS "GLSL Shaders: ${CGAME_GLSL_SHADERS_ENABLED}")
