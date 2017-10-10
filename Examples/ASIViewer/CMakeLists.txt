if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ASI_VIEWER)

set(PROJECT_FILES
    ASIViewer.cpp
)

add_executable(example_asi_viewer ${PROJECT_FILES})
target_link_libraries(example_asi_viewer PRIVATE simVis simUtil)
set_target_properties(example_asi_viewer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - ASI Simple Viewer"
)
vsi_install_target(example_asi_viewer SDK_Examples)
