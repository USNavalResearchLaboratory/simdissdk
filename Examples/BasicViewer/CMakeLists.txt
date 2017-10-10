if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_BASIC_VIEWER)

set(PROJECT_FILES
    BasicViewer.cpp
)

add_executable(example_basic_viewer ${PROJECT_FILES})
target_link_libraries(example_basic_viewer PRIVATE simVis simUtil)
set_target_properties(example_basic_viewer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Basic Viewer"
)
vsi_install_target(example_basic_viewer SDK_Examples)
