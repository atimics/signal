# SIGNAL Engine - WebAssembly Platform Configuration
# ============================================================================
# Platform-specific settings for WebAssembly/Emscripten builds

message(STATUS "Configuring for WebAssembly platform")

# ============================================================================
# EMSCRIPTEN VALIDATION
# ============================================================================

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "WebAssembly build requires Emscripten toolchain")
endif()

# ============================================================================
# GRAPHICS BACKEND
# ============================================================================

# Use GLES2 backend for Sokol on WebAssembly
add_compile_definitions(SOKOL_GLES2)
add_compile_definitions(CGAME_WASM_BUILD)

# ============================================================================
# EMSCRIPTEN SETTINGS
# ============================================================================

# Core Emscripten flags
set(CGAME_WASM_FLAGS
    -s USE_WEBGL2=1
    -s FULL_ES3=1
    -s WASM=1
    -s USE_GLFW=3
)

# Memory settings
set(CGAME_WASM_MEMORY_FLAGS
    -s INITIAL_MEMORY=64MB
    -s MAXIMUM_MEMORY=512MB
    -s ALLOW_MEMORY_GROWTH=1
    -s STACK_SIZE=8MB
)

# Async and threading settings
set(CGAME_WASM_ASYNC_FLAGS
    -s ASYNCIFY=1
    -s ASYNCIFY_STACK_SIZE=32768
)

# File system settings
set(CGAME_WASM_FS_FLAGS
    -s FORCE_FILESYSTEM=1
    --preload-file assets@/assets
)

# Export settings
set(CGAME_WASM_EXPORT_FLAGS
    -s EXPORTED_FUNCTIONS=['_main','_malloc','_free']
    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
    -s MODULARIZE=1
    -s EXPORT_NAME='SIGNAL'
)

# Combine all WASM flags
set(CGAME_PLATFORM_WASM_FLAGS
    ${CGAME_WASM_FLAGS}
    ${CGAME_WASM_MEMORY_FLAGS}
    ${CGAME_WASM_ASYNC_FLAGS}
    ${CGAME_WASM_FS_FLAGS}
    ${CGAME_WASM_EXPORT_FLAGS}
)

# ============================================================================
# BUILD TYPE SPECIFIC SETTINGS
# ============================================================================

# Debug settings
set(CGAME_WASM_DEBUG_FLAGS
    -s ASSERTIONS=1
    -s SAFE_HEAP=1
    -s STACK_OVERFLOW_CHECK=1
    -s DEMANGLE_SUPPORT=1
    -g3
)

# Release settings
set(CGAME_WASM_RELEASE_FLAGS
    -s ASSERTIONS=0
    -O3
    -s ELIMINATE_DUPLICATE_FUNCTIONS=1
    -s AGGRESSIVE_VARIABLE_ELIMINATION=1
    --closure 1
)

# ============================================================================
# APPLY SETTINGS
# ============================================================================

# Apply platform flags
foreach(flag ${CGAME_PLATFORM_WASM_FLAGS})
    add_compile_options(${flag})
    add_link_options(${flag})
endforeach()

# Apply build-type specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    foreach(flag ${CGAME_WASM_DEBUG_FLAGS})
        add_compile_options(${flag})
        add_link_options(${flag})
    endforeach()
    message(STATUS "WebAssembly Debug mode enabled")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Performance")
    foreach(flag ${CGAME_WASM_RELEASE_FLAGS})
        add_compile_options(${flag})
        add_link_options(${flag})
    endforeach()
    message(STATUS "WebAssembly Release mode enabled")
endif()

# ============================================================================
# LIBRARIES
# ============================================================================

# No additional libraries needed for WebAssembly
# Math library is built into Emscripten
set(CGAME_PLATFORM_LIBS "")

# ============================================================================
# ASSET PIPELINE
# ============================================================================

# WebAssembly-specific asset settings
set(CGAME_WASM_ASSET_FLAGS
    --preload-file assets@/assets
    --use-preload-plugins
)

