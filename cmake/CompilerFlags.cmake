# Compiler Flags Configuration
# ============================================================================

# Common flags for all configurations
set(CGAME_COMMON_FLAGS "")
set(CGAME_C_FLAGS "")

# Warning flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    list(APPEND CGAME_COMMON_FLAGS
        -Wall
        -Wextra
        -Werror
        -Wno-error=unused-but-set-variable
        -Wno-error=null-pointer-subtraction
    )
    
    # C standard
    list(APPEND CGAME_COMMON_FLAGS -std=c99)
    
    # For macOS, we need to compile certain files as Objective-C
    if(CGAME_PLATFORM_MACOS)
        set_source_files_properties(
            ${CMAKE_SOURCE_DIR}/src/graphics_api.c
            ${CMAKE_SOURCE_DIR}/src/gpu_resources.c
            PROPERTIES COMPILE_FLAGS "-x objective-c"
        )
    endif()
    
    # Development build flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND CGAME_COMMON_FLAGS
            -g
            -O0
            -DDEBUG
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND CGAME_COMMON_FLAGS
            -O3
            -DNDEBUG
        )
    endif()
    
    # Note: Test mode flags are applied separately in tests/CMakeLists.txt
    # Main engine should NOT have TEST_MODE defined
    
elseif(MSVC)
    list(APPEND CGAME_COMMON_FLAGS
        /W4
        /WX
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND CGAME_COMMON_FLAGS
            /Od
            /DEBUG
            /DDEBUG
        )
    endif()
endif()

# Sanitizer configuration
if(CGAME_ENABLE_ASAN)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        list(APPEND CGAME_COMMON_FLAGS -fsanitize=address -fno-omit-frame-pointer)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    endif()
endif()

if(CGAME_ENABLE_TSAN)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        list(APPEND CGAME_COMMON_FLAGS -fsanitize=thread)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
    endif()
endif()

if(CGAME_ENABLE_UBSAN)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        list(APPEND CGAME_COMMON_FLAGS -fsanitize=undefined)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    endif()
endif()

# Apply flags to cgame_engine target
target_compile_options(cgame_engine PRIVATE ${CGAME_COMMON_FLAGS})

# Platform-specific include paths
if(CGAME_PLATFORM_MACOS)
    target_include_directories(cgame_engine PUBLIC /opt/homebrew/include)
elseif(CGAME_PLATFORM_LINUX)
    target_include_directories(cgame_engine PUBLIC /usr/local/include)
endif()

# Output configuration
message(STATUS "Compiler Configuration:")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  C Flags: ${CGAME_COMMON_FLAGS}")
if(CGAME_ENABLE_ASAN OR CGAME_ENABLE_TSAN OR CGAME_ENABLE_UBSAN)
    message(STATUS "  Sanitizers: Enabled")
endif()