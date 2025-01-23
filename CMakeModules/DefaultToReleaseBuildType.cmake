# Include this file to set the CMAKE_BUILD_TYPE, if it is unset.  This is useful
# on non-MSVC systems where the typical configuration choices are Debug, Release,
# RelWithDebInfo, and MinSizeRel, but the default entry is empty string.  This
# allows the default to now be defined as Release.

# If no build type, fall back to release
if(NOT MSVC AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# See workaround at https://gitlab.kitware.com/cmake/cmake/-/issues/20319
if(WIN32 AND DEFINED CMAKE_CONFIGURATION_TYPES) # Generating with a multi config generator
    # If CMake does not have a mapping for MinSizeRel and RelWithDebInfo in imported targets
    # it will map those configuration to the first valid configuration in CMAKE_CONFIGURATION_TYPES.
    # By default this is the debug configuration which is wrong. This works for RelWithDebInfo,
    # but not MinSizeRel for some reason, so we only set for RelWithDebInfo.
    if(NOT DEFINED CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO)
        set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "RelWithDebInfo;Release;")
    endif()
endif()
