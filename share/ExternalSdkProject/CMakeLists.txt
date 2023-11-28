# MSVC 2022 support in 3.20
cmake_minimum_required(VERSION 3.20)

# Create a new project
project(ExternalSDKExamples)

# Build as close to CXX20 as possible
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use an external script to load the SIMDIS SDK targets
include(CMakeModules/ImportSIMDIS_SDK.cmake)

# Add a simCore example
add_subdirectory(CoreExample)
# Add a simVis example
add_subdirectory(VisExample)

