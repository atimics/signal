# CGame Tests - Simplified Configuration
# ============================================================================

# Unity test framework
add_library(unity STATIC vendor/unity.c)
target_include_directories(unity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Define compiler flags for tests
set(TEST_COMPILE_FLAGS
    -DTEST_MODE
    -DUNITY_TESTING
    -DSOKOL_DUMMY_BACKEND
    -DTEST_STANDALONE
    -Wno-error=unused-function
    -Wno-error=unused-variable
)

# Common include directories for all tests
set(TEST_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

# Test support library
add_library(test_support STATIC
    support/test_utilities.c
    stubs/graphics_api_test_stub.c
    stubs/engine_test_stubs.c
    stubs/asset_stubs.c
    stubs/ui_function_stubs.c
)
target_include_directories(test_support PUBLIC ${TEST_INCLUDE_DIRS})
target_compile_options(test_support PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_support PUBLIC unity m)

# Core engine sources for tests (minimal set)
set(CORE_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/system/physics.c
)

# Core tests
add_executable(test_core_math 
    core/test_math.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
)
target_include_directories(test_core_math PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_core_math PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_core_math PRIVATE unity test_support m)
add_test(NAME test_core_math COMMAND test_core_math)

add_executable(test_core_components 
    core/test_components.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/system/physics.c
)
target_include_directories(test_core_components PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_core_components PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_core_components PRIVATE unity test_support m)
add_test(NAME test_core_components COMMAND test_core_components)

add_executable(test_core_world 
    core/test_world.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/system/physics.c
)
target_include_directories(test_core_world PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_core_world PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_core_world PRIVATE unity test_support m)
add_test(NAME test_core_world COMMAND test_core_world)

# Rendering tests (with stubs)
add_executable(test_rendering 
    rendering/test_rendering.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_3d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_camera.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_lighting.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_mesh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_pass_guard.c
)
target_include_directories(test_rendering PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_rendering PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_rendering PRIVATE unity test_support m)
add_test(NAME test_rendering COMMAND test_rendering)

add_executable(test_render_layers 
    rendering/test_render_layers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_layers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/render_pass_guard.c
)
target_include_directories(test_render_layers PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_render_layers PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_render_layers PRIVATE unity test_support m)
add_test(NAME test_render_layers COMMAND test_render_layers)

add_executable(test_ui_rendering_pipeline 
    rendering/test_ui_rendering_pipeline.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
)
target_include_directories(test_ui_rendering_pipeline PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_ui_rendering_pipeline PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_ui_rendering_pipeline PRIVATE unity test_support m)
add_test(NAME test_ui_rendering_pipeline COMMAND test_ui_rendering_pipeline)

# Input service tests
add_executable(test_input_action_maps 
    services/test_input_action_maps.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/services/input_action_maps.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/services/input_service.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/hal/input_hal_mock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core.c
)
target_include_directories(test_input_action_maps PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(test_input_action_maps PRIVATE ${TEST_COMPILE_FLAGS})
target_link_libraries(test_input_action_maps PRIVATE unity test_support m)
add_test(NAME test_input_action_maps COMMAND test_input_action_maps)

# All tests target
add_custom_target(build_all_tests
    DEPENDS
        test_core_math
        test_core_components
        test_core_world
        test_rendering
        test_render_layers
        test_ui_rendering_pipeline
        test_input_action_maps
)