# Custom asset processing for web
set(CGAME_WASM_OPTIMIZE_ASSETS ON)
message(STATUS "WebAssembly asset optimization enabled")

# ============================================================================
# OUTPUT CONFIGURATION
# ============================================================================

# Set output file extensions
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# Generate additional files
set(CGAME_WASM_GENERATE_FILES
    --emit-symbol-map
    --source-map-base ./
)

foreach(flag ${CGAME_WASM_GENERATE_FILES})
    add_link_options(${flag})
endforeach()

# ============================================================================
# TESTING CONFIGURATION
# ============================================================================

# WebAssembly testing limitations
set(CGAME_WASM_TESTING_ENABLED OFF)
message(WARNING "WebAssembly testing is limited - use Node.js for headless tests")

# Node.js testing support
find_program(NODE_JS node)
if(NODE_JS)
    message(STATUS "Node.js found for WebAssembly testing: ${NODE_JS}")
    set(CGAME_WASM_NODE_TESTING ON)
else()
    message(STATUS "Node.js not found - WebAssembly tests will be skipped")
    set(CGAME_WASM_NODE_TESTING OFF)
endif()

# ============================================================================
# DEVELOPMENT SETTINGS
# ============================================================================

# Enable source maps for debugging
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_link_options(-g4)
    add_link_options(--source-map-base=./)
endif()

# Emscripten-specific optimizations
add_compile_definitions(CGAME_WASM_OPTIMIZED)

# ============================================================================
# WEB TEMPLATE CONFIGURATION
# ============================================================================

# HTML shell template
set(CGAME_WASM_SHELL_FILE "${CMAKE_SOURCE_DIR}/assets/web/shell.html")
if(EXISTS ${CGAME_WASM_SHELL_FILE})
    add_link_options(--shell-file ${CGAME_WASM_SHELL_FILE})
    message(STATUS "Using custom HTML shell: ${CGAME_WASM_SHELL_FILE}")
else()
    message(STATUS "Using default Emscripten HTML shell")
endif()

# ============================================================================
# PERFORMANCE OPTIMIZATIONS
# ============================================================================

# WebAssembly-specific performance settings
if(CMAKE_BUILD_TYPE STREQUAL "Performance")
    add_compile_options(-s FAST_MATH=1)
    add_link_options(-s FAST_MATH=1)
    
    # Enable SIMD if supported
    add_compile_options(-msimd128)
    add_link_options(-msimd128)
    
    message(STATUS "WebAssembly performance optimizations enabled")
endif()

# ============================================================================
# CUSTOM TARGETS
# ============================================================================

# Create a custom target for serving the web build
add_custom_target(serve-web
    COMMAND python3 -m http.server 8000
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Serving WebAssembly build on http://localhost:8000"
)

# ============================================================================
# COMPATIBILITY SETTINGS
# ============================================================================

# Disable features not supported in WebAssembly
add_compile_definitions(CGAME_NO_NATIVE_DIALOGS)
add_compile_definitions(CGAME_NO_FILE_SYSTEM_WRITE)
add_compile_definitions(CGAME_WEB_PLATFORM)

# ============================================================================
# SUMMARY
# ============================================================================

message(STATUS "WebAssembly platform configuration complete")
message(STATUS "Output format: ${CMAKE_EXECUTABLE_SUFFIX}")
message(STATUS "Memory: Initial ${CGAME_WASM_MEMORY_FLAGS}")
message(STATUS "Node.js testing: ${CGAME_WASM_NODE_TESTING}")
message(STATUS "Asset optimization: ${CGAME_WASM_OPTIMIZE_ASSETS}")

# List all applied flags for debugging
string(JOIN " " CGAME_WASM_ALL_FLAGS ${CGAME_PLATFORM_WASM_FLAGS})
message(STATUS "WebAssembly flags: ${CGAME_WASM_ALL_FLAGS}")
