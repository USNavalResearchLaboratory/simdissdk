if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_ANTENNA_PATTERN)

set(PROJECT_FILES
    src/ExampleAntennaPattern.cpp
)

add_executable(example_antennapattern ${PROJECT_FILES})
target_link_libraries(example_antennapattern PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_antennapattern PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Antenna Pattern"
)
vsi_install_target(example_antennapattern SDK_Examples)
