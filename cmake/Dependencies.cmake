# Dependencies Configuration
# ============================================================================

# Set up paths for finding dependencies
if(CGAME_PLATFORM_MACOS)
    # Homebrew paths
    if(CGAME_APPLE_SILICON)
        list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
        set(HOMEBREW_PREFIX /opt/homebrew)
    else()
        list(APPEND CMAKE_PREFIX_PATH /usr/local)
        set(HOMEBREW_PREFIX /usr/local)
    endif()
    
    # Add specific library paths
    list(APPEND CMAKE_LIBRARY_PATH ${HOMEBREW_PREFIX}/lib)
    list(APPEND CMAKE_INCLUDE_PATH ${HOMEBREW_PREFIX}/include)
endif()

# Threads
find_package(Threads REQUIRED)

# Python (for asset pipeline)
find_package(Python3 COMPONENTS Interpreter)
if(NOT Python3_FOUND)
    message(WARNING "Python3 not found - asset pipeline will not be available")
    set(CGAME_BUILD_ASSETS OFF)
endif()

# Platform-specific graphics dependencies
if(CGAME_PLATFORM_MACOS)
    # Metal framework dependencies are found in main CMakeLists.txt
elseif(CGAME_PLATFORM_LINUX)
    # OpenGL dependencies are found in main CMakeLists.txt
endif()

# External library configuration
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

message(STATUS "Dependency Search Paths:")
message(STATUS "  CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "  CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
message(STATUS "  CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")