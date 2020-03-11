if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ANGLE_TEST)

set(PROJECT_FILES
    AngleTestExample.cpp
)

add_executable(example_angletest ${PROJECT_FILES})
target_link_libraries(example_angletest PRIVATE simVis simUtil)
set_target_properties(example_angletest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Angle Test"
)
vsi_install_target(example_angletest SDK_Examples)
