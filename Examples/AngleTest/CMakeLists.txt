if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ANGLE_TEST)

set(PROJECT_FILES
    src/AngleTestExample.cpp
)

add_executable(example_angle_test ${PROJECT_FILES})
target_link_libraries(example_angle_test PRIVATE simVis simUtil)
set_target_properties(example_angle_test PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Angle Test"
)
vsi_install_target(example_angle_test SDK_Examples)
