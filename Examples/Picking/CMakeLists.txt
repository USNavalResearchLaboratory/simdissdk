if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PICKING)

set(PROJECT_FILES
    CustomRender.h
    ExamplePicking.cpp
)

add_executable(example_picking ${PROJECT_FILES})
target_link_libraries(example_picking PRIVATE simVis simUtil)
set_target_properties(example_picking PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Picking"
)
vsi_install_target(example_picking SDK_Examples)
