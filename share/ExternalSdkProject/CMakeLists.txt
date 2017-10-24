cmake_minimum_required(VERSION 2.8.12)

# disable autolinking to qtmain as we have our own main() functions (new in Qt 5.1)
if(POLICY CMP0020)
    cmake_policy(SET CMP0020 OLD)
endif()

# Create a new project
project(ExternalSDKExamples)

# Configure flags for GCC -- this is a noop for non-GCC compilers
include(CMakeModules/EnableGccNewCxxFeatures.cmake)
# Use an external script to load the SIMDIS SDK targets
include(CMakeModules/ImportSIMDIS_SDK.cmake)

# Add a simCore example
add_subdirectory(CoreExample)
# Add a simVis example
add_subdirectory(VisExample)

