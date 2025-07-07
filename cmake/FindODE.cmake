# FindODE.cmake - Find Open Dynamics Engine
# ============================================================================
# This module defines:
#  ODE_FOUND - System has ODE
#  ODE_INCLUDE_DIRS - The ODE include directories
#  ODE_LIBRARIES - The libraries needed to use ODE
#  ODE_VERSION - The version of ODE found

# Try to find ODE using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_ODE QUIET ode)
endif()

# Find include directory
find_path(ODE_INCLUDE_DIR
    NAMES ode/ode.h
    HINTS
        ${PC_ODE_INCLUDEDIR}
        ${PC_ODE_INCLUDE_DIRS}
    PATHS
        /opt/homebrew/include
        /usr/local/include
        /usr/include
)

# Find library
find_library(ODE_LIBRARY
    NAMES ode
    HINTS
        ${PC_ODE_LIBDIR}
        ${PC_ODE_LIBRARY_DIRS}
    PATHS
        /opt/homebrew/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
)

# Set variables
set(ODE_LIBRARIES ${ODE_LIBRARY})
set(ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIR})

# Version detection
if(PC_ODE_VERSION)
    set(ODE_VERSION ${PC_ODE_VERSION})
elseif(ODE_INCLUDE_DIR AND EXISTS "${ODE_INCLUDE_DIR}/ode/version.h")
    file(STRINGS "${ODE_INCLUDE_DIR}/ode/version.h" ODE_VERSION_LINE
         REGEX "^#define[\t ]+dODE_VERSION[\t ]+\".*\"")
    if(ODE_VERSION_LINE)
        string(REGEX REPLACE "^#define[\t ]+dODE_VERSION[\t ]+\"(.*)\"" "\\1"
               ODE_VERSION "${ODE_VERSION_LINE}")
    endif()
endif()

# Handle REQUIRED and QUIET arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODE
    REQUIRED_VARS ODE_LIBRARY ODE_INCLUDE_DIR
    VERSION_VAR ODE_VERSION
)

# Mark as advanced
mark_as_advanced(ODE_INCLUDE_DIR ODE_LIBRARY)

# Create imported target
if(ODE_FOUND AND NOT TARGET ODE::ODE)
    add_library(ODE::ODE UNKNOWN IMPORTED)
    set_target_properties(ODE::ODE PROPERTIES
        IMPORTED_LOCATION "${ODE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ODE_INCLUDE_DIR}"
    )
endif()