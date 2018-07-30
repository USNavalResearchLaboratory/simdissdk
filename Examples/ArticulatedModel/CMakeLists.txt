if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ARTICULATED_MODEL)

set(PROJECT_FILES
    ExampleArticulatedModel.cpp
)

add_executable(example_articulatedmodel ${PROJECT_FILES})
target_link_libraries(example_articulatedmodel PRIVATE simVis simUtil)
set_target_properties(example_articulatedmodel PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Articulated Model"
)
vsi_install_target(example_articulatedmodel SDK_Examples)
