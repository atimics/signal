# Asset Pipeline Configuration
# ============================================================================

# Try to use virtual environment Python if available
set(VENV_PYTHON_EXECUTABLE "")
if(EXISTS "${CMAKE_SOURCE_DIR}/.venv/bin/python3")
    set(VENV_PYTHON_EXECUTABLE "${CMAKE_SOURCE_DIR}/.venv/bin/python3")
    message(STATUS "Found virtual environment Python: ${VENV_PYTHON_EXECUTABLE}")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
    set(VENV_PYTHON_EXECUTABLE "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
    message(STATUS "Found virtual environment Python: ${VENV_PYTHON_EXECUTABLE}")
endif()

# Use venv Python if available, otherwise fall back to system Python
if(VENV_PYTHON_EXECUTABLE)
    set(ASSET_PYTHON_EXECUTABLE ${VENV_PYTHON_EXECUTABLE})
    message(STATUS "Using virtual environment Python for assets")
elseif(Python3_FOUND)
    set(ASSET_PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    message(STATUS "Using system Python for assets: ${Python3_EXECUTABLE}")
else()
    message(WARNING "Python3 not found - skipping asset pipeline")
    return()
endif()

# Check for numpy with the selected Python
execute_process(
    COMMAND ${ASSET_PYTHON_EXECUTABLE} -c "import numpy; print('NumPy', numpy.__version__, 'available')"
    RESULT_VARIABLE NUMPY_RESULT
    OUTPUT_VARIABLE NUMPY_OUTPUT
    ERROR_QUIET
)

if(NUMPY_RESULT EQUAL 0)
    message(STATUS "NumPy check: ${NUMPY_OUTPUT}")
else()
    message(STATUS "NumPy not found, attempting to install...")
    execute_process(
        COMMAND ${ASSET_PYTHON_EXECUTABLE} -m pip install numpy
        RESULT_VARIABLE PIP_RESULT
        OUTPUT_VARIABLE PIP_OUTPUT
        ERROR_VARIABLE PIP_ERROR
    )
    
    if(NOT PIP_RESULT EQUAL 0)
        message(WARNING "Failed to install NumPy automatically. Please install manually:")
        message(WARNING "  ${ASSET_PYTHON_EXECUTABLE} -m pip install numpy")
        message(WARNING "Skipping asset pipeline.")
        return()
    else()
        message(STATUS "NumPy installed successfully")
    endif()
endif()


# Asset directories
set(CGAME_ASSET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets)
set(CGAME_ASSET_BUILD_DIR ${CMAKE_BINARY_DIR}/assets)

# Asset pipeline script
set(CGAME_ASSET_PIPELINE_SCRIPT ${CMAKE_SOURCE_DIR}/tools/build_pipeline.py)

# Check if asset pipeline script exists
if(NOT EXISTS ${CGAME_ASSET_PIPELINE_SCRIPT})
    message(WARNING "Asset pipeline script not found: ${CGAME_ASSET_PIPELINE_SCRIPT}")
    return()
endif()

# Collect asset sources
file(GLOB_RECURSE ASSET_MESH_SOURCES
    ${CGAME_ASSET_SOURCE_DIR}/meshes/*.obj
    ${CGAME_ASSET_SOURCE_DIR}/meshes/*.mtl
)

file(GLOB_RECURSE ASSET_TEXTURE_SOURCES
    ${CGAME_ASSET_SOURCE_DIR}/meshes/*.png
    ${CGAME_ASSET_SOURCE_DIR}/meshes/*.jpg
)

file(GLOB_RECURSE ASSET_CONFIG_SOURCES
    ${CGAME_ASSET_SOURCE_DIR}/meshes/*.json
    ${CGAME_ASSET_SOURCE_DIR}/meshes/*.yaml
)

set(ASSET_SOURCES ${ASSET_MESH_SOURCES} ${ASSET_TEXTURE_SOURCES} ${ASSET_CONFIG_SOURCES})

# Create asset build directory
file(MAKE_DIRECTORY ${CGAME_ASSET_BUILD_DIR})

# Asset compilation command
add_custom_command(
    OUTPUT ${CGAME_ASSET_BUILD_DIR}/meshes/index.json
    COMMAND ${CMAKE_COMMAND} -E echo "🔨 Compiling assets to binary format..."
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR} 
            ${ASSET_PYTHON_EXECUTABLE} ${CGAME_ASSET_PIPELINE_SCRIPT}
    DEPENDS ${ASSET_SOURCES} ${CGAME_ASSET_PIPELINE_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building game assets..."
    VERBATIM
)

# Asset compilation target
add_custom_target(assets ALL
    DEPENDS ${CGAME_ASSET_BUILD_DIR}/meshes/index.json
)

# Make sure cgame depends on assets
add_dependencies(cgame assets)

# Asset cleaning
add_custom_target(clean-assets
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CGAME_ASSET_BUILD_DIR}
    COMMENT "Cleaning asset build directory..."
)

# Copy assets to source directory (for compatibility)
add_custom_command(
    TARGET assets POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
            ${CGAME_ASSET_BUILD_DIR}/meshes/index.json 
            ${CGAME_ASSET_SOURCE_DIR}/meshes/index.json
    COMMENT "Copying asset index to source directory..."
)

message(STATUS "Asset Pipeline Configuration:")
message(STATUS "  Python: ${ASSET_PYTHON_EXECUTABLE}")
message(STATUS "  Source Dir: ${CGAME_ASSET_SOURCE_DIR}")
message(STATUS "  Build Dir: ${CGAME_ASSET_BUILD_DIR}")
message(STATUS "  Pipeline Script: ${CGAME_ASSET_PIPELINE_SCRIPT}")
list(LENGTH ASSET_MESH_SOURCES MESH_COUNT)
message(STATUS "  Asset Count: ${MESH_COUNT} meshes")