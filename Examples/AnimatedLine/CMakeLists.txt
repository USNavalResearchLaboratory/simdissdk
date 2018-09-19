if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ANIMATED_LINE)

set(PROJECT_FILES
    ExampleAnimatedLine.cpp
)

add_executable(example_animatedline ${PROJECT_FILES})
target_link_libraries(example_animatedline PRIVATE simVis simUtil)
set_target_properties(example_animatedline PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Animated Line"
)
vsi_install_target(example_animatedline SDK_Examples)
