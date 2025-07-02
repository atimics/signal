# CGame Engine - Test Discovery and Management
# ============================================================================
# Automated test discovery, organization, and execution system
# Maintains compatibility with existing test structure and Makefile

# ============================================================================
# TEST FRAMEWORK CONFIGURATION
# ============================================================================

# Unity test framework configuration
set(UNITY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(UNITY_SRC "${UNITY_ROOT}/unity.c")
set(UNITY_INCLUDE "${UNITY_ROOT}")

if(NOT EXISTS ${UNITY_SRC})
    message(FATAL_ERROR "Unity test framework not found at ${UNITY_SRC}")
endif()

# Test support infrastructure
set(TEST_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/support")
set(TEST_STUBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stubs")

# Test support sources
set(TEST_SUPPORT_SOURCES
    ${TEST_SUPPORT_DIR}/test_utilities.c
)

# Test stub sources (for mocking engine components)
set(TEST_STUB_SOURCES
    ${TEST_STUBS_DIR}/graphics_api_test_stub.c
    ${TEST_STUBS_DIR}/engine_test_stubs.c
)

# ============================================================================
# TEST COMPILATION FLAGS
# ============================================================================

# Common test flags (matching existing Makefile)
set(CGAME_TEST_FLAGS
    -Wall
    -Wextra
    -std=c99
    -O2
    -g
    -DUNITY_TESTING
    -DTEST_MODE
    -DSOKOL_DUMMY_BACKEND
    -Wno-error=unused-function
    -Wno-error=unused-variable
)

# Test include directories
set(CGAME_TEST_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${UNITY_INCLUDE}
    ${TEST_SUPPORT_DIR}
    ${TEST_STUBS_DIR}
)

# ============================================================================
# TEST DISCOVERY FUNCTIONS
# ============================================================================

# Function to discover and create test targets
function(cgame_discover_tests category test_dir)
    message(STATUS "Discovering tests in ${test_dir} for category: ${category}")
    
    # Find all test files
    file(GLOB_RECURSE TEST_FILES "${test_dir}/test_*.c")
    
    if(NOT TEST_FILES)
        message(STATUS "No test files found in ${test_dir}")
        return()
    endif()
    
    set(TEST_TARGETS "")
    
    foreach(TEST_FILE ${TEST_FILES})
        # Extract test name
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        set(TARGET_NAME "${category}_${TEST_NAME}")
        
        # Create test executable
        add_executable(${TARGET_NAME} 
            ${TEST_FILE}
            ${UNITY_SRC}
            ${TEST_SUPPORT_SOURCES}
            ${TEST_STUB_SOURCES}
        )
        
        # Configure test target
        target_compile_options(${TARGET_NAME} PRIVATE ${CGAME_TEST_FLAGS})
        target_include_directories(${TARGET_NAME} PRIVATE ${CGAME_TEST_INCLUDES})
        
        # Link math library
        target_link_libraries(${TARGET_NAME} m)
        
        # Add to CTest
        add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
        
        # Set test properties
        set_target_properties(${TARGET_NAME} PROPERTIES
            FOLDER "Tests/${category}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        )
        
        # Add to category target list
        list(APPEND TEST_TARGETS ${TARGET_NAME})
        
        message(STATUS "Created test target: ${TARGET_NAME}")
    endforeach()
    
    # Create category target
    if(TEST_TARGETS)
        add_custom_target(${category}
            DEPENDS ${TEST_TARGETS}
            COMMENT "Running ${category} tests..."
        )
        
        # Create run target for category
        add_custom_target(run-${category}
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^${category}_"
            DEPENDS ${TEST_TARGETS}
            COMMENT "Executing ${category} tests..."
        )
    endif()
    
    # Store targets for parent scope
    set(CGAME_${category}_TARGETS ${TEST_TARGETS} PARENT_SCOPE)
endfunction()

# Function to create system test targets with dependencies
function(cgame_discover_system_tests category test_dir system_sources)
    message(STATUS "Discovering system tests in ${test_dir} for category: ${category}")
    
    # Find all test files
    file(GLOB_RECURSE TEST_FILES "${test_dir}/test_*.c")
    
    if(NOT TEST_FILES)
        message(STATUS "No system test files found in ${test_dir}")
        return()
    endif()
    
    set(TEST_TARGETS "")
    
    foreach(TEST_FILE ${TEST_FILES})
        # Extract test name
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        set(TARGET_NAME "${category}_${TEST_NAME}")
        
        # Create test executable with system dependencies
        add_executable(${TARGET_NAME} 
            ${TEST_FILE}
            ${UNITY_SRC}
            ${TEST_SUPPORT_SOURCES}
            ${TEST_STUB_SOURCES}
            ${system_sources}
        )
        
        # Configure test target
        target_compile_options(${TARGET_NAME} PRIVATE ${CGAME_TEST_FLAGS})
        target_include_directories(${TARGET_NAME} PRIVATE ${CGAME_TEST_INCLUDES})
        
        # Link math library
        target_link_libraries(${TARGET_NAME} m)
        
        # Add to CTest
        add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
        
        # Set test properties
        set_target_properties(${TARGET_NAME} PROPERTIES
            FOLDER "Tests/${category}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        )
        
        # Add to category target list
        list(APPEND TEST_TARGETS ${TARGET_NAME})
        
        message(STATUS "Created system test target: ${TARGET_NAME}")
    endforeach()
    
    # Create category target
    if(TEST_TARGETS)
        add_custom_target(${category}
            DEPENDS ${TEST_TARGETS}
            COMMENT "Running ${category} tests..."
        )
        
        # Create run target for category
        add_custom_target(run-${category}
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^${category}_"
            DEPENDS ${TEST_TARGETS}
            COMMENT "Executing ${category} tests..."
        )
    endif()
    
    # Store targets for parent scope
    set(CGAME_${category}_TARGETS ${TEST_TARGETS} PARENT_SCOPE)
endfunction()

# ============================================================================
# SPECIALIZED TEST BUILDERS
# ============================================================================

# Function to create integration tests with full engine dependencies
function(cgame_create_integration_tests)
    set(INTEGRATION_DIR "${CMAKE_CURRENT_SOURCE_DIR}/integration")
    
    # Integration test dependencies (more comprehensive)
    set(INTEGRATION_SOURCES
        ${CMAKE_SOURCE_DIR}/src/core.c
        ${CMAKE_SOURCE_DIR}/src/systems.c
        ${CMAKE_SOURCE_DIR}/src/system/physics.c
        ${CMAKE_SOURCE_DIR}/src/system/control.c
        ${CMAKE_SOURCE_DIR}/src/system/thrusters.c
        ${CMAKE_SOURCE_DIR}/src/system/input.c
        ${CMAKE_SOURCE_DIR}/src/system/camera.c
    )
    
    cgame_discover_system_tests("integration" ${INTEGRATION_DIR} "${INTEGRATION_SOURCES}")
endfunction()

# Function to create performance tests with optimizations
function(cgame_create_performance_tests)
    set(PERFORMANCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/performance")
    
    # Find performance test files
    file(GLOB_RECURSE PERF_TEST_FILES "${PERFORMANCE_DIR}/test_*.c")
    
    set(PERF_TARGETS "")
    
    foreach(TEST_FILE ${PERF_TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        set(TARGET_NAME "performance_${TEST_NAME}")
        
        # Create performance test with optimizations
        add_executable(${TARGET_NAME} 
            ${TEST_FILE}
            ${UNITY_SRC}
            ${TEST_SUPPORT_SOURCES}
            ${CMAKE_SOURCE_DIR}/src/core.c
            ${CMAKE_SOURCE_DIR}/src/system/performance.c
        )
        
        # Performance-specific flags
        target_compile_options(${TARGET_NAME} PRIVATE
            ${CGAME_TEST_FLAGS}
            -O3
            -DPERFORMANCE_BENCHMARKS
        )
        
        target_include_directories(${TARGET_NAME} PRIVATE ${CGAME_TEST_INCLUDES})
        target_link_libraries(${TARGET_NAME} m)
        
        add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
        set_target_properties(${TARGET_NAME} PROPERTIES
            FOLDER "Tests/Performance"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        )
        
        list(APPEND PERF_TARGETS ${TARGET_NAME})
    endforeach()
    
    if(PERF_TARGETS)
        add_custom_target(performance-benchmarks
            DEPENDS ${PERF_TARGETS}
            COMMENT "Running performance benchmarks..."
        )
    endif()
endfunction()

# ============================================================================
# TEST COVERAGE SUPPORT
# ============================================================================

function(cgame_setup_coverage)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_COMPILER_IS_GNUCC)
        # Add coverage flags
        set(COVERAGE_FLAGS --coverage -fprofile-arcs -ftest-coverage)
        
        # Apply to all test targets
        get_property(ALL_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
        foreach(TARGET ${ALL_TARGETS})
            get_target_property(TARGET_TYPE ${TARGET} TYPE)
            if(TARGET_TYPE STREQUAL "EXECUTABLE")
                get_target_property(TARGET_NAME ${TARGET} NAME)
                if(TARGET_NAME MATCHES "^test_" OR TARGET_NAME MATCHES "_test_")
                    target_compile_options(${TARGET} PRIVATE ${COVERAGE_FLAGS})
                    target_link_options(${TARGET} PRIVATE ${COVERAGE_FLAGS})
                endif()
            endif()
        endforeach()
        
        # Create coverage report target
        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage-report
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '*/tests/*' --output-file coverage.info
                COMMAND ${GENHTML_PATH} coverage.info --output-directory coverage-html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating coverage report..."
            )
            message(STATUS "Coverage reporting enabled")
        endif()
    endif()
endfunction()

# ============================================================================
# TEST REPORTING
# ============================================================================

function(cgame_setup_test_reporting)
    # Create test reports directory
    set(TEST_REPORTS_DIR "${CMAKE_BINARY_DIR}/test_reports")
    file(MAKE_DIRECTORY ${TEST_REPORTS_DIR})
    
    # Test dashboard target
    add_custom_target(test-dashboard
        COMMAND ${CMAKE_COMMAND} -E echo "Generating test dashboard..."
        COMMAND ${CMAKE_CTEST_COMMAND} --output-log ${TEST_REPORTS_DIR}/test_results.log
        COMMENT "Generating test dashboard..."
    )
    
    # Test summary target
    add_custom_target(test-summary
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --progress
        COMMENT "Running test summary..."
    )
endfunction()

# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

# Function to print test configuration summary
function(cgame_print_test_summary)
    message(STATUS "")
    message(STATUS "CGame Test Configuration Summary:")
    message(STATUS "================================")
    message(STATUS "Unity Framework: ${UNITY_SRC}")
    message(STATUS "Test Support: ${TEST_SUPPORT_DIR}")
    message(STATUS "Test Stubs: ${TEST_STUBS_DIR}")
    message(STATUS "Test Output: ${CMAKE_BINARY_DIR}/tests")
    message(STATUS "Test Reports: ${CMAKE_BINARY_DIR}/test_reports")
    message(STATUS "CTest Available: ${CMAKE_CTEST_COMMAND}")
    message(STATUS "")
endfunction()

# CGame Engine - Test Discovery CMake Module
# ============================================================================
# This module provides functions for discovering and configuring test suites
# based on the project's directory structure.

# Function to add a test suite with multiple test cases
#
# Parameters:
#   NAME: The name of the test suite (e.g., core, rendering)
#   SOURCES: A list of source files for the tests in this suite
#   LIBS: A list of libraries to link against
#   STUBS: (Optional) A list of stub files to include in the build
#
function(cgame_add_test_suite)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES LIBS STUBS)
    cmake_parse_arguments(SUITE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SUITE_NAME ${SUITE_NAME})
    set(TEST_EXECUTABLE "test_${SUITE_NAME}")

    # Collect all source files for the test executable
    set(TEST_SOURCES "")
    foreach(source_file ${SUITE_SOURCES})
        list(APPEND TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
    endforeach()

    # Add common test sources
    list(APPEND TEST_SOURCES
        "${CMAKE_SOURCE_DIR}/tests/vendor/unity.c"
        "${CMAKE_SOURCE_DIR}/tests/support/test_utilities.c"
        "${CMAKE_SOURCE_DIR}/tests/stubs/engine_test_stubs.c"
    )

    # Add specific stub sources if provided
    if(SUITE_STUBS)
        foreach(stub_file ${SUITE_STUBS})
            list(APPEND TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stubs/${stub_file}.c")
        endforeach()
    endif()

    # Add the test executable
    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCES})

    # Add include directories
    target_include_directories(${TEST_EXECUTABLE} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/tests/vendor # For unity.h
        ${CMAKE_SOURCE_DIR}/tests/support
        ${CMAKE_SOURCE_DIR}/tests/stubs
    )

    # Link libraries
    target_link_libraries(${TEST_EXECUTABLE} PRIVATE ${SUITE_LIBS})

    # Add the test to CTest
    add_test(NAME ${SUITE_NAME} COMMAND ${TEST_EXECUTABLE})

endfunction()
