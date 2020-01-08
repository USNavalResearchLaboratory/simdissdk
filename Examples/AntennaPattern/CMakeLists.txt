if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ANTENNA_PATTERN)

set(PROJECT_FILES
    ExampleAntennaPattern.cpp
)

add_executable(example_antennapattern ${PROJECT_FILES})
target_link_libraries(example_antennapattern PRIVATE simVis simUtil)
set_target_properties(example_antennapattern PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Antenna Pattern"
)
vsi_install_target(example_antennapattern SDK_Examples)
