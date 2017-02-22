if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_ANIMATED_LINE)

set(PROJECT_FILES
    src/ExampleAnimatedLine.cpp
)

add_executable(example_animatedline ${PROJECT_FILES})
target_link_libraries(example_animatedline PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_animatedline PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Animated Line"
)
vsi_install_target(example_animatedline SDK_Examples)
