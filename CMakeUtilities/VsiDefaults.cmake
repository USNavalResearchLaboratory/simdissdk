if("$ENV{PEOPLE_DIR}" STREQUAL "")
    # Initialize empty values for the various _DIR parameters for end users without the 3rd party packs
    set(OSG_DIR "" CACHE PATH "OpenSceneGraph root directory")
    set(OSGEARTH_DIR "" CACHE PATH "osgEarth root directory")
    set(MRSID_DIR "" CACHE PATH "MrSID root directory")
    return()
endif()

message(STATUS "VSI development environment detected; auto-configuring defaults.")

# Update the THIRDPARTY_LIBRARY_DIR settings
if(IS_DIRECTORY "$ENV{PEOPLE_DIR}/../3rd/${BUILD_SYSTEM_CANONICAL_NAME}")
    set(THIRDPARTY_LIBRARY_DIR "$ENV{PEOPLE_DIR}/../3rd/${BUILD_SYSTEM_CANONICAL_NAME}" CACHE PATH "3rd party library directory")
else()
    set(THIRDPARTY_LIBRARY_DIR "$ENV{PEOPLE_DIR}/../3rd" CACHE PATH "3rd party library directory")
endif()

# Set all 3rd party variables relative to 3rd directory
set(THIRD_DIR "${THIRDPARTY_LIBRARY_DIR}")

# Force all VSI builds to have both unit tests and CDash projects
if(NOT ENABLE_CDASH_PROJECTS)
    set(ENABLE_UNIT_TESTING ON CACHE BOOL "Enable unit testing" FORCE)
    set(ENABLE_CDASH_PROJECTS ON CACHE BOOL "Generate the CDash test projects" FORCE)
    include(CTest)
endif()
