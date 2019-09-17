# Include this file to set the CMAKE_BUILD_TYPE, if it is unset.  This is useful
# on non-MSVC systems where the typical configuration choices are Debug, Release,
# RelWithDebInfo, and MinSizeRel, but the default entry is empty string.  This
# allows the default to now be defined as Release.

# If no build type, fall back to release
if(NOT MSVC AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()
