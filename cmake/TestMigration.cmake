# Temporary solution: Use existing make test for now
# This allows the CMake system to work while test migration is completed

if(CGAME_BUILD_TESTS)
    # Use the existing Makefile test system
    add_custom_target(run-tests
        COMMAND make test
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running tests via legacy Makefile (temporary)"
    )
    
    add_custom_target(test-core
        COMMAND make test-core
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running core tests via legacy Makefile"
    )
    
    add_custom_target(test-rendering
        COMMAND make test-rendering
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running rendering tests via legacy Makefile"
    )
    
    message(STATUS "Test Integration: Using legacy Makefile (temporary)")
    message(STATUS "  To run tests: cmake --build build --target run-tests")
    message(STATUS "  Or directly: make test")
endif()