if("$ENV{PEOPLE_DIR}" STREQUAL "")
    # Initialize empty values for the various _DIR parameters for end users without the 3rd party packs
    set(OSG_DIR "" CACHE PATH "OpenSceneGraph root directory")
    set(OSGEARTH_DIR "" CACHE PATH "osgEarth root directory")
    set(PROTOBUF_DIR "" CACHE PATH "protobuf root directory")
    set(SQLITE3_DIR "" CACHE PATH "SQLite root directory")
    set(MRSID_DIR "" CACHE PATH "MrSID root directory")
    set(GDAL_DIR "" CACHE PATH "GDAL library directory")
    return()
endif()

message(STATUS "VSI development environment detected; auto-configuring defaults.")

# Update the THIRDPARTY_LIBRARY_DIR settings
if(IS_DIRECTORY "$ENV{PEOPLE_DIR}/../3rd/${BUILD_SYSTEM_LIB_SUFFIX}")
    set(THIRDPARTY_LIBRARY_DIR "$ENV{PEOPLE_DIR}/../3rd/${BUILD_SYSTEM_LIB_SUFFIX}" CACHE PATH "3rd party library directory")
else()
    set(THIRDPARTY_LIBRARY_DIR "$ENV{PEOPLE_DIR}/../3rd" CACHE PATH "3rd party library directory")
endif()

set(PROTOBUF_VERSION "2.6.0")
# Note that OSG 3.6.3 is used against Qt 5.9.8 builds internally
set(OSG_VERSION "3.6.3")
set(SQLITE_VERSION "3.8.11.1")
set(GDAL_VERSION "2.1.1")

set(MRSID_VERSION 9.5.1.4427)
# MSVC 2017+ uses a different MrSID
if(BUILD_COMPILER STREQUAL "vc-14.1" OR BUILD_COMPILER STREQUAL "vc-14.2")
    set(MRSID_VERSION 9.5.4.4709)
endif()

# Our osgEarth directory is versioned with the OSG version
set(OSGEARTH_VERSION "OSG-${OSG_VERSION}")

# Set all 3rd party variables relative to 3rd directory
set(OSG_DIR "${THIRDPARTY_LIBRARY_DIR}/OpenSceneGraph/${OSG_VERSION}" CACHE PATH "OpenSceneGraph root directory")
set(OSGEARTH_DIR "${THIRDPARTY_LIBRARY_DIR}/OSGEarth/${OSGEARTH_VERSION}" CACHE PATH "osgEarth root directory")
set(PROTOBUF_DIR "${THIRDPARTY_LIBRARY_DIR}/protobuf/${PROTOBUF_VERSION}" CACHE PATH "protobuf root directory")
set(SQLITE3_DIR "${THIRDPARTY_LIBRARY_DIR}/SQLite/${SQLITE_VERSION}" CACHE PATH "SQLite root directory")
set(SQLITE3_LIB_NAME "sqlite-3.8")
set(GDAL_DIR "${THIRDPARTY_LIBRARY_DIR}/GDAL/${GDAL_VERSION}" CACHE PATH "3rd party library directory")
set(MRSID_DIR "${THIRDPARTY_LIBRARY_DIR}/MrSID_DSDK/${MRSID_VERSION}" CACHE PATH "3rd party library directory")

# Build Release mode by default on Linux, unless otherwise specified
if(NOT MSVC AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.")
endif()

# Force all VSI builds to have both unit tests and CDash projects
if(NOT ENABLE_CDASH_PROJECTS)
    set(ENABLE_UNIT_TESTING ON CACHE BOOL "Enable unit testing" FORCE)
    set(ENABLE_CDASH_PROJECTS ON CACHE BOOL "Generate the CDash test projects" FORCE)
    INCLUDE(CTest)
endif()

mark_as_advanced(
    OSGEARTH_DIR
    OSG_DIR
    OSG_VERSION_EXE
    PROTOBUF_DIR
    QT_QMAKE_EXECUTABLE
    SQLITE3_DIR
    MRSID_DIR
)
