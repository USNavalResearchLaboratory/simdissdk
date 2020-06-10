cmake_minimum_required(VERSION 2.8.12)

# Create a new project
project(ExternalSDKExamples)

# Configure flags for C++11 or newer
include(CMakeModules/EnableNewCxxFeatures.cmake)
# Use an external script to load the SIMDIS SDK targets
include(CMakeModules/ImportSIMDIS_SDK.cmake)

# Add a simCore example
add_subdirectory(CoreExample)
# Add a simVis example
add_subdirectory(VisExample)

