# FindYAML.cmake - Find libyaml
# ============================================================================
# This module defines:
#  YAML_FOUND - System has libyaml
#  YAML_INCLUDE_DIRS - The libyaml include directories
#  YAML_LIBRARIES - The libraries needed to use libyaml
#  YAML_VERSION - The version of libyaml found

# Try to find YAML using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_YAML QUIET yaml-0.1)
endif()

# Find include directory
find_path(YAML_INCLUDE_DIR
    NAMES yaml.h
    HINTS
        ${PC_YAML_INCLUDEDIR}
        ${PC_YAML_INCLUDE_DIRS}
    PATHS
        /opt/homebrew/include
        /usr/local/include
        /usr/include
)

# Find library
find_library(YAML_LIBRARY
    NAMES yaml libyaml
    HINTS
        ${PC_YAML_LIBDIR}
        ${PC_YAML_LIBRARY_DIRS}
    PATHS
        /opt/homebrew/lib
        /usr/local/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
)

# Set variables
set(YAML_LIBRARIES ${YAML_LIBRARY})
set(YAML_INCLUDE_DIRS ${YAML_INCLUDE_DIR})

# Version detection
if(PC_YAML_VERSION)
    set(YAML_VERSION ${PC_YAML_VERSION})
elseif(YAML_INCLUDE_DIR AND EXISTS "${YAML_INCLUDE_DIR}/yaml.h")
    file(STRINGS "${YAML_INCLUDE_DIR}/yaml.h" YAML_VERSION_MAJOR_LINE
         REGEX "^#define[\t ]+YAML_VERSION_MAJOR[\t ]+[0-9]+")
    file(STRINGS "${YAML_INCLUDE_DIR}/yaml.h" YAML_VERSION_MINOR_LINE
         REGEX "^#define[\t ]+YAML_VERSION_MINOR[\t ]+[0-9]+")
    file(STRINGS "${YAML_INCLUDE_DIR}/yaml.h" YAML_VERSION_PATCH_LINE
         REGEX "^#define[\t ]+YAML_VERSION_PATCH[\t ]+[0-9]+")
    
    if(YAML_VERSION_MAJOR_LINE)
        string(REGEX REPLACE "^#define[\t ]+YAML_VERSION_MAJOR[\t ]+([0-9]+)" "\\1"
               YAML_VERSION_MAJOR "${YAML_VERSION_MAJOR_LINE}")
        string(REGEX REPLACE "^#define[\t ]+YAML_VERSION_MINOR[\t ]+([0-9]+)" "\\1"
               YAML_VERSION_MINOR "${YAML_VERSION_MINOR_LINE}")
        string(REGEX REPLACE "^#define[\t ]+YAML_VERSION_PATCH[\t ]+([0-9]+)" "\\1"
               YAML_VERSION_PATCH "${YAML_VERSION_PATCH_LINE}")
        set(YAML_VERSION "${YAML_VERSION_MAJOR}.${YAML_VERSION_MINOR}.${YAML_VERSION_PATCH}")
    endif()
endif()

# Handle REQUIRED and QUIET arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YAML
    REQUIRED_VARS YAML_LIBRARY YAML_INCLUDE_DIR
    VERSION_VAR YAML_VERSION
)

# Mark as advanced
mark_as_advanced(YAML_INCLUDE_DIR YAML_LIBRARY)

# Create imported target
if(YAML_FOUND AND NOT TARGET YAML::YAML)
    add_library(YAML::YAML UNKNOWN IMPORTED)
    set_target_properties(YAML::YAML PROPERTIES
        IMPORTED_LOCATION "${YAML_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${YAML_INCLUDE_DIR}"
    )
endif